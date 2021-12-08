#include "util.h"

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk * BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk * BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}

int tokenize(char *pathname)
{
   int i;
   char *s;
   printf("tokenize %s\n", pathname);

   strcpy(gpath, pathname); // tokens are in global gpath[ ]
   n = 0;

   s = strtok(gpath, "/");
   while (s)
   {
      name[n] = s;
      n++;
      s = strtok(0, "/");
   }
   name[n] = 0;

   for (i = 0; i < n; i++)
      printf("%s  ", name[i]);
   printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
   int i;
   MINODE *mip;
   char buf[BLKSIZE];
   int blk, offset;
   INODE *ip;

   for (i = 0; i < NMINODE; i++)
   {
      mip = &minode[i];
      if (mip->refCount && mip->dev == dev && mip->ino == ino)
      {
         mip->refCount++;
         //printf("inode number =  %2d, refcount = %d\n", ino, mip->refCount);
         //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
         return mip;
      }
   }

   for (i = 0; i < NMINODE; i++)
   {
      mip = &minode[i];
      if (mip->refCount == 0)
      {
         //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
         mip->refCount = 1;
         //printf("inode number =  %2d, refcount = %d\n", ino, mip->refCount);
         mip->dev = dev;
         mip->ino = ino;

         // get INODE of ino to buf
         blk = (ino - 1) / 8 + iblk;
         offset = (ino - 1) % 8;

         //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

         get_block(dev, blk, buf);
         ip = (INODE *)buf + offset;
         // copy INODE to mp->INODE
         mip->INODE = *ip;
         return mip;
      }
   }
   printf("PANIC: no more free minodes\n");
   return 0;
}

void iput(MINODE *mip)
{
   int i, block, offset;
   char buf[BLKSIZE];
   INODE *ip;

   if (mip == 0)
      return;

   mip->refCount--;
   //printf("inode number =  %2d, refcount = %d\n", mip->ino, mip->refCount);

   if (mip->refCount > 0)
      return;
   if (!mip->dirty)
      return;

   block = (mip->ino - 1) / 8 + iblk;
   offset = (mip->ino - 1) % 8;
   // get block containing this inode
   get_block(mip->dev, block, buf);
   ip = (INODE *)buf + offset;      // ip points at INODE
   *ip = mip->INODE;                // copy INODE to inode in block
   put_block(mip->dev, block, buf); // write back to disk
}

int search(MINODE *mip, char *name)
{
   int i;
   int block = 0;
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
   ip = &(mip->INODE);

   for (int i = 0; i < 12; i++)
   {
      block = ip->i_block[i];
      if (!block)
      {
         return 0;
      }
      get_block(dev, block, sbuf);
      dp = (DIR *)sbuf;
      cp = sbuf;
      printf("  ino   rlen  nlen  name\n");

      while (cp < sbuf + BLKSIZE)
      {
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;
         printf("%4d  %4d  %4d    %s\n",
                dp->inode, (int)dp->rec_len, (int)dp->name_len, temp);
         if (strcmp(temp, name) == 0)
         {
            printf("{ %s:%d } found %s : ino = %d\n", __FILE__, __LINE__, temp, dp->inode);
            printf("===========================================\n");
            return dp->inode;
         }
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }
   return 0;
}

int getino(char *pathname)
{
   int i, ino, blk, offset;
   char buf[BLKSIZE];
   INODE *ip;
   MINODE *mip;

   printf("getino: pathname=%s\n", pathname);
   if (strcmp(pathname, "/") == 0)
      return 2;

   // starting mip = root OR CWD
   if (pathname[0] == '/')
      mip = root;
   else
      mip = running->cwd;

   ino = mip->ino;
   mip->refCount++; // because we iput(mip) later
   //printf("inode number =  %2d, refcount = %d\n", ino, mip->refCount);

   tokenize(pathname);

   for (i = 0; i < n; i++)
   {
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);

      ino = search(mip, name[i]);

      if (ino == 0)
      {
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      ino = search(mip, name[i]);
      if (!ino)
      {
         printf("no such component name %s\n", name[i]);
         iput(mip);
         return 0;
      }
      iput(mip);            // release current minode
      mip = iget(dev, ino); // switch to new minode
   }

   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[])
{
   char buf[BLKSIZE];
   int block = 0;
   char *cp;
   for (int i = 0; i < 12; i++)
   {
      block = parent->INODE.i_block[i];
      if (!block)
      {
         printf("child directory entry not found in parent I blocks\n");
         return 0;
      }
      get_block(dev, block, buf);
      dp = (DIR *)buf;
      cp = buf;
      myname[0] = 0;

      while (cp < buf + BLKSIZE)
      {
         if (!(dp->inode))
         {
            break;
         }
         strncpy(myname, dp->name, dp->name_len);
         myname[dp->name_len] = 0;

         if (myino == dp->inode)
         {
            return 0;
         }

         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }
   return 1;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
   int parent_ino;
   char block[BLKSIZE], temp[256];
   char *cp;
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

      if (!strcmp(temp, ".."))
      {
         parent_ino = dp->inode;
      }
      else if (!strcmp(temp, "."))
      {
         *myino = dp->inode;
      }

      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
   return parent_ino;
}

int count_dir_entries(char *buf)
{
   DIR *dp = (DIR *)buf;
   char *cp = buf;
   int count = 0;
   while (cp < buf + BLKSIZE)
   {
      count++;
      cp += dp->rec_len;
      dp = (DIR *)cp;
   }
   return count;
}

void print_minode_table() 
{
   for(int i = 0; i < NMINODE; i++)
   {
      printf("minode[%d]: dev = %d, inode = %d\n", i, minode[i].dev, minode[i].ino);
   }
}
