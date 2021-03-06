#pragma once
char *getarg(char *prompt);
int isargs();
void cmdflush();
int ask(char *s);
int ques(char *s);
int input(char *prompt, char *buff, int len);
void fmconout(char c);
void mconout(char c);
int mconin();
int mconstat();
void pollkbd();
void put_c(unsigned char c);
void mconflush();
void mprintf(char *f, ...);
void mputs(char *s);
void limitchk();
void modout(char c);
int modin(unsigned n);
void delay(int n);
int cd();
void flush(int n);
