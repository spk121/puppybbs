/*
	Puppy's include file
	T. Jennings 25 Jan 88

"Fido" is a trademark of Tom Jennings. It you utter it send me a dollar.
"FidoNet" is a also registered trademark of Tom Jennings. If you even think 
it send me two dollars. If you use both, send me ten dollars and your first
born child. All rights reserved. So there.


Puppy (k) All Rights Reversed

	Fido Software
	164 Shipley
	San Francisco CA 94107
	(415)-764-1629, FidoSW BBS
	(415)-882-9835, ch@os, one day will be known as 1:125/164
*/

/* These are my assumptions as to data element sizes for the source here. You
will need to change these if the following assumptions arent right:

char			8 bits or more, signed or not
int 			more than 8 bits, though 8 will work 99% of the time
			(that last 1% if left as an exercise for the programmer)

long			more than 16 bits, preferably 32. Note as above

FLAG			at least one bit long; set to 0 or 1, and tested
			for != 0
BYTE			8 bits long. You can define as necessary.
WORD			16 bits. Ditto
LONG			32 bits. Ditto Ditto

BYTE, WORD and LONG are used for two reasons: to take advantage of the word
length in modulo arithmetic (ie. XMODEM block numbers, 0...255) or because
they are an interface to other code, ie. WORD baud rates passed to the 
drivers. Changing these assumptions may not be trivial, since they are
frequently buried into the algorithms. (Check out XMODEM.C for a classic
example of this.) */

#define FLAG char	/* just a boolean */
#define BYTE char	/* Lattice chars are 8 bit */
#define WORD unsigned	/* Lattice unsigneds are 16 bits */
#define LONG long	/* Lattice ... */

#define BITSWORD 16	/* bits per WORD, above */

/* 
Time and date are stored compressed in a 16 bit integer each. This has
obvious advantages for storage, but also for comparison; time and date
may be treated as integers; less than, greater than, etc. This is also 
the MSDOS internal storage format.

TIME:	h h h h h m m m m m m s s s s s

	h = (time >> 11) & 0x1f
	m = (time >> 5) & 0x3f
	s = (time < 1) & 0x3f

NOTE: seconds has a resolution of 2 seconds, since there are only 5 bits
available to store it.


DATE:	0 y y y y y y m m m m d d d d d

	y = (date >> 9) & 0x3f
	m = (date >> 5) & 0x0f
	d = date & 0x1f
*/



/* Absolute Truths (current version ...) */

#define SS 80				/* standard, universal, string size */
#define TSYNC 0xae			/* FidoNet sync character */

/* Fancy numbers for protocol module. */

#define TELINK 1			/* extended nightmare */
#define MODEM7 2			/* modemers nightmare */
#define XMODEM 3			/* Volkswagen */
#define KERMIT 4			/* disgusto blob */
#define FIDONET 5			/* XMODEM with diverter */
#define IFNA 6				/* design by default */
#define SEALINK 7			/* trademark SEA Associates */

#define NULL 0				/* nothing */

#define MINS_HR	60			/* minutes in an hour */
#define DAYS_WK 7			/* days in a week */
#define MINS_DAY (24 * 60)		/* minutes in an hour */
#define MINS_WK (MINS_DAY * DAYS_WK)

/* Standard node structure used throughout Fido. This is buried within all
the other major structures. */

struct _node {
	int zone;		/* zone number */
	int net;		/* net number */
	int number;		/* node number */
};

/*
The date is stored in a single WORD as:

	0 y y y y y y m m m m d d d d d
*/

/* These are all in the "library", MS-C.C and MS-ASM.ASM. */

char *str_node();
char *skip_delim();
char *next_arg();
char *strip_path();
char *getarg();

/* These are in the msg base mostly */

struct _msg *getmsg();
struct _msg *newmsg();

/* These are elsewhere */

long lseek();
char *getmem();


/* OK, OK already, lets start a caller file. I hate caller files. */

struct _clr {
	char name[36];			/* the miserable SOBs name, */
	WORD date[BITSWORD];		/* newest read for each topic */
	WORD topic;			/* last topic(s) selected */
	char lines;			/* number of lines, */
	char cols;			/* number of columns, */
	int calls;			/* sigh ... how many times, */
	int extra;
};

/* Message header structure: the headers are contained in a single file;
the message bodies are contained in another. The headers are kept in
memory at all times. */

struct _msg {
	char from[36];		/* who from, */
	char to[36];		/* who to, */
	char subj[36];		/* message subject, */
	WORD date;		/* message creation date, */
	WORD time;		/* message creation time, */
	WORD extra;		/* unused space */
	WORD attr;		/* attribute bits (see below) */
	WORD topic;		/* topic selection(s) */
	WORD topic_map;		/* shared topics */
};

/* Message attribute bits */

#define MSGEXISTS 1		/* message slot occupied */
#define MSGREAD 2		/* read by addressee */
#define MSGSENT 4		/* sent OK (remote) */
#define MSGTAG 8		/* general purpose tag bit */		    


#define SCHEDS 35		/* size of scheduler event table */

/* Scheduler event structure within PUP.SYS. Tags defined:

A - W		FIDONET events
X		ERRORLEVEL event
all others	RESERVED
*/

struct _sched {
	char bits;		/* see below */
	char tag;		/* event type, above, */
	char hr;		/*   hour, */
	char min;		/*   minute, */
	int len;		/* event length, or ERRORLEVEL for tag X */
};

#define SCHED_OPTIONAL 1	/* this event may be skipped */
#define SCHED_COMPLETE 2	/* this event already run */

/* PUPPY.SYS: This is where Puppy keeps its shit together. */

struct _pup {
	long callers;		/* number of callers to the system */
	long quote_pos;		/* quote file position */
	struct _node id;	/* our node ID */
	int nlimit;		/* normal callers limit, */
	int klimit;		/* K byte limit, */
	int top;		/* current top of the pile */
	unsigned msgnbr;	/* current highest message number */

	unsigned callsize;	/* maximum number of callers in caller file*/
	int messages;		/* total messages allowed */
	int msgsize;		/* size of each message body record */
	struct {
		char name[8];	/* topic name, */
		char desc[24];	/* its description */
	} topic[BITSWORD];	/* 16 of 'em */

	int maxbaud;		/* maximum baud rate */
	char mdmstr[SS];	/* modem initialization string */
	WORD cd_bit;		/* bit to test for Carrier Detect, */
	int iodev;		/* default serial channel number */	

	int tries;		/* FidoNet dial attempts w/o connects */
	int connects;		/* FidoNet dial attempts w/ connects */

	struct _sched sched[SCHEDS]; /* the schedulers event table */

	char filepref[SS];	/* file upload & download prefix */
};


/* This is the structure used to make getinfo() calls, and both returns
the desired information and stores information needed by the getinfo()
function itself between interations. The first half is the interface part;
Pup code depends on those fields being there. The lower half is used by
the getinfo() function itself. Change as you see fit. */

struct _fileinfo {

	char name[13];		/* REQUIRED filename (no path, drive, etc) */
	long size;		/* REQUIRED file size */
	WORD date;		/* REQUIRED creation date (may be 0's) */
	WORD time;		/* REQUIRED creation time (may be 0's) */

/* This is the block of shit that getinfo() uses internally. */

	struct {
		char s_attrib;	/* Search attribute */
		char x[21];
		WORD time;	/* MSDOS packed time */
		WORD date;	/* MSDOS packed date */
		long fsize;	/* MSDOS file size */
		char name[13];	/* MSDOS packed name */
	} xfbuf;
};

/* The topic map is the list that ties topics to the outside world. */

struct _tm {
	struct _node node;	/* the node number, */
	WORD topic;		/* topic(s) this node sees, */
	WORD date;		/* last time updated */
	WORD time;
	unsigned bits;		/* attributes, see below */

	char tries;		/* number of tries to call, */
	char connects;		/* number of times it connected */
	unsigned connect_time;	/* total connect time */

	char phone[SS];		/* ALPHA phone number */
	unsigned baud;		/* ALPHA baud rate */
};

#define TM_HOLD 1		/* hold for pickup */

/* Message packet header. The "illegal" (ie. not in FSC001) definitions
are in the interim Packet Type 2. */

#define PKTVER 2		/* v10 = 1, v11 = 2, v12 = ... */

/* PUBLIC */
struct _pkthdr {
	int orig_number;	/* v9 originating Node # */
	int dest_number;	/* v9 destination node */
	int year,month,day,hour,minute,second;
	int rate;		/* v9 OBSO RESERVED baud rate */
	int ver;		/* packet version */

	int orig_net;		/* v11 originating net number */
	int dest_net;		/* v11 destination net number */

	char product1;		/* v11 product type */
	char product2;		/* v11 extra byte */
	char pwd[8];		/* password */

	int orig_zone;		/* originating zone */
	int dest_zone;		/* destination zone */

	char extra[16];		/* extra bytes */
	long prod;		/* product dependent */
};
