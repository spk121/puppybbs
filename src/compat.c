#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include "compat.h"
#include "pupmem.h"

LONG millisec;		/* MSDOS driver: G.P. milliseconds */
LONG millis2;		/* MSDOS driver */
WORD seconds, minutes, hours; /* MSDOS driver */

int _mbusy()
{
	int ret = 0;
	printf("in dummy func _mbusy() returns %d\n", ret);
	return ret;
}

/* Return TRUE if a character is avaiable to be read from the modem. */
int _mconstat()
{
	int ret = 0;
	printf("in dummy func _mconstat() returns %d\n", ret);
	return ret;
}

/* Return the next available character from the modem. */
int _mconin()
{
	int ret = 'x';
	printf("in dummy func _mconin() returns %d\n", ret);
	return ret;
}

/* Write a character to the modem. */
void _mconout(char c)
{
	printf("in dummy func _mconin(%d)\n", c);
}

/* Return true if modem is connected. */
int _cd()
{
	int ret = 0;
	printf("in dummy func _cd() returns %d\n", ret);
	return ret;
}

/* What did this do? */
int _ioctl(int a, int b, int c, int d)
{
	int ret = 0;
	printf("in dummy func _ioctl(%d, %d, %d, %d) returning %d", a, b, c, d, ret);
	return ret;
}

void baud(uint16_t datarate)
{
	printf("in dummy func baud(%u)\n", datarate);
}

#if 0
/* bdos() was a buffered i/o operation scheme that sent or received one byte
 * at a time. */
 /* Performs an MS-DOS BDOS call by placing fn in the AH register and dx in the
 * DX register and calling the BDOS operation.  Obviously obsolete.
 * FN 1 == translated console read
 * FN 2 == translated console write.
 * FN 7 == untranslated console read.
 * FN 6 == untranslated console write
 * 'translated' means Newline reads as CRLF
 */

int bdos(int x)
{
	int ret = 0;
	if (x == 7)
		ret = _getch();
	printf("in dummy func bdos(%d) returning %d\n", x, ret);
	return ret;
}

int bdos2(int fn, int dx)
{
	int ret = 'x';

	printf("in dummy func bdos(%d,%d) returning %d\n", fn, dx, ret);
	return ret;
}
#endif

void lower_dtr()
{
	printf("in dummy func lower_dtr()\n");
}

void raise_dtr()
{
	printf("in dummy func raise_dtr()\n");
}

void init()
{
	printf("in dummy func init()\n");
}

void uninit()
{
	printf("in dummy func uninit()\n");
}

void clr_clk()
{
	printf("in dummy func clr_clk()\n");
}

void set_clk()
{
	printf("in dummy func set_clk()\n");
}

void reset_clk()
{
	printf("in dummy func reset_clk()\n");
}

static long mempool_size = 1024 * 1024;
static long mempool_alloc;

/* allmem() allocated all available memory as a pool to be used by the
 * special allocator getmem(). Returned -1 on failure, 0 on success. */
int allmem()
{
	int ret = 0;
	mempool_alloc = mempool_size;
	printf ("in dummy func allmem() returning %d\n", ret);
	return ret;
}

/* sizmem() returned the number of unallocated bytes in the memory pool
 * allocated by allmem() and used by getmem(). */
long sizmem()
{
	printf("in dummy func sizmen() returning %ld\n", mempool_alloc);
	return mempool_alloc;
}

/* getmem() returned a memory block from the special memory pool created by 
 * allmem(), or NULL on failure */
char *getmem(unsigned n)
{
	if (mempool_alloc >= n)
	{
		mempool_alloc -= n;
		char *ret = malloc(n);
		printf("in dummy func getmem() returning buffer with %ld bytes\n", n);
		return ret;
	}
	else
	{
		printf("in dummy func getmem() returning NULL\n");
		return NULL;
	}
}

int xaccess(const char *filename, int how)
{
#ifdef _WIN32
	return _access(filename, how);
#else
  return access(filename, how);
#endif
}

int xclose(int filedes)
{
#ifdef _WIN32
	return _close(filedes);
#else
  return close(filedes);
#endif
}

int xcreat(const char *filename, int mode)
{
#ifdef _WIN32
	return _creat(filename, mode);
#else
	return creat(filename, (mode_t) mode);
#endif
}

int xopen2(const char *filename, int flags)
{
#ifdef _WIN32
	return _open(filename, flags);
#else
	return open(filename, flags);
#endif
}

int xopen3(const char *filename, int flags, int mode)
{
#ifdef _WIN32
	return _open(filename, flags, mode);
#else
  return open(filename, flags, (mode_t) mode);
#endif
}

int xread(int filedes, void *buffer, unsigned int size)
{
#ifdef _WIN32
	return _read(filedes, buffer, size);
#else
	return (int)read(filedes, buffer, (size_t)size);
#endif
}

int xwrite(int filedes, const void *buffer, unsigned int size)
{
#ifdef _WIN32
	return _write(filedes, buffer, size);
#else
  return (int) write(filedes, buffer, (size_t) size);
#endif
}

long xseek(int filedes, long offset, int origin)
{
	return _lseek(filedes, offset, origin);
}

