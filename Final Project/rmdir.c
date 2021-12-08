#include "rmdir.h"

int bdalloc(int dev, int block)
{
    char buf[BLKSIZE];

    if (block > nblocks)
    {
        printf("block number %d out of range\n", block);
        return;
    }

    get_block(dev, bmap, buf); // get block bitmap block into buf[]

    clr_bit(buf, block - 1); // clear bit block-1 to 0

    put_block(dev, bmap, buf); // write buf back
}

int idalloc(int dev, int ino)
{
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return;
    }

    get_block(dev, imap, buf); // get inode bitmap block into buf[]

    clr_bit(buf, ino - 1); // clear bit ino-1 to 0

    put_block(dev, imap, buf); // write buf back
}

int clr_bit(char *buf, int bit)
{
    buf[bit / 8] &= ~(1 << (bit % 8));
}

int removedir()
{
    MINODE *mip;
    MINODE *pmip;
    int ino, pino;
    int entries = 0;
    char buf[BLKSIZE];
    char temp[64], name[128];
    char *cp;
    DIR *dp;

    ino = getino(pathname);
    mip = iget(dev, ino);
    if (!((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR))
    {
        printf("Error: directory not found\n");
        iput(mip);
        return 0;
    }
    if (mip->refCount != 1)
    {
        printf("ref count = %d, Direcotry in use and can not be removed\n", mip->refCount);
        iput(mip);
        return 0;
    }
    get_block(mip->dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;
    while (cp < buf + BLKSIZE)
    {
        if (entries > 1)
        {
            printf("directory is not empty and cannot be removed\n");
            iput(mip);
            return 0;
        }
        strcpy(temp, dp->name);
        temp[dp->name_len] = '\0';
        if (!strcmp(temp, ".."))
        {
            pino = dp->inode;
        }
        entries++;
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    pmip = iget(mip->dev, pino);
    findmyname(pmip, mip->ino, name);
    printf("name = %s\n", name);

    rm_child(pmip, name);
    pmip->INODE.i_links_count--;
    pmip->dirty = 1;
    iput(pmip);

    bdalloc(mip->dev, mip->INODE.i_block[0]);
    idalloc(mip->dev, mip->ino);
    iput(mip);
}

int rm_child(MINODE *pmip, char *name)
{
    int child_inode;
    int child_found = 0;
    char dp_name[64];
    char buf[BLKSIZE];
    char temp[128];
    DIR *dp;
    int prev_rec_len = 0;
    char *cp;
    int index = 0;
    int block;

    child_inode = search(pmip, name);
    for (index; index < 12; index++)
    {
        block = pmip->INODE.i_block[index];
        if (!block)
        {
            printf("Error: could not find file\n");
            return 0;
        }
        get_block(pmip->dev, block, buf);
        dp = (DIR *)buf;
        cp = buf;
        while (cp < buf + BLKSIZE)
        {
            if (dp->inode == child_inode)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;
                if (!strcmp(temp, name))
                {
                    child_found++;
                    break;
                }
            }
            prev_rec_len = dp->rec_len;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        if (child_found)
        {
            break;
        }
    }
    if (dp->rec_len == BLKSIZE)
    {
        printf("removed first and only directory in block\n");
        for (int j = index + 1; j < 12; j++)
        {
            pmip->INODE.i_block[j - 1] = pmip->INODE.i_block[j];
        }
        pmip->INODE.i_block[11] = 0;

        bdalloc(pmip->dev, block);
    }
    else if (cp + dp->rec_len == buf + BLKSIZE)
    {
        //this is the last entry in the block;
        cp -= prev_rec_len;
        int rec_len = dp->rec_len;
        dp = (DIR *)cp;
        dp->rec_len += rec_len;
        put_block(pmip->dev, block, buf);
    }
    else
    {
        int rec_len = dp->rec_len;
        int count = count_dir_entries(buf);
        count--;
        char *src = cp + rec_len;
        int size = buf + BLKSIZE - src;
        memmove(cp, src, size);
        cp = buf;
        for (int i = 0; i < count; i++)
        {
            dp = (DIR *)cp;
            strcpy(dp_name, dp->name);
            dp_name[dp->name_len] = 0;
            printf("name: %s\n", dp_name);
            cp += dp->rec_len;
        }
        dp->rec_len += rec_len;
        put_block(pmip->dev, block, buf);
    }
}
