#include "open.h"


int open(char* filename, int flags) 
{
    int ino;
    MINODE* mip;
    int oft_index;
    int fd_index;

    ino = getino(filename);
    if(!ino)
    {
        //strcpy(pathname, filename);
        create();
        ino = getino(filename);
    }
    mip = iget(dev, ino);

    for(oft_index = 0; oft_index < 64; oft_index++)
    {
        if(!(oft[oft_index].refCount))
        {
            break;
        }
    }
    OFT *fd = &(oft[oft_index]);
    fd->mode = flags;
    fd->minodePtr = mip;
    fd->refCount = 1;
    fd->offset = 0;
    if(flags == 3)
    {
        fd->offset = mip->INODE.i_size;
    }
    for(fd_index = 0; fd_index < NFD; fd_index++)
    {
        if(!(proc->fd[fd_index]))
        {
            break;
        }
    }
    proc->fd[fd_index] = fd;

    return fd_index;

}
