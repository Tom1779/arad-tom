#ifndef OPEN_CLOSE_LSEEK
#define OPEN_CLOSE_LSEEK


#include "type.h"
#include "util.h"

int myopen(char* filename, int flags);
int myclose(int fd);
int truncate_indirect1(int dev, int blk);
int truncate_indirect2(int dev, int blk);
int truncate(MINODE *mip);
int mylseek(int fd, int position);
int pfd();
int dup(int fd);
int dup2(int fd, int gd);


#endif