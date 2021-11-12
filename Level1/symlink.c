#include "symlink.h"
int symlink() 
{
    char *old_file;
    char *new_file;
    char* cp;
    DIR* dp;
    char buf[BLKSIZE];
    int block;
    int old_file_name = 0;
    int oino;
    int nino = 0;
    MINODE* mip;
    MINODE* pmip;

    old_file = strtok(pathname, " ");
    new_file = strtok(0, "\n");

    old_file_name = strlen(old_file);

    oino = getino(old_file);
    if(!oino)
    {
        printf("old file does not exist\n");
        return 0;
    }
    nino = getino(new_file);
    if(nino)
    {
        printf("new file already exists\n");
        return 0;
    }
    strcpy(pathname, new_file);
    create();
    nino = getino(new_file);
    if(!nino)
    {
        printf("new file was not created\n");
        return 0;
    }
    mip = iget(dev, nino);
    block = mip->INODE.i_block[0];
    mip->INODE.i_mode = 0xA1ED;
    get_block(mip->dev, block, buf);
    strcpy(buf, old_file);
    put_block(mip->dev, block, buf);
    mip->INODE.i_size = old_file_name;
    mip->dirty = 1;
    iput(mip);


}

int readlink(char* filename, char* buf) 
{
    
}
