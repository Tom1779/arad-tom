#ifndef CD_LS_PWD_H
#define CD_LS_PWD_H

#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <ext2fs/ext2fs.h>

#include "type.h"
#include "symlink.h"

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

/* inode: i_mode */
#define EXT2_S_IFMT 0xF000  /* format mask  */
#define EXT2_S_IFDIR 0x4000 /* directory */

int cd();
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
int ls(char *pathname);
void rpwd(MINODE *wd);
void pwd(MINODE *wd);



#endif