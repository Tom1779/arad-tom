#include "rmdir.h"

int bdalloc(int dev, int block) // deallocates a block
{
    char buf[BLKSIZE];
    int mnt_index = search_mnt_dev(dev);

    if (mnt_index == -1) // could not find dev in mnt table. error
    {
        printf("could not find mount entry\n");
        exit(1);
    }
    bmap = mountTable[mnt_index].bmap; // update block bit map and nblocks in case current dev is not root
    nblocks = mountTable[mnt_index].nblocks;

    if (block > nblocks) //check if block is a valid num
    {
        printf("block number %d out of range\n", block);
        return;
    }

    get_block(dev, bmap, buf); // get block bitmap block into buf[]

    clr_bit(buf, block - 1); // clear bit block-1 to 0

    put_block(dev, bmap, buf); // write buf back
}

int idalloc(int dev, int ino) // deallocate inode
{
    char buf[BLKSIZE];
    int mnt_index = search_mnt_dev(dev);

    if (mnt_index == -1) //current dev cant be found in mount table, error
    {
        printf("could not find mount entry\n");
        exit(1);
    }
    imap = mountTable[mnt_index].imap; //update imap and ninodes in case current dev is not root
    ninodes = mountTable[mnt_index].ninodes;

    if (ino > ninodes) // check ninodes is valid number
    {
        printf("inumber %d out of range\n", ino);
        return;
    }

    get_block(dev, imap, buf); // get inode bitmap block into buf[]

    clr_bit(buf, ino - 1); // clear bit ino-1 to 0

    put_block(dev, imap, buf); // write buf back
}

int clr_bit(char *buf, int bit) // set corresponding bit to 0
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
    if (!((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR)) // check inode is indded a directory
    {
        printf("Error: directory not found\n");
        iput(mip);
        return 0;
    }
    if (mip->refCount != 1) //direcory is being used by another process and cannot be removed
    {
        printf("ref count = %d, Direcotry in use and can not be removed\n", mip->refCount);
        iput(mip);
        return 0;
    }
    get_block(mip->dev, mip->INODE.i_block[0], buf); //get 1st logical block of the inode
    dp = (DIR *)buf;
    cp = buf;
    while (cp < buf + BLKSIZE) //loop until the end of the block
    {
        if (entries > 1)// contains more than . and ..
        {
            printf("directory is not empty and cannot be removed\n");
            iput(mip);
            return 0;
        }
        strcpy(temp, dp->name);
        temp[dp->name_len] = '\0';
        if (!strcmp(temp, ".."))
        {
            pino = dp->inode; //get parent inode number
        }
        entries++;
        cp += dp->rec_len; //go to next entry
        dp = (DIR *)cp;
    }
    pmip = iget(mip->dev, pino); // get parent mip
    findmyname(pmip, mip->ino, name); // get your name from parent inode 
    printf("name = %s\n", name);

    rm_child(pmip, name); // remove entry from parent inode
    pmip->INODE.i_links_count--; // reduce link count of parent
    pmip->dirty = 1;
    iput(pmip); // write back to dev

    bdalloc(mip->dev, mip->INODE.i_block[0]); // deallocate the logical block
    idalloc(mip->dev, mip->ino); //deallocate the dir's inode
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

    child_inode = search(pmip, name); //find child inode number
    for (index; index < 12; index++) // loopon direct blocks
    {
        block = pmip->INODE.i_block[index];
        if (!block) //did no encounter the file we wanted to remove
        {
            printf("Error: could not find file\n");
            return 0;
        }
        get_block(pmip->dev, block, buf);
        dp = (DIR *)buf;
        cp = buf;
        while (cp < buf + BLKSIZE) // loop until block ends
        {
            if (dp->inode == child_inode) // found file
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;
                if (!strcmp(temp, name)) // make sure name corresponds
                {
                    child_found++; // so we can leave outer loop
                    break;
                }
            }
            prev_rec_len = dp->rec_len; // keeping track of the rec len
            cp += dp->rec_len; // update to next entry
            dp = (DIR *)cp;
        }
        if (child_found)
        {
            break;
        }
    }
    if (dp->rec_len == BLKSIZE) // only entry in the block
    {
        printf("removed first and only entry in block\n");
        for (int j = index + 1; j < 12; j++)
        {
            pmip->INODE.i_block[j - 1] = pmip->INODE.i_block[j]; //0 out all of the blocks from index until block 11
        }
        pmip->INODE.i_block[11] = 0; // 0 last bloack

        bdalloc(pmip->dev, block); // deallocate block
    }
    else if (cp + dp->rec_len == buf + BLKSIZE)
    {
        //this is the last entry in the block;
        cp -= prev_rec_len; // go back to last entry
        int rec_len = dp->rec_len;
        dp = (DIR *)cp;
        dp->rec_len += rec_len; // increase prev rec len to the end of the block
        put_block(pmip->dev, block, buf); // write back to dev
    }
    else //entry is somewhere in the middle of the block
    {
        int rec_len = dp->rec_len;
        int count = count_dir_entries(buf); //count how many entries are in the block
        count--;
        char *src = cp + rec_len;
        int size = buf + BLKSIZE - src;
        memmove(cp, src, size); // copy everything from next entry to the end of the block to where the entry to be removed is
        cp = buf;
        for (int i = 0; i < count; i++) // loop until reaching last entry
        {
            dp = (DIR *)cp;
            strcpy(dp_name, dp->name);
            dp_name[dp->name_len] = 0;
            printf("name: %s\n", dp_name);
            cp += dp->rec_len;
        }
        dp->rec_len += rec_len; //update last rec len to the end of the block
        put_block(pmip->dev, block, buf);
    }
}
