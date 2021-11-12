#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "type.h"
#include "util.h"



#define EXT2_S_IFMT 0xF000  /* format mask  */
#define EXT2_S_IFDIR 0x4000 /* directory */

int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int ialloc(int dev);
int balloc(int dev);
int check_path(MINODE** pmip, char* base_name);
int kmkdir(MINODE* pmip, char* basename);
int makedir();
int kcreat(MINODE* pmip, char* basename);
int create();
int enter_name(MINODE *pip, int ino, char *name);






#endif