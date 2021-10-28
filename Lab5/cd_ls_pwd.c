#include "cd_ls_pwd.h"

/************* cd_ls_pwd.c file **************/
int cd()
{
  printf("cd: under construction READ textbook!!!!\n");

  int ino = getino(pathname); // return error if ino=0
  MINODE *mip = iget(dev, ino);
  if ((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR)
  {
    iput(running->cwd); // release old cwd
    running->cwd = mip; // change cwd to mip
    return 0;
  }
  return -1;
}

int ls_file(MINODE *mip, char *name)
{
  struct passwd *pw;
  struct group *gr;
  struct tm *timeinfo;
  char buf[1024];
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  pw = getpwuid(mip->INODE.i_uid);
  gr = getgrgid(mip->INODE.i_gid);
  timeinfo = localtime(&(mip->INODE.i_mtime));
  printf("0x%x\n", mip->INODE.i_mode);
  sprintf(buf, ((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR) ? "d" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IRUSR) ? "r" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IWUSR) ? "w" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IXUSR) ? "x" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IRGRP) ? "r" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IWGRP) ? "w" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IXGRP) ? "x" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IROTH) ? "r" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IWOTH) ? "w" : "-");
  sprintf(buf + strlen(buf), (mip->INODE.i_mode & S_IXOTH) ? "x" : "-");
  sprintf(buf + strlen(buf), " %3d", mip->INODE.i_links_count);
  sprintf(buf + strlen(buf), " %s", (pw) ? pw->pw_name : "---");
  sprintf(buf + strlen(buf), " %s", (gr) ? gr->gr_name : "---");
  sprintf(buf + strlen(buf), " %5d", mip->INODE.i_size);
  sprintf(buf + strlen(buf), " %s", asctime(timeinfo));
  sprintf(buf + strlen(buf) - 1, " %s", name);
  printf("%s\n", buf);
}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char block[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  /*if (mip->INODE.i_blocks > 1)
  {
    printf("warning cant deal with directories that have multiple blocks\n");
    return;
  }*/

  get_block(dev, mip->INODE.i_block[0], block);
  dp = (DIR *)block;
  cp = block;

  while (cp < block + BLKSIZE)
  {
    if (!(dp->inode))
    {
      break;
    }
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    MINODE *file_mip = iget(dev, dp->inode);

    //printf("%s  ", temp);
    ls_file(file_mip, temp);

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  printf("\n");
}

int ls(char *pathname)
{
  int inod_num;
  MINODE *mip;
  if (!strcmp(pathname, ""))
  {
    ls_dir(running->cwd);
  }
  else
  {
    inod_num = getino(pathname);
    mip = iget(dev, inod_num);
    ls_dir(mip);
  }
}

void rpwd(MINODE *wd)
{
  char block[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  __u32 my_ino, parent_ino;
  MINODE* pip;

  if (wd == root)
  {
    return;
  }
  get_block(dev, wd->INODE.i_block[0], block);
  dp = (DIR *)block;
  cp = block;

  while (cp < block + BLKSIZE)
  {
    if (!(dp->inode))
    {
      break;
    }
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    if(!strcmp(temp, ".."))
    {
      parent_ino = dp->inode;
    }
    else if(!strcmp(temp, "."))
    {
      my_ino = dp->inode;
    }

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  pip = iget(dev, parent_ino);
  get_block(dev, pip->INODE.i_block[0], block);
  dp = (DIR *)block;
  cp = block;
  temp[0] = 0;

  while (cp < block + BLKSIZE)
  {
    if (!(dp->inode))
    {
      break;
    }
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    if(my_ino == dp->inode)
    {
      break;
    }

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  rpwd(pip);
  printf("/%s", temp);
}

void pwd(MINODE *wd)
{
  printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root)
  {
    printf("/\n");
    return;
  }
  rpwd(wd);
  printf("\n");
}
