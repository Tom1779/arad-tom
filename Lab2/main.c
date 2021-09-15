#include <stdio.h>  // for I/O
#include <stdlib.h> // for I/O
#include <libgen.h> // for dirname()/basename()
#include <string.h>

typedef struct node
{
    char name[64]; // node's name string
    char type;     // 'D' for DIR; 'F' for file
    struct node *child, *sibling, *parent;
} NODE;

NODE *root, *cwd, *start;
char line[128];
char command[16], gpath[64];
char *name[64];

//               0       1      2       3         4       5
char *cmd[] = {"mkdir", "ls", "quit", "rmdir", "creat", "rm", 0};

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

NODE *search_child(NODE *parent, char *name)
{
    NODE *p;
    printf("search for %s in parent DIR\n", name);
    p = parent->child;
    if (p == 0)
        return 0;
    while (p)
    {
        if (strcmp(p->name, name) == 0)
            return p;
        p = p->sibling;
    }
    return 0;
}

int insert_child(NODE *parent, NODE *q)
{
    NODE *p;
    printf("insert NODE %s into END of parent child list\n", q->name);
    p = parent->child;
    if (p == 0)
        parent->child = q;
    else
    {
        while (p->sibling)
            p = p->sibling;
        p->sibling = q;
    }
    q->parent = parent;
    q->child = 0;
    q->sibling = 0;
}

int remove_child(NODE *parent, NODE *q) //new
{
    NODE *p;
    printf("remove NODE %s into END of parent child list\n", q->name);
    p = parent->child;
    if (p == q)
    {
        parent->child = q->sibling;
        free(q);
        return 0;
    }
    while (p)
    {
        if (p->sibling == q)
        {
            p->sibling = p->sibling->sibling;
            free(q);
            return 0;
        }
        p = p->sibling;
    }
    return -1;
}

NODE *path2node(char *pathname)
{
    /* Converts pathname to a NODE pointer OR NULL if pathaname invalid
  e.g. NODE *p = path2node("/a/b/c") returns pointer to the node of /a/b/c 

	                Algorithm:
  ------------------------------------------------------------------- 
          start = root  OR  cwd;

	  tokenize(pathname);
              // copy pathname into (global) gpath[];
	      // tokenize string in gpath[] into n token strings, pointed by
	      // name[0], name[1], ..., name[n-1];

	  NODE *p = start;

	  for (int i=0; i < n; i++){
	      // handle name[i] = . OR .. case;

	      p = search_child(p, name[i]);
	      if (p==0)
	         return 0;
	  }
          return p; */

    if (!pathname || pathname[0] == '\0')
    {
        return NULL;
    }

    NODE *p;
    int n = 0;

    start = pathname[0] == '/' ? root : cwd;

    p = start;

    printf("path2node: %s\n", pathname);
    name[n] = strtok(pathname, "/");

    while (name[n])
    {
        printf("component: %s\n", name[n]);
        n++;
        name[n] = strtok(NULL, "/");
    }

    for (int i = 0; i < n; i++)
    {
        if (!strcmp(name[i], "."))
        {
            continue;
        }
        if (!strcmp(name[i], ".."))
        {
            p = p->parent;
            continue;
        }

        p = search_child(p, name[i]);
        if (!p)
        {
            return NULL;
        }
    }

    return p;
}

void normalize(char *name, char *normalized_path)
{
    char *temp = malloc(strlen(name));
    strcpy(temp, name);
    memset(normalized_path, '\0', strlen(name));
    if (temp[0] != '/')
    {
        strcpy(normalized_path, cwd->name);
    }
    else
    {
        normalized_path[0] = '/';
    }
    char *token = strtok(temp, "/");
    while (token)
    {
        if (strcmp(token, ".") == 0 || strlen(token) == 0)
        {
            token = strtok(NULL, "/");
            continue;
        }
        if (strcmp(token, "..") == 0)
        {
            int i = strlen(normalized_path) - 1;
            while (normalized_path[i] != '/')
            {
                i--;
            }
            if (!i)
            {
                i++;
            }
            normalized_path[i] = '\0';
        }
        else
        {
            if (normalized_path[strlen(normalized_path) - 1] != '/')
            {
                strcat(normalized_path, "/");
            }
            strcat(normalized_path, token);
        }
        token = strtok(NULL, "/");
    }
}

void parent_of(char *name, char *parent_path)
{
    int index = -1;
    for (int i = 0; name[i]; i++)
    {
        if (name[i] == '/')
        {
            index = i;
        }
    }
    if (index == -1)
    {
        strcpy(parent_path, cwd->name);
        return;
    }
    if (index == 0)
    {
        index++;
    }
    strncpy(parent_path, name, index);
}

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

int ls(char *name) // doesnt work with cwd
{
    NODE* p;
    if (!name[0])
    {
        p = path2node(cwd->name)->child;
    }
    else
    {
        p = path2node(name)->child; // new
    }
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
    printf("Program exit\n");
    exit(0);
    // improve quit() to SAVE the current tree as a Linux file
    // for reload the file to reconstruct the original tree
}

int initialize()
{
    root = (NODE *)malloc(sizeof(NODE));
    strcpy(root->name, "/");
    root->parent = root;
    root->sibling = 0;
    root->child = 0;
    root->type = 'D';
    cwd = root;
    printf("Root initialized OK\n");
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
        default:
            printf("invalid command, try again\n");
            break;
        }
    }
}
