#pragma once
#include <unistd.h>
#include <sys/stat.h>

#define XS_IREAD (S_IREAD)
#define XS_IWRITE (S_IWRITE)
#define XO_CREAT (O_CREAT)
int xaccess(const char *filename, int how);
int xclose(int filedes);
int xcreat(const char *filename, mode_t mode);
int xopen2(const char *filename, int flags);
int xopen3(const char *filename, int flags, mode_t mode);
ssize_t xread(int filedes, void *buffer, size_t size);
ssize_t xwrite(int filedes, const void *buffer, size_t size);
