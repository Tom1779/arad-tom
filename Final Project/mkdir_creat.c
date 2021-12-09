#include "mkdir_creat.h"

int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8)); //check if 1 or 0
}

int set_bit(char *buf, int bit)
{
    buf[bit / 8] |= (1 << (bit % 8)); // set corresponding bit to 1
}

int ialloc(int dev) // allocate an inode number from inode_bitmap
{
    int i;
    char buf[BLKSIZE];
    int mnt_index = search_mnt_dev(dev);

    if (mnt_index == -1)
    {
        printf("could not find mount entry\n");
        exit(1);
    }
    imap = mountTable[mnt_index].imap; //update imap and bmap in case mounted device is not root device
    ninodes = mountTable[mnt_index].ninodes;

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i = 0; i < ninodes; i++) // loop until the end of the inode bitmap
    {
        if (tst_bit(buf, i) == 0) // inode is free
        {
            set_bit(buf, i); // set bit to 1
            put_block(dev, imap, buf); // write imap back to disk
            printf("allocated ino = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];
    int mnt_index = search_mnt_dev(dev);

    if (mnt_index == -1)
    {
        printf("could not find mount entry\n");
        exit(1);
    }
    bmap = mountTable[mnt_index].bmap; //update bmap and nblocks in case mounted device is not root
    nblocks = mountTable[mnt_index].nblocks;

    // read inode_bitmap block
    get_block(dev, bmap, buf);

    for (i = 0; i < nblocks; i++)
    {
        if (tst_bit(buf, i) == 0) //block is free
        {
            set_bit(buf, i); //set bit to 1 block is now taken
            put_block(dev, bmap, buf); // write block bit map back to dev
            printf("allocated block = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

int kmkdir(MINODE *pmip, char *basename) //allocates block and inode for dir. initializes its mip and DIR entries to . and .., puts entry in parent
{
    int ino, blk;
    MINODE *mip;
    char *cp;
    char buf[BLKSIZE];

    ino = ialloc(dev);
    printf("makedir: %s, inode = %2d\n", basename, ino);
    blk = balloc(dev);
    {
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
        // make . entry (the entry with information about itself)
        dp->inode = ino;
        dp->rec_len = 12;
        dp->name_len = 1;
        dp->name[0] = '.';
        // make .. entry: entry with info about parent
        dp = (char *)dp + 12; //next DIR entry start point
        dp->inode = pmip->ino;
        dp->rec_len = BLKSIZE - 12; // rec_len spans block
        dp->name_len = 2;
        dp->name[0] = dp->name[1] = '.';
        put_block(dev, blk, buf); // write to blk on disks
    }
    enter_name(pmip, ino, basename, 1);

    return 0;
}

int check_path(MINODE **pmip, char *base_name) // checking if path provided is valid for the operation
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
    r = check_path(&pmip, base_name); // check if mkdir can be done on provided location
    if (r == -1)
    {
        iput(pmip);
        return;
    }
    kmkdir(pmip, base_name);
    return 0;
}

int kcreat(MINODE *pmip, char *basename)// allocates an inode fo the file and initilizes it, also puts entry in parent
{
    int ino, blk;
    MINODE *mip;
    char *cp;
    char buf[BLKSIZE];

    ino = ialloc(dev);
    printf("create: %s, inode = %2d\n", basename, ino);
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
    r = check_path(&pmip, base_name); // check location provided is valid for creating the file
    if (r == -1)
    {
        iput(pmip);
        return 0;
    }
    kcreat(pmip, base_name);
    iput(pmip);
    return 0;
}

int enter_name(MINODE *pip, int ino, char *name, int i_links)// puts entry name in the parent dir
{
    printf("name: %s\n", name);
    int blk;
    char buf[BLKSIZE];
    DIR *dp;
    char *cp;
    int empty_i_block = 0;

    for (int i = 0; i < 12; i++) // loop until reaching the 1st empty direct i block
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
    blk = pip->INODE.i_block[empty_i_block - 1]; // get last full logical block
    get_block(pip->dev, blk, buf);
    dp = (DIR *)buf;
    cp = buf;
    while (cp + dp->rec_len < buf + BLKSIZE) // loop until reaching the last entry in the logical block
    {
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    int ideal_len = 4 * ((8 + dp->name_len + 3) / 4);
    int need_length = 4 * ((8 + strlen(name) + 3) / 4); // calculate amount of length needed to add the new entry
    int remain = dp->rec_len - ideal_len; // calculate the amount of space left on the block

    if (remain >= need_length) // there is space for new entry in this block
    {
        dp->rec_len = ideal_len; //update the last entries record length
        cp += dp->rec_len; //go to where next entry will be
        dp = (DIR *)cp;
    }
    else // no space for new entry
    {
        blk = balloc(dev); // allocate a new block
        pip->INODE.i_block[empty_i_block] = blk;
        memset(buf, 0, BLKSIZE);
        dp = (DIR *)buf;
        remain = BLKSIZE;
    }
    dp->rec_len = remain; //place new entry info in corresponding spot on block
    strncpy(dp->name, name, strlen(name));
    dp->name_len = strlen(name);
    dp->inode = ino;
    put_block(dev, blk, buf); // write back to device
    pip->INODE.i_links_count += i_links; // update link count as needed
    iput(pip);

    return 0;
}
