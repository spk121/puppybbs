#pragma once
#include "puppy.h"

char *strfnd(char *string, char *pattern);
int getinfo(char *name, int n, struct _fileinfo *fileinfo);
int badname(char *name);
void close_up();
int lconin();
char keyhit();
void lconout(char c);

/* Mising functions */
void allmem();
