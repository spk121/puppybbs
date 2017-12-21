#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <string.h>
#include "ascii.h"
#include "compat.h"
#include "modemio.h"
#include "ms-asm.h"
#include "ms-c.h"
#include "printf.h"
#include "puppy.h"
#include "pupmem.h"
#include "support.h"
#include "xmodem.h"

/*
	XMODEM, TELINK drivers

	T. Jennings 20 Sep 87

*/

/* Protocol driver return codes */

#define OK 0		/* alls well */
#define ERROR -1	/* generic error */
#define ABORT -2	/* manually aborted */
#define DISKFULL -3	/* disk full error */

/* Global statistics */

extern int totl_files;	/* how many sent/received */
extern int totl_blocks;	/* 128 bytes blocks */
extern int totl_errors;	/* block retries, etc */

static FLAG diverter;	/* packeter/unpacketer byte diverter */

static void xmitdone(int files);
static int getfname(char *name);
static int getfile(int f);
static int sendfname(char *name);
static int sendfile(int file, int statflg, struct _fileinfo *fileinfo);
static int getack();
static int waitnak();
static int sendtel(struct _fileinfo *fileinfo);
static int getblock(int crcflag, char *buffer);
static void sendblock(BYTE sector, int crcflag, char *buffer);
static int get_buff(int f, char *buff, int n);
static int put_buff(int f, char *buff, int n);
static int sendfn(char *name);
static int getfn(char *name);
static void cvt_from_fcb(char *inname, char *outname);
static inline int fnc(char c);
static void cvt_to_fcb(char *inname, char *outname);
static void _cvt2(char *inname, char *outname, int n);
static void xferstat(int n, char *s, ...);
static int get_pkt();
static void put_pkt(char c);

/* Transmit one or more files. The list fn contains one or more file names;
if explicit is true, then they should be used as is else the system path
name should be used. */

int transmit(char *fn, /* filename for XMODEM */
	char *namelist,    /* list of sent names */
	int listlen)       /* max. len of the list */
{
	char fspec[SS];		/* full file to search for */
	char fname[SS];		/* full filename to send */
	int i,f,files;
	int error;
	struct _fileinfo fileinfo;
	FLAG dofn;		/* 1 == do modem7 filename */
	FLAG dotb;		/* 1 == do telink block */

	switch (filemode) {
		case TELINK:	diverter= 0; dotb= 1; dofn= 1; break;
		case MODEM7:	diverter= 0; dotb= 0; dofn= 1; break;
		case XMODEM:	diverter= 0; dotb= 0; dofn= 0; break;
		case FIDONET:	diverter= 1; dotb= 0; dofn= 0; break;
	}

	while (*fn) {
		cpyarg(fspec,fn);			/* get an atom, */
		fn= next_arg(fn);			/* for next time */
		i= 0; 
		while (getinfo(fspec,i++,&fileinfo)) {
			strip_path(fname,fspec);	/* get the prefix, */
			strcat(fname,fileinfo.name);	/* add the filename */
			xferstat(1,"File: %s",fname);	/* display it, */

			if (dofn) error= sendfname(fileinfo.name); else error= OK;
			if (error != OK) break;		/* ERROR or ABORT */

			f= xopen2(fname,0);		/* open it, */
			if (f == -1) {
				xferstat(2,"Can't open \"%s\"",fname);
				continue;
			}
			error= sendfile(f,dotb,&fileinfo);
			xclose(f);			/* close the file, */
			if (error != OK) break;		/* check errors */
			if (strlen(fname) < listlen) {	/* keep a list */
				strcat(namelist,fname);
				strcat(namelist," ");
				listlen -= strlen(fname);
			}
			++totl_files;			/* count another */
			++files;
		}
		if (error != OK) break;
	}
	xmitdone(files);				/* complete transmission */
	return(error);
}

/* Complete transmission. */

static void xmitdone(int files)
{
	int i;

	if ((filemode != XMODEM) && files) {
		xferstat(1,"Last file");
		do {i= modin(100);}
		while ((i != -1) && (i != NAK) && (i != ETX));
		if (i == NAK) {
			xferstat(0,"NAK");
			modout(ACK);		/* name ACK, */
			modout(EOT);		/* no more, */
		}
		xferstat(1,"No More Files EOT");
		modout(EOT);
	}
}

/* Receive files: the filename fn is used for XMODEM only; for TELINK
it is a dummy. If explicit is true, the path from fn is used, else
the system path is used. */


int receive(char *fn, /* fn is filename for XMODEM */
	char *namelist,   /* namelist is list of received names */
	int listlen)      /* listlen is max. len of the list */
{
	int f,oops;
	char path[SS];		/* path/drive from input filename */
	char name[SS];		/* plain filename only */
	char fname[SS];		/* fully filename with prefix */

	FLAG dofn;		/* 1 == do modem7 filename */
	FLAG dotb;		/* 1 == do telink block */

	cpyarg(name,strip_path(path,fn));		/* seperate filename/path */

	switch (filemode) {
		case TELINK:	diverter= 0; dotb= 1; dofn= 1; break;
		case MODEM7:	diverter= 0; dotb= 0; dofn= 1; break;
		case XMODEM:	diverter= 0; dotb= 0; dofn= 0; break;
		case FIDONET:	diverter= 1; dotb= 0; dofn= 0; break;
	}

	while (1) {
		if (dofn) {				/* for TELINK/MODEM7 */
			oops= getfname(name);		/* get the filename */
			if (oops != OK) {		/* remotely */
				xferstat(1,"Can't get a filename");
				break;			/* error */
			}
			if (badname(name)) name[1]= '$'; /* reserved filenames */
		}
		cpyarg(fname,path);			/* build the full */
		strcat(fname,name);			/* filename */
		xferstat(1,"File: %s",fname);		/* display it */

		f= xopen2(fname,0);			/* make sure it does NOT */
		if (f != -1) {				/* exist */
			xclose(f);
			xferstat(2,"File already exists!");
			oops= ERROR;
			break;
		}
		f= xcreat(fname,2);			/* create it, */
		if (f == -1) {
			xferstat(2,"Cannot create file");
			break;
		}
		oops= getfile(f);			/* fill it, */
		xclose(f);				/* close it, */

		if (oops != OK) break;			/* oops, ABORT or EOT */
		if (strlen(fname) < listlen) {		/* keep a list */
			strcat(namelist,fname);
			strcat(namelist," ");
			listlen -= strlen(fname);
		}
		++totl_files;				/* another ... */
		if (! dofn) break;			/* only one */
	}
	flush(0);					/* flush garbage, */
	return(oops);
}		

/* Receive a MODEM7 filename, put into the passed array; return ERROR if error,
OK if we get a filename, EOT if no more files. */

static int getfname(char *name)
{
	int i;
	char aborts;

	aborts= 0;
	for (i= 30; i-- > 0; ) {
		switch (getfn(name)) {
			case OK: return(OK);	/* got a name, */
			case EOT: return(EOT); 	/* no more files */
			case ETX: if (++aborts > 2) return(ABORT);
				break;
		}
	}
	return(ERROR);
}

/* Get a single file from the remote computer. Return ERROR if something
went wrong, SOH if the remote is not in batch mode, or 0 if transfer 
complete. If batch mode, the name is a prefix we should stick on the 
beginning of the disk file we create. Does CRC mode if that mode is set,
or a newer TELINK is used, that transmits the CRC flag. */

static int getfile(int f) /* f is open file */
{
	int i;
	int blknum;				/* file block number, */
	BYTE sector;				/* XMODEM block number */
	BYTE buff[128];				/* XMODEM data block */
	int errors;				/* errors 0 - 9 */
	BYTE ackchar;				/* ACK/NAK */
	BYTE aborts;				/* # of Control-Cs in a row */

	long fsize;				/* file size or -1 */
	long ftime;				/* creation time/date */

	blknum= 0;			/* file block number */
	aborts= 0;			/* no Control-Cs yet */
	errors= 0;			/* no errors */
	fsize= -1L;			/* unknown so far */
	ftime= -1L;
	sector= 1;			/* XMODEM block, first time */
	
	ackchar= NAK;
	if (crcmode) ackchar= 'C';	/* use right initial char, */

	while (errors < 10) {				/* retry count */
		if (ackchar != ACK) xferstat(0,"NAK");	/* status report */
		modout(ackchar);			/* previous ACK/NAK */
		++errors;				/* assume an error */
		i= modin(400);				/* get initial character */
		switch (i) {				/* see what it is */
			case SOH:			/* XMODEM data */
				i= getblock(crcmode,buff); /* get sect number */
				ackchar= NAK;		/* assume bad, */
				if (i < 0) break;	/* check for error */

				if (i < sector) {	/* duplicate */
					xferstat(1,"Duplicate");
					goto goodblk;	/* ACK it */

				} else if (i > sector) {/* out of sync */
					xferstat(1,"Block Sync Error");
					break;
				}
				i= 128;			/* set bytes in buffer */
				if (fsize > 0L) {	/* correct if known */
					i= (fsize > 128L) ? 128 : fsize;
					fsize -= i;
				}
				if (put_buff(f,buff,i) != i) {
					xferstat(2,"DISK FULL!");
					++totl_errors;
					return(DISKFULL);
				}
				++totl_blocks;
				++sector;
				++blknum;

goodblk: ;			aborts= 0;
				errors= 0;
				ackchar= ACK;
				break;

			case SYN:			/* TELINK block */
				ackchar= NAK;		/* assume bad */
				if (getblock(0,buff) != 0) break;
				fsize= _ctol(&buff[0]);	/* FSC001-8 */
				ftime= _ctol(&buff[4]);	/* FSC001-8 */
				xferstat(1,"Telink Block");
				goto goodblk;

			case EOT:			/* end of file */
				xferstat(1,"End of File EOT");
				modout(ACK);		/* ACK the EOT, */
				if (ftime != -1L)	/* set filetime */
					_ftime(1,f,&ftime);
				return(OK);

			case ETX:			/* Control-C */
				xferstat(1,"Control-C");
				if (++aborts > 2) return(ABORT);
				break;

			case -1:			/* timeout */
				break;

			default:			/* garbage */
				flush(10);		/* quiet line */
				break;
		}
		xferstat(0,"Blk %u",blknum);
		if (errors) flush(1);		/* flush garbage if error */
	}
	if (errors) {				/* if errors */
		++totl_errors;			/* flush the line */
		flush(1);			/* was 100 */
	}
	return(ERROR);
}

/* Send a filename, MODEM7 style. */

static int sendfname(char *name)
{
	int i,ci,aborts;

	aborts= 0;
	for (i= 10; i-- > 0; ) {
		switch (sendfn(name)) {
			case ETX: if (++aborts > 2) return(ABORT);
			case OK: return(OK);
		}
	}
	return(ERROR);
}

/* Send a file in XMODEM. */

static int sendfile(int file,          /* open file */
	int statflg,                /* 1 == send TELINK block */
	struct _fileinfo *fileinfo) /* file info struct */
{
	char c;
	int i;
	int blknum;				/* file block number, */
	BYTE sector;				/* >>> 8 BIT <<< XMODEM block number */
	BYTE buff[128];				/* XMODEM data block */
	char errors;				/* errors 0 - 9 */
	char aborts;				/* # of Control-Cs */

	blknum= 0;			/* file block number */
	sector= 1;			/* first XMODEM block number, */
	aborts= 0;			/* no Control-Cs yet */

	crcmode= waitnak();			/* get initial NAK/CRC mode */
	if (crcmode == ERROR) return(ERROR);	/* if not NAK/CRC, error */
	if (statflg) sendtel(fileinfo);		/* file statistics */

	for (errors= 0; errors < 10; ) {		/* (no, not a mistake ...) */
		i= get_buff(file,buff,128);		/* read some file data, */
		if (! i) break;				/* end of file */
		while (i < 128) buff[i++]= 26;		/* pad last block */

		for (errors= 0; errors < 10; ++errors) { /* (... its really OK) */
			xferstat(0,"Block %u",blknum);	/* stats for the nosy sysop */
			modout(SOH);			/* tell rcvr a block comes */
			sendblock(sector,crcmode,buff);
			i= getack();			/* get an acknowledge */
			if (i == ACK) {
				++sector;
				++blknum;
				++totl_blocks;
				errors= 0;
				break;

			} else {
				++totl_errors;		/* another error, */
				flush(0);		/* flush garbage */
				if (i != NAK) errors= 10;
			}
		}
	}

/* All blocks sent, or too many errors. Tell the remote no more. */

	xferstat(1,"");
	if (! errors) {
		for (i= 5; --i > 0; ) {
			xferstat(0,"File EOT");
			modout(EOT);
			if (modin(1000) == ACK) break;
			flush(0);
		}
		return(OK);
	}
	return(ERROR);
}

/* Get an acknowledge character or timeout. Return the character
received, or TIMEOUT for a timeout, ABORT for 3 Control-Cs in a row. */

static int getack()
{
	char aborts;
	int c;

	aborts= 0;

	while (1) {
		c= modin(1000);
		switch (c) {
			case ACK: return(c);

			case NAK: xferstat(0,"NAK"); return(c);

			case ETX: xferstat(1,"Control-C");
				if (++aborts > 2) return(ABORT); 
				break;

			case -1: return(ERROR); break;
		}
	}
}

/* Wait for an initial NAK or CRC; return 0 for NAK, 1 for CRC, or ERROR
for anything else. */

static int waitnak()
{
	char aborts,errors,c;

	aborts= 0;
	xferstat(0,"Waiting NAK");
	for (errors= 60; errors-- > 0; ) {	/* 60 seconds max */

		switch (modin(100)) {
			case NAK:
				xferstat(0,"NAK      ");
				return(0);

			case 'C':
				xferstat(0,"CRC      ");
				return(1);

			case ETX:
				xferstat(1,"Control-C ");
				if (++aborts > 2) return(ABORT);
				break;
		}
		flush(0);
	}
	return(ERROR);
}

/* Attempt to send the TELINK data block. Do only 4 attempts; if not received 
by then, assume the other end is not capable of receiving it. If not received, 
then the file will be handled like XMODEM or MODEM7; time and date lost, 
filesize rounded up to the nearest 128 bytes. */

static int sendtel(struct _fileinfo *fileinfo)
{
int i;
char buffer[128],aborts;

	for (i= 0; i < sizeof(buffer); i++)
		buffer[i]= 0;		/* clear data block, */

	cpyarg(&buffer[8],fileinfo-> name);/* install filename, */
	strcpy(&buffer[25],"Pup sez: Hi");
	buffer[41]= crcmode;		/* send the CRC flag, acknowledge it, */

	aborts= 0;
	for (i= 4; i-- > 0; ) {
		xferstat(0,"TELINK block");
		modout(SYN);
		sendblock(0,0,buffer);	/* send in checksum, */
		switch (modin(1000)) {
			case ACK: return(OK);
			case ETX: if (++aborts > 2) return(ABORT); break;
		}
	}
	return(ERROR);
}

/* Get an XMODEM block, return its sequence number, or -1 if error. This
gets the sector number, its 1's complement, the 128 data bytes and does
the checksum or CRC. */

static int getblock(int crcflag, char *buffer)
{
	int i,v;
	int sector;
	int chksum;

	sector= modin(100);		/* get the sector number, */
	chksum= modin(100);		/* 1's compl (use chksum as temp) */
	if (sector + chksum != 255) return(-1); /* bad sector! */

	chksum= 0;			/* initialize check sum and CRC, */
	clrcrc();			/* we maintain both in parallel, */
	for (i= 0; i < 128; i++) {
		v= modin(100);
		if (v == -1) return(-1);
		buffer[i]= v;
		chksum += v;
		updcrc(v);
	}
	if (crcflag) {			/* if CRC mode, get the two */
		v= modin(100);		/* CRC bytes and do those, */
		if (v == -1) return(-1);
		updcrc(v);
		v= modin(100);
		if (v == -1) return(-1);
		updcrc(v);
		if (chkcrc()) return(-1); /* -1 if bad CRC */

	} else {			/* checksum */
		v= modin(100);		/* get the checksum, */
		if (v == -1) return(-1);
		if (v != (chksum & 255)) return(-1);
	}
	return(sector);			/* good block */
}

/* Transmit an XMODEM block with the specified sector number and checksum/CRC
mode. No return value. */

static void sendblock(BYTE sector, int crcflag, char *buffer)
{
	int i;
	WORD crc;
	BYTE chksum;

	modout(sector);			/* send the sector number */
	modout(~sector);		/* and its complement */

	chksum= 0;			/* maintain both CRC and checksum, */
	clrcrc();

	for (i= 0; i < 128; i++) modout(buffer[i]);
	for (i= 0; i < 128; i++) {	/* output data then do CRC/checksum */
		chksum += buffer[i];
		updcrc(buffer[i]);
	}

	flush(0);			/* flush out garbage, */
	if (crcflag) {
		crc= fincrc();		/* get CRC bytes, */
		modout(crc >> 8);	/* MS byte first, */
		modout(crc);		/* then LS byte, */

	} else modout(chksum);		/* send checksum, */
}

/* Get data to send; if the diverter is on, get them from the packeter. 
Return the number of bytes actually read. */

static int get_buff(int f, /* file handle, or dummy */
	char *buff,     /* buffer to put data in, */
	int n)          /* requested byte count */
{
	int i;

	if (diverter) while (n--) {
		i= get_pkt();		/* get a byte from the packeter, */
		if (i == -1) break;	/* stop if "EOF" */
		*buff++= i;		/* stuff it */

	} else n= xread(f,buff,n);	/* else from a disk file */

	return(n);
}

/* Output the buffer to the disk file or unpacketer. Return the number of bytes
sucessfully written. */

static int put_buff(int f, /* file handle, or dummy */
	char *buff,     /* buffer to put data in, */
	int n)          /* requested byte count */
{
	int i;

	if (diverter) while (n--) put_pkt(*buff++);	/* to the unpacketer, */
	else n= xwrite(f,buff,n);			/* or to the file */
	return(n);					/* say what we did */
}

/* Transmit a filename for batch mode tranmission. Return ERROR if cant do it,
or OK if sent properly. It is assumed that the receiver is ready to receive
the filename we have to send. */

static int sendfn(char *name)
{
	BYTE chksum;
	char c,localname[SS];
	int i;

	chksum= 0;				/* mis-use as Control-C counter */
	for (i= 60; i-- > 0; ) {		/* get initial character */
		c= modin(100);
		if (c == NAK) break;		/* got name NAK */

		if (c == ETX) {			/* if Control-C */
			if (++chksum > 2) return(ABORT);
		}
		flush(0);
	}
	if (! i) return(ERROR);			/* timeout */

	cvt_to_fcb(name,localname);		/* convert name, */
	chksum= 0;				/* start checksum, */
	modout(ACK);				/* all ready, */
	for (i= 0; i < 11; i++) {
		c= localname[i];		/* get name char, */
		chksum += c;			/* maintain checksum, */
		modout(c);			/* send name char, */
		if (modin(200) != ACK) break;	/* if not an ACK */
	}
	if (i >= 11) {				/* if all sent OK */	
		modout(SUB);			/* end of name, */
		chksum += SUB;			/* stupid protocol */
		if (modin(100) == (chksum & 0xff)) { /* get recvr's checksum, */
			modout(ACK);		/* if good, say so, */
			return(0);		/* return happy :-), */
		}
	}
	modout('u');				/* else not sent OK, */
	return(ERROR);				/* return sad :-( */
}

/* Get a filename from the sender. */

static int getfn(char *name)
{
	int i;
	int c;			/* NOTE: 'c' is an integer !!! */
	BYTE chksum;
	char newname[SS];

	modout(NAK);					/* sync sender, */
	c= modin(200);					/* get sync character, */
	if (c != ACK) return(c);

	chksum= 0;
	for (i= 0; i < 11; i++) {			/* 11 char max, */
		c= modin(100);				/* get a char, */
		switch (c) {
			case -1: return(ERROR);		/* timeout */
			case 'u': return(ERROR);	/* error */
			case EOT: return(EOT);		/* no more files */
			case SUB: break;		/* end of name */
			default:
				chksum += c;		/* file name char I guess */
				newname[i]= c;		/* stash it */
				modout(ACK);		/* say "ok" */
				break;
		}
	}
	if (modin(200) == SUB) {			/* if end of name, */
		modout(chksum & 0xff);			/* send check sum, */
		if (modin(200) == ACK) {
			cvt_from_fcb(newname,name);	/* fix name, */
			return(OK);
		}
	}
	return(ERROR);					/* filename too long */
}

/* Convert a CP/M like filename to a normal ASCIZ name. */

static void cvt_from_fcb(char *inname, char *outname)
{
	int i;
	char c;

	for (i= 8; --i > 0; ) {
		c= fnc(*inname++);
		if (c != ' ') *outname++= c;	/* ignore spaces */
	}					/* but do all 8 */
	*outname++= '.';			/* add the dot, */

	for (i= 3; --i > 0; ) {
		c= fnc(*inname++);
		if (c == ' ') break;
		*outname++= c;
	}
	*outname= NUL;				/* terminate it, */
}

/* Fix this filename character */

static inline int fnc(char c)
{
	if ((c < ' ') || (c == '\\') || (c == '/')) c= '$';
	return(c);
}


/* Convert a normal asciz string to MSDOS/CPM FCB format. Make the filename
portion 8 characters, extention 3 maximum. Supports wildcards, skips drive
specs. */

static void cvt_to_fcb(char *inname, char *outname)
{
	char c;
	int i;

	if (inname[1] == ':') inname= &inname[2];
	for (i= 0; i < 11; i++)
		outname[i]= ' ';		/* clear out name, */
	_cvt2(inname,outname,8);		/* do name portion, */
	while (*inname) {			/* skip to dot, if any */
		if (*inname++ == '.') break;
	}
	_cvt2(inname,&outname[8],3);		/* do extention, */
}
/* Do part of a name. */

static void _cvt2(char *inname, char *outname, int n)
{
	int i;
	for (i= 0; i < n; i++) {		/* NAME PART */
		if (*inname == '\0')		/* if null, */
			break;			/* quit, */
		else if (*inname == '*')	/* if *, fill with ?, */
			outname[i]= '?';
		else if (*inname == '.')	/* if a dot, */
			break;			/* skip to extention, */
		else {
			outname[i]= toupper(*inname);
			++inname;
		}
	}
}

/* Display a file transfer status message. N is how to handle it:

	0	CR only; stay on same line
	1	CR/LF after text
	2	CR/LF before & after text
 */

static void xferstat(int n, char *s, ...)
{
	char buff[SS * 4];

	if (n == 2) puts("\r\n");
	switch (filemode) {
		case XMODEM:	puts("-X"); break;
		case TELINK:	puts("-T"); break;
		case MODEM7:	puts("-B"); break;
		case KERMIT:	puts("-K"); break;
		case FIDONET:	puts("-F"); break;
		case SEALINK:	puts("-S"); break;
		case IFNA:	puts("-I"); break;
	}
	if (crcmode) puts("C");
	puts(": ");	

	_spr(buff,&s);
	puts(buff);

	puts("\r"); if (n) puts("\n");
}

static int get_pkt()
{
	return 0;
}

static void put_pkt(char c)
{

}