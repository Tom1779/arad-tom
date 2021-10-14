#include "remote.h"

static char ans[MAX];
static int n;

int remote_rm(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);

    return 0;
}

int remote_mkdir(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);

    return 0;
}

int remote_rmdir(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);

    return 0;
}

int remote_pwd(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);

    return 0;
}

int remote_cd(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);

    return 0;
}

int remote_ls(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; echo=%s\n", n, ans);
    while (1)
    {
        if(!strcmp(ans, "end"))
        {
            break;
        }
        printf("client: read n=%d bytes; dir entry=%s\n", n, ans);
        n = read(sock, ans, MAX);
    }

    return 0;
}

int remote_put(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    int is_file = 0;
    char *lpath;
    char *rpath;
    char line_copy[MAX];
    strcpy(line_copy, line);
    //parse command
    strtok(line_copy, " ");
    lpath = strtok(0, " ");
    rpath = strtok(0, " ");
    //check file exists and is readable
    //if okay to proceed send file
    int source_fd = open(lpath, O_RDONLY);
    if (-1 == source_fd)
    {
        printf("failed to open source file %s, errno = %d\n", lpath, errno);
        printf("error: %s\n", strerror(errno));
        return -1;
    }
    //get local file size
    struct stat st;
    stat(lpath, &st);
    printf("local file size is %ld bytes\n", st.st_size);
    char buf[MAX];
    sprintf(buf, "put %ld %s", st.st_size, rpath);
    //send a message to the server checking if the location to be written is writable
    n = write(sock, buf, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, buf);

    //wait for response from server if ok to proceed

    n = read(sock, ans, MAX);
    printf("client: read n=%d bytes; line=%s\n", n, ans);

    //error message return -1
    if (strcmp(ans, "ok"))
    {
        printf("could not upload file, server response: %s\n", ans);
        return -1;
    }
    ssize_t byte_size;
    int byte_sent = 0; 
    while (1)
    {
        byte_size = read(source_fd, buf, MAX);
        if(!byte_size)
        {
            break;
        }
        if (-1 == byte_size)
        {
            printf("failed to read bytes\n");
            return -1;
        }
        n = write(sock, buf, MAX);
        byte_sent += byte_size;
        if(byte_sent > st.st_size)
        {
            printf("read more than file size, byte sent = %d bytes\n", byte_sent);
        }
        printf("client: sent %d bytes\n", n);
    }
    close(lpath);

    return 0;
}

int remote_get(char *line)
{
    printf("function: %s\n", __FUNCTION__);
    //parse the command

    //check destination file is writeable open for append and close

    // Send line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=%s\n", n, line);
    // server sends size

    // read file from server

    return 0;
}
