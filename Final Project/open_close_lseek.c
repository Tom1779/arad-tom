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
        //create file if it doesnt exist
        create();
        ino = getino(filename);
    }
    mip = iget(dev, ino);

    for (oft_index = 0; oft_index < 64; oft_index++) // find first empty open file table entry
    {
        if (!(oft[oft_index].refCount))
        {
            break;
        }
    }
    OFT *fd = &(oft[oft_index]); // allocate and initialize file descriptor
    fd->mode = flags;
    fd->minodePtr = mip;
    fd->refCount = 1;
    fd->offset = 0;
    if (flags == 3) //Append
    {
        fd->offset = mip->INODE.i_size; //start at end of file
    }
    for (fd_index = 0; fd_index < NFD; fd_index++) //find 1st empty fd index in proc
    {
        if (!(proc->fd[fd_index]))
        {
            break;
        }
    }
    proc->fd[fd_index] = fd; //store the open file table pointer in the empty fd index

    return fd_index;
}

int myclose(int fd)
{
    if (!(proc->fd[fd]))
    {
        printf("invalid file descriptor\n");
        return 0;
    }
    proc->fd[fd]->refCount--; //decrease ref count so next time someone else can use the open file table index
    if (!(proc->fd[fd]->refCount)) // last process that uses this open file table pointer
    {
        iput(proc->fd[fd]->minodePtr); // release minode
    }
    proc->fd[fd] = 0; //file is closed corresponding fd index is now 0 waiting for a new open file table*
}

int truncate_indirect1(int dev, int blk)
{
    char buf[BLKSIZE];
    int *kbuf_int;
    get_block(dev, blk, buf);
    kbuf_int = (int *)buf;
    while (*kbuf_int) //deallocate all logical blocks in this indirect block
    {
        bdalloc(dev, *kbuf_int);
        kbuf_int++;
    }
    bdalloc(dev, blk); //deallocate indirect block
}

int truncate_indirect2(int dev, int blk)
{
    char buf[BLKSIZE];
    int *kbuf_int;
    get_block(dev, blk, buf);
    kbuf_int = (int *)buf;
    while (*kbuf_int) //loop while there are still indirect blocks
    {
        truncate_indirect1(dev, *kbuf_int); //release all the blocks and indirect blocks of double indirect
        kbuf_int++;
    }
    bdalloc(dev, blk); //deallocate double indirect block
}

int truncate(MINODE *mip)
{
    int blk;
    char buf[BLKSIZE];
    int *kbuf_int;

    for (int index = 0; index < 12; index++) //deallocate direct blocks
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
    if (blk) // check if indirect block exists
    {
        truncate_indirect1(mip->dev, blk);//deallocate indirect and logical blocks
        mip->INODE.i_block[12] = 0;
    }
    blk = mip->INODE.i_block[13];
    if (blk) //check if double indirect block exists
    {
        truncate_indirect2(mip->dev, blk);// deallocate all indirect and logical blocks as well as double indirect block
        mip->INODE.i_block[13] = 0;
    }
    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); //update file modify time
    mip->INODE.i_blocks = 0; //update block count
    mip->dirty = 1;
    mip->INODE.i_size = 0; //update size to 0 cause empty
    iput(mip); //write mip back to dev
}

int mylseek(int fd, int position)
{
    int original_offset;
    MINODE *mip;
    original_offset = running->fd[fd]->offset;
    mip = running->fd[fd]->minodePtr;
    if (position >= 0 && position < mip->INODE.i_size) // check position is not out of bounds of file
    {
        running->fd[fd]->offset = position; //update current offset to specified position
    }

    return original_offset;
}

int pfd() // print file info
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

int dup(int fd)  // copy open file table pointer to 
{
    if(running->fd[fd])
    {
        for(int i = 0; i < 64; i++)
        {
            if(!running->fd[i]) //copy to 1st epty fd slot
            {
                running->fd[i] = running->fd[fd];
            }
        }
    }
    oft->refCount++; //increase ref count because now another place is using the open file table pointer
}

int dup2(int fd, int gd) 
{
    close(gd); //close existing file with decriptor gd
    running->fd[gd] = running->fd[fd]; // copy open file table pointer to corresponding slot
}
