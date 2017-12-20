#pragma once
#include <stdint.h>
/* Standard Fido driver parameters. These must be static variables. These
are assumed to be set properly by init() in the hardware specific driver. */

/* CRT variables */

unsigned cols = 80;		/* number of CRT columns */
unsigned lines = 24;		/* number of CRT lines, total */
unsigned top = 0;		/* first line NUMBER */
unsigned bottom = 23;		/* bottom line NUMBER */

char ulcorner = '+';		/* graphic characters for boxes */
char urcorner = '+';
char llcorner = '+';
char lrcorner = '+';
char vertbar = '|';
char horizbar = '-';

char graph_on[80] = "";		/* enable graphics */
char graph_off[80] = "";	/* disable graphics */

/* Modem variables */

uint16_t cd_bit = 0xffff;	/* bit to test for Carrier Detect, */
uint16_t iodev = 0;		/* serial channel number */

/* Time stuff */

extern long millisec;		/* G.P. milliseconds */
extern long millis2;
extern int seconds,minutes,hours;

void set_clk();
void init();
void reset_clk();
void uninit();

