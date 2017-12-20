#include <ascii.h>
#include <puppy.h>
#include <pupmem.h>

/*

Low level modem support. These depend on and set the following:

FLAG cd_flag;		1 == ignore cd(). Used while initializing,
			dialing, disconnecting, etc.

unsigned cd_bit;	modem CD mask bit. 

unsigned datarate;	data baud rate, at which file transfer times
			and such are computed. It may be the same as
			the link rate.

unsigned rate;		modem baud rate; the speed at which Fido talks
			to the modem. For 300/1200/2400 type modems, it
			is the same as datarate.

See statics below for mdmfunc. internal variables.


setbaud(r)
	Set the baud rate to r. For variable speed modems, this sets
	the data rate and the link rate the same. For fixed link rate
	modems it sets only the data rate; the link rate never changes.

answer()
	This is called while waiting for modem activity or an event.
	Answer the phone if it is ringing. This looks for a RING result
	code, and issues an ATA command and waits for connect if one
	is found. 

	Returns:	-1	modem commands in progress (dont dial!)
			0	modem idle
			1	modem connected & online

	If answer() says connected, then cd_flag is false and true
	carrier can be watched. 

	If mdmstate == 1 and carrier is not present, the modem is
	assumed to be idle.

discon()
	Disconnects the modem and readies it for the next call. This does
	various obscure things to make lousy hobbiest modems perform more or
	less reliably. 

result= dial(s);
char *s;
	Dialer module for FidoNet. Given a string, dial the
	number, and return true if connected, else 0 if no connection.
	The number passed is in telink/DC Hayes format.

	0,1,2,3,4,5,6,7,8,9,#,*
	( ) - ' '		ignored
	.			delay 1 sec
	P T			Pulse, Touch Tone
	?			Wait for tone
	$			script file

chk_modem()	
	Poll the modem, and return any result code found or -1 if
	none received. This returns the numeric value of the result 
	code string. The buffer must be initialized to a null string
	the first pass through, and contents preserved between calls.

connect(result)
int result;
	Interprets the result code from chk_modem() and displays a message
	and returns a status word as follows: 0: no connection made; 
	1: connected, variable 'rate' set and baud rate set, -1 error 
	in dialing.

char *lastconnect()
	Returns the string defining the last modem result.

*/

static int mdmstate;		/* what the modem is up to:
					0	idling
					1	online
					-1	awaiting result from the modem
				*/

static char mdmbuff[SS] = "";	/* we store results here, etc */
static char mdmresult[SS] = "";	/* last modem result */

/* Send any modem initialization file to the modem. */

init_modem(initstr)
char *initstr;
{
char *si;
int i,n;

	printf("Pup says: \"Initializing the modem\"\r\n");
	cd_flag= 1;			/* ignore carrier */
	setbaud(pup.maxbaud);		/* set the data rate */
	flush(0);
	discon();			/* disconnect, */

	cd_flag= 1;			/* ignore carrier */
	modout(CR);			/* clear the modem command buffer */
	flush(50);

	atp(initstr);			/* send it, */
	sendwt("\r");			/* CR & wait for result, */
	modem_chk();			/* wake it up, */
	mdmstate= 0;			/* modem is now idle */
	cd_flag= 0;
}

/* Output a modem command string, wait for the result code, or a timeout
after 2 seconds. Returns the result code or -1 if timeout. */

sendwt(s)
char *s;
{
int n;
	atp(s);				/* send the string, */
	for (millisec= 0L; millisec < 2000L; ) {
		n= chk_modem();		/* check for result, */
		if (n >= 0) {		/* stop if we get one */
			delay(50);
			break;
		}
	}
	return(n);
}

/* Send a command sequence to the modem, */

atp(s)
char *s;
{
	while (*s) {
		modout(*s++);
		delay(2);
	}
}

/* Set the desired baud rate. Bound it to the maximum the modem can handle. */

setbaud(n)
unsigned n;
{
	if (n > pup.maxbaud) n= pup.maxbaud;	/* bound it */
	datarate= n;				/* set data rate, */
	baud(datarate);				/* set it */
}

/* Disconnect. First, it delays to let (output buffered) characters out,
then drops DTR and waits for carrier loss. (Timeout if it does not
go away.) A delay is done to ensure that stupid modems see DTR and dont 
get upset. Then DTR is raised. */

discon() {

int i;

	limit= 0;				/* no time limit, we check explicitly */

/* Wait until the output buffer is flushed. If carrier is lost, flush
anything that remains and stop waiting. Output a CR to flush the modems
command buffer in case anything went out with CD low. */

	cd_flag= 0;				/* need to look at CD */
	while (cd() && _mbusy());		/* while carrier & outputting */
	flush(0);				/* flush anything remaining */
	cd_flag= 1;				/* ignore CD, */
	flush(20);				/* flush buffers */
	modout(CR);				/* abort dialing, etc USR modem */

/* With DTR low, wait for CD to go away. If it doesnt, complain like hell,
and wait for a Control-C, then quit everything. This aborts dialing also
for some modems. */

	cd_flag= 0;				/* watch CD again */
	lower_dtr();				/* drop DTR, */
	for (i= 10; i--;) {			/* wait for no CD */
		delay(10);			/* 100 mS minimum, */
		if (! cd()) break;
	}
	raise_dtr();				/* enable the modem again */
	delay(10);				/* short delay for DTR */

/* If that didnt work, try +++; if there is still carrier (USR disconnects
on +++) issue ATH0 (which will disconnect the Hayes etc) */

	if (cd()) {				/* if STILL carrier,*/
		for (i= 1; i++;) {		/* try +++ */
			cd_flag= 1;
			flush(0);
			sendwt("+++");		/* try to disconnect */
			cd_flag= 0;
			if (! cd()) break;	/* stop if disconnected, */

			cd_flag= 1;
			sendwt("ATH0\r");	/* try H0 next ... */

			cd_flag= 0;
			if (! cd()) break;	/* did that work??? */

			if (i > 5) {
				printf("      Puppy cannot make the modem disconnect!\r\n");
				printf("      Type Control-C to abort to DOS: ");
				doscode= 1;
				if (bdos(6,0xff) == ETX) return;
			}
		}
	}

/* Now see if the modem is still alive; this flushes garbage and makes sure
the modem is ready before continuing. */

	setbaud(pup.maxbaud);			/* set the max. data rate */
	cd_flag= 1;				/* ignore DTR */
	modem_chk();				/* check if modem is dead */
	mdmstate= 0;				/* modem is now idle */
	cd_flag= 0;
}
/* Issue AT commands a few times and try to get the modems attention. */

modem_chk() {

int i;

	cd_flag= 1;
	for (i= 3; i--; ) {
		if (sendwt("AT\r") >= 0) return;
	}
	printf("Pup says: \"Modem not responding!\"\r\n");
}

/* Poll the modem looking for activity; returns a code indicating whats
happening. If 0, the modem is idle, awaiting calls etc. -1 means a 
RING was received, and ATA issued, and we're waiting for connect/fail.
1 means we are connected and online. */

answer() {
int n;

	cd_flag= 0;				/* watch true carrier */
	if (!cd() && (mdmstate == 1)) 		/* if we're supposedly connected, */
		mdmstate= 0;			/* idle if no carrier */

	n= chk_modem();				/* check for a result, */
	if (n < 0) return(mdmstate);		/* none yet, same state */
	switch (connect(n)) {			/* see what happened, */
		case 1:				/* dial sucessful */
			cd_flag= 0;		/* wait for CD */
			mdmstate= 0;		/* (assume not connected) */
			for (millisec= 0L; millisec < 20000L;) {
				if (cd()) {	/* if we find it, */
					delay(200); /* delay for modem/telco */
					mdmstate= 1; /* flag the connection */
					break;
				}
			}
			break;			/* go return modem state */

		case -1:			/* dialing/command error */
			mdmstate= 0;		/*   reset state, */
			break;

		case 0:				/* modem idle, no connection */
			if (n == 2) {		/*   but we're answering! */
				printf("Pup says: \"Attemping a connection\"\r\n");
				flush(20);	/*   wait til its ready (& flush RINGs) */
				atp("ATA\r");	/*   force answer if RING */
				mdmstate= -1;	/*   somethings happening */

			} else {		/* not RING, */
				mdmstate= 0;	/*   so now idle */
			}
			break;
	}
	return(mdmstate);			/* say what it was */
}

/* Given a result code from the modem, return 1 if connected OK and
baud rate set, 0 if no connection, or -1 if an error. Though in theory the
modem COULD return any result code at any time, in practice it doesn't; 
the modem will not say RINGING while waiting for an incoming call ... hence
some of these messages dont apply to all cases. */

connect(result)
int result;
{
char *s;
int r,n;
long x;

	n= -1;
	s= "";

	switch (result) {
/* These are connect for either incoming calls or dialing */
		case 1: n= 1; r= 300; break;
		case 5: n= 1; r= 1200; break;
		case 9: n= 1; r= 600; break;
		case 10: n= 1; r= 2400; break;
		case 13: n= 1; r= 9600; break;

/* These are from the Hayes Courier HST */
		case 11: strcpy(mdmresult,"Ringing!"); n= 0; break;
		case 15: n= 1; r= 1200; s= "/ARQ"; break;
		case 16: n= 1; r= 2400; s= "/ARQ"; break;
		case 17: n= 1; r= 9600; s= "/ARQ"; break;

/* Stupid MNP things from telebit */
		case 20: n= 1; r= 300; s= "/REL"; break;
		case 22: n= 1; r= 1200; s= "/REL"; break;
		case 23: n= 1; r= 2400; s= "/REL"; break;

/* These are Telebit TrailBlazer */
		case 50: n= 1; r= (80L * linkrate) / 100L; s= "/FAST"; break;
		case 61: n= 1; r= (80L * linkrate) / 100L; s= "/FAST/KERMIT"; break;
		case 62: n= 1; r= (80L * linkrate) / 100L; s= "/FAST/XMODEM"; break;
		case 63: n= 1; r= (80L * linkrate) / 100L; s= "/FAST/UUCP"; break;
		case 52: strcpy(mdmresult,"R-R-Ring!"); n= 0; break;

/* These cause idle for incoming calls, but terminate dialing. RINGING is
special cased in dial(), and incoming RING is special cased in answer(). */
		case 2: strcpy(mdmresult,"Ring!"); n= 0; break;
		case 8: strcpy(mdmresult,"No Answer"); n= 0; break;
		case 3: strcpy(mdmresult,"No Carrier"); n= 0; break;
		case 6: strcpy(mdmresult,"No Dial Tone!"); n= 0; break;
		case 7: strcpy(mdmresult,"Busy"); n= 0; break;

/* These cause idle for incoming calls, and error-terminate dialing */
		case 4: strcpy(mdmresult,"Command error!"); break;
		case 12: strcpy(mdmresult,"Voice!"); break;

/* This is just a plain old fucking error */
		case 0:
		default: 
			 *mdmresult= NUL; n= 0; break;	/* usually nothing */
	}

/* Do some setup if we are connected. */

	if (n == 1) {
		setbaud(r);			/* set baud rate, */
		flush(20);			/* flush trash */
		sprintf(mdmresult,"Connected at %,d%s",datarate,s);
	}
	if (*mdmresult) {
		puts("Modem says: \"");
		puts(mdmresult);
		puts("\"\r\n");
	}
	return(n);
}

/* Return the last connect string. */

lastconnect() {

	return(mdmresult);
}

/* Check the modem for a result code, return its numerical value or -1
if none yet. This uses the modem buffer to store characters between
iterations. */

chk_modem() {
#define i (mdmbuff[0])		/* the index into the buffer */
#define buff (&mdmbuff[1])	/* what we use as the buffer */

	cd_flag= 1;				/* we are NOT online! */
	if (_mconstat()) {			/* if a character there, */
		buff[i]= _mconin() & 0x7f;	/* get it, */
		if (buff[i] == CR) {		/* if a complete line, */
			i= 0;			/* empty the line, */
			return(atoi(buff));	/* return the result */

		} else if (isdigit(buff[i])) {	/* if a new digit, */
			++i;			/* install it, */
		}
		buff[i]= NUL;			/* terminate string & erase non-digit */
	}
	return(-1);				/* nothing happened */
}

/* Dial a numeric string. Returns: 0 for no connection, 1 for connected,
-1 for error, such as voice or no dial tone. The speed to dial at is assumed
to be set before calling. This returns with the actual connect rate set
if a connection is made. */

dial(string)
char *string;
{
char c,*p;
int n;

	p= string;
	atp("ATDT");			/* dial command (default Touch Tone), */
	while (*p) {
		c= *p++;
		switch (c) {
			case 't': c= 'T'; break;	/* lower to upper case */
			case 'p': c= 'P'; break;

/*			case '$': return(script(p,next_arg(p)));
				break;
*/
			case '(':
			case ')':
			case ' ':
			case '-': c= NUL; break;	/* ignore these */

			case '.': c= ','; break;	/* dot means delay */
		}
		if (c != NUL) modout(c);
	}

	limit= 1;
	clr_clk();			/* one minute to dial */

	modout(CR);			/* start the command, */
	flush(10);

/* Now wait for a result. Note that if we detect an incoming ring, it
will terminate the dial with a no-connection, and presumably the main
loop will detect the incoming call on the next ring. */

	while (1) {			/* wait for a connect, no connect or error */
		limitchk();		/* check for timeout */
		n= chk_modem();		/* get the result code, */
		if (n < 0) continue;
		if (n != 11) break;	/* quit if we get one ... */
		printf(" * Ringing!\r\n"); /* repeat if RINGING */
	}
	return(connect(n));		/* go complete it */
}

