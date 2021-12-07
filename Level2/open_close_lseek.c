#include "open_close_lseek.h"

int myopen(char *filename, int flags)
{
    int ino;
    MINODE *mip;
    int oft_index;
    int fd_index;

    ino = getino(filename);
    if (!ino)
    {
        //strcpy(pathname, filename); might work later
        create();
        ino = getino(filename);
    }
    mip = iget(dev, ino);

    for (oft_index = 0; oft_index < 64; oft_index++)
    {
        if (!(oft[oft_index].refCount))
        {
            break;
        }
    }
    OFT *fd = &(oft[oft_index]);
    fd->mode = flags;
    fd->minodePtr = mip;
    fd->refCount = 1;
    fd->offset = 0;
    if (flags == 3)
    {
        fd->offset = mip->INODE.i_size;
    }
    for (fd_index = 0; fd_index < NFD; fd_index++)
    {
        if (!(proc->fd[fd_index]))
        {
            break;
        }
    }
    proc->fd[fd_index] = fd;

    return fd_index;
}

int myclose(int fd)
{
    if (!(proc->fd[fd]))
    {
        printf("invalid file descriptor\n");
        return 0;
    }
    proc->fd[fd]->refCount--;
    if (!(proc->fd[fd]->refCount--))
    {
        iput(proc->fd[fd]->minodePtr);
    }
    proc->fd[fd] = 0;
}

int truncate_indirect1(int dev, int blk)
{
    char buf[BLKSIZE];
    int *kbuf_int;
    get_block(dev, blk, buf);
    kbuf_int = (int *)buf;
    while (*kbuf_int)
    {
        bdalloc(dev, *kbuf_int);
        kbuf_int++;
    }
    bdalloc(dev, blk);
}

int truncate_indirect2(int dev, int blk)
{
    char buf[BLKSIZE];
    int *kbuf_int;
    get_block(dev, blk, buf);
    kbuf_int = (int *)buf;
    while (*kbuf_int)
    {
        truncate_indirect1(dev, *kbuf_int);
        kbuf_int++;
    }
    bdalloc(dev, blk);
}

int truncate(MINODE *mip)
{
    int blk;
    char buf[BLKSIZE];
    int *kbuf_int;

    for (int index = 0; index < 12; index++)
    {
        if (mip->INODE.i_block[index])
        {
            bdalloc(mip->dev, mip->INODE.i_block[index]);
            mip->INODE.i_block[index] = 0;
        }
        else
        {
            break;
        }
    }
    blk = mip->INODE.i_block[12];
    if (blk)
    {
        truncate_indirect1(mip->dev, blk);
        mip->INODE.i_block[12] = 0;
    }
    blk = mip->INODE.i_block[13];
    if (blk)
    {
        truncate_indirect2(mip->dev, blk);
        mip->INODE.i_block[13] = 0;
    }
    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_blocks = 0;
    mip->dirty = 1;
    mip->INODE.i_size = 0;
    iput(mip);
}

int mylseek(int fd, int position)
{
    int original_offset;
    MINODE *mip;
    original_offset = running->fd[fd]->offset;
    mip = running->fd[fd]->minodePtr;
    if (position >= 0 && position < mip->INODE.i_size)
    {
        running->fd[fd]->offset = position;
    }

    return original_offset;
}

int pfd()
{
    int file_number = 0;
    int mode;
    int offset;
    char file_mode[32];
    MINODE *mip;
    printf(" fd     mode    offset    INODE\n"
           "----    ----    ------   --------\n");
    for (int i = 0; i < 64; i++)
    {
        if (running->fd[i])
        {
            mip = running->fd[i]->minodePtr;
            mode = running->fd[i]->mode;
            offset = running->fd[i]->offset;
            if (mode == 0)
            {
                strcpy(file_mode, "READ\0");
            }
            if (mode == 1)
            {
                strcpy(file_mode, "WRITE\0");
            }
            if (mode == 2)
            {
                strcpy(file_mode, "RW\0");
            }
            if (mode == 3)
            {
                strcpy(file_mode, "APPEND\0");
            }
            printf("%d       %s    %d        [%d, %d]\n", file_number, file_mode, offset, mip->dev, mip->ino);
            file_number++;
        }
    }
}

int dup(int fd) 
{
    if(running->fd[fd])
    {
        for(int i = 0; i < 64; i++)
        {
            if(running->fd[i])
            {
                running->fd[i] = running->fd[fd];
            }
        }
    }
    oft->refCount++;
}

int dup2(int fd, int gd) 
{
    close(gd);
    running->fd[gd] = running->fd[fd];
}
