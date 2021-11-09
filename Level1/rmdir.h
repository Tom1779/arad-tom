#ifndef RMDIR_H
#define RMDIR_H

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

#define EXT2_S_IFMT 0xF000  /* format mask  */
#define EXT2_S_IFDIR 0x4000 /* directory */

int idalloc(int dev, int ino);




#endif