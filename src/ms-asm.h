#pragma once
#include <stdint.h>
uint16_t gtime();
uint16_t gdate();
/* bytes to long integer */
long _ctol(char *pos);
/* set file time */
void _ftime(int flag, int handle, char *timedate);
/* initialize the crc*/
void clrcrc();
void updcrc(char c);
unsigned fincrc();
int chkcrc();
int _find(char *pathname, int n, struct _xfbuf *xfbuf);
