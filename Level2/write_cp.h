#ifndef WRITE_CP_H
#define WRITE_CP_H

#include "type.h"
#include "util.h"

int mywrite(int fd, char buf[], int nbytes);
int get_blk_num(MINODE* mip, int lblk);
int cp(char* src, char* dest);

#endif