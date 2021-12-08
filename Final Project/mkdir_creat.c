#include "mkdir_creat.h"

int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
    buf[bit / 8] |= (1 << (bit % 8));
}

int ialloc(int dev) // allocate an inode number from inode_bitmap
{
    int i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i = 0; i < ninodes; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);
            printf("allocated ino = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

// WRITE YOUR OWN balloc(dev) function, which returns a FREE disk block number
int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, bmap, buf);

    for (i = 0; i < nblocks; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, bmap, buf);
            printf("allocated block = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

// Finish iput(MINODE *mip) code in util.c

int kmkdir(MINODE *pmip, char *basename)
{
    int ino, blk;
    MINODE *mip;
    char *cp;
    char buf[BLKSIZE];

    ino = ialloc(dev);
    printf("makedir: %s, inode = %2d\n", basename, ino);
    blk = balloc(dev);
    {
        //section 4.2 page 334
        mip = iget(dev, ino);
        INODE *ip = &mip->INODE;
        ip->i_mode = 0x41ED;      // 040755: DIR type and permissions
        ip->i_uid = running->uid; // owner uid
        ip->i_gid = running->gid; // group Id
        ip->i_size = BLKSIZE;     // size in bytes
        ip->i_links_count = 2;    // links count=2 because of . and ..
        ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
        ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
        ip->i_block[0] = blk;
        for (int i = 1; i < 14; i++)
        {
            ip->i_block[i] = 0;
        }
        mip->dirty = 1; // mark minode dirty
        iput(mip);      // write INODE to disk
    }
    {
        bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
        DIR *dp = (DIR *)buf;
        // make . entry
        dp->inode = ino;
        dp->rec_len = 12;
        dp->name_len = 1;
        dp->name[0] = '.';
        // make .. entry: pino=parent DIR ino, blk=allocated block
        dp = (char *)dp + 12;
        dp->inode = pmip->ino;
        dp->rec_len = BLKSIZE - 12; // rec_len spans block
        dp->name_len = 2;
        dp->name[0] = dp->name[1] = '.';
        put_block(dev, blk, buf); // write to blk on disks
    }
    enter_name(pmip, ino, basename, 1);

    return 0;
}

int check_path(MINODE **pmip, char *base_name)
{
    char dirname[128];
    int index;
    int pino;

    if (!strcmp(pathname, "") || !strcmp(pathname, "/"))
    {
        printf("invalid pathname\n");
        return -1;
    }
    for (index = strlen(pathname) - 1; index >= 0; index--)
    {
        if (pathname[index] == '/')
        {
            break;
        }
    }
    if (index < 0)
    {
        strcpy(dirname, "");
        strcpy(base_name, pathname);
    }
    else
    {
        strncpy(dirname, pathname, index + 1);
        dirname[index + 1] = '\0';
        strcpy(base_name, pathname + index + 1);
    }
    printf("basename: %s\n", base_name);
    pino = getino(dirname);
    if (!pino)
    {
        printf("Error: invalid directory pathname\n");
        return -1;
    }
    *pmip = iget(dev, pino);
    if (!(((*pmip)->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR))
    {
        printf("Error: new directory location is not valid\n");
        return -1;
    }
    if (search(*pmip, base_name))
    {
        printf("Error: directory already exists\n");
        return -1;
    }
    return 0;
}

int makedir()
{
    char base_name[64];
    int r = 0;
    MINODE *pmip;
    r = check_path(&pmip, base_name);
    if (r == -1)
    {
        iput(pmip);
        return;
    }
    kmkdir(pmip, base_name);
    return 0;
}

int kcreat(MINODE *pmip, char *basename)
{
    int ino, blk;
    MINODE *mip;
    char *cp;
    char buf[BLKSIZE];

    ino = ialloc(dev);
    printf("create: %s, inode = %2d\n", basename, ino);
    //section 4.2 page 334
    mip = iget(dev, ino);
    INODE *ip = &mip->INODE;
    ip->i_mode = 0x81A4;      // 040644: DIR type and permissions
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id
    ip->i_size = 0;           // size in bytes
    ip->i_links_count = 1;
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = blk;
    for (int i = 1; i < 14; i++)
    {
        ip->i_block[i] = 0;
    }
    mip->dirty = 1; // mark minode dirty
    iput(mip);      // write INODE to disk

    enter_name(pmip, ino, basename, 0);
}

int create()
{
    int r;
    char base_name[64];
    MINODE *pmip;
    r = check_path(&pmip, base_name);
    if (r == -1)
    {
        iput(pmip);
        return 0;
    }
    kcreat(pmip, base_name);
    iput(pmip);
    return 0;
}

int enter_name(MINODE *pip, int ino, char *name, int i_links)
{
    printf("name: %s\n", name);
    int blk;
    char buf[BLKSIZE];
    DIR *dp;
    char *cp;
    int empty_i_block = 0;

    for (int i = 0; i < 12; i++)
    {
        if (!pip->INODE.i_block[i])
        {
            empty_i_block = i;
            break;
        }
    }

    if (!empty_i_block)
    {
        printf("Severe error no . or .. directories\n");
        return 0;
    }
    blk = pip->INODE.i_block[empty_i_block - 1];
    get_block(pip->dev, blk, buf);
    dp = (DIR *)buf;
    cp = buf;
    while (cp + dp->rec_len < buf + BLKSIZE)
    {
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
    int need_length = 4 * ((8 + strlen(name) + 3) / 4);
    int remain = dp->rec_len - ideal_len;

    if (remain >= need_length)
    {
        dp->rec_len = ideal_len;
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    else
    {
        blk = balloc(dev);
        pip->INODE.i_block[empty_i_block] = blk;
        memset(buf, 0, BLKSIZE);
        dp = (DIR *)buf;
        remain = BLKSIZE;
    }
    dp->rec_len = remain;
    strncpy(dp->name, name, strlen(name));
    dp->name_len = strlen(name);
    dp->inode = ino;
    put_block(dev, blk, buf);
    pip->INODE.i_links_count += i_links;
    iput(pip);

    return 0;
}
