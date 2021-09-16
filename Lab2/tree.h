#pragma once

typedef struct node
{
    char name[64]; // node's name string
    char type;     // 'D' for DIR; 'F' for file
    struct node *child, *sibling, *parent;
} NODE;

extern NODE *root, *cwd, *start;

int initialize();

NODE *search_child(NODE *parent, char *name);

int insert_child(NODE *parent, NODE *q);

int remove_child(NODE *parent, NODE *q); //new

NODE *path2node(char *pathname);

void node2path_helper(NODE *node, char *name);

void node2path(NODE* node, char* name);

void normalize(char *name, char *normalized_path);

void parent_of(char *name, char *parent_path);