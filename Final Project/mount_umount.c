#include "mount_umount.h"

MOUNT mountTable[8];

MOUNT *getmptr(int dev)
{
    for (int i = 0; i < 8; i++)
    {
        if (mountTable[i].dev == dev)
        {
            return &(mountTable[i]);
        }
    }
    printf("no device found\n");
    return 0;
}

int mount(char *filesys, char *mount_point)
{
    int index;
    int ino;
    MINODE *mip;
    char buf[BLKSIZE];
    if (!strcmp(filesys, "") || !strcmp(mount_point, ""))
    {
        print_mounted();
        return 0;
    }
    printf("filesys=%s, mount_point=%s\n", filesys, mount_point);
    index = search_mnt_point(mount_point);
    if (index >= 0)
    {
        printf("a filesystem is already mounted at %s\n", mount_point);
        return 0;
    }
    index = mnt_alloc();
    if (index < 0)
    {
        printf("mountTable full\n");
        return 0;
    }
    int fd = open(filesys, O_RDWR);
    get_block(fd, 1, buf);
    SUPER *s_ptr = (SUPER *)buf;
    if (s_ptr->s_magic != 0xEF53)
    {
        printf("magic: %d, inodes count: %d, blocks count: %d\n", sp->s_magic, sp->s_inodes_count, sp->s_blocks_count);
        printf("%s is not an ext2 filesystem\n", filesys);
        close(fd);
        return 0;
    }
    ino = getino(mount_point);
    if (!ino)
    {
        printf("mount point does not exist\n");
        close(fd);
        return 0;
    }
    mip = iget(dev, ino);
    if (!((mip->INODE.i_mode & EXT2_S_IFMT) == EXT2_S_IFDIR))
    {
        printf("mount point is not a directory\n");
        close(fd);
        return 0;
    }
    if (mip->refCount > 2)
    {
        printf("mount point is busy\n");
        close(fd);
        return 0;
    }
    get_block(fd, 2, buf);
    GD *g_ptr = (GD *)buf;

    mountTable[index].dev = fd;
    mountTable[index].bmap = g_ptr->bg_block_bitmap;
    mountTable[index].iblk = g_ptr->bg_inode_table;
    mountTable[index].imap = g_ptr->bg_inode_bitmap;
    mountTable[index].nblocks = s_ptr->s_blocks_count;
    mountTable[index].ninodes = s_ptr->s_inodes_count;
    strcpy(mountTable[index].name, filesys);
    strcpy(mountTable[index].mount_name, mount_point);

    mip->mounted = 1;
    mip->mptr = &mountTable[index];
    mountTable[index].mounted_inode = mip;

    print_mounted();

    return 0;
}

void print_mounted()
{
    for (int i = 0; i < 8; i++)
    {
        MOUNT *mptr = &mountTable[i];
        if (!mptr->dev)
        {
            continue;
        }
        printf("[%d] ", i);
        printf("dev: %2d ", mptr->dev);
        printf("ninodes: %4d ", mptr->ninodes);
        printf("nblocks: %4d ", mptr->nblocks);
        printf("name: %-16s ", mptr->name);
        printf("mount: %-16s\n", mptr->mount_name);
    }
}

int search_mnt_point(char *mount_point)
{
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(mountTable[i].mount_name, mount_point))
        {
            return i;
        }
    }
    return -1;
}

int mnt_alloc()
{
    for (int i = 0; i < 8; i++)
    {
        if (!mountTable[i].dev)
        {
            return i;
        }
    }
    return -1;
}

int umount(char *filesys)
{
    int index;
    int device;
    MINODE *mip;
    index = search_mnt_name(filesys);
    if (index < 0)
    {
        printf("filesys %s is not mounted\n", filesys);
        return 0;
    }
    device = mountTable[index].dev;
    printf("device = %d\n", device);

    for (int i = 0; i < NMINODE; i++)
    {

        if (minode[i].dev == device)
        {

            if (minode[i].refCount)
            {
                printf("filesys is busy cannot do umount\n");
                return 0;
            }
        }
    }
    mip = mountTable[index].mounted_inode;
    mip->mounted = 0;
    mip->mptr = 0;
    iput(mip);

    return 0;
}

int search_mnt_name(char *filesys)
{
    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(mountTable[i].name, filesys))
        {
            return i;
        }
    }
    return -1;
}

int search_mnt_dev(int dev) 
{
    for (int i = 0; i < 8; i++)
    {
        if (mountTable[i].dev == dev)
        {
            return i;
        }
    }
    return -1;
}
