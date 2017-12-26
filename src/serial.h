#ifndef PUPPY_SERIAL_H
#define PUPPY_SERIAL_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <termios.h>
#endif
#include <stddef.h>

#ifdef _WIN32
HANDLE serial_port_open(const char *port_name);
void serial_port_close(HANDLE h_serial);
void serial_port_set_baud_rate(HANDLE port, int baud);
int serial_port_char_available(HANDLE h_serial);
int serial_port_read_char(HANDLE h_serial);
int serial_port_write_char(HANDLE h_serial, char c);
int serial_port_get_carrier_detect(HANDLE h_serial);
void serial_port_lower_dtr(HANDLE h_serial);
void serial_port_raise_dtr(HANDLE h_serial);
#else
int serial_port_open(const char *port_name);
void serial_port_close(int port);
void serial_port_set_baud_rate(int port);
#endif

#endif
