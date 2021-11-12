#ifndef LINK_UNLINK_H
#define LINK_UNLINK_H


#include "type.h"
#include "util.h"
#include "rmdir.h"

#define EXT2_S_IFMT 0xF000  /* format mask  */
#define EXT2_S_IFLNK 0xA000 /* symbolic link */
#define EXT2_S_IFREG 0x8000 /* directory */


int unlink();
int link();



#endif