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

int _mconstat();
int _mconin();
int _mbusy();
void _mconout(char c);
int _cd();
int _ioctl(int a, int b, int c, int d);
int bdos(int x);
int bdos2(int x, int y);
void baud(uint16_t datarate);
void lower_dtr();
void raise_dtr();

void init();
void uninit();

void clr_clk();
void set_clk();
void reset_clk();

void allmem();
long sizmem();	/* MSDOS */
char *getmem();	/* MSDOS */

#ifdef _WIN32
#define XS_IREAD (_S_IREAD)
#define XS_IWRITE (_S_IWRITE)
#define XO_CREAT (_O_CREAT)
#define XO_RDONLY (_O_RDONLY)
#else
#define XS_IREAD (S_IREAD)
#define XS_IWRITE (S_IWRITE)
#define XO_CREAT (O_CREAT)
#define XO_RDONLY (O_RDONLY)
#endif
int xaccess(const char *filename, int how);
int xclose(int filedes);
int xcreat(const char *filename, int mode);
int xopen2(const char *filename, int flags);
int xopen3(const char *filename, int flags, int mode);
int xread(int filedes, void *buffer, unsigned int size);
int xwrite(int filedes, const void *buffer, unsigned int size);
long xseek(int filedes, long offset, int origin);

