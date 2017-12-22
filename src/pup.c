#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <ctype.h>
#include <string.h>
#include "abort.h"
#include "ascii.h"
#include "driver.h"
#include "edit.h"
#include "files.h"
#include "login.h"
#include "modemio.h"
#include "ms-asm.h"
#include "ms-c.h"
#include "msgbase.h"
#include "printf.h"
#include "puppy.h"
#include "pupmem.h"
#include "support.h"

/*
	Puppy's Main Section. 

This handles one caller then returns. This source contains the
command prompt and the message base commands below it.

*/

static int msgnbr;		/* current message number */
static WORD last_topic;		/* topic from last msg read */
static int topic;		/* currently selected topic(s) */

static void goodbye();
static void msg_base();
static void pickmsg();
static WORD getdate(char *cp);
static int ismonth(char *cp);
static void getnext(WORD date);
static int nxtnbr(int direction);
static int gettopic();
static int mfind();
static int msend();
static void msgline(int n);
static int getlno(char *ps, int lim);

int puppy()
{
	char *cp;

	clr_clk();				/* start counting */
	lmtstate= 0;				/* initialize the warning msg */

/* The set_abort(), was_abort() frc_abort() trio are the mechanism that
takes care of the problem of inline carrier checks sprinkled throughout 
the code. They are all contained in ABORT.ASM. It's really quite simple; it
relies on the fact that as you call functions deeper and deeper, the "top
level" (here) is safely stored on the stack somewhere. 

What set_abort does is just remember the stack pointer at this point, and
clears a flag private to the ???_abort() functions. If the flag is clear
(it is, set_abort cleared it) then was_abort() returns FALSE. So the first
time through, the code sequence here is set_abort(), was_abort(), openmsg(),
etc. 

The I/O drivers call frc_abort() if carrier is lost or a time limit expires.
(Well, it calls logoff() which does) What frc_abort() does is restore the
saved stack pointer (losing the one called from) and set the flag, and 
just "returns". Where it goes to though is where the first set_abort() 
call returned from!

This time though the flag is SET (remember that?) so was_abort() takes
its TRUE branch, and Pup cleans up. */

	set_abort(0);				/* set carrier loss trap */
	if (was_abort()) {			/* if we get a frc_abort() */
		printf("\r\n\r\n");		/* close up */
		closemsg();			/* close the message base, */
		closeclr();			/* close the caller file */
		putsys();			/* write system file */
		close_up();			/* close aborted files */
		return(0);			/* not TSYNC */
	}
	openmsg();				/* open message base, */
	openclr();				/* open the caller file */
	mconflush();				/* flush interrupt buffers */
	cmdflush();				/* flush command buffers */

	login();				/* login some user */

/* Now execute commands forever. */

	msgnbr= oldest_msg();			/* first time through */
	topic= 0;

	while (1) {
		cp= getarg("Command: M)essages H)ello! G)oodbye! D)ownload U)pload (?=help): ");
		switch (tolower(*cp)) {
			case 'm': msg_base(); break;
			case 'h': setscreen();		/* LOGIN.C */
				if (ask("Want to see the \"Hello\" display again")) dspfile("welcome.pup");
				stuff(); break;		/* LOGIN.C */
			case 'd': download(); break;	/* FILES.C */
			case 'u': upload(); break;	/* FILES.C */
			case 'g': goodbye(); break;
			case '?': dspfile("main.hlp"); break;
			case NUL: break;
			default:
				mprintf("eh?\r\n");
				cmdflush();
				break;
		}
	}
}

/* Me, I disconnect from you. */

static void goodbye()
{
	if (ask("Want to disconnect now")) logoff(0,1);
}

/*
	Message Base commands

*/

static void msg_base()
{
	char *cp;
	FLAG again;
	int n;

	gettopic();					/* select initial topic, */

	while (1) {
		mputs("\r\n");
		if (ismsg(msgnbr,topic)) {		/* (msgnbr set elsewere) */
			last_topic= getmsg(msgnbr)-> topic;
			listhdr(msgnbr);		/* display current message */
			n= line;			/* remember current line */
			again= ques("Read it? ");	/* ask the question, */
			mputs("\r");
			if (again) {			/* if it was Yes, */
				line= n;		/* keep header on screen */
				listmsg(msgnbr);	/* read it */
				updclr(msgnbr,topic);	/* update newest read */
			}
		}
		do {
			again= 0;			/* command error flag */
			cp= getarg("E)nter P)revious J)ump! G)oodbye! T)opic Q)uit (?=help CR=Next-Msg): ");
			switch (tolower(*cp)) {
				case 't': gettopic(); break;
				case 'j': pickmsg(); break;
				case NUL: nxtnbr(1); break;
				case 'p': nxtnbr(-1); break;
				case 'q': return;

				case 'g': again= 1; goodbye(); break;
				case '?': again= 1; dspfile("message.hlp"); break;
				case 'e': again= 1; msend(); break;
				default: again= 1; mprintf("eh?\r\n"); break;
			}
		} while (again);
	}
}

/* Pick a place to start reading messages. OK, now we might get anything as 
input, since there's no agreeing on the "right" way to enter dates. This 
mess below will accept "12 dec", "dec 12", "dec" (means 1 Dec) "12 dec 87", 
"dec 12 87" or "dec" when this is Jan; it assumes you mean Dec. of the 
previous year. */

static void pickmsg()
{
	int i;
	char *cp,buff[SS];
	WORD w;

	cp= getarg("Jump to message: T)oday U)nread S)earch, or enter a date: ");

/* This isnt pretty, but it means we can use any single letter as a 
command name, even if it is the first letter of a month name. */

	if (ismonth(cp)) goto manual;

	switch (tolower(*cp)) {
		case NUL: return;			/* blank line */
		case 's': mfind(); break;		/* text search */
		case 't': w= gdate(); break;		/* from today */
		case 'u': w= oldest(topic); break;	/* from last call */
manual:;	default: w= getdate(cp); break;		/* get a date, */
	}
	if (w) getnext(w); 				/* duh, do it */

/* This is due to a design defeciency in the command line code; no way to
tell it "I took (n) args". So we'll take 'em all! */

	cmdflush();
}

/* Convert the passed string into a date in a standard time WORD;
return 0 if it failed. Fill in date components not specified with
"today", ie year, month. */

static WORD getdate(char *cp)
{
	int i;
	WORD now;
	char day,month,year;

	day= atoi(cp);				/* maybe day is first */
	if (day) cp= next_arg(cp);		/* if so month follows */
	month= ismonth(cp);			/* does it? */
	if (! month) return(0);			/* sorry */

	year= 0;				/* no year specified */
	cp= next_arg(cp);			/* if a number follows */
	i= atoi(cp);				/* its the day or year */
	if (i > 1900) year= i - 1900;		/* its year, obviously */
	else if (!day && (i < 32)) day= i;	/* if no day yet assume that */
	else year= i;				/* damfino */
	i= atoi(next_arg(cp));			/* if another follows */
	if (i) year= i;				/* gotta be the year */

/* If the specified month is after this month, then assume its the
previous year thats wanted. Otherwise the fool doesnt know what month it 
is, or they are calling from beyond the International Date Line, which is
their problem. */

	if (! year) {				/* if no year specified */
		now= gdate();
		year= ((now >> 9) & 0x3f) + 80;	/* default to this year */
		if (month > ((now >> 5) & 0x0f)) --year; /* make it last year */
	}
	if (! day) day= 1;			/* in case no day specified */

	return(((year - 80) << 9) | (month << 5) | day);
}

/* Return the number of the month (1 - 12) if this is a month name, or 0 if
its not a month name. */

static int ismonth(char *cp)
{
	char m[SS];
	int i;

	cpyarg(m,cp);				/* a clean copy, */
	m[3]= NUL;				/* first three chars only */
	stolower(m); m[0]= toupper(m[0]);	/* lower case first char upper */
	for (i= 1; i < 13; i++) {		/* 0 is "Eh?" */
		if (strcmp(m,months[i]) == 0) return(i);
	}
	return(0);				/* not a month */
}

/* Locate the first message since the passed time & date. */

static void getnext(WORD date)
{
	int n;

	n= msgnbr= oldest_msg();		/* start at the bottom, */

/* If the message was created on or after the callers last login ... break */

	while (1) {
		if (getmsg(msgnbr)-> date >= date) break;
		if (! nxtnbr(1)) break;		/* try the next one */
		n= msgnbr;			/* remember last good one */
	}
	msgnbr= n;
}

/* Pick the next message number that exists and is in the right topic.
This sets & returns 0 if we hit the top or the bottom. */

static int nxtnbr(int direction)
{

	while (1) {
		if (msgnbr > oldest_msg()) {	/* at the bottom? */
			msgnbr= oldest_msg();
			mprintf("\r\nOLDEST MESSAGE\r\n");
			return(0);

		} 
		if (msgnbr < newest_msg()) {	/* hit the top? */
			msgnbr= newest_msg();
			mprintf("\r\nNEWEST MESSAGE\r\n");
			return(0);
		}
		msgnbr -= direction;		/* next number, */
		if (ismsg(msgnbr,topic)) return(msgnbr);
	}
}

/* Make the user select a topic; return the bit fields set. */

static int gettopic()
{
	int n;
	char *cp;

	while (1) {
		if (! isargs()) {
			mputs("Choose one or more TOPIC numbers:\r\n");
			for (n= 0; n < 16; n++) {	/* list them all, */
				if (*pup.topic[n].name) 
					//mprintf("#%d %-8s %s\r\n",n + 1,pup.topic[n].name,pup.topic[n].desc);
					printf("#%d %-8s %s\r\n",n + 1,pup.topic[n].name,pup.topic[n].desc);
			}
			mprintf("#A          Choose ALL topics\r\n");
		}
		topic= 0;				/* start with this */
		cp= getarg("Choose topic(s) #");	/* ask for them, */
		while (*cp) {				/* check each one */
			if (tolower(*cp) == 'a') {	/* if ALL */
				topic= alltopics();	/* select all */

			} else {			/* must be a number */
				n= atoi(cp);
				if ((n > 0) && (n < 16)) { /* if in range, */
					--n;
					if (*pup.topic[n].name) 
						topic |= (1 << n);
				}
			}
			cp= next_arg(cp);		/* ptr to next arg (if any) */
		}
		cmdflush();				/* pitch input line we just ate */
		if (topic) break;
	}	
	last_topic= topic;				/* change of topics */
	getnext(oldest(topic));				/* select the next to read */
	return(topic);					/* return 'em when we get 'em */
}

/* Search the message base for content. Return true if the thing was found. */

static int mfind()
{
	char pattern[SS];
	struct _msg *m;

	while (1) {
		cpyarg(pattern,getarg("Search for what? (CR=Quit, ?=Help): "));
		if (*pattern == '?') dspfile("find.hlp"); /* ? is help */
		else if (! *pattern) return(0);		/* just a CR */
		else break;				/* anything else */
	}
	mputs("Wait ... (Control-C to abort)\r\n");
	while (nxtnbr(1)) {				/* find a message, */
		pollkbd();				/* check the keyboard */
		if (_abort) break;			/* allow stopping */
		m= getmsg(msgnbr);			/* get this msg */
		if (strfnd(m-> to,pattern)) return(1);	/* check each field */
		if (strfnd(m-> from,pattern)) return(1);
		if (strfnd(m-> subj,pattern)) return(1);
		loadmsg(msgnbr);			/* get the msg body, */
		if (strfnd(text,pattern)) return(1);	/* search that */
	}
	return(0);
}

/* Edit-message commands. Returns true if the message should be saved. */

static int msend()
{
	int lineno;		/* last used line number */
	char *cp;
	int i;
	struct _msg *msg;	/* ptr to new message */

	msg= newmsg();				/* ptr to our new message */
	msg-> date= gdate();			/* get the current time, */
	msg-> time= gtime();
	strcpy(msg-> from,caller.name);		/* set From: */
	mprintf("FROM: %s\r\n",caller.name);
	input("TO: ",msg-> to,sizeof(msg-> to));
	if (! *msg-> to) return(0);
	msg-> topic= last_topic;		/* same as previous topic */
	if (! msg-> topic) msg-> topic= topic;	/* or a new one */
	mputs("TOPIC:"); topics(msg-> topic); mputs("\r\n");
	input("TITLE: ",msg-> subj,sizeof(msg-> subj));
	if (! *msg-> subj) return(0);

	cp= text; i= pup.msgsize;		/* clear out the text buffer */
	while (i--) *cp++= NUL;

	mprintf("Enter your message,\r\nControl-C or Control-K to end\r\n");
	lineno= edit(0);			/* get some new text */

	while (1) {
		cp= "\r\nEdit: C)ancel A)dd L)ist I)ns D)el S)ave (?=help): ";
		cp= getarg(cp);
		switch (tolower(*cp)) { 
			case '?': dspfile("edit.hlp"); break;
			case NUL: break;
			case 'c': if (ask("Cancel this message")) return(0); break;
			case 'a': lineno= edit(lineno); break;
			case 'l': listhdr(oldest_msg());
				for (i= 0; i < lineno; i++) {
					msgline(i);
					if (_abort) break;
				}
				break;

			case 'i':
				i= getlno("Insert before line #",lineno);
				if (i < 0) break;
				lineno= edit(i);
				break;

			case 'd':
				i= getlno("Delete line #",lineno);
				if (i < 0) break;
				lineno= del_line(i,lineno);
				break;

			case 's':
				mprintf("Saving new message\r\n");
				savemsg(lineno);
				++msgnbr;		/* new top means all shift down */
				return(1);

			default:
				mprintf("eh?\r\n");
				cmdflush();
				break;
		}
	} 
}

/* Display a message line. */

static void msgline(int n)
{
	char *s;

	s= &text[n * caller.cols];		/* s == ptr to line to display */
	mprintf("%2u: ",n + 1);			/* display line number, */
	while (*s && !_abort) {			/* until done or Control-K */
		if (*s >= ' ') mconout(*s);	/* output them */
		++s;
	}
	mputs("\r\n");
}

/* Get a line number; return -1 if invalid number. */

static int getlno(char *ps, int lim)
{
	char *cp;
	int n;

	n= atoi(getarg(ps)) - 1;		/* get one, (garbage becomes -1) */
	if (n >= lim) {
		mprintf("Must be 1 to %d\r\n",lim);
		n= -1;
	}
	return(n);
}
