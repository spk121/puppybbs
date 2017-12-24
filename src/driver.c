/* Standard Fido driver parameters. */

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
long millisec;
long millis2;
int seconds, minutes, hours;


void init()
{
	printf("in dummy func init()\n");
}

void uninit()
{
	printf("in dummy func uninit()\n");
}

void baud(uint16_t datarate)
{
	//printf("in dummy func baud(%u)\n", datarate);
	fakemodem_baud_set(datarate);
}

/* Return TRUE if a character is avaiable to be read from the modem. */
int _mconstat()
{
	if (fakemodem.connected)
		return _kbhit();
	else
		return 0;
#if 0
	int ret = 0;
	printf("in dummy func _mconstat() returns %d\n", ret);
	return ret;
#endif
}

/* Return the next available character from the modem. */
int _mconin()
{
#if 0
	int ret = 'x';
	printf("in dummy func _mconin() returns %d\n", ret);
	return ret;
#endif
	return _getch();
}

/* Write a character to the modem. */
void _mconout(char c)
{
#if 0
	printf("in dummy func _mconin(%d)\n", c);
#endif
	_putchar_nolock(c);
}

/* Return true if modem is connected. */
int _cd()
{
	int ret = fakemodem_connect_get();
	// printf("in dummy func _cd() returns %d\n", ret);
	return ret;
}

void lower_dtr()
{
	printf("in dummy func lower_dtr()\n");
}

void raise_dtr()
{
	printf("in dummy func raise_dtr()\n");
}

int _mbusy()
{
	int ret = 0;
	printf("in dummy func _mbusy() returns %d\n", ret);
	return ret;
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
    printf("in dummy func reset_clk()");
}





