/***** LAB3 base code *****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

char gpath[128]; // hold token strings
char *arg[64];   // token string pointers
int nargs;       // number of token strings

char dpath[128]; // hold dir strings in PATH

char path[512]; // number of dirs

int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
    char *s;
    strcpy(gpath, pathname); // copy into global gpath[]
    s = strtok(gpath, " ");
    nargs = 0;
    while (s)
    {
        arg[nargs++] = s; // token string pointers
        s = strtok(0, " ");
    }
    arg[nargs] = 0; // arg[n] = NULL pointer
}

void parse_path(char *pathname, char *dir[], int *ndir)
{
    *ndir = 0;
    dir[0] = strtok(pathname, ":");
    for (int i = 1; dir[i - 1]; i++)
    {
        dir[i] = strtok(NULL, ":");
        (*ndir)++;
        if (*ndir == 64)
        {
            break;
        }
    }
}

void parent_proc(int *pid, int *status)
{
    printf("sh %d forked a child sh %d\n", getpid(), *pid);
    printf("sh %d wait for child sh %d to terminate\n", getpid(), *pid);
    *pid = wait(status);
    printf("ZOMBIE child=%d exitStatus=%x\n", *pid, *status);
    printf("main sh %d repeat loop\n", getpid());
}

void get_command_line(int ndir, char *dirs[], char *line, char *cmd)
{
    int is_file = 0;
    for (int i = 0; i < ndir; i++)
    {
        strncpy(line, dirs[i], 128);
        strcat(line, "/");
        strcat(line, cmd);
        printf("line = %s\n", line);
        if (fopen(line, "r"))
        {
            is_file = 1;
            break;
        }
    }
    if (!is_file)
    {
        printf("invalid command %s\n", arg[0]);
        exit(1);
    }
}

void set_out_redir()
{
    for (int i = 0; i < nargs; i++)
    {
        if (!strcmp(arg[i], ">"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, arg[i + 1], 128);
                printf("redirecting output to %s\n", filename);
                //is_out_redir = 1;
                arg[i] = 0;
                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                printf("line: %d\n", __LINE__);
                break;
            }
        }
        if (!strcmp(arg[i], ">>"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, arg[i + 1], 128);
                //is_append_redir = 1;
                arg[i] = 0;
                close(1);
                open(filename, O_WRONLY | O_APPEND, 0644);
                break;
            }
        }
    }
}

void set_in_redir()
{
    for (int i = 0; i < nargs; i++)
    {
        if (!arg[i])
        {
            continue;
        }
        if (!strcmp(arg[i], "<"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, arg[i + 1], 128);
                //is_in_redir = 1;
                arg[i] = 0;
                close(0);
                open(filename, O_RDONLY);
                printf("line: %d\n", __LINE__);
                break;
            }
        }
    }
}

void child_proc(int ndir, char *dirs[], char *line, char *cmd, char *env[])
{
    int is_out_redir = 0;
    int is_in_redir = 0;
    int is_append_redir = 0;
    printf("child sh %d running\n", getpid());

    get_command_line(ndir, dirs, line, cmd);

    set_out_redir();
    set_in_redir();
    
    int r = execve(line, arg, env);

    printf("execve failed r = %d\n", r);
    exit(1);
}

int main(int argc, char *argv[], char *env[])
{
    int pid, status;
    char *cmd;
    char line[128];
    char *line_head;
    char *dirs[64] = {NULL}; // dir string pointers
    int ndir = 0;

    strncpy(path, getenv("PATH"), 512);
    printf("PATH: %s\n", path);
    parse_path(path, dirs, &ndir);

    // The base code assume only ONE dir[0] -> "/bin"
    // YOU do the general case of many dirs from PATH !!!!
    // dir[0] = "/bin";
    // ndir   = 1;
    printf("ndir = %d\n", ndir);

    // show dirs
    for (int i = 0; i < ndir; i++)
    {
        printf("dirs[%d] = %s\n", i, dirs[i]);
    }

    while (1)
    {
        printf("sh %d running\n", getpid());
        printf("enter a command line : ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;
        if (line[0] == 0)
            continue;

        tokenize(line);

        for (int i = 0; i < nargs; i++)
        { // show token strings
            printf("arg[%d] = %s\n", i, arg[i]);
        }

        cmd = arg[0]; // line = arg0 arg1 arg2 ...

        if (strcmp(cmd, "cd") == 0)
        {
            chdir(arg[1]);
            continue;
        }
        if (strcmp(cmd, "exit") == 0)
            exit(0);

        pid = fork();

        if (pid)
        {
            parent_proc(&pid, &status);
        }
        else
        {
            child_proc(ndir, dirs, line, cmd, env);
        }
    }
}