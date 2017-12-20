#pragma once
#include <stdint.h>
uint16_t gtime();
/* bytes to long integer */
long _ctol(char *pos);
/* set file time */
void _ftime(int flag, int handle, char *timedate);
/* initialize the crc*/
void clrcrc();
void updcrc(char c);
unsigned fincrc();
int chkcrc();
