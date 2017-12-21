#pragma once
#include "puppy.h"
int badname(char *name);
int getinfo(char *name, int n, struct _fileinfo *fileinfo);
char keyhit();

char *strfnd(char *string, char *pattern);
void lconout(char c);

/* Mising functions */
void allmem();
void close_up();