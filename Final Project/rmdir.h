#ifndef RMDIR_H
#define RMDIR_H

#include "type.h"
#include "util.h"


#define EXT2_S_IFMT 0xF000  /* format mask  */
#define EXT2_S_IFDIR 0x4000 /* directory */

int bdalloc(int dev, int block);
int idalloc(int dev, int ino);
int clr_bit(char *buf, int bit);
int removedir();
int rm_child(MINODE* pmip, char* name);



#endif