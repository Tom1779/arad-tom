#ifndef MOUNT_UMOUNT_H
#define MOUNT_UMOUNT_H

#include "util.h"
#include "open_close_lseek.h"

extern MOUNT mountTable[8];

MOUNT *getmptr(int dev);
int mount(char* filesys, char* mount_point);
void print_mounted();
int mnt_search(char* mount_point);
int mnt_alloc();



#endif