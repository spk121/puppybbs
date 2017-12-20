#pragma once
#include "puppy.h"

char *str_date(WORD t);
char *str_time(WORD t);
int append(char *s);
void makefname(char *d, char *s);
int dspfile(char *filename);
void dumptext(int file);
void putsys();
char rline(int file, char *buf, int len);
int num_args(char *s);
char *next_arg(char *s);
char *skip_delim(char *s);
void cpyatm(char *to, char *from);
void cpyarg(char *to, char *from);
char *strip_path(char *out, char *in);
void stolower(char *s);
void stoupper(char *s);
int atoi(char *s);
int delim(char c);