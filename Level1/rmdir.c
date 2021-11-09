#include "rmdir.h"

int idalloc(int dev, int ino)
{
    int i;
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return;
    }

    get_block(dev, imap, buf); // get inode bitmap block into buf[]

    //clr_bit(buf, ino - 1); // clear bit ino-1 to 0

    put_block(dev, imap, buf); // write buf back
}