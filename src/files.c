#include <ctype.h>
#include <io.h>
#include <string.h>
#include "ascii.h"
#include "files.h"
#include "modemio.h"
#include "ms-c.h"
#include "pupmem.h"
#include "puppy.h"
#include "support.h"
#include "xmodem.h"

/* 
	Puppy's file system. 
*/

static int listfile();
static int dl_check(char *name);
static int okname(char *name);
static int fileerr(int f);

/* Display downloadable files by reading lines from FILES.BBS. Pull off the 
filename (supposed to be in col 0) and see if it matches. If so, display it, 
then the rest of the line. Stop if we find a line beginning with @. */

static int listfile()
{
	struct _fileinfo fileinfo;
	char line[SS];			/* possibly long line */
	char name[SS];			/* current file name */
	char buff[SS];			/* assembled filename */
	char *cp;
	int file;
	int files;			/* how many we found */

	makefname(name,"files.pup");
	file= open(name,0);
	if (file == -1) {
		mprintf("\r\nSORRY: No Files\r\n");
		return(0);
	}

	files= 0;					/* none so far */
	while (rline(file,line,sizeof(line)) && !_abort) {
		cp= line;				/* ptr to line text */
		if (*cp == 26) break;			/* stop if ^Z */
		if (*cp == '@') break;			/* logical end of file */

		if ((*cp == '-') || (*cp == ' ')) {	/* display comments */
			mputs(cp);			/* as is */
			mputs("\r\n");
			continue;
		}
		cpyarg(name,cp);			/* get filename, */
		cp= next_arg(cp);			/* cp is ptr to comments */
		name[12]= NUL;				/* for idiot proofing */
		mprintf("%-12s ",name);			/* display the name, */
		makefname(buff,name);			/* make full pathname, */
		if (getinfo(buff,0,&fileinfo)) 		/* get file information */
			mprintf("%8lu ",fileinfo.size);
		else mputs(" MISSING ");		/* cant find it */
		mputs(cp);				/* display the comments, */
		mputs("\r\n");
		++files;
	}
	close(file);
	return(files);
}

/* Download files. */

void download()
{
	char name[SS];		/* FILENAME.EXE(nul) (13 bytes) name being processed */
	char fname[SS];		/* full filename with prefix */
	char c;

	if (ask("Want the list of files")) {
		if (!listfile()) return;	/* possibly display files, */
	}
	cpyarg(name,getarg("File: "));		/* get the filename, */
	if (! *name) return;			/* nothing entered */
	name[13]= NUL;				/* max length */
	stoupper(name);				/* upper case */

	if (!okname(name)) return;		/* illegal names, */
	makefname(fname,name);			/* make full filename */
	if (!dl_check(fname)) return;		/* check limits, etc */

	while (1) {				/* how to download, */
		c= tolower(*getarg("How to download (T=Text, X=Xmodem, CR=Quit): "));
		switch (c) {
			case NUL: return;	/* CR only */

			case 'x':		/* XMODEM */
				mprintf("Ready to send \"%s\" using XMODEM\r\n",name);
				mprintf("Start now, Control-C to abort\r\n");
				filemode= XMODEM; 
				crcmode= 1;
				fileerr(transmit(fname,text,textsize));/* do it! */
				return;

			case 't':		/* Text */
				mputs("Hit a key to start, or Control-C to abort\r\n");
				if (mconin() == ETX) return;
				dspfile(fname);
				return;
		}
	}
}

/* Upload a file, ask for a description, add it to the file list. */

void upload()
{
	char name[SS];
	char buff[SS];
	int f;

	cpyarg(name,getarg("File: "));		/* get the filename, */
	if (! *name) return;
	name[13]= NUL;
	stoupper(name);				/* make upper case */

	if (!okname(name)) return;		/* device names, etc */
	makefname(buff,name);			/* make full filename */
	f= open(buff,0);			/* make sure it does NOT */
	if (f != -1) {				/* exist yet */
		close(f);
		mprintf("SORRY: \"%s\" already exists!\r\n",name);
		return;
	}
	mprintf("Ready to receive \"%s\" using XMODEM\r\n",name);
	mprintf("Start now, Control-C to abort\r\n");

	filemode= XMODEM; 
	crcmode= 1;
	f= receive(buff,text,textsize);		/* do the upload */
	if (fileerr(f)) return;			/* stop if error */

	mconflush();
	cmdflush();				/* flush typeahead */
	mputs("Wait ...\r\n");			/* may take a while */
	makefname(buff,"FILES.PUP");		/* file list file name */
	f= append(buff);			/* open/create, seek to end */
	if (f == -1) {
		mputs("OOPS: Cant open/create the file list\r\n");
		return;
	}
	write(f,name,strlen(name));		/* write the file name, */
	write(f," ",1);				/* a seperator */
	input("Describe: ",buff,40);		/* get the description, */
	write(f,buff,strlen(buff));		/* write that, */
	write(f,"\r\n",2);			/* new line */
	close(f);
}

/* Perform checks on the passed filename for downloading. Check time limits,
K limits, file existence, etc. Return 0 if this file cannot/should not
be downloaded. */

static int dl_check(char *name)
{
	struct _fileinfo fileinfo;
	int n;
	int bpm;						/* blocks per minute */

	if (! getinfo(name,0,&fileinfo)) {		/* get info on it */
		mputs("SORRY: There is no such file!\r\n");
		return(0);
	}

/* OK, I guess I ought to explain this. Download time is:

	bytes= file size, bytes
	bps= data rate, bytes per second, ie baud / 10

	download time, seconds =  bytes / baud / 10

This is stupid, I want it in blocks not bytes, and minutes not seconds,
so why do all this high school arithmetic?, we're programmers and we 
don't know any better anyways:

	blocks= (bytes + 127) / 128
	blocks per minute [bpm]= ((baud / 10 * 60) + 127) / 128
	bpm= ((baud * 6) + 127) / 128
so ...
	minutes= ((bytes + 127) / 128) / (((baud / 10 * 60) + 127) / 128)
or ...
	minutes= blocks / bpm

lets replace the division and multiplication by constants to shifts
and addition because it's so awful on Z80s or whatever without hardware
mult/div. */

	n= fileinfo.size;			/* MSDOS, n= file size */
	n += 127L;				/* round up, */
	n >>= 7;				/* n= (bytes + 127) / 128 */
	mprintf("%d blocks\r\n",n);		/* display number of blocks, */

	bpm= (datarate << 2) + (datarate << 1);	/* b= baud * 6 */

	n= n / bpm;  				/* time to download, */
	++n;					/* toss in an extra */

	if (limit > n) return(1);		/* return OK if less than limit */
	mprintf("Pup says: \"You don't have enough time left\"\r\n");
	return(0);
}

/* Return true if this filename is OK to use. */

static int okname(char *name)
{
	if (badname(name) || (strcmp(name,"FILES.PUP") == 0)) {
		mputs("SORRY: That is a filename reserved for Pup\r\n");
		return(0);
	}
	return(1);
}

/* Check the download/upload error code and return true if there is an
error. */

static int fileerr(int f)
{
	mputs("\r\n");
	switch (f) {
		case 0: mputs("File sent OK\r\n"); return(0);
		case -2: mputs("You canceled the transfer\r\n"); break;
		case -3: mputs("SHIT! The disk is full!\r\n"); break;
		default: mputs("Something went wrong\r\n"); break;
	}
	return(-1);
}
