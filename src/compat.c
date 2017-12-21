#include <fcntl.h>
#include <unistd.h>
#include "compat.h"

int xaccess(const char *filename, int how)
{
  return access(filename, how);
}

int xclose(int filedes)
{
  return close(filedes);
}

int xcreat(const char *filename, mode_t mode)
{
  return creat(filename, mode);
}

int xopen2(const char *filename, int flags)
{
  return open(filename, flags);
}

int xopen3(const char *filename, int flags, mode_t mode)
{
  return open(filename, flags, mode);
}

ssize_t xread(int filedes, void *buffer, size_t size)
{
  return read(filedes, buffer, size);
}

ssize_t xwrite(int filedes, const void *buffer, size_t size)
{
  return write(filedes, buffer, size);
}

