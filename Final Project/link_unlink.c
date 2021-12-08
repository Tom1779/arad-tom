#include "link_unlink.h"

int unlink()
{
    int ino = 0;
    int pino;
    int block = 0;
    MINODE *mip;
    MINODE *pmip;
    char *parent;
    char *child;
    char buf[128];
    strcpy(buf, pathname);

    ino = getino(pathname);
    if (!ino)
    {
        printf("file does not exist\n");
        return 0;
    }
    mip = iget(dev, ino);
    if (!(((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFREG) || ((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFLNK)))
    {
        printf("file is not symbolic link or regular and cannot be unlinked\n");
        return 0;
    }
    parent = dirname(buf);
    printf("parent: %s\n", parent);
    child = basename(pathname);
    printf("child: %s\n", child);
    pino = getino(parent);
    pmip = iget(dev, pino);
    rm_child(pmip, child);
    pmip->dirty = 1;
    iput(pmip);
    mip->INODE.i_links_count--;
    if (mip->INODE.i_links_count > 0)
    {
        mip->dirty = 1; // for write INODE back to disk
    }
    else
    { // if links_count = 0: remove filename
        //deallocate all data blocks in INODE;
        for (int i = 0; i < 12; i++)
        {
            block = mip->INODE.i_block[i];
            if (!block)
            {
                break;
            }
            bdalloc(mip->dev, block);
        }
        // deallocate INODE;
        idalloc(mip->dev, ino);
    }
    iput(mip); // release mip
}

int link()
{
    char *old_file;
    char *new_file;
    char *parent;
    char *child;
    int oino = 0;
    int pino;
    MINODE *omip;
    MINODE *pmip;

    old_file = strtok(pathname, " ");
    new_file = strtok(0, "\n");

    oino = getino(old_file);
    if (!oino)
    {
        printf("old file does not exist\n");
        return 0;
    }
    omip = iget(dev, oino);
    if ((omip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR)
    {
        printf("old file provided is a directory, cannot link\n");
        return 0;
    }
    if (getino(new_file))
    {
        printf("new file already exists\n");
        return 0;
    }
    parent = dirname(new_file);
    child = basename(new_file);
    pino = getino(parent);
    pmip = iget(dev, pino);
    // creat entry in new parent DIR with same inode number of old_file
    enter_name(pmip, oino, child);
    omip->INODE.i_links_count++; // inc INODE’s links_count by 1
    omip->dirty = 1;              // for write back by iput(omip)
    iput(omip);
    iput(pmip);
}
