#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "type.h"
#include "util.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

int kmkdir();
int makedir();
int create();
int enter_name(MINODE *pip, int ino, char *name);






#endif