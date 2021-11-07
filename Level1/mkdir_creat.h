#ifndef MKDIR_CREAT_H
#define MKDIR_CREAT_H

#include "type.h"
#include "util.h"

extern char pathname[128];

int kmkdir();
int makedir();
int create();
int enter_name(MINODE *pip, int ino, char *name);






#endif