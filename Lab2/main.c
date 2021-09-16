#include <stdio.h>  // for I/O
#include <stdlib.h> // for I/O
#include <libgen.h> // for dirname()/basename()
#include <string.h>

#include "commands.h"
#include "tree.h"

char line[128];
char command[16], gpath[64];

//               0       1      2       3         4       5     6     7      8         9        10
char *cmd[] = {"mkdir", "ls", "quit", "rmdir", "creat", "rm", "cd", "pwd", "save", "reload", "menu", 0};

int findCmd(char *command)
{
    int i = 0;
    while (cmd[i])
    {
        if (strcmp(command, cmd[i]) == 0)
            return i;
        i++;
    }
    return -1;
}

int main()
{
    int index;

    initialize();

    printf("NOTE: commands = [mkdir|ls|quit]\n");

    while (1)
    {
        printf("Enter command line : ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        command[0] = gpath[0] = 0;
        sscanf(line, "%s %s", command, gpath);
        printf("command=%s pathname=%s\n", command, gpath);

        if (command[0] == 0)
            continue;

        index = findCmd(command);

        switch (index)
        {
        case 0:
            mkdir(gpath);
            break;
        case 1:
            ls(gpath);
            break;
        case 2:
            quit();
            break;
        case 3:
            rmdir(gpath);
            break;
        case 4:
            create(gpath);
            break;
        case 5:
            rm(gpath);
            break;
        case 6:
            cd(gpath);
            break;
        case 7:
            pwd(gpath);
            break;
        case 8:
            save(gpath);
            break;
        case 9:
            reload(gpath);
            break;
        case 10:
            menu();
            break;
        default:
            printf("invalid command, try again\n");
            break;
        }
    }
}
