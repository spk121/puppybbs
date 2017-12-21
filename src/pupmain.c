#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#define open(FN,FLAG) (_open((FN),(FLAG)))
#define close(H) (_close((H)))
#define read(H,BUF,SIZ) (_read((H),(BUF),(SIZ)))
#define creat(FN,P) (_creat((FN),(P)))
#define write(H,BUF,SIZ) (_write((H),(BUF),(SIZ)))
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ascii.h"
#include "driver.h"		/* MSDOS */
#include "mdmfunc.h"
#include "ms-c.h"
#include "pup.h"
#include "puppy.h"
#include "sched.h"

struct _pup pup;		/* main system file */
struct _clr caller;		/* current caller logged in */

struct _msg *msg;	/* msg base index */

FLAG test;		/* 1 == test mode, (no modem) */
FLAG localin;		/* 1 == simultaneous keyboards */
BYTE lmtstate;		/* 0 - 2, caller time limit warning state */
int limit;		/* time limit in force, or 0 for no limit */
int klimit;		/* download limit in force */

/* Local system shit */

FLAG _abort;		/* True if ^C typed. */
FLAG doscode;		/* DOS error code */
char column;		/* column number, */
char line;
char months[13][4] = {	/* table of month names */
	"Eh?","Jan","Feb","Mar","Apr","May","Jun","Jul",
	"Aug","Sep","Oct","Nov","Dec"
};

/* XMODEM protocol module */

int totl_files;		/* how many failed, */
int totl_errors;	/* error count, soft errors incl */
int totl_blocks;	/* number blocks sent, */
char crcmode;		/* 1 if CRC mode, */
char filemode;		/* transfer type; XMODEM, MODEM7, TELINK */

/* Modem variables */

WORD linkrate;		/* baud rate to/from modem */
WORD datarate;		/* baud rate to/from caller */
FLAG cd_flag;		/* true == ignore CD line */

extern WORD cd_bit;	/* MSDOS driver: bit to test for Carrier Detect, */
extern WORD iodev;	/* MSDOS driver: serial channel number */	

/* Local text buffer */

char *text;		/* work buffer */
unsigned textsize;	/* and its size */

extern long sizmem();	/* MSDOS */
extern char *getmem();	/* MSDOS */

/*************************************************************

	These are DUMMIES so I can leave out the
	FidoNet code until it is working. 

**************************************************************/

get_pkt() {}		/* dummy */
put_pkt() {}		/* dummy */



int main(int argc, char **argv)
{
	int i,n;
	FLAG evtmsg;		/* 1 == we announced upcoming event */
	FLAG rdymsg;		/* 1 == we announced "waiting ... " */
	FLAG mdmmsg;		/* 1 == we initialized the modem */

	printf("Pup bulletin board, version 2a, 23 Dec 87\r\n");
	printf("Tom Jennings, 164 Shipley\r\n");
	printf("San Francisco CA 94107 USA\r\n");
	printf("(k) all rights reversed\r\n");
	test= 0;				/* not test mode */

	i= open("puppy.sys",0);			/* load the system file */
	if (i == -1) {
		printf("Can't find PUPPY.SYS\r\n");
		exit(1);
	}
	read(i,&pup,sizeof(struct _pup));	/* read it in, */
	close(i);

	iodev= pup.iodev;			/* MSDOS stuff the drivers */
	cd_bit= pup.cd_bit;			/* MSDOS with setup info */
	allmem();				/* MSDOS get all available memory */
	textsize= sizmem();			/* MSDOS how much mem we have */
	text= getmem(textsize);			/* MSDOS get it all, */

	i= pup.messages * sizeof(struct _msg);
	if (i > textsize) {			/* allocate room for msg index */
		printf("You have too many messages!\r\n");
		exit(1);
	}
	msg= (struct _msg *) text;		/* ptr to message file index */
	textsize -= i;				/* account for it, */
	text += i;				/* advance the pointer */

	if (pup.msgsize > textsize) {		/* room for message entry */
		printf("Message-size is too big!\r\n");
		exit(1);
	}

/* 
	*************** The Big Loop. **************
*/
	set_clk();				/* install clock */
	init();					/* start up hardware, */

	evtmsg= 0;				/* no event warning yet */
	rdymsg= 0;				/* no ready message yet */
	mdmmsg= 0;				/* no modem init */

	while (doscode == 0) {			/* exit-to-DOS code */

		switch (keyhit()) {		/* poll the keyboard */
			case '?':
				printf("\"L\"    Login to Pup\r\n");
				printf("\"I\"    Init the modem\r\n");
				printf("^C     Return to DOS\r\n");
				rdymsg= 0;
				break;
				
			case ETX: doscode= 1; break;
			case 'i': case 'I': 	/* Init modem */
				mdmmsg= 0;
				rdymsg= 0;
				break;

			case 'l': case 'L':	/* Local Login */
				test= 1;	/* do test mode */
				puppy();	/* take the pup for a WOC */
				test= 0;
				rdymsg= 0;
				break;
		}
		if (doscode) break;

		i= til_sched('?',0 /*,0 */);		/* ask the scheduler whats up */
		if (i != -1) {			/* if an event NOW */
			if (pup.sched[i].tag == 'X') {
				doscode= pup.sched[i].len;
				markevt(i);	/* flag it as run */
				printf("Pup says: \"Event #%d X ERRORLEVEL %d\"\r\n",i,doscode);
				break;		/* terminate Big Loop */
			}
			printf("Pup says: \"FidoNet %c\"\r\n",pup.sched[i].tag);
			evtmsg= 0;		/* need messages */
			rdymsg= 0;
			mdmmsg= 0;
			continue;		/* check for more events, etc */
		}
		i= til_sched('?',10 /*,0 */);		/* ask the scheduler whats up */
		if (i != -1) {			/* if an event within 10 minutes */
			if (! evtmsg) printf("Pup says: \"Event within 10 minutes\"\r\n");
			evtmsg= 1;
			continue;		/* no modem/caller stuff */
		}
		if (! mdmmsg) {
			mdmmsg= 1;
			rdymsg= 0;
			init_modem(pup.mdmstr);	/* wake up the modem */
		}
		if (! rdymsg) printf("Pup says: \"Waiting for something to do (?=help)\"\r\n");
		rdymsg= 1;

/* OK, no events etc. If the phone rings, answer it. If TSYNC is received 
during the signon process, it drops into Incoming Mail. */

		if (answer() > 0) {		/* if an incoming call, */
			puppy();
			rdymsg= 0;
		}
	}

	reset_clk();				/* turn off clock, */
	if (! test) uninit();			/* always ints off, etc */
	exit(doscode);				/* back to DOS */
}
