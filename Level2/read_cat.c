#include "read_cat.h"

int myread(int fd, char *buf, int nbytes)
{
    int count = 0;
    int remain;
    int lblk, start;
    int blk;
    char kbuf[BLKSIZE];
    int *kbuf_int;
    char *cp;
    MINODE *mip = proc->fd[fd]->minodePtr;
    int offset = proc->fd[fd]->offset;
    int avil = mip->INODE.i_size - offset;

    while (nbytes && avil)
    {
        lblk = offset / BLKSIZE;
        start = offset % BLKSIZE;
        if (lblk < 12)
        {
            blk = mip->INODE.i_block[lblk];
        }
        else if (lblk < 12 + 256)
        {
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
        cp = kbuf + start;
        remain = BLKSIZE - start;
        while (remain)
        { // copy bytes from kbuf[ ] to buf[ ]
            *buf++ = *cp++;
            offset++;
            count++; // inc offset, count;
            remain--;
            avil--;
            nbytes--; // dec remain, avail, nbytes;
            if (nbytes == 0 || avil == 0)
            {
                break;
            }
        } // end of while(remain)
    }
    return count;
}

void cat()
{
    int byte_count;
    char buf[4097];
    int fd = myopen(pathname, 0);
    do
    {
        byte_count = myread(fd, buf, 4096);
        buf[byte_count] = '\0';
        printf("%s", buf);

    } while (byte_count == 4096);
    printf("\n");
    myclose(fd);
}

// lblk     0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
// i_block  0                                           11  12  12  12  12
// indeirect                                                [0   1   2   3   4   5   ...]

// 12 + 256 + 256 + 10 = 512 + 22 = 534
// 534 - 12 - 256 = 266
// 266 / 256 = 1
// 266