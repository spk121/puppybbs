#include <puppy.h>
#include <ascii.h>

/* Structure for find first/next (_find()).
NOTE: the DPB pointer is a double word 
character pointer; here it is declared as a
long, to avoid problems with the various
size of char *'s.

   This must be compiled with -B. */

struct _xfbuf {
 char s_attrib;		/* Search attribute */

/* These do not need to be initialized */

 char drive;		/* 0 == current, 1 == A: ... */
 char fcbname[11];	/* FCB name, */
 unsigned ent;
 long dpb_ptr;
 unsigned dirstart;

/* These are returned by DOS */

 char f_attrib;		/* found attribute, */
 unsigned time;		/* Packed time, */
 unsigned date;		/* Packed date */
 long fsize;		/* file size, */
 char name[13];		/* found name */
};

/* Message header structure. The message text is just a long string. The
zone:net/node of course do not conform to the new (v12) standard. */

/* PUBLIC */
struct _omsg {
	char from[36];		/* who from, */
	char to[36];		/* who to, */
	char subj[72];		/* message subject, */
	char date[20];		/* creation date, */
	int times;		/* number of times read, */
	int dest_node;		/* destination node, */
	int orig_node;		/* originating node */
	int cost;		/* actual cost of this msg */
	int orig_net;		/* v10 originating net */
	int dest_net;		/* v10 destination net */
	int dest_zone;		/* v12 originating zone */
	int orig_zone;		/* v12 destination zone */
	int dest_point;
	int orig_point;
	unsigned reply;		/* thread to previous msg. (reply-to) */
	unsigned attr;		/* message type, see below */
	unsigned up;		/* thread to next msg. (replied-to) */
};

struct _pup pup;		/* main system file */

struct _omsg omsg;	/* Fido message header */
struct _msg *msg;	/* msg base index */
int oldfile;		/* Fido msg file */
int msgfile;		/* message index file */
int txtfile;		/* message text file */
int pupfile;		/* PUPPY.SYS file */

int msg_total;
int msg_highest;

/* Local text buffer */

char *text;		/* work buffer */
unsigned textsize;	/* and its size */

main(argc,argv)
int argc;
char **argv;
{
int i,n;
int msgnbr;		/* Fido message number */
long o;
char *cp;

	printf("Fido to Puppy message converter, 17 Nov 87\r\n");
	printf("Tom Jennings, 164 Shipley\r\n");
	printf("San Francisco CA 94107 USA\r\n");
	printf("(k) all rights reversed\r\n");

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

	pupfile= open("puppy.sys",2);		/* load the system file */
	if (pupfile == -1) {
		printf("Can't find PUPPY.SYS\r\n");
		exit(1);
	}
	read(pupfile,&pup,sizeof(struct _pup));	/* read it in, */

	msgcount();				/* count Fido messages */
	printf("There are %d Fido messages\r\n",msg_total);

	openmsg();				/* open the message base */
	msgnbr= 1;
	while (msgnbr= findmsg(msgnbr)) {
		if (--pup.top < 0) 
			pup.top= pup.messages - 1;/* next message ... */
		i= recd(0);			/* new goes at the top */
		strncpy(msg[i].to,omsg.to,35);
		strncpy(msg[i].from,omsg.from,35);
		strncpy(msg[i].subj,omsg.subj,35);
		fixtime(omsg.date,&msg[i].time);
		msg[i].attr= 0;
		msg[i].topic= 1;
		msg[i].topic_map= 0;

		n= read(oldfile,text,textsize);	/* read the Fido msg text, */
		if (n >= pup.msgsize) 		/* fix max size */
			n= pup.msgsize - 1;	/* plus room for ^Z */

		msg[i].attr |= MSGEXISTS;	/* new message */
		writemsg(i);			/* write out the msg header, */

		o= 0L + i;
		o *= pup.msgsize;		/* copy the msg body */
		lseek(txtfile,o,0);		/* seek there, */
		write(txtfile,text,n);

		n= pup.msgsize - n;		/* amt we need to fill out msg */
		cp= text; for (i= n; i--;) 
			*cp++= SUB;		/* fill with ^Zs */
		write(txtfile,text,n);		/* pad it out */

		close(oldfile);			/* done with it, */
		++msgnbr;			/* next Fido message ... */
	}
	closemsg();
	lseek(pupfile,0L,0);			/* keep top */
	write(pupfile,&pup,sizeof(struct _pup));
	close(pupfile);
}

/* Convert message number (1 - N) into record number (0 - N). */

recd(n)
int n;
{
int i;
	i= ((n - 1 + pup.top) % pup.messages);
	return(i);
}

/* Open the message base; open both files, load the index. */

openmsg() {

	msgfile= open("message.idx",2);		/* open the msg base */
	txtfile= open("message.dat",2);
	if ((msgfile < 0) || (txtfile < 0)) {
		printf("Message base file(s) missing!\r\n");

	} else read(msgfile,msg,pup.messages * sizeof(struct _msg));
}

/* Close the message base. */

closemsg() {

	if (msgfile != -1) close(msgfile);
	if (txtfile != -1) close(txtfile);
	msgfile= txtfile= -1;
}

/* Count the number of messages. */

msgcount() {
char spec[SS];
int n;
struct _xfbuf xfbuf;

	msg_total= msg_highest= 0;
	xfbuf.s_attrib= 0;
	while (_find("*.MSG",msg_total,&xfbuf)) {
		++msg_total; 		/* count another, */
		n= atoi(xfbuf.name);	/* change name to number */
		if (n > msg_highest) msg_highest= n;	/* pick highest one, */
	}
}

/* Find a message by number. This will loop up or down, or not at all,
depending on the flag passed. The file, if found, is left open (global
'msgfile' so that it can be worked on. This returns "not found" or the 
message is skipped if it doesnt meet various criteria, such as privacy,
being forwarded, etc. best to look at the code. */

findmsg(num)
int num;
{
char *cp,name[SS];

	while (1) {
		if (num > msg_highest) return(0);
		sprintf(name,"%d.MSG",num);
		oldfile= open(name,0);
		if (oldfile != -1) {
			read(oldfile,&omsg,sizeof(struct _omsg));
			return(num);
		}
		++num;
	}
}

/* Functions to calculate differences in days and minutes from an ASCII string:

	"dd Mon yy  hh:mm:ss"	(exactly)

	[0]	day of month,
	[3]	Month, (3 char ASCII MSDOS name)
	[7]	year, two digits,
	[11]	hours,
	[14]	minutes,
	[17]	seconds.

*/

/* Convert a Fido string date into a Puppy struct time. */

fixtime(s,t)
char *s;
struct _time *t;
{

	t-> year= atoi(&s[7]);
	t-> month= getmn(&s[3]); 
	t-> day= atoi(&s[0]);
	t-> hour= atoi(&s[11]);
	t-> minute= atoi(&s[14]); 
	t-> second= 0;
}

/* Return which month this is. */


char months[13][4] = {
	"XXX",		/* make index 1 - 12 */
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

getmn(s)
char *s;
{
int i;

	for (i= 1; i < 13; i++) {
		if (stcpma(s,months[i])) return(i);
	}
	return(1);
}
/* Write out the specified message header record. */

writemsg(n)
int n;
{
long o;
	o= 0L + n;
	o *= sizeof(struct _msg);
	lseek(msgfile,o,0);
	write(msgfile,&msg[n],sizeof(struct _msg));
}
