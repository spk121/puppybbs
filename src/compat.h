#pragma once
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

int _ioctl(int a, int b, int c, int d);
// int bdos(int fn);
// int bdos2(int fn, int dx);

int allmem();
long sizmem();	/* MSDOS */
char *getmem(unsigned n);	/* MSDOS */


void fakemodem_baud_set(unsigned b);
void fakemodem_disconnect();
void fakemodem_chk(const char *str);
void fakemodem_answer();
int fakemodem_connect_get();

#ifdef _WIN32
#define XS_IREAD (_S_IREAD)
#define XS_IWRITE (_S_IWRITE)
#define XO_CREAT (_O_CREAT)
#define XO_RDONLY (_O_RDONLY)
#define XO_RDWR (_O_RDWR)
#else
#define XS_IREAD (S_IREAD)
#define XS_IWRITE (S_IWRITE)
#define XO_CREAT (O_CREAT)
#define XO_RDONLY (O_RDONLY)
#define XO_RDWR (O_RDWR)
#endif
int xaccess(const char *filename, int how);
int xclose(int filedes);
int xcreat(const char *filename, int mode);
int xopen2(const char *filename, int flags);
int xopen3(const char *filename, int flags, int mode);
int xread(int filedes, void *buffer, unsigned int size);
int xwrite(int filedes, const void *buffer, unsigned int size);
long xseek(int filedes, long offset, int origin);

