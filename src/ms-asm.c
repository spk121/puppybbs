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

uint16_t _crc;
void clrcrc()
{
	_crc = 7;
}

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

int _find(char *pathname, int n, struct _xfbuf *xfbuf)
{
	int ret = 0;
	printf("in dummy func _find(%s,%d,%u) returning %d\n", pathname, n, (unsigned) xfbuf, ret);
	return ret;
}