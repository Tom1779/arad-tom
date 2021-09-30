/***** LAB3 base code *****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

char path[512]; // number of dirs

int tokenize(char *line, int *narg, char *args[64]) // YOU have done this in LAB2
{                                                   // YOU better know how to apply it from now on
    char *tokens;
    int size = strlen(line) + 1;
    tokens = malloc(size);
    memset(tokens, 0, size);
    char *s;
    strcpy(tokens, line);
    s = strtok(tokens, " ");
    *narg = 0;
    while (s)
    {
        args[(*narg)++] = s; // token string pointers
        s = strtok(0, " ");
    }
    args[*narg] = 0; // arg[n] = NULL pointer
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
    waitpid(*pid, status, 0);
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
        fprintf(stderr, "line = %s\n", line);
        if (fopen(line, "r"))
        {
            is_file = 1;
            break;
        }
    }
    if (!is_file)
    {
        fprintf(stderr, "invalid command %s\n", cmd);
        exit(1);
    }
}

void set_out_redir(int nargs, char* args[])
{
    for (int i = 0; i < nargs; i++)
    {
        if (!strcmp(args[i], ">"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, args[i + 1], 128);
                printf("redirecting output to %s\n", filename);
                //is_out_redir = 1;
                args[i] = 0;
                int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                break;
            }
        }
        if (!strcmp(args[i], ">>"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, args[i + 1], 128);
                //is_append_redir = 1;
                args[i] = 0;
                close(1);
                open(filename, O_WRONLY | O_APPEND, 0644);
                break;
            }
        }
    }
}

void set_in_redir(int nargs, char* args[])
{
    for (int i = 0; i < nargs; i++)
    {
        if (!args[i])
        {
            continue;
        }
        if (!strcmp(args[i], "<"))
        {
            if (i + 1 < nargs)
            {
                char filename[128];
                strncpy(filename, args[i + 1], 128);
                //is_in_redir = 1;
                args[i] = 0;
                close(0);
                open(filename, O_RDONLY);
                printf("line: %d\n", __LINE__);
                break;
            }
        }
    }
}

void pipe_writer(char *line_head, int pd[], int ndir, char *dirs[], char *env[])
{
    fprintf(stderr, "starting writer process, pid =  %d\n", getpid());
    int nargs;
    char *args[64];
    tokenize(line_head, &nargs, args);
    set_in_redir(nargs, args);
    close(pd[0]);
    dup2(pd[1], STDOUT_FILENO);
    close(pd[1]);
    get_command_line(ndir, dirs, line_head, args[0]);
    int r = execve(line_head, args, env);

    printf("writer: execve failed r = %d\n", r);
    exit(1);
}

void pipe_reader(char *line_tail, int pd[], int ndir, char *dirs[], char *env[])
{
    fprintf(stderr, "starting reader process, pid =  %d\n", getpid());
    int nargs;
    char *args[64];
    tokenize(line_tail, &nargs, args);
    set_out_redir(nargs, args);
    close(pd[1]);
    dup2(pd[0], STDIN_FILENO);
    close(pd[0]);
    get_command_line(ndir, dirs, line_tail, args[0]);
    fprintf(stderr, "line_tail = %s, len = %d\n", line_tail, strlen(line_tail));
    int r = execve(line_tail, args, env);

    printf("reader: execve failed r = %d\n", r);
    exit(1);
}

void child_proc(int ndir, char *dirs[], int nargs, char* args[], char *line, char *cmd, char *env[])
{
    int is_out_redir = 0;
    int is_in_redir = 0;
    int is_append_redir = 0;
    int is_pipe = 0;
    int writer_pid, reader_pid;
    int child_pid, child_status;
    char *line_head = malloc(128);
    char *line_tail = malloc(128);
    char *line_copy = malloc(128);
    printf("child sh %d running\n", getpid());

    for (int i = 0; i < nargs; i++)
    { // show token strings
        printf("arg[%d] = %s\n", i, args[i]);
        if (!strcmp(args[i], "|"))
        {
            is_pipe = 1;
        }
    }
    if (is_pipe)
    {
        strncpy(line_head, line, 128);
        line_head = strtok(line_head, "|");
        line_tail = strtok(NULL, "|");
        char *child_arg[64];
        int child_narg;
        int status;
        int pd[2];
        pipe(pd);
        //child_pid = fork();
        writer_pid = fork();
        if (!writer_pid) // writer
        {
            pipe_writer(line_head, pd, ndir, dirs, env);
        }
        fprintf(stderr, "waiting for writer pid = %d\n", writer_pid);
        waitpid(writer_pid, &status, 0);

        reader_pid = fork();
        if (!reader_pid)
        {
            pipe_reader(line_tail, pd, ndir, dirs, env);
        }

        //fprintf(stderr, "waiting for reader pid = %d\n", reader_pid);
        sleep(1);
        //waitpid(reader_pid, &status, 0);
        //fprintf(stderr, "status = %d\n", status);
        fprintf(stderr, "child process done, pid = %d\n", getpid());
        exit(0);
    }
    else
    {
        get_command_line(ndir, dirs, line, cmd);
        set_out_redir(nargs, args);
        set_in_redir(nargs, args);
        strncpy(line_copy, line, 128);
    }
    int r = execve(line_copy, args, env);

    printf("execve failed r = %d\n", r);
    exit(1);
}

int main(int argc, char *argv[], char *env[])
{
    char *arg[64]; // token string pointers
    int nargs;     // number of token strings
    int pid, status;
    char *cmd;
    char line[128];
    char *dirs[64] = {NULL}; // dir string pointers
    int ndir = 0;

    strncpy(path, getenv("PATH"), 512);
    printf("PATH: %s\n", path);
    parse_path(path, dirs, &ndir);

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

        tokenize(line, &nargs, arg);

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
            child_proc(ndir, dirs, nargs, arg, line, cmd, env);
        }
    }
}