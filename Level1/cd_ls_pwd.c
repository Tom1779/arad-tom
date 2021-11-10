#include "cd_ls_pwd.h"

/************* cd_ls_pwd.c file **************/
int cd()
{
  //printf("cd: under construction READ textbook!!!!\n");

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
  //printf("ls_file\n");
  struct passwd *pw;
  struct group *gr;
  struct tm *timeinfo;
  char buf[1024];
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  pw = getpwuid(mip->INODE.i_uid);
  gr = getgrgid(mip->INODE.i_gid);
  timeinfo = localtime(&(mip->INODE.i_mtime));
  //printf("0x%x\n", mip->INODE.i_mode);
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
  int block = 0;
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  /*if (mip->INODE.i_blocks > 1)
  {
    printf("warning cant deal with directories that have multiple blocks\n");
    return;
  }*/
  for (int i = 0; i < 12; i++)
  {
    block = mip->INODE.i_block[i];
    if(!block)
    {
      break;
    }
    get_block(dev, block, buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE)
    {
      if (!(dp->inode))
      {
        break;
      }
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      MINODE *file_mip = iget(dev, dp->inode);

      //printf("%s  ", temp);
      iput(file_mip);
      ls_file(file_mip, temp);

      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  printf("\n");
}

int ls(char *pathname)
{
  //printf("ls\n");
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
  char temp[256];
  u32 my_ino, parent_ino;
  MINODE *pip;
  if (wd == root)
  {
    return;
  }
  parent_ino = findino(wd, &my_ino);
  pip = iget(dev, parent_ino);
  int r = findmyname(pip, my_ino, temp);
  if (r)
  {
    printf("\nError: could not find directory name\n");
    return;
  }
  rpwd(pip);
  printf("/%s", temp);
}

void pwd(MINODE *wd)
{
  //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root)
  {
    printf("/\n");
    return;
  }
  rpwd(wd);
  printf("\n");
}
