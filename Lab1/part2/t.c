/* sample code for Part 2 */
#include <stdio.h>
#include <fcntl.h>

#include <sys/types.h>
#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct partition
{
    // SAME AS GIVEN ABOVE
    u8 drive; // drive number FD=0, HD=0x80, etc.

    u8 head;     // starting head
    u8 sector;   // starting sector
    u8 cylinder; // starting cylinder

    u8 sys_type; // partition type: NTFS, LINUX, etc.

    u8 end_head;     // end head
    u8 end_sector;   // end sector
    u8 end_cylinder; // end cylinder

    u32 start_sector; // starting sector counting from 0
    u32 nr_sectors;   // number of of sectors in partition
};

char *dev = "vdisk";
int fd;

// read a disk sector into char buf[512]
int read_sector(int fd, int sector, char *buf)
{
    lseek(fd, sector * 512, SEEK_SET); // lssek to byte sector*512
    read(fd, buf, 512);                // read 512 bytes into buf[ ]
}

int main()
{
    struct partition *p;
    char buf[512];

    fd = open(dev, O_RDONLY); // open dev for READ
    read_sector(fd, 0, buf);  // read in MBR at sector 0

    p = (struct partition *)(&buf[0x1be]); // p->P1

    // print P1's start_sector, nr_sectors, sys_type;
    printf("start_sector: %2d, nr_sectors: %8d, sys_type: %2x\n", p->start_sector, p->nr_sectors, p->sys_type);

    // Write code to print all 4 partitions;
    for (int i = 0; i < 4; i++)
    {
        printf("P %2d: start_sector: %8d, nr_sectors: %8d, sys_type: %2x\n", i + 1, p[i].start_sector, p[i].nr_sectors, p[i].sys_type);
    }

    //ASSUME P4 is EXTEND type:
    //Let int extStart = P4's start_sector; print extStart to see it;
    int extStart = p[3].start_sector;
    printf("extStart: %8d\n", extStart);

    int localMBR = extStart;
    int i = 5;
    while (1)
    {
        read_sector(fd, localMBR, buf);
        p = (struct partition *)(&buf[0x1be]); // p->P5
        printf("1st entry start_sector: %8d, nr_sectors: %8d\n", p->start_sector, p->nr_sectors);
        printf("P %2d: start_sector: %8d, nr_sectors: %8d, end: %8d, sys_type: %2x\n", i, localMBR + p->start_sector, p->nr_sectors,
        localMBR + p->start_sector + p->nr_sectors, p->sys_type);
        if (p[1].start_sector == 0)
        {
            break;
        }
        localMBR = extStart + p[1].start_sector;
        printf("localMBR: %8d\n", localMBR);
        i++;
    }
}
