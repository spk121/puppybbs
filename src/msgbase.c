#include <string.h>
#include <stdio.h>
#include "ascii.h"
#include "compat.h"
#include "msgbase.h"
#include "modemio.h"
#include "puppy.h"
#include "pupmem.h"
#include "support.h"

/*
	Message Base functions

	Tom Jennings
	20 Sept 87


This is the message base (its too crude to be called a "database") for
Puppy. 

To the user, it appears to be a Pile of messages of fixed length. You
start at the top of the pile, and read messages one by one until you
hit the bottom. When a new message is enter, it goes on the Top of the
pile and the oldest one falls off the bottom.

Physically this is of course a linear file, and is implemented as a
"ring buffer". There is a "place keeper" counter called the "top". 
Initially, the message base is empty, and the top is the beginning
of the file. 

It is the nature of this ring buffer that since the "top" points to 
the newest record, (top - 1) is therefore the oldest record. This is
the feature the whole thing is based on. (top - 1) is called the "bottom".

(When Top excrements beyond the end or beginning of the physical file,
it is wrapped to the opposite end as you might expect.)

When a new message is entered, it is stored at the bottom, ie. in the
oldest record. The top is then decremented so that it points to the
new record; this was the oldest, and now is the newest. The previous
newest is now 2nd newest, etc.


The interface to this module is the "message number". Message numbers
run 1 (newest, Top of the pile) to N (oldest, bottom of the pile.)


Internally of course we gotta deal in records, which get converted
into a byte file position. Conversion from MessageNumber to Record is:

	recd= (MessageNumber + Top) % pup.Messages

*/

int msgfile;		/* message index file */
int txtfile;		/* message text file */

/*

openmsg()	Open/start the message base.
closemsg()	Close the message base.

Frequently used message base primitives

recd(n)		Converts message number into record number.
getmsg(n)	Returns a pointer to message header #n.
ismsg(n,t)	Returns true if the specified message number exists and
		matches the selected topics.
alltopics()	Returns all possible message topic bits.

Various message characteristic routines

oldest_msg()	Returns the oldest (highest) message number.
newest_msg()	Returns the newest (lowest) message number.
newmsg()	Returns a pointer to the next free message
		header structure.
killmsg(n)	Marks a message as deleted.

Message data manipulation routines

savemsg(lines)	Add the message in memory to the file.
loadmsg(n)	Load message #n into the text buffer.

Message display routines

topics(t)	List the topics specified in the bit field.
listhdr(n)	Displays the contents of message header #N.
listmsg(n)	Displays the contents of the message body.
*/

static void killmsg(int n);
static void writehdr(int n);

/* Open the message base; open both files, load the index. */

void openmsg()
{
	msgfile= xopen2("PUPPY.IDX",2);		/* open the msg base */
	txtfile= xopen2("PUPPY.DAT",2);
	if ((msgfile < 0) || (txtfile < 0)) {
		printf("Message base file(s) missing!\r\n");

	} else xread(msgfile,msg,pup.messages * sizeof(struct _msg));
}

/* Close the message base. */

void closemsg()
{

	if (msgfile != -1) xclose(msgfile);
	if (txtfile != -1) xclose(txtfile);
	msgfile= txtfile= -1;
}

/* Expand the topics bit field into topic names. Handle the special
case NONE. */

void topics(WORD t)
{
	int i,n;

	n= 0;
	for (i= 0; i < 16; i++) {
		if ((t & (1 << i)) && *pup.topic[i].name) {
			mprintf(" %s",pup.topic[i].name);
			++n;
		}
	}
	if (! n) mputs(" NONE");
}

/* List message header #n. */

void listhdr(int n)
{
	struct _msg *t;
	char *sep;

	sep= ", ";					/* assume wide screen */
	if (caller.cols < 80) sep= "\r\n";		/* if not, break into 5 lines */
	t= getmsg(n);					/* ptr to this message */

	mputs(str_date(t-> date)); mputs(sep); 
	mputs("TOPIC:"); topics(t-> topic); mputs("\r\n");
	mprintf("FROM: %s%s",t-> from,sep);		/* FROM name, space or CR */
	mprintf("TO: %s%s",t-> to,sep);			/* TO name, space or CR */
	mprintf("TITLE: %s\r\n",t-> subj);
}

/* Display the body of a message. This just seeks to the right
place in the file and dumps the text. */

void listmsg(int n)
{
	long l;

	l= 0L + pup.msgsize;
	l *= (long) recd(n);
	xseek(txtfile,l,0);
	dumptext(txtfile);
}

/* ---------------------------------------------------------------- */

/* Return true if this message number exists and is part of 
the selected topic(s). */

int ismsg(int n, int t)
{
	if (n > oldest_msg()) return(0);	/* too high! */
	if (n < newest_msg()) return(0);	/* too low! */

	return(getmsg(n)-> topic & t);		/* and topics match */
}

/* Mark this message as deleted. */

static void killmsg(int n)
{
	if (ismsg(n,alltopics())) getmsg(n)-> topic= 0;
}

/* Return all possible topic bits set. */

WORD alltopics()
{
	int i;
	WORD t;
	
	for (t= i= 0; i < BITSWORD; i++) 
		if (*pup.topic[i].name) t |= (1 << i);
	return(t);
}

/* Convert message number (1 - N) into record number (0 - N). */

int recd(int n)
{
	int i;
	i= ((n - 1 + pup.top) % pup.messages);
	return(i);
}

/* Return the oldest (highest) message number. (They run 1 - N) */

int oldest_msg()
{

	return(pup.messages);
}

/* Return the newest message number. This is always the logical top
of the pile, or 1. */

int newest_msg()
{

	return(1);
}

/* Return a pointer to message header #n. */

struct _msg *getmsg(int n)
{
	return(&msg[recd(n)]);
}

/* Return a pointer to the next free message header. */

struct _msg *newmsg() 
{
	return(getmsg(oldest_msg()));
}

/* Add this message to the message file. Since the message header is
created in the bottom message header in the array, advancing it is the
same as saving it. The message body must be written out however. Since 
the file is preallocated, we dont have to check for write errors. (right) */

void savemsg(int lines)
{
	int n,i;
	char *cp;
	long o;

	if (--pup.top < 0) 
		pup.top= pup.messages - 1;	/* next message ... */
	++pup.msgnbr;				/* another message ... */

	msg[pup.top].topic_map= 0;		/* clear the map */
	writehdr(pup.top);			/* write out the msg header, */

/* We must write out exactly (msgsize) bytes. */

	o= 0L + pup.top;
	o *= pup.msgsize;
	xseek(txtfile,o,0);
	for (n= i= 0; i < lines; i++) {
		cp= &text[i * caller.cols];	/* write each line of text */
		xwrite(txtfile,cp,strlen(cp));
		n += strlen(cp);		/* bozo check: dont exceed max */
		if (n >= pup.msgsize - 1) break;/* plus room for a terminator */
	}

	n= pup.msgsize - n;			/* amt we need to fill out msg */
	cp= text; for (i= n; i--;) *cp++= SUB;	/* fill with ^Zs */
	xwrite(txtfile,text,n);			/* pad it out */
}

/* Write out the specified message header record. */

static void writehdr(int n)
{
	long o;
	o= 0L + n;
	o *= sizeof(struct _msg);
	xseek(msgfile,o,0);
	xwrite(msgfile,&msg[n],sizeof(struct _msg));
}

/* Load message body #n into the text buffer. */

void loadmsg(int n)
{
	long o;

	o= 0L + recd(n);
	o *= pup.msgsize;
	xseek(txtfile,o,0);
	xread(txtfile,text,pup.msgsize);
}
