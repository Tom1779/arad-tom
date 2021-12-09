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

    while (nbytes && avil) //run until theres nothing more to read or you read nbytes bytes
    {
        lblk = offset / BLKSIZE; //get i block index for loctation where offset is
        start = offset % BLKSIZE; //get starting byte of offset in i block index
        if (lblk < 12) // direct block
        {
            blk = mip->INODE.i_block[lblk];
        }
        else if (lblk < 12 + 256) //indirect block
        {
            get_block(mip->dev, mip->INODE.i_block[12], kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[lblk - 12]; 
        }
        else // double indirect block (mailman algorithm)
        {
            get_block(mip->dev, mip->INODE.i_block[13], kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[(lblk - 12 - 256) / 256];
            get_block(mip->dev, blk, kbuf);
            kbuf_int = (int *)kbuf;
            blk = kbuf_int[(lblk - 12 - 256) % 256];
        }
        get_block(mip->dev, blk, kbuf);
        cp = kbuf + start; //starting point of reading
        remain = BLKSIZE - start; //how much left in block
        int min = (remain < nbytes) ? remain : nbytes; //optimization 
        min = (avil < min) ? avil : min; //optimizaion
        memcpy(buf, cp, min); // cpy the amount left to read, can be avil, nbytes, remain depending on which one is the smallest
        buf += min; // update all the variables for the next loop
        offset += min;
        avil -= min;
        nbytes -= min;
        count += min;
    }
    proc->fd[fd]->offset = offset;
    return count;
}

void cat()
{
    int byte_count;
    char buf[4097];
    int fd = myopen(pathname, 0); //open file for reading
    do
    {
        byte_count = myread(fd, buf, 4096); //read 4096 bytes every iteration until reaching the end of the file
        buf[byte_count] = '\0'; // terminate buf with 0 at the end
        printf("%s", buf);

    } while (byte_count == 4096);
    printf("\n");
    myclose(fd); // close file
}

// lblk     0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
// i_block  0                                           11  12  12  12  12
// indeirect                                                [0   1   2   3   4   5   ...]

// 12 + 256 + 256 + 10 = 512 + 22 = 534
// 534 - 12 - 256 = 266
// 266 / 256 = 1
// 266