#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <string.h>
#include "ascii.h"
#include "compat.h"
#include "modemio.h"
#include "ms-c.h"
#include "printf.h"
#include "puppy.h"
#include "pupmem.h"
#include "support.h"

/*
	These are all "medium low" level I/O for Puppy. There shouldnt be
any DOS dependencies in here; the hardware drivers do that. These call the
"generic" hardware as described in the interface docs.

	They may seem large for just a bunch of I/O routines, but there are
things in here that eliminate many so-called "high-level" functions such as
word wrap, command-ahead and input buffer control, etc. 


	T. Jennings
	Fido Software
	164 Shipley
	San Francisco CA 94107

	(k) all rights reversed


	All of the following functions call functions from the same
	list, or ones below it. 


	Puppy's Console I/O. These will log the caller off due to
	time limits, inactivity for 10 minutes or carrier loss. 

getargs(prompt)	Displays the prompt, returns a pointer to the inputted string.
isargs()	Return true if there are more args that could be read.
input(prompt,buff,length) Displays a prompt, inputs a line of text. This is
		the thing that does the actual prompting and input when
		getargs runs out of things to return.
ask(ques)	Prints a string, inputs Y or N, returns true if Y. This
		does a nice neat question input.
cmdflush()	Flush any command-ahead.

	These do "formatted" I/O; printf() type stuff, word wrap, etc.

mprintf()	Same as PRINTF, except goes to the modem. 
mputs()		Put a string to the modem.
fmconout(c)	Outputs characters to the console, wrapping words to fit
		within the set screen width. NOTE: You must issue a ^Z
		after the last character to flush out any characters
		stuck in the word buffer. 

	These are "plain" console I/O.

mconout(c)	Output C to the console. Maintains line/column
		cursor position, handles control codes, etc.
mconin()	Returns a character from the console. 
mconstat()	Return true if a character available.
mconflush()	Throw away all keyboard input.
ques(q)		A low level question asker: accepts various things
		for input. Does minimal cursor motion & cleanup.

	Puppy's Modem I/O. These will log the caller off if carrier
	is lost. These do no control code processing or other
	console type functions.

modin(n)	Returns character available, else -1 if N centiseconds elapse.
modout(c)	Outputs C to the modem.
modstat()	Returns true if a modem character available.

	Puppy's low level I/O support functions.

limitchk()	Checks the set time limit, and logs off if exceeded.
flush(n)	Flush the modem until it is clear for n centiseconds.
cd()		Returns true if Carrier Detect is true or cd_flag is true.
carrierchk()	Checks for modem carrier via cd() and logs off if not present.
limitchk()	Checks caller time limits in effect and logs off if exceeded.

*/

/* NOTE: There is a subtle kludge here that enables the detection of 
incoming Crash Mail.

	The keyboard input routines in BUFIO.C (mconstat(),
mconin(), etc) strip parity of incoming characters, EXCEPT for TSYNC.

	If input() detects a TSYNC at anytime, it clears any
input it may have accumulated and returns a string with the single
character TSYNC in it. 

	If getarg() detects a TSYNC in its input, it immediately
returns a pointer to the TSYNC.

	All this for Crash Mail. I hate this kludge.

*/


static char line_buff[SS] = "";		/* command line buffer */
static char *_nargp = line_buff;	/* pointer into it */

/* These are used by fmconout() */

static char word[SS];			/* word we're building */
static char wordi = 0;			/* index/length of word */
#define MARGIN 2			/* right margin on screen */

/* This buffer lets Puppy do typeahead even on systems that
dont have interrupts. The input is polled during output & input
and any characters are stored away in here. Input requests check the
buffer and do further polling. */

static char conbuf[SS];			/* console type ahead buffer */
static int head = 1;			/* indices into it */
static int tail = 0;

static void limitusr();
static void carrierchk();

/* Main command line input. This function returns a pointer to the
next available argument, if none is available, it displays the prompt
and inputs a string from the console. */

char *getarg(char *prompt)
{
	char *cp;

	if (! isargs()) {			/* if line empty, */
		input(prompt,line_buff,sizeof(line_buff)); /* get some input, */
		_nargp= skip_delim(line_buff);	/* init the arg pointer */
	}
	cp= _nargp;				/* ptr to current arg */
	_nargp= next_arg(_nargp);		/* skip to the next one, */
	return(cp);
}
/* Return true if there are any args left. */

int isargs()
{

	return(num_args(_nargp));
}

/* Flush the command buffer */

void cmdflush()
{
	_nargp= line_buff;
	*_nargp= NUL;
}

/* Ask a question, return true if "Y" pressed, 0 if "N". */

int ask(char *s)
{
	char c,cp,buff[SS];

	strcpy(buff,s);
	strcat(buff,"? [Y,n]: ");

	while (1) {
		switch (tolower(*getarg(buff))) {
			case NUL: return(1);	/* CR is Yes */
			case 'y': return(1);	/* Y is Yes */
			case 'n': return(0);	/* N is No */
		}
		cmdflush();			/* else garbage */
	}
}

/* Ask a question, get a "hot" (single key) answer. Must ignore ^S and ^Q, 
as it may get things all out of sync. */

int ques(char *q)
{
	line= 0;			/* AVOID INFINITE RECURSION!!! */
	while (*q) mconout(*q++);	/* output directly, */

	while (1) {
		switch (mconin()) {
			case XOF:	/* ignore Control-S */
			case XON:	/* and Control-Q */
			case ACK:	/* Ignore Control-F */
				break;

			case 'n':
			case 'N':
			case ETX:
			case VT:
				return(0);
				break;

			case ' ':
			case 'y':
			case 'Y':
			case CR:
				return(1);
				break;

			default: mconout(BEL); break;
		}
	}
}

/* Issue a prompt, get a line of input. */

int input(char *prompt, char *buff, int len)
{
	char c;
	int n,i;

	mputs(prompt);				/* output the prompt, */
	i= 0;
	line= 0;				/* reset "more" */
	while (1) {
		c= mconin();			/* get a key, */
		switch (c) {
			case TSYNC:		/* KLUDGE */
				i= 0;		/* clear buffer ... */
				buff[i]= NUL;
				return(i);

			case CR:		/* end of input */
				buff[i]= NUL;
				mputs("\r\n");
				_abort= 0;
				return(i);

			case BS:		/* delete character */
			case DEL:
				n= 1; goto del;
			case CAN:		/* delete line */
			case ETX:
			case VT:
				n= i;
del:;				while (n && i) {
					mconout(BS);
					mconout(' ');
					mconout(BS);
					--n; --i;
				}
				break;

			case ACK: break;	/* ignore Control-F */

			default:		/* insert character */
				if ((c >= ' ') && (c <= '~') && (i < len-1)) {
					buff[i++]= c;
					mconout(c);

				} else mconout(BEL);
				break;
		}
	}
}

/* Output characters to the screen, building words and wrapping them
as necessary to fit within the current width. */

void fmconout(char c)
{
	if (c >= ' ') {				/* printable chars */
		word[wordi++]= c;		/* build a word */
		if ((wordi + MARGIN < caller.cols) && !delim(c)) return;
	}
	word[wordi]= NUL;			/* terminate it for output */

/* End of a word, word too large, or a control character. */

	if (column + wordi + MARGIN > caller.cols) {/* if word is too long, */
		mconout(CR);			/* go to next line first */
		mconout(LF);
	}
	if (*word) {				/* if there is a word there, */
		for (wordi= 0; word[wordi]; wordi++)
			mconout(word[wordi]);	/* output it */
		wordi= 0; 			/* now its gone */
	}
	if ((c < ' ') && (c != SUB)) mconout(c); /* do control chars */
}

/* Output a character to the console, and poll for ^C, etc. */

void mconout(char c)
{
	if (mconstat() == XOF) {		/* if a pause, */
		while (mconin() == XOF);	/* wait for anything but XOF */
	}

	modout(c);				/* to the modem, */
	if (c && (c != BEL)) lconout(c);	/* one to console if not bell */

	if (c >= ' ') ++column;			/* "cursor" motion */
	else if (c == CR) column= 0;
	else if (c == LF) {			/* next line */
		++line;
		if (line >= caller.lines - 1) {	/* screen pause, set abort */
			line= 0;		/* reset it, */
			if (caller.lines) {	/* if line pause enabled */
				mconflush();	/* flush typeahead, */
				_abort= ! ques(" <- More\r");	
			}
		}
	}
}

/* Wait for a character from the keyboard. Check for activity, log the guy 
off if he times out. */

#define ONEMIN (60L * 1000L)			/* milliseconds in one minute */
#define EIGHTMIN (ONEMIN * 8L)
#define TENMIN (ONEMIN * 10L)			/* ten minutes */

int mconin()
{
	FLAG warned;

	timer1_reset();
	warned= 0;				/* not warned yet */
	while (! mconstat()) {
		if ((timer1_get() > EIGHTMIN) && (!warned)) {
			mputs("\r\nPup says: \07\"I'll hang up on you if you sit idle too long!\"\r\n");
			++warned;
		}
		if (timer1_get() > TENMIN) {
			mputs("\r\nPup says: \"You were warned!\"\r\n");
			logoff(0,1);
		}
	}
	tail= ++tail % sizeof(conbuf);
	return( (int) conbuf[tail]);
}

/* Return 0 if no key available, else the key. */

int mconstat()
{
	int t;
	pollkbd();				/* poll for chars, */
	t= (tail + 1) % sizeof(conbuf);		/* wrap pointer, */
	return((t == head) ? 0 : conbuf[t]);	/* the char or 0 */
}

/* Poll both keyboards, stuff characters into the ring buffer. This handles
all background tasks, like type mode, etc. NOTE: This strips parity off
all incoming characters EXCEPT TSYNC! This lets us support Continuous Mail. */

void pollkbd()
{
	char c;

	limitusr();				/* maybe forced logoff */

	if (! test) {				/* if online, */
		carrierchk();			/* watch carrier! */
		if (_mconstat()) {		/* if modem character there */
			c= _mconin();		/* read it, */
			if (c != TSYNC) c &= 0x7f;
			put_c(c);		/* use it */
		}

	} else {
		c= keyhit();
		if (c) put_c(c);		/* else just local input */
	}
}

/* Put this character into the ring buffer, process the special
control characters as necessary. */

void put_c(char c)
{
	switch (c) {
		case ETX:			/* Control-C */
		case VT:			/* Control-K */
			_abort= 1;		/* flag as an abort, */
		case TSYNC:			/* Continuous Mail ... */
		case XOF: 			/* Control-S */
		case ACK:			/* Control-F */
			mconflush();		/* flush typeahead, */
			cmdflush();		/* flush command buffer, */
			break;

		case NUL: return;		/* ignore NULs */
	}
	if (head != tail) {			/* if room, install it */
		conbuf[head]= c;
		head= ++head % sizeof(conbuf);
	}
}

/* Flush the console buffer. */

void mconflush()
{

	while (mconstat()) mconin();		/* flush all characters, */
}

/* Formatted print to the modem. */

void mprintf(char *f, ...)
{
	char buf[500];

	_spr(buf,&f);
	mputs(buf);

}

/* Output a string to the modem. */

void mputs(char *s)
{
	while (*s) fmconout(*s++);
	fmconout(SUB);				/* flush the word buffer */
}

/* Watch the time limits set, and give warnings when the limit comes 
up. When the limit reaches 0, display a message and log the caller off. */

static void limitusr()
{
	int n;

	n= limit - minutes;			/* time left */

	if (limit == 0) {			/* if no limit, */
		return;				/* do nothing */

	} else if (n < 1) {			/* time is over */
		if (lmtstate < 3) {
			lmtstate= 3;		/* only once */
			limit= 0;		/* DISABLE LIMITS */
			mprintf("\r\n\r\n\07 Pup Says: \"Your time is up!\"\r\n\r\n");
			logoff(0,1);		/* disconnect */
		}

	} else if (n < 2) {			/* final PUP SAYS: */
		if (lmtstate < 2) {
			lmtstate= 2;		/* do only once */
			mprintf("\r\n\r\n\07Pup Says: \"I'm gonna hang up on you in 60 seconds!\"r\n\r\n");
		}

	} else if (n < 5) {			/* two minute PUP SAYS: */
		if (lmtstate < 1) {		/* if no PUP SAYS: yet, */
			lmtstate= 1;		/* remember this, BEFORE THE MSG */
			mprintf("\r\n\07Pup Says: \"You have %d minutes left!\"\r\n",n);
		}
	}
}

/* Check the time limit, and if over, bump the guy off. */

void limitchk()
{

	if (limit && (minutes >= limit)) logoff(0,1);
}
/* Output to the modem. */

void modout(char c)
{
	if (test) return;

	carrierchk();				/* always watch this */
	_mconout(c);				/* limit will get checked eventually */
}
/* Get a character from the modem, with a maximum wait time, in
centiseconds. Returns -1 if timeout. */

int modin(unsigned n)
{
	long dly;

	if (test) return(-1);		/* cant in test mode */

	if (_mconstat()) return(_mconin() & 0xff); /* why wait? */

	dly= n * 10L;			/* time, in milliseconds */

	timer2_reset();
	while (timer2_get() < dly) {
		carrierchk();		/* watch disconnect, */
		if (_mconstat()) 	/* if a char avail, return it. */
			return(_mconin() & 0xff);
	}
	return(-1);			/* timeout */
}

/* Delay N centiseconds. Do nothing in the mean time. */

void delay(int n)
{
	long dly;

	dly= n * 10L;
	timer1_reset();
	while (timer1_get() < dly);
}

/* Logoff if carrier is lost. */

static void carrierchk()
{

	if (! cd()) logoff(0,1);
}

/* Return true if carrier detect is true or "ignore CD" is true. */

int cd()
{

	if (cd_flag || test) return(1);	/* fake on-line */
	return(_cd());			/* real online */
}

/* Flush the modem until its quiet for N centiseconds or just flush
whatever is there if n == 0. */

void flush(int n)
{
	if (test) return;

	if (n) while (modin(n) != -1);
	else while (_mconstat()) _mconin();
}
