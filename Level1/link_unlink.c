#include "link_unlink.h"


int unlink() 
{
    int ino;
    MINODE* mip;

    ino = getino(pathname);
    mip = iget(dev, ino);
    
}
