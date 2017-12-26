#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif
#include "serial.h"

#ifdef _WIN32
#define READ_BUF_SIZE 1
#define READ_TIMEOUT 1
DWORD dwRead;
char inbuf[READ_BUF_SIZE + 1];
LPVOID lpBuf = &inbuf[0];
BOOL fWaitingOnRead = FALSE;
OVERLAPPED osReader = { 0 };
#endif

static void serial_perror(const char* str, int code);

#ifdef _WIN32
HANDLE serial_port_open(const char *port_name)
{
	HANDLE h_serial;
	BOOL Status;

	h_serial = CreateFileA(port_name,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);
	if (h_serial == INVALID_HANDLE_VALUE)
	{
		serial_perror("opening serial port", GetLastError());
	}

	// Queue up first read event

	// Create the overlapped event. Must be closed before exiting
	// to avoid a handle leak.
	osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (osReader.hEvent == NULL)
	{
		serial_perror("creating serial port event", GetLastError());
		serial_port_close(h_serial);
		h_serial = INVALID_HANDLE_VALUE;
	}

	return h_serial;
}
#else
int serial_port_open(const char *port_name)
{
}
#endif

#ifdef _WIN32
void serial_port_close(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return;

	BOOL ret = CloseHandle(h_serial);
	if (!ret)
		serial_perror("Closing serial port", GetLastError());
}
#else
void serial_port_close(int port)
{
	close(port);
}
#endif

#ifdef _WIN32
int serial_port_char_available(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return 0;

	// If a read has alread completed.
	if (dwRead > 0)
	{
		assert(fWaitingOnRead == FALSE);
		return dwRead;
	}

	DWORD dwRes;

	if (!fWaitingOnRead)
	{
		// Issue read operation.
		if (!ReadFile(h_serial, lpBuf, READ_BUF_SIZE, &dwRead, &osReader))
		{
			if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				serial_perror("reading from serial port", GetLastError());
			else
				fWaitingOnRead = TRUE;
		}
		else
		{
			// immediate found a char to read;
			return dwRead;
		}
	}

	if (fWaitingOnRead) {
		dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
		switch (dwRes)
		{
			// Read completed.
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(h_serial, &osReader, &dwRead, FALSE))
				serial_perror("Checking for available chars on serial port", GetLastError());
			else
			{
				//  Reset flag so that another opertion can be issued.
				fWaitingOnRead = FALSE;
				// Read completed successfully.
				assert(dwRead > 0);
				return dwRead;
			}
			break;

		case WAIT_TIMEOUT:
			// Operation isn't complete yet. fWaitingOnRead flag isn't
			// changed since I'll loop back around, and I don't want
			// to issue another read until the first one finishes.
			return 0;
			break;

		default:
			// Error in the WaitForSingleObject; abort.
			// This indicates a problem with the OVERLAPPED structure's
			// event handle.
			serial_perror("Checking for available chars on serial port", GetLastError());
			return 0;
			break;
		}
	}
	return 0;
}
#else
int serial_port_char_available(HANDLE h_serial)
#endif

#ifdef _WIN32
int serial_port_read_char(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return 0;

	if (dwRead > 0)
	{
		dwRead--;
		return inbuf[0];
	}

	DWORD dwRes;

	if (!fWaitingOnRead)
	{
		// Issue read operation.
		if (!ReadFile(h_serial, lpBuf, READ_BUF_SIZE, &dwRead, &osReader))
		{
			if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				serial_perror("Reading from serial port", GetLastError());
			else
				fWaitingOnRead = TRUE;
		}
		else
		{
			// immediate found a char to read;
			dwRead--;
			return inbuf[0];
		}
	}

	if (fWaitingOnRead) {
		dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
		switch (dwRes)
		{
			// Read completed.
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(h_serial, &osReader, &dwRead, FALSE))
				serial_perror("Reading from serial port", GetLastError());
			else
			{
				//  Reset flag so that another opertion can be issued.
				fWaitingOnRead = FALSE;
				// Read completed successfully.
				dwRead--;
				return inbuf[0];
			}
			break;

		case WAIT_TIMEOUT:
			// Operation isn't complete yet. fWaitingOnRead flag isn't
			// changed since I'll loop back around, and I don't want
			// to issue another read until the first one finishes.
			return 0;
			break;

		default:
			// Error in the WaitForSingleObject; abort.
			// This indicates a problem with the OVERLAPPED structure's
			// event handle.
			serial_perror("Reading from serial port", GetLastError());
			return 0;
			break;
		}
	}
	return 0;
}
#else
#endif

#ifdef _WIN32
int serial_port_write_char(HANDLE h_serial, char c)
{
	char outbuf[2];
	LPVOID pbuf = &outbuf[0];
	DWORD dwWritten = 0;

	if (h_serial == INVALID_HANDLE_VALUE)
		return 0;
	outbuf[0] = c;
	outbuf[1] = 0;

	if (!WriteFile(h_serial, pbuf, 1, &dwWritten, NULL))
	{
		serial_perror("Reading from serial port", GetLastError());
		return 0;
	}
	return 1;
}
#else
#endif

#ifdef _WIN32
int serial_port_get_carrier_detect(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return 0;

	DWORD stat;
	BOOL ret = GetCommModemStatus(h_serial, &stat);
	if (ret == FALSE)
	{
		serial_perror("checking carrier detect", GetLastError());
		return 0;
	}
	if (stat & MS_RLSD_ON)
		return 1;
	return 0;
}
#else
#endif

#ifdef _WIN32
void serial_port_lower_dtr(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return;
	BOOL ret = EscapeCommFunction(h_serial, CLRDTR);
	if (ret == FALSE)
		serial_perror("lowering DTR", GetLastError());
}
#else
#endif

#ifdef _WIN32
void serial_port_raise_dtr(HANDLE h_serial)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return;
	BOOL ret = EscapeCommFunction(h_serial, SETDTR);
	if (ret == FALSE)
		serial_perror("raising DTR", GetLastError());
}
#else
#endif

#ifdef _WIN32
void serial_port_set_baud_rate(HANDLE h_serial, int baud)
{
	if (h_serial == INVALID_HANDLE_VALUE)
		return;

	DCB dcbSerialParams = { 0 };                        // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL Status = GetCommState(h_serial, &dcbSerialParams);     //retreives  the current settings
	if (Status == FALSE)
	{
		serial_perror("querying serial port state", GetLastError());
		return;
	}

	dcbSerialParams.BaudRate = baud;
	Status = SetCommState(h_serial, &dcbSerialParams);  //Configuring the port according to settings in DCB 
	if (Status == FALSE)
	{
		serial_perror("querying serial port state", GetLastError());
		return;
	}
}
#else
#endif

#ifdef _WIN32
static void serial_perror(const char* str, int code)
{
	static char msgbuf[256];
	msgbuf[0] = L'\0';

	if (code != -1)
	{
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
			NULL,                // lpsource
			code,                 // message id
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
			msgbuf,              // output buffer
			256,     // size of msgbuf, bytes
			NULL);               // va_list of arguments

		if (!*msgbuf)
			printf("error #%d", code);
		else
			printf("%s: %s\n", str, msgbuf);
	}
}
#endif
