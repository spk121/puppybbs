#pragma once
#include <stdint.h>

struct _orig_xfbuf {
/* These are returned by DOS */

 char f_attrib;         /* found attribute, */
 uint16_t time;         /* Packed time, */
 uint16_t date;         /* Packed date */
 uint32_t fsize;            /* file size, */
 char name[13];         /* found name */
};

uint16_t gtime();
uint16_t gdate();
/* bytes to long integer */
//long _ctol(char *pos);
/* set file time */
int _ftime(int flag, int handle, uint16_t *timedate);
/* initialize the crc*/
void clrcrc();
void updcrc(char c);
unsigned fincrc();
int chkcrc();
//int _find(char *pathname, int n, struct _orig_xfbuf *xfbuf);
