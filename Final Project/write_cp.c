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
    while (nbytes) //still have more to write
    {
        lblk = offset / BLKSIZE; // get starting block
        startbyte = offset % BLKSIZE; // get starting byte in the block
        blk = get_blk_num(mip, lblk); //get a block number from dev, allocate one if doesnt exist
        get_block(mip->dev, blk, wbuf);
        remain = BLKSIZE - startbyte; //bytes left in block
        int min = (remain < nbytes) ? remain : nbytes; //optimization
        memcpy(wbuf + startbyte, buf + buf_offset, min); //copy amount needed from wbuf to the block
        nbytes -= min;
        offset += min;
        buf_offset += min;
        if (offset > mip->INODE.i_size) //update file size if needed
        {
            mip->INODE.i_size = offset;
        }
        put_block(mip->dev, blk, wbuf); // write block back to dev
        proc->fd[fd]->offset = offset; // update file offset
    }
    mip->dirty = 1;
    iput(mip); //write mip back to dev
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
    int fd = myopen(src, 0); //opens src and dest for reading and writing
    strcpy(pathname, dest);
    int gd = myopen(dest, 2);
    while (n = myread(fd, buf, BLKSIZE)) // keep going while you are still able to read any bytes
    {
        mywrite(gd, buf, n); // notice the n in write()
    }
    myclose(fd); // close src and dest
    myclose(gd);
}
