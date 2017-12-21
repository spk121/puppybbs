#include "ms-asm.h"

uint16_t gtime()
{
	unsigned ret = 0;
	printf("in dummy func gtime() returning %u\n", ret);
	return ret;
}

uint16_t gdate()
{
	unsigned ret = 0;
	printf("in dummy func gdate() returning %u\n", ret);
	return ret;
}

void clrcrc()
{
	printf("in dummy function clrcrc()\n");
}

void updcrc(char c)
{
	printf("in dummy function updcrc(%d)\n", c);
}

unsigned fincrc()
{
	unsigned ret = 0;
	printf("in dummy func fincrc() returning %u\n", ret);
	return ret;
}

int chkcrc()
{
	int ret = 0;
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