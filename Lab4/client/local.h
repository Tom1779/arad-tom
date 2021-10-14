#ifndef LOCAL_H
#define LOCAL_H

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include "client.h"

int local_rm(char* line);
int local_mkdir(char* line);
int local_rmdir(char* line);
int local_pwd(char* line);
int local_cd(char* line);
int local_ls(char* line);
int local_cat(char* line);

#endif