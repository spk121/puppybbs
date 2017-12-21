#pragma once
#include <stdint.h>
/* Standard Fido driver parameters. These must be static variables. These
are assumed to be set properly by init() in the hardware specific driver. */

/* CRT variables */

const static unsigned cols = 80;		/* number of CRT columns */
const static unsigned lines = 24;		/* number of CRT lines, total */
const static unsigned top = 0;		/* first line NUMBER */
const static unsigned bottom = 23;		/* bottom line NUMBER */

const static char ulcorner = '+';		/* graphic characters for boxes */
const static char urcorner = '+';
const static char llcorner = '+';
const static char lrcorner = '+';
const static char vertbar = '|';
const static char horizbar = '-';

const static char graph_on[80] = "";		/* enable graphics */
const static char graph_off[80] = "";	/* disable graphics */

/* Modem variables */

// const static uint16_t cd_bit = 0xffff;	/* bit to test for Carrier Detect, */
// const static uint16_t iodev = 0;		/* serial channel number */

/* Time stuff */

extern long millisec;		/* G.P. milliseconds */
extern long millis2;
extern uint16_t seconds,minutes,hours;




