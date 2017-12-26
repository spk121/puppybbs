#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN32
#else
#include <termios.h>
#include <sys/ioctl.h>
#endif
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "compat.h"
#include "ms-asm.h"
#include "ms-c.h"
#include "puppy.h"

/*

	MSDOS dependent C routines

	T. Jennings
	Fido Software
	164 Shipley
	San Francisco CA 94107
	(415)-764-1688

	(k) all rights reversed


char *strfnd(text,pattern)
		Search through the text for the specified pattern,
		return a pointer to the text if found else zero. This
		is a case insensitive match.

getinfo(filename,n,fileinfo)
		Returns specific information on the requested filename.
		This also is used for search first/next to locate a 
		series of matching files, using the usual wildcards
		* and ?. The function returns 0 when no matching file
		is found.

		filename is a string specifying the file; it is fully
		qualified, ie. it has drive letter/pathname info in it.
		It may be in either upper or lower case.

		N is an integer that indicates which pass # this is. N is 0
		for the first time, and increments for each subsequent
		call.

		fileinfo is the structure defined in puppy.h that the 
		information is returned in. Its generic enough that it
		should be easy enough to fill in for most DOSs, or the
		information left zeroed. The contents of the structure
		is preserved between calls; it can be used to store stuff
		between iterations. n == 0 should be used to set the info
		for later calls. (The MSDOS version uses the _find()
		function defined in MS-ASM.ASM, which stores its info
		in added space in the fileinfo structure.)


		Unlike search first/next in CP/M, there may be any number
		of file system calls made in between calls to getinfo().
		If this is a problem (CP/M ...) then getinfo() should make
		a list of files when n == 0 and return them one by one
		in each call.

		It is even possible to merely get basic file info when
		n == 0, and return 0 for all other calls; the only effect
		will be that wildcards cant be used in various places.

badname(s)	Returns true if the passed filename is not legal for
		this DOS. MSDOS: Checks for devices, directories, etc.

close_up()	MSDOS kludge: when carrier is lost (and we stack jump
		out) we gotta close any open files else the handles
		stay busy and unavailable. MSDOS: just close all
		handles 6 - 20. 1 - 5 are standard devices stdin, stdout,
		stdprn, etc.

lconout(c)	Output a character to the local console.

lconin()	Get a character from the local keyboard.

keyhit()	Get a character from the local keyboard if one is
		available, else return 0.
*/

/* Find a string, return a pointer to it or null if not found. */

#if 0
static uint16_t msdos_time_sec(uint16_t x)
{
	return 2 * (x & 0x01f);
}

static uint16_t msdos_time_min(uint16_t x)
{
	return (x >> 5) & 0x3f;
}

static uint16_t msdos_time_hour(uint16_t x)
{
	return (x >> 11) & 0x3f;
}
#endif

// MS-DOS date
// 5 bits for day
// 4 bits for month
// 7 bits for years since 1980

char *strfnd(char *string, char *pattern)
{
	char *s,*p;

	while (1) {
		s= string; p= pattern;
		if ((*s == 0) || (*s == 26)) break;	/* NUL or ^Z is end */
		while (*p && (tolower(*s) == tolower(*p))) {
			++s; ++p;			/* mismatch or */
		}					/* end of pattern, */
		if (*p == 0) return(string);		/* found it! */
		++string;				/* next ... */
	}
	return NULL;
}

/* Get file info. */

int getinfo(char *name,  /* filename, */
	int n,               /* interation counter */
	struct _fileinfo *fileinfo)  /* returned file information */
{
	fileinfo->xfbuf.s_attrib= 0;		/* set search attribute */
	if (!_find(name,n,(struct _xfbuf *)&fileinfo->xfbuf))	/* do the MSDOS thing, */
		return(0);			/* no matches */

	strcpy(fileinfo->name,fileinfo->xfbuf.name);

	fileinfo->size= fileinfo->xfbuf.fsize; /* copy in the basic info */
#if 0
	fileinfo->time = fileinfo-> xfbuf.date & 0x0f;
	fileinfo->time |= ((fileinfo-> xfbuf.date >> 5) & 0x0f) << 5;
	fileinfo->time |= ((fileinfo-> xfbuf.date >> 9) & 0x3f) + 80;

/* Unpack the MSDOS time. MSDOS keeps file time seconds in 2 sec. 
resolution. Who needs it anyways. */

	fileinfo->time = (fileinfo->xfbuf.time >> 11) << 11;
	fileinfo->time |= ((fileinfo-> xfbuf.time >> 5) & 0x3f) << 5;
#endif

	return(1);
}

/* Return true if this name is a reserved MSDOS filename. ie. a 
directory or device name. */

int badname(char *name)
{
	int i,f;

	if (*name == '.') return(1);	/* directory */
	f= xopen2(name,2);		/* if we can't open it */
	if (f == -1) return(0);		/* it aint there */
	i= _ioctl(0,f,0,0);		/* see if device */
	xclose(f);			/* close handle */
	if (i == -1) return(0);		/* some error */
	return(i & 0x80);		/* 0x80 is the 'device' bit */
}

/* Since we may have been aborted at any point, we probably
had open files. This is gross, but this closes all possible handles
except the ones we know of (log file, nodemap file)  */

void close_up()
{
	int i;

	for (i= 6; i < 20; i++) {
		xclose(i);
	}
}





/* get a character from the console. */

int lconin()
{
#ifdef _WIN32
	//return(bdos(7));
	return _getch();
#else
	return getchar();
#endif
}
/* If a key is hit, return the key, else 0. */

char keyhit()
{
#ifdef _WIN32
	if (_kbhit())
		return _getch();
	else
		return 0;
#else
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	cfmakeraw(&newt);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != -1)
		return ch;
	return 0;
#endif
}

/* Type a character, dont change cursor position. */

void lconout(char c)
{
	// FIXME: this changes cursor position
	putchar(c);
	// bdos2(6,c);
}
