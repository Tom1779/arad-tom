#include "write.h"

int mywrite(int fd, char buf[], int nbytes)
{
    int remain;
    int lblk, startbyte;
    int blk;
    char kbuf[BLKSIZE];
    int *kbuf_int;
    char *cp;
    MINODE *mip = proc->fd[fd]->minodePtr;
    int offset = proc->fd[fd]->offset;
    while (nbytes)
    {
        lblk = offset / BLKSIZE;
        startbyte = offset % BLKSIZE;
        if (lblk < 12)
        {
            if(!mip->INODE.i_block[lblk])
            {
                mip->INODE.i_block[lblk] = balloc(mip->dev);
            }
            blk = mip->INODE.i_block[lblk] = balloc(mip->dev);
        }
        else if (lblk < 12 + 256)
        {
            //if(!)
            get_block(mip->dev, mip->INODE.i_block[12], kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[lblk - 12];
        }
        else
        {
            get_block(mip->dev, mip->INODE.i_block[13], kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[(lblk - 12 - 256) / 256];
            get_block(mip->dev, blk, kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[(lblk - 12 - 256) % 256];
        }
        get_block(mip->dev, blk, kbuf);
        cp = kbuf + offset;
        remain = BLKSIZE - offset;
        while (remain)
        { // copy bytes from kbuf[ ] to buf[ ]
            *buf++ = *cp++;
            offset++;
            remain--;
            nbytes--; // dec remain, avail, nbytes;
        } // end of while(remain)
        put_block(mip->dev, blk, buf);
    }
    mip->dirty = 1;
    iput(mip);
    return nbytes;
}
