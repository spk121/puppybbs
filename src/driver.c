/* Standard Fido driver parameters. */
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "driver.h"

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

int serial_port_fd = -1;
int next_char = -1;

/* Sets the screen variables. */
void scrinit()
{
    cols = 80;		/* number of CRT columns */
    lines = 24;		/* number of CRT lines, total */
    top = 0;		/* first line NUMBER */
    bottom = 23;		/* bottom line NUMBER */

    ulcorner = '+';		/* graphic characters for boxes */
    urcorner = '+';
    llcorner = '+';
    lrcorner = '+';
    vertbar = '|';
    horizbar = '-';

    // set graph_on and graph_off
    /* Modem variables */

    cd_bit = 0xffff;	/* bit to test for Carrier Detect, */
    iodev = 0;		/* serial channel number */
}

/* Full hardware initialization.  Calls scrinit().
 * Raises DTR line on serial port. */
void init()
{
    scrinit();

    // Open serial port
    serial_port_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_port_fd == -1)
    {
        perror("Opening serial port /dev/ttyUSB0");
    }
    raise_dtr();
}

void uninit()
{
    if (serial_port_fd != -1)
    {
        int ret = close(serial_port_fd);
        if (ret == -1)
            perror("Closing serial port");
    }
	printf("in dummy func uninit()\n");
}

void baud(int datarate)
{
    struct termios tios;
    if (serial_port_fd != -1)
    {
        if(tcgetattr(serial_port_fd, &tios) == -1)
        {
            perror("getting baud rate");
            return;
        }
        int code = B9600;
        if (datarate == 50) code = B50;
        if (datarate == 75) code = B75;
        if (datarate == 110) code = B110;
        if (datarate == 134) code = B134;
        if (datarate == 150) code = B150;
        if (datarate == 200) code = B200;
        if (datarate == 300) code = B300;
        if (datarate == 600) code = B600;
        if (datarate == 1200) code = B1200;
        if (datarate == 1800) code = B1800;
        if (datarate == 2400) code = B2400;
        if (datarate == 4800) code = B4800;
        if (datarate == 9600) code = B9600;
        if (datarate == 19200) code = B19200;
        if (datarate == 38400) code = B38400;
        if (datarate == 57600) code = B57600;
        if (datarate == 115200) code = B115200;
        cfsetispeed(&tios, code);
        cfsetospeed(&tios, code);
        if (tcsetattr(serial_port_fd, TCSANOW, &tios) == -1)
        {
            perror("setting baud rate");
            return;
        }
    }
}

/* Return TRUE if a character is avaiable to be read from the modem. */
int _mconstat()
{
    if (next_char != -1)
        return 1;

    unsigned char c;
    if (read (serial_port_fd, &c, 1) != -1)
    {
        next_char = c;
        return 1;
    }

    return 0;
}

/* Return the next available character from the modem. */
unsigned char _mconin()
{
    if (next_char != -1)
    {
        int c = next_char;
        next_char = -1;
        return c;
    }

    unsigned char c;
    if (read (serial_port_fd, &c, 1) != -1)
    {
        return c;
    }

    return 0;
}

/* Write a character to the modem. */
void _mconout(char c)
{
    if (serial_port_fd != -1)
    {
        if (write(serial_port_fd, &c, 1) == -1)
            perror("writing to serial port");
    }
}

/* Return true if modem is connected. */
int _cd()
{
    if (serial_port_fd != -1)
    {
        // Check serial carrier detect flag.
        int DCD_flag;
        ioctl(serial_port_fd, TIOCMGET, &DCD_flag);
        return DCD_flag & TIOCM_CAR;
    }
    return 0;
}

void lower_dtr()
{
    if (serial_port_fd != -1)
    {
        int DTR_flag;
        DTR_flag = TIOCM_DTR;
        ioctl(serial_port_fd, TIOCMBIC, &DTR_flag);
    }
}

void raise_dtr()
{
    if (serial_port_fd != -1)
    {
        int DTR_flag;
        DTR_flag = TIOCM_DTR;
        ioctl(serial_port_fd, TIOCMBIS, &DTR_flag);
    }
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





