#include "close.h"


int close(int fd) 
{
    if(!(proc->fd[fd]))
    {
        printf("invalid file descriptor\n");
        return 0;
    }
    proc->fd[fd]->refCount--;
    if(!(proc->fd[fd]->refCount--))
    {
        iput(proc->fd[fd]->minodePtr);
    }
    proc->fd[fd] = 0;
}
