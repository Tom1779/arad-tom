#ifndef SYMLINK_H
#define SYMLINK_H

#include "util.h"
#include "type.h"
#include "mkdir_creat.h"

#define EXT2_S_IFLNK 0xA000 /* symbolic link */

int symlink();
int readlink(char* filename, char* buf);







#endif