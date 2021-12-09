#include "cd_ls_pwd.h"

/************* cd_ls_pwd.c file **************/
int cd() //update cwd
{

  int ino = getino(pathname); // return error if ino=0
  MINODE *mip = iget(dev, ino);
  if ((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR) // checking if the inode is a directory with bit masking
  {
    iput(running->cwd); // release old cwd
    running->cwd = mip; // change cwd to mip
    return 0;
  }
  return -1; // not a directory
}

int ls_file(MINODE *mip, char *name)
{
  //printf("ls_file\n");
  struct passwd *pw; //used to get user id
  struct group *gr; //used to get group id
  struct tm *timeinfo;
  char file_path[512];
  char buf[1024];
  char lnk[128];
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  pw = getpwuid(mip->INODE.i_uid);
  gr = getgrgid(mip->INODE.i_gid);
  timeinfo = localtime(&(mip->INODE.i_mtime));
  //printf("0x%x\n", mip->INODE.i_mode);
  sprintf(buf, ((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR) ? "d" : "-"); //checking premissions with bit masking
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
  if(((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFLNK)) // checking if file is symbolic link
  {
    readlink(mip, lnk); // gets file name of linked file
    sprintf(buf + strlen(buf), " -> %s", lnk);
  }
  printf("%s\n", buf);
}

int ls_dir(MINODE *mip)
{
  int block = 0;
  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  for (int i = 0; i < 12; i++) // looping through direct blocks of the inode iblock
  {
    block = mip->INODE.i_block[i]; // get current logical block
    if(!block) // no more info to read
    {
      break;
    }
    get_block(dev, block, buf); //get data from logical block on device
    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE) //run while cp hasnt gone past te end of the block
    {
      if (!(dp->inode)) // end of block
      {
        break;
      }
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;

      MINODE *file_mip = iget(dev, dp->inode);

      //printf("%s  ", temp);
      iput(file_mip);
      ls_file(file_mip, temp);

      cp += dp->rec_len; //increase cp by the record length of the current DIR entry
      dp = (DIR *)cp; //update DIR entry to the next one
    }
  }
  printf("\n");
}

int ls(char *pathname)
{
  //printf("ls\n");
  int inod_num;
  MINODE *mip;
  if (!strcmp(pathname, "")) //determine if ls cwd
  {
    ls_dir(running->cwd);
  }
  else //ls on the pathname
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
  if (wd == root) // reached the start of root dev
  {
    return;
  }
  my_ino = wd->ino;
  if(my_ino == 2) // reached the root dir of the mounted dev
  {
    int index = search_mnt_dev(wd->dev);
    if(index < 0) //device nout mount. error
    {
      printf("could not find mounted dev %d\n", wd->dev);
      exit(1);
    }
    if(index) // get the mip of the directory which belongs to the mount point on the original dev and update dev and ino number
    {
      wd = mountTable[index].mounted_inode;
      my_ino = wd->ino;
      dev = wd->dev;
    }
  }
  parent_ino = findino(wd, &my_ino); //get parent inode number
  pip = iget(dev, parent_ino); // get parent mip
  int r = findmyname(pip, my_ino, temp); //get string with current inode's name
  if (r)
  {
    printf("\nError: could not find directory name\n");
    return;
  }
  rpwd(pip); // run recursively until reaching root of root device
  printf("/%s", temp);
}

void pwd(MINODE *wd)
{
  if (wd == root) //current cwd is root
  {
    printf("/\n");
    return;
  }
  rpwd(wd);
  printf("\n");
}
