#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ascii.h"
#include "compat.h"
#include "driver.h"
#include "login.h"
#include "ms-asm.h"
#include "modemio.h"
#include "puppy.h"
#include "pupmem.h"
#include "quote.h"
#include "support.h"

/*
	Pups caller login process.

openclr()	Open the caller file. Must be done before any 
		login functions are called.

closeclr()	Close the caller file. This should be called when
		the caller is logged off or carrier is lost; if
		a caller was on, their record is updated. 

getclr(n)	Retrieve caller record #n. It is assumed that
		the record number is valid; no checks are done.

oldest(t)	Return the date of the oldest topic of those specified.

updclr(m,t)	Update the callers record to record the newest message
		read in the specified topic(s). There may be more
		than one topic selected, and a given message may belong
 		to more than one topic.

login()		Locate and sign in a caller, and set all the
		vital statistics. This sets limits in force,
		prompts for screen size for new callers, displays
		the welcome message, gives a quotation, etc.

stuff(f)	Various user type commands; display welcome file, 
		get screen width/length, etc. If f is zero, then
		the caller gets to see WELCOME.PUP regardless; other
		wise it asks.

*/

static int clrfile;		/* open caller file */
static int clrno;		/* currently open caller record or -1 if none */

static void getclr(int n);
static void posclr(int n);

void login()
{
	char full_name[SS];		/* callers full name */
	char *cp,buff[SS];		/* working space */
	int n,i;
	int hash;			/* hash code from the name */

	limit= pup.nlimit;
	klimit= pup.klimit;				/* set limits */

	caller.cols= 40; 
	caller.lines= 24;				/* std. screen */
	column= line= 0;				/* sync up "more" stuff, */

	mprintf("\r\nPup says: \"Hello!\"\r\n");
	while (1) {
		full_name[0]= NUL;			/* no name yet */
		cp= getarg("Your name please (short & simple): ");
		while (*cp) {
			cpyarg(buff,cp);		/* copy each word */
			strcat(full_name,buff);		/* to eliminiate excess */
			cp= next_arg(cp);		/* spaces and such */
			if (*cp) strcat(full_name," ");
		}
		if (! *full_name) continue;		/* must enter something */
		full_name[sizeof(caller.name)]= NUL;	/* maximum length pleez */
		if (ask(full_name)) break;		/* verify it */
	}


/* Now try to locate this caller in the file. */

	mprintf("Wait ...\r\n");			/* search for it */
	strcpy(buff,full_name);				/* make a clean copy */
	stolower(buff);					/* for comparision */
	cp= buff; clrcrc();				/* generate a hash code */
	while (*cp) updcrc(*cp++);
	hash= fincrc() % pup.callsize;			/* well distributed */

	for (i= 0; i < 10; i++) {			/* make a number of attempts */
		n= (hash + i) % pup.callsize;		/* fudge the hash code */
		getclr(n);				/* iteration */
		stolower(caller.name);			/* for comparision. */
		if (strcmp(caller.name,buff) == 0) {	/* if a match, */
			mprintf("Hello again\r\n");	/* case may have changed */
			i= 0;				/* mark as logged in */
			break;
		}
	}

/* If the login fails, this person was never on, or their record got hashed
out by someone else. Do it all again. New callers have a blank date; this
makes all messages newer than they are. */

	if (i) {
		cp= (char *) &caller;				/* clear the record */
		for (i= 0; i < sizeof(struct _clr); i++) *cp++= NUL;	
		caller.cols= 40; 
		caller.lines= 24;				/* std. screen */
		mprintf("\r\nUmm, excuse me. Would you mind coming with me. ");
		mprintf("We want to ask you a few questions. This will only ");
		mprintf("take a few minutes. (he he)\r\n\r\n");
		setscreen();
	}
	++caller.calls;
	strcpy(caller.name,full_name);				/* set the name, */
	clrno= hash;						/* login complete */
	dspfile("welcome.pup");
	stuff();						/* show other stuff */
	quote();						/* do a quotation */
	mprintf("\r\nYou are caller #%lu\r\n\r\n",++pup.callers);
}

/* Set screen stuff. */

void setscreen()
{
	char *cp;

	cp= getarg("How wide is your screen? (CR=80): ");
	if (*cp) caller.cols= atoi(cp);
	else caller.cols= 80;
	if ((caller.cols < 20) || (caller.cols > 80)) caller.cols= 40;

	cp= getarg("Pause after how many lines? (CR=24, 0=DONT): ");
	if (*cp) caller.lines= atoi(cp);	/* assume its a number, */
	else caller.lines= 24;			/* else CR is 24, */
	if ((caller.lines < 4) || (caller.lines > 66)) caller.lines= 0;
}
/* Display stuff. */

void stuff()
{
	int i;
	WORD time,date;

	date= gdate(); time= gtime();			/* get time, */
	mputs("It is now: "); mputs(str_date(date)); mputs(str_time(time)); mputs("\r\n");
	mprintf("You have been on for %d minutes, %d left\r\n",minutes,limit - minutes);

	mprintf("Topic    last read on\r\n");
	for (i= 0; i < 16; i++) {
		if (*pup.topic[i].name) {
			mprintf("%-8s ",pup.topic[i].name);
			if (caller.date[i]) mputs(str_date(caller.date[i]));
			else mputs("Never");
			mputs("\r\n");
		}
	}
}

/* Open the caller file. */

void openclr()
{

	clrfile= xopen2("PUPPY.CLR",2);
	if (clrfile == -1) printf("THE CALLER FILE IS MISSING!\r\n");
	clrno= -1;
}

/* Close the caller file. */

void closeclr()
{
	if (clrno) {
		posclr(clrno);				/* position there, */
		xwrite(clrfile,&caller,sizeof(struct _clr)); /* write it, */
	}
	if (clrfile != -1) xclose(clrfile);		/* close the file */
	clrfile= -1;
}

/* Get caller #n in the list. */

static void getclr(int n)
{
	posclr(n);
	xread(clrfile,&caller,sizeof(struct _clr));
}

/* Position to caller record. */

static void posclr(int n)
{
	long o;
	o= 0L + n;
	o *= sizeof(struct _clr);
	xseek(clrfile,o,0);
}

/* Return the oldest date of all the specified topics from the caller record. */

WORD oldest(WORD t)
{
	WORD d;
	int i;

	d= gdate();
	for (i= 0; i < BITSWORD; i++) {		/* for each possible topic, */
		if ((t & 1) && (caller.date[i] < d)) /* if this is older */
			d= caller.date[i];	/* remember it */
		t >>= 1;
	}
	return(d);
}

/* Update the dates in the caller record to reflect the most recently
read message. Since there may be more than one topic selected, and a
given message can belong to more than one message, this could possibly 
set more than one date. 

	message		selected topics		caller dates updated:
	1 2 4		1 2 3 4 5 6		1 2 4
*/


void updclr(int msgnbr, WORD t)
{
	WORD d;
	int i;

	t &= getmsg(msgnbr)-> topic;			/* topic(s) to update */
	d= getmsg(msgnbr)-> date;			/* messages date */

	for (i= 0; i < BITSWORD; i++) {			/* now check each */
		if ((t & 1) && (d > caller.date[i]))	/* if newer, */
			caller.date[i]= d;		/* remember it */
		t >>= 1;				/* next topic ... */
	}
}
