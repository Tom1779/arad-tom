/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"
#include "cd_ls_pwd.h"
#include "mkdir_creat.h"
#include "rmdir.h"
#include "link_unlink.h"
#include "symlink.h"
#include "open_close_lseek.h"
#include "mount_umount.h"

extern MINODE *iget();

MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int n;           // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128];
char disk[128] = "diskimage";

int init()
{
  int i, j;
  MINODE *mip;
  PROC *p;

  printf("init()\n");

  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i = 0; i < NPROC; i++)
  {
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = i;
    p->cwd = 0;

    for (int j = 0; j < NFD; j++)
    {
      p->fd[j] = 0;
    }
  }
  for (int i = 0; i < 64; i++)
  {
    oft[i].refCount = 0;
  }
  root = 0;
}

// load root INODE and set root pointer to it
int mount_root()
{

  printf("mount_root()\n");
  mountTable[0].dev = dev;
  mountTable[0].bmap = bmap;
  mountTable[0].iblk = iblk;
  mountTable[0].imap = imap;
  mountTable[0].nblocks = nblocks;
  mountTable[0].ninodes = ninodes;
  strcpy(mountTable[0].name, disk);
  strcpy(mountTable[0].mount_name, "/");

  root = iget(dev, 2);
  proc[0].cwd = iget(dev, 2);
  proc[1].cwd = iget(dev, 2);
}

int main(int argc, char *argv[])
{
  int ino;
  int error = 0;
  char buf[BLKSIZE];
  if(argc > 1)
  {
    strcpy(disk, argv[1]);
  }

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0)
  {
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd; // global dev same as this fd

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53)
  {
    printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
    exit(1);
  }
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  // running->cwd = iget(dev, 2); proc 0 is initialized in mount_root()
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  while (1)
  {
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|cat|cp|mount|umount|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;

    if (line[0] == 0)
      continue;
    pathname[0] = 0;

    sscanf(line, "%s %[^\t\n]", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls") == 0)
      ls(pathname);
    else if (strcmp(cmd, "cd") == 0)
    {
      error = cd();
      if (-1 == error)
      {
        printf("Error: could not find directory\n");
      }
    }
    else if (strcmp(cmd, "pwd") == 0)
      pwd(running->cwd);
    else if (strcmp(cmd, "mkdir") == 0)
      makedir();
    else if (strcmp(cmd, "creat") == 0)
      create();
    else if (strcmp(cmd, "rmdir") == 0)
      removedir();
    else if (strcmp(cmd, "unlink") == 0)
      unlink();
    else if (strcmp(cmd, "link") == 0)
      link();
    else if (strcmp(cmd, "symlink") == 0)
      symlink();
    else if ((strcmp(cmd, "pfd") == 0))
      pfd();
    else if ((strcmp(cmd, "cat") == 0))
      cat();
    else if ((strcmp(cmd, "cp") == 0))
    {
      char src[128];
      char dest[128];
      char *tok;
      tok = strtok(pathname, " ");
      strcpy(src, tok);
      tok = strtok(0, "\n");
      strcpy(dest, tok);
      cp(src, dest);
    }
    else if ((strcmp(cmd, "mount") == 0))
    {
      char mount_point[128] = {0};
      char filesys[128] = {0};
      char *tok;
      tok = strtok(pathname, " ");
      if (tok)
      {
        strcpy(filesys, tok);
      }
      tok = strtok(0, "\n");
      if (tok)
      {
        strcpy(mount_point, tok);
      }
      mount(filesys, mount_point);
    }
    else if ((strcmp(cmd, "umount") == 0))
      umount(pathname);
    else if (strcmp(cmd, "quit") == 0)
      quit();
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
