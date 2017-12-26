/* Standard Fido driver parameters. */
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#ifdef _WIN32
#else
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <pthread.h>
#endif
#include "compat.h"
#include "driver.h"
#include "serial.h"

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
#ifdef _WIN32
	serial_port_fd = serial_port_open("COM3");
#else
    serial_port_fd = xopen2("/dev/ttyUSB0", XO_RDWR | XO_NOCTTY | XO_NONBLOCK);
    if (serial_port_fd == -1)
    {
        perror("Opening serial port /dev/ttyUSB0");
    }
#endif
    raise_dtr();
}
 

void uninit()
{
    reset_clk();
#ifdef _WIN32
	if (serial_port_fd != INVALID_HANDLE_VALUE)
		serial_port_close(serial_port_fd);
#else
    if (serial_port_fd != -1)
    {
        int ret = close(serial_port_fd);
        if (ret == -1)
            perror("Closing serial port");
    }
#endif
}

void baud(int datarate)
{
#ifdef _WIN32
	if (serial_port_fd != INVALID_HANDLE_VALUE)
		serial_port_set_baud_rate(serial_port_fd, datarate);
#else
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
#endif
}

/* Return TRUE if a character is avaiable to be read from the modem. */
int _mconstat()
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return FALSE;
	return serial_port_char_available(serial_port_fd);
#else
    if (next_char != -1)
        return 1;

    unsigned char c;
    if (read (serial_port_fd, &c, 1) != -1)
    {
        next_char = c;
        return 1;
    }

    return 0;
#endif
}

/* Return the next available character from the modem. */
unsigned char _mconin()
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return FALSE;
	return serial_port_read_char(serial_port_fd);
#else
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
#endif
}

/* Write a character to the modem. */
void _mconout(char c)
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return;
	serial_port_write_char(serial_port_fd, c);
#else
    if (serial_port_fd != -1)
    {
        if (write(serial_port_fd, &c, 1) == -1)
            perror("writing to serial port");
    }
#endif
}

/* Return true if modem is connected. */
int _cd()
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return FALSE;
	return serial_port_get_carrier_detect(serial_port_fd);
#else
    if (serial_port_fd != -1)
    {
        // Check serial carrier detect flag.
        int DCD_flag;
        ioctl(serial_port_fd, TIOCMGET, &DCD_flag);
        return DCD_flag & TIOCM_CAR;
    }
    return 0;
#endif
}

void lower_dtr()
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return;
	serial_port_lower_dtr(serial_port_fd);
#else
    if (serial_port_fd != -1)
    {
        int DTR_flag;
        DTR_flag = TIOCM_DTR;
        ioctl(serial_port_fd, TIOCMBIC, &DTR_flag);
    }
#endif
}

void raise_dtr()
{
#ifdef _WIN32
	if (serial_port_fd == INVALID_HANDLE_VALUE)
		return;
	serial_port_raise_dtr(serial_port_fd);
#else
    if (serial_port_fd != -1)
    {
        int DTR_flag;
        DTR_flag = TIOCM_DTR;
        ioctl(serial_port_fd, TIOCMBIS, &DTR_flag);
    }
#endif
}

int _mbusy()
{
	int ret = 0;
	printf("in dummy func _mbusy() returns %d\n", ret);
	return ret;
}

#ifdef _WIN32
uintptr_t thd;
#else
pthread_t thd;
#endif
int thd_continue;

void clr_clk()
{
    millisec = 0;
    millis2 = 0;
    seconds = 0;
    minutes = 0;
}


#ifdef _WIN32
void clk_handler(void *arg)
{
	int tic = 1;
	while (thd_continue)
	{
		Sleep(1);

		millisec++;
		millis2++;
		if (tic % 1000 == 0)
			seconds++;
		if (seconds > 59)
		{
			seconds = 0;
			minutes++;
		}

		tic++;
	}
}
#else
void *clk_handler(void *arg)
{
    int tic = 1;
    while(thd_continue)
    {
        usleep(1000);

        millisec++;
        millis2++;
        if (tic % 1000 == 0)
            seconds ++;
        if (seconds > 59)
        {
            seconds = 0;
            minutes ++;
        }

        tic ++;
    }
    return NULL;
}
#endif

void set_clk()
{
    int ret;
    thd_continue = 1;
#ifdef _WIN32
	thd = _beginthread(clk_handler, 0, NULL);
	if (thd == -1)
		printf("error creating clock thread\n");
#else
    ret = pthread_create(&thd, NULL, clk_handler, NULL);
#endif
}

void reset_clk()
{
    thd_continue = 0;
}





