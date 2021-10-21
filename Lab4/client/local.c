#include "local.h"

int local_rm(char *line)
{
    char line_copy[MAX];
    strcpy(line_copy, line);
    char *path;
    strtok(line_copy, " ");
    path = strtok(0, " ");
    r = unlink(path);
    if (-1 == r)
    {
        printf("failed to remove file: %s\n", path);
    }
    return r;
}

int local_mkdir(char *line)
{
    char line_copy[MAX];
    strcpy(line_copy, line);
    char *path;
    strtok(line_copy, " ");
    path = strtok(0, " ");
    r = mkdir(path, 0755);
    if (-1 == r)
    {
        printf("failed to make directory: %s\n", path);
    }
    return r;
}

int local_rmdir(char *line)
{
    char line_copy[MAX];
    strcpy(line_copy, line);
    char *path;
    strtok(line_copy, " ");
    path = strtok(0, " ");
    r = rmdir(path);
    if (-1 == r)
    {
        printf("failed to remove directory: %s\n", path);
    }
    return r;
}

int local_pwd(char *line)
{
    char buf[MAX];
    getcwd(buf, MAX);

    printf("pwd: %s\n", buf);

    return 0;
}

int local_cd(char *line)
{
    char line_copy[MAX];
    strcpy(line_copy, line);
    char *path;
    strtok(line_copy, " ");
    path = strtok(0, " ");
    int r = chdir(path);
    if (-1 == r)
    {
        printf("failed to change to directory: %s\n", path);
    }
    return r;
}

int local_ls(char *line)
{
    DIR *directory;
    struct dirent *contents;
    struct stat fileStat;
    char dir_name[MAX];
    int files = 0;
    if (strcmp(line, "lls"))
    {
        char line_copy[MAX];
        strcpy(line_copy, line);
        strtok(line_copy, " ");
        strcpy(dir_name, strtok(0, " "));
    }
    else
    {
        strcpy(dir_name, ".");
    }
    printf("dirname = %s\n", dir_name);
    directory = opendir(dir_name);
    if (!directory)
    {
        printf("failed to read directory %s, errno = %d\n", dir_name, errno);
        printf("error: %s\n", strerror(errno));
        return -1;
    }
    while ((contents = readdir(directory)))
    {
        stat(contents->d_name, &fileStat);
        files++;
        printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileStat.st_mode & S_IXOTH) ? "x " : "- ");
        printf("\tFile Size: %d bytes ", fileStat.st_size);
        printf("\tNumber of Links: %d ", fileStat.st_nlink);
        printf("\tFile inode: \t%d ", fileStat.st_ino);

       
        printf("File Name: %s\n", contents->d_name);
    }
    closedir(directory);
    return 0;
}

int local_cat(char *line)
{
    char buf[4097];
    char line_copy[MAX];
    strcpy(line_copy, line);
    char *source;
    strtok(line_copy, " ");
    source = strtok(0, " ");
    printf("source = %s\n", source);

    int source_fd = open(source, O_RDONLY);
    if (-1 == source_fd)
    {
        printf("failed to open source file %s, errno = %d\n", source, errno);
        printf("error: %s\n", strerror(errno));
        return -1;
    }
    ssize_t byte_size;
    while (byte_size = read(source_fd, buf, 4096))
    {
        if (-1 == byte_size)
        {
            printf("failed to read bytes\n");
            return -1;
        }
        buf[byte_size] = 0;
        printf("%s", buf);
    }
    printf("\n");
    close(source);
    return 0;
}
