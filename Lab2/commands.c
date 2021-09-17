#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "commands.h"
#include "tree.h"

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname
****************************************************************/

int mkdir(char *name)
{
    NODE *p, *q;
    char *normalized_path = malloc(strlen(name));
    char *parent_path = malloc(strlen(name)); // new
    char *base_path;
    printf("mkdir: name=%s\n", name);

    if (!strcmp(name, "/") || !strcmp(name, ".") || !strcmp(name, "..") || !strcmp(name, ""))
    {
        printf("can't mkdir with %s\n", name);
        return -1;
    }

    normalize(name, normalized_path);
    printf("check whether %s already exists\n", name);
    parent_of(normalized_path, parent_path);          // new
    printf("%s, %s\n", normalized_path, parent_path); // remove
    start = path2node(parent_path);                   //new
    if (!start)                                       //new
    {
        printf("Path does not exist\n");
        return -1;
    }
    if (start->type == 'F') //new
    {
        printf("Cannot insert directory under a file\n");
        return -1;
    }
    base_path = normalized_path + strlen(parent_path);
    if (strcmp(parent_path, "/"))
    {
        base_path++;
    }
    p = search_child(start, base_path);
    if (p)
    {
        printf("name %s already exists, mkdir FAILED\n", name);
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to mkdir %s\n", base_path);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'D';
    strcpy(q->name, base_path); // new
    insert_child(start, q);
    printf("mkdir %s OK\n", base_path);
    printf("--------------------------------------\n");

    free(parent_path);
    free(normalized_path);

    return 0;
}

int create(char *name) // new
{
    NODE *p, *q;
    char *normalized_path = malloc(strlen(name));
    char *parent_path = malloc(strlen(name)); // new
    char *base_path;
    printf("create: name=%s\n", name);

    if (!strcmp(name, "/") || !strcmp(name, ".") || !strcmp(name, "..") || !strcmp(name, ""))
    {
        printf("can't create with %s\n", name);
        return -1;
    }

    normalize(name, normalized_path);
    printf("check whether %s already exists\n", name);
    parent_of(normalized_path, parent_path); // new
    start = path2node(parent_path);          //new
    if (!start)                              //new
    {
        printf("Path does not exist\n");
        return -1;
    }
    if (start->type == 'F') //new
    {
        printf("Cannot insert file under a file\n");
        return -1;
    }
    base_path = normalized_path + strlen(parent_path);
    if (strcmp(parent_path, "/"))
    {
        base_path++;
    }
    p = search_child(start, base_path);
    if (p)
    {
        printf("name %s already exists, create FAILED\n", name);
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to create %s\n", base_path);
    q = (NODE *)malloc(sizeof(NODE));
    q->type = 'F';
    strcpy(q->name, base_path);
    insert_child(start, q);
    printf("create %s OK\n", base_path);
    printf("--------------------------------------\n");

    free(parent_path);
    free(normalized_path);

    return 0;
}

int rmdir(char *pathname) // new
{
    printf("rmdir: name=%s\n", pathname);

    if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, ".."))
    {
        printf("can't rmdir with %s\n", pathname);
        return -1;
    }

    printf("check whether %s exists\n", pathname);
    start = path2node(pathname); //new
    if (!start)                  //new
    {
        printf("Path does not exist\n");
        return -1;
    }
    if (start->type == 'F') //new
    {
        printf("Cannot remove file with rmdir\n");
        return -1;
    }
    if (start->child)
    {
        printf("Cannot remove non empty directory\n");
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to rmdir %s\n", pathname);
    if (remove_child(start->parent, start))
    {
        printf("rmdir %s failed\n", pathname);
        printf("--------------------------------------\n");
        return -1;
    }
    printf("rmdir %s OK\n", pathname);
    printf("--------------------------------------\n");

    return 0;
}

int rm(char *pathname) // new
{
    printf("rm: name=%s\n", pathname);

    if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, ".."))
    {
        printf("can't rm with %s\n", pathname);
        return -1;
    }

    printf("check whether %s exists\n", pathname);
    start = path2node(pathname); //new
    if (!start)                  //new
    {
        printf("Path does not exist\n");
        return -1;
    }
    if (start->type == 'D') //new
    {
        printf("Cannot remove directory with rm\n");
        return -1;
    }
    printf("--------------------------------------\n");
    printf("ready to rm %s\n", pathname);
    if (remove_child(start->parent, start))
    {
        printf("rm %s failed\n", pathname);
        printf("--------------------------------------\n");
        return -1;
    }
    printf("rm %s OK\n", pathname);
    printf("--------------------------------------\n");

    return 0;
}

int cd(char *name) //new
{
    NODE *p;
    printf("cd into %s\n", name);
    p = path2node(name);
    if (!p)
    {
        printf("path does not exist, please try again\n");
        return -1;
    }
    cwd = p;
    printf("cwd: %s\n", cwd->name);
}

int pwd() //new
{
    char path[128];
    node2path(cwd, path);
    printf("pwd: %s\n", path);
}

int ls(char *name) // doesnt work with cwd
{
    NODE *p;
    if (!name[0])
    {
        p = cwd;
    }
    else
    {
        p = path2node(name); // new
    }
    if (!p)
    {
        printf("invalid path\n");
        return -1;
    }
    p = p->child;
    printf("directory contents = ");
    while (p)
    {
        printf("[%c %s] ", p->type, p->name);
        p = p->sibling;
    }
    printf("\n");
}

int quit()
{
    save("systemTree.txt");
    printf("Program exit\n");
    exit(0);
    // improve quit() to SAVE the current tree as a Linux file
    // for reload the file to reconstruct the original tree
}

void save_helper(FILE* fp, NODE* node) 
{
    char pathname[128];
    node2path(node, pathname);
    fprintf(fp, "%c %s\n", node->type , pathname); // print a line to file
    if(node->child)
    {
        save_helper(fp, node->child);
    }
    if(node->sibling)
    {
        save_helper(fp, node->sibling);
    }
}

void save(char *filename)
{
    FILE *fp = fopen(filename, "w+");      // fopen a FILE stream for WRITE
    save_helper(fp, root);
    fclose(fp);                            // close FILE stream when done
}


void reload(char *filename)
{
    FILE *fp = fopen(filename, "r");
    char line[132];

    while(fgets(line, sizeof(line), fp))
    {
        line[strlen(line) - 1] = '\0';
        if(line[0] == 'D')
        {
            mkdir(line + 2);
        }
        else
        {
            create(line +2);
        }
    }
    fclose(fp);
}

void menu() 
{
    printf("Command List:\n-mkdir pathname\n-rmdir pathname\n-creat pathname\n-rm pathname\n-save\n-reload filename\n-quit\n");
}