#include "symlink.h"
int symlink() 
{
    char old_file[128];
    char new_file[128];
    char* tok;
    char* cp;
    int block;
    int old_file_name = 0;
    int oino;
    int nino = 0;
    int pino;
    MINODE* mip;
    MINODE* pmip;

    tok = strtok(pathname, " ");
    strcpy(old_file, tok);
    tok = strtok(0, "\n");
    strcpy(new_file, tok);

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
    mip->INODE.i_mode = 0xA1ED;
    mip->INODE.i_size = old_file_name;
    cp = (char*)mip->INODE.i_block;
    strncpy(cp, old_file, strlen(old_file));
    mip->dirty = 1;
    iput(mip);

    pino = getino(dirname(new_file));
    pmip = iget(dev, pino);
    pmip->dirty = 1;
    iput(pmip);

}

int readlink(MINODE* mip, char* buf) 
{
    char* cp;

    if(!((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFLNK))
    {
        printf("file is not a link type\n");
        return 0;
    }
    cp = (char*) mip->INODE.i_block;
    strncpy(buf, cp, mip->INODE.i_size);
    
    buf[mip->INODE.i_size] = '\0';

    return mip->INODE.i_size;
}
