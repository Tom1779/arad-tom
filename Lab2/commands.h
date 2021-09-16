#pragma once

#include "tree.h"

int mkdir(char *name);

int create(char *name); // new

int rmdir(char *pathname); // new

int rm(char *pathname); // new

int cd(char *name); //new

int pwd(); //new

int ls(char *name); // doesnt work with cwd

int quit();

void save_helper(FILE* fp, NODE* node);

void save(char* filename);

void reload(char* filename);

void menu();