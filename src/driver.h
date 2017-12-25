#pragma once
#include <stdint.h>
/* Standard Fido driver parameters. These must be static variables. These
are assumed to be set properly by init() in the hardware specific driver. */

/* CRT variables */

extern unsigned cols;		/* number of CRT columns */
extern unsigned lines;		/* number of CRT lines, total */
extern unsigned top;		/* first line NUMBER */
extern unsigned bottom;		/* bottom line NUMBER */

extern char ulcorner;		/* graphic characters for boxes */
extern char urcorner;
extern char llcorner;
extern char lrcorner;
extern char vertbar;
extern char horizbar;
extern char graph_on[80];		/* enable graphics */
extern char graph_off[80];	/* disable graphics */

/* Modem variables */

extern uint16_t cd_bit;	/* bit to test for Carrier Detect, */
extern uint16_t iodev;		/* serial channel number */

/* Time stuff */
extern long millisec;
extern long millis2;
extern int seconds, minutes, hours;

void scrinit();
void init();
void uninit();
void baud(int rate);
int _mconstat();
unsigned char _mconin();
void _mconout(char c);
int _cd();
void lower_dtr();
void raise_dtr();
int _mbusy();
void clr_clk();
void set_clk();
void reset_clk();






