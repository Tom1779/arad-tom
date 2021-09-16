#include "tree.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

NODE *root, *cwd, *start;

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
    char *name[64];
    char temp[128];
    strcpy(temp, pathname);

    NODE *p;
    int n = 0;

    start = pathname[0] == '/' ? root : cwd;

    p = start;

    printf("path2node: %s\n", pathname);
    name[n] = strtok(temp, "/");

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

void node2path_helper(NODE *node, char *name)
{
    if (node->parent != node)
    {
        node2path_helper(node->parent, name);
    }
    else
    {
        *name = '\0';
    }
    strcat(name, node->name);
    if (node->parent != node)
    {
        strcat(name, "/");
    }
}

void node2path(NODE *node, char *name)
{
    node2path_helper(node, name);
    if (strlen(name) > 1)
    {
        name[strlen(name) - 1] = '\0';
    }
}

void normalize(char *name, char *normalized_path)
{
    char *temp = malloc(strlen(name));
    strcpy(temp, name);
    memset(normalized_path, '\0', strlen(name));
    if (temp[0] != '/')
    {
        node2path(cwd, normalized_path);
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
