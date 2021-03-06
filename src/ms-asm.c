#ifdef _WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <time.h>
#include "ms-asm.h"

/* MS-DOS 16-bit time appears to have been
   Bits 0-4: seconds divided by 2
   Bits 5-10: Minutes 0 to 59
   Bits 11-15: Hour 0 to 23
*/
uint16_t gtime()
{
	time_t now;
	time(&now);
	struct tm *tt = localtime(&now);
	return (tt->tm_sec / 2 | (tt->tm_min << 5) | (tt->tm_hour << 11));
}

/* MS-DOS 16-bit date appears to have been
  Bits 0-4 day of month 1-31
  Bits 5-8 Month with 1 = January, 2 = February, etc
  Bits 9-15 Years - 1980
  */
uint16_t gdate()
{
	time_t now;
	time(&now);
	struct tm *tt = localtime(&now);
	return (tt->tm_mday | (tt->tm_mon + 1) << 5 | ((tt->tm_year - 80) << 9));
}

#if 0
/* puts a MS-DOS long, really an int32, into
 * a byte array. 
 */
void _ltoc(char *dst, uint32_t src)
{
	memcpy (dst, &src, 4);
}

/* Puts 4 bytes into an MS-DOS long, really an int32
*/
void _ctol(uint32_t *dst, const char *src)
{
	memcpy(dst, src, 4);
}
#endif

/* Note on 8086 registers
 * AX, BX, CX, DX are 16-bit general purpose
 * SP, and BP are stack pointer and base pointer
 * SI, DI, BX, and BP are address registers */

/* MS-DOS call 0x44 IOCTL.
  0 Get device info
  1 set device info
  1 read bytes into buf
  3 write butes from buf
  4 read bytes int buf, but handle is drive number, 0 = default 1 = A
  5 write bytes from buf except handle is drive number
  6 get input status
  7 get output status

  The only device that puppybbs uses is _ioctl(0, f, 0, 0)
  It is supposed to return a device information byte
  And then it only checks bits masked by 0x80.

  Really this is just used to check if a file name is invalid, so
  it is not necessary.
*/
#if 0
int _ioctl(uint16_t func_al, uint16_t handle_bx, uint16_t data_dx, uint16_t dummy_cx)
{

}
#endif

uint16_t _crc;
void clrcrc()
{
	_crc = 0;
}

/** Hmm. copy byte into AX
 * rotate AL left with carry
 * rotate BX left with carry
 * XOR BX with 0x1021
 * or whatever
 * */

void updcrc(char c)
{
	_crc = _crc * 37 + c;
}

unsigned fincrc()
{
	return _crc;
}

int chkcrc()
{
	int ret = 1;
	printf("in dummy func chkcrc() returning %d\n", ret);
	return ret;
}

long _ctol(char *pos)
{
	return *(long *)pos;
}

/* This is analagous to the old MS-DOS INT 21,4Eh and INT 21,4Fh interrupt API call.
 * There was a file attribute input flag
 * bit 0 = read only
 * bit 1 = hidden
 * bit 2 = system
 * bit 3  = volume label
 * bit 4 = subdirectory
 * bit 5 = archive
 * 
 * And there was an 8.3 filename with MS-DOS wildcards
 * 
 * The return value was zero on success or one of the DOS error codes
 * 
 *  The return structure was
 * x00 -- 14 undocumented
 * x15 byte -- attribute of matching file
 * x16 word -- MS-DOS file time
 * x18 word -- MS-DOS file time
 * x1A word -- MS-DOS file size
 * x1E char[13] -- ms-dos 8.3 filename
 * 

 * which later morphed into the FindFirstFile and FindNextFile Win32 API
 * On the Win32 API, each call populates a WIN32_FIND_DATA struct;
 *
 * Searches a directory for a file or subdirectory with a name that matches
 * a specific name, or a partial name is wildcards are used.
 * 
 * Keep calling it until the return value is zero to get all the matches.
 * 
 * This is really only used in getinfo(), so it could be bypassed altogether.
 */
int _find(char *pathname, int n, struct _orig_xfbuf *xfbuf)
{
	int ret = 0;
	printf("in dummy func _find(%s, %d, %lld) returning %d\n", pathname, n, (unsigned long long) xfbuf, ret);
	return ret;
}

#if 0
_open();

_creat();

_close();

_lseek();
#endif

/* As far as I can tell, this sets (if op = 1) or gets
 * if (op = 0) the timestamp on the file given by HANDLE,
 * where time is given as a 4-byte of 16-bit MS-DOS time
 * and 16-bit MS-DOS date.
 * 
 * It is used in xmodem.c to preserve the creation date
 * of files downloaded via xmodem.
 * */
#ifdef _WIN32
int _filetime(int op, HANDLE handle, uint16_t *timedat)
#else
int _filetime(int op, int handle, uint16_t *timedat)
#endif
{
	if (op == 1)
	{
		/* setting file time */
		time_t now = time(0);
		struct tm *brokentime = localtime(&now);
		brokentime->tm_sec = 2 * (timedat[0] & 0x1f);
		brokentime->tm_min = (timedat[0] >> 5) & 0x3f;
		brokentime->tm_hour = (timedat[0] >> 11);
		brokentime->tm_mday = timedat[1] & 0x1f;
		brokentime->tm_mon = ((timedat[1] >> 5) & 0xf) - 1;
		brokentime->tm_year = (timedat[1] >> 9) + 80;	
		time_t timestamp = mktime(brokentime);
		struct timeval TVP[2];
		TVP[0].tv_sec = timestamp;
		TVP[0].tv_usec = 0;
		TVP[1].tv_sec = timestamp;
		TVP[1].tv_usec = 0;
#ifdef _WIN32
		SYSTEMTIME systime;
		FILETIME win32time;
		systime.wYear = brokentime->tm_year + 1900;
		systime.wMonth = brokentime->tm_mon;
		systime.wDay = brokentime->tm_mday;
		systime.wHour = brokentime->tm_hour;
		systime.wMinute = brokentime->tm_min;
		systime.wSecond = brokentime->tm_sec;
		SystemTimeToFileTime(&systime, &win32time);
		BOOL ret = SetFileTime(handle, &win32time, &win32time, &win32time);
		return ret;
#else
		futimes(handle, TVP);
#endif
		return 1;
	}
	return 0;
}
