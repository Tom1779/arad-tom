#include "write_cp.h"

int mywrite(int fd, char buf[], int nbytes)
{
    int remain;
    int buf_offset = 0;
    int lblk, startbyte;
    int blk;
    char wbuf[BLKSIZE];
    int *wbuf_int;
    MINODE *mip = proc->fd[fd]->minodePtr;
    int offset = proc->fd[fd]->offset;
    while (nbytes)
    {
        lblk = offset / BLKSIZE;
        startbyte = offset % BLKSIZE;
        blk = get_blk_num(mip, lblk);
        get_block(mip->dev, blk, wbuf);
        remain = BLKSIZE - startbyte;
        int min = (remain < nbytes) ? remain : nbytes;
        memcpy(wbuf + startbyte, buf + buf_offset, min);
        nbytes -= min;
        offset += min;
        buf_offset += min;
        if (offset > mip->INODE.i_size)
        {
            mip->INODE.i_size = offset;
        }
        put_block(mip->dev, blk, wbuf);
        proc->fd[fd]->offset = offset;
    }
    mip->dirty = 1;
    iput(mip);
    return nbytes;
}

int get_blk_num(MINODE *mip, int lblk)
{
    int blk;
    int blk1;
    char buf[BLKSIZE];
    int *buf_int = (int *)buf;

    if (lblk < 12)
    {
        if (!mip->INODE.i_block[lblk])
        {
            mip->INODE.i_block[lblk] = balloc(mip->dev);
        }
        blk = mip->INODE.i_block[lblk];
    }
    else if (lblk < 12 + 256)
    {
        if (!mip->INODE.i_block[12])
        {
            mip->INODE.i_block[12] = balloc(mip->dev);
            memset(buf, 0, BLKSIZE);
            blk = balloc(mip->dev);
            *buf_int = blk;
            put_block(mip->dev, mip->INODE.i_block[12], buf);
        }
        else
        {
            get_block(mip->dev, mip->INODE.i_block[12], buf);
            blk = buf_int[lblk - 12];
            if (!blk)
            {
                blk = balloc(mip->dev);
                buf_int[lblk - 12] = blk;
                put_block(mip->dev, mip->INODE.i_block[12], buf);
            }
        }
    }
    else // double indirect
    {
        if (!mip->INODE.i_block[13])
        {
            mip->INODE.i_block[13] = balloc(mip->dev);
            memset(buf, 0, BLKSIZE);
            blk1 = balloc(mip->dev);
            *buf_int = blk1;
            put_block(mip->dev, mip->INODE.i_block[13], buf);
            blk = balloc(mip->dev);
            *buf_int = blk;
            put_block(mip->dev, blk1, buf);
        }
        else
        {
            get_block(mip->dev, mip->INODE.i_block[13], buf);
            blk1 = buf_int[(lblk - 12 - 256) / 256];
            if (!blk1)
            {
                blk1 = balloc(mip->dev);
                buf_int[(lblk - 12 - 256) / 256] = blk1;
                put_block(mip->dev, mip->INODE.i_block[13], buf);
                memset(buf, 0, BLKSIZE);
                blk = balloc(mip->dev);
                *buf_int = blk;
                put_block(mip->dev, blk1, buf);
            }
            else
            {
                get_block(mip->dev, blk1, buf);
                blk = buf_int[(lblk - 12 - 256) % 256];
                if (!blk)
                {
                    blk = balloc(mip->dev);
                    buf_int[(lblk - 12 - 256) % 256] = blk;
                    put_block(mip->dev, blk1, buf);
                }
            }
        }
    }
    return blk;
}

int cp(char *src, char *dest)
{
    char buf[32768];
    strcpy(pathname, src);
    int fd = myopen(src, 0);
    strcpy(pathname, dest);
    int gd = myopen(dest, 2);
    while (n = myread(fd, buf, BLKSIZE))
    {
        mywrite(gd, buf, n); // notice the n in write()
    }
    myclose(fd);
    myclose(gd);
}
