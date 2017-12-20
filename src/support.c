#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#define open(FN,FLAG) (_open((FN),(FLAG)))
#define close(H) (_close((H)))
#define read(H,BUF,SIZ) (_read((H),(BUF),(SIZ)))
#define creat(FN,P) (_creat((FN),(P)))
#define write(H,BUF,SIZ) (_write((H),(BUF),(SIZ)))
#define lseek(H,X,Y) (_lseek((H),(X),(Y)))
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "abort.h"
#include "ascii.h"
#include "mdmfunc.h"
#include "modemio.h"
#include "pupmem.h"
#include "puppy.h"
#include "support.h"

/*
Various low level and extremely important support routines. These
do not need to be customized generally to fit the target machine. 


append(fname)	Open or create file fname for appending.

logoff(code,disc)
		Terminate this call; disconnect from the modem if
		disc is true; sets code as the DOS ERRORLEVEL for this
		connect. This writes out the caller record as necessary, 
		etc.

makefname(buf,name)
		Creates a full pathname from the current (Msg, File,
		Upload, pup.bbs) area. NOTE: If 'ovpath' is not blank,
		it is used instead of the one from the area.

rline(f,line,len)
		Read a line of text; a line is defined as characters
		delimited by a CR. See the code for special cases. Returns
		0 if end of file.

int num_args(string)	Returns the number of args in the string, seperated
			by delims (see delim(), below). Leading delimiters
			are ignored.

char *next_arg(string) Returns a pointer to the next arg, delimited 
			by a delim, skipping over the current arg. Use via
			ptr= nextarg(ptr) to skip to each argument. All 
			switches at the end of the current arg are skipped.


char *skip_delim(string) Skips leading delims in a string. returns a pointer.

cpyarg(to,from)	Copies a string, up to the next delim or switch.
			Leading and trailing delimiters are stripped (from 
			the output string) and a null terminator is added.

			after cpyarg()		FROM: foo/b foobar fipple
						TO: foo

char *strip_path(out,in) Copies the disk specifier or pathname to the output
			array, and returns a pointer to the name in the input
			string.	Drive specs are considered a path, and are
			treated as such by DOS. Stripping "a:foo" and
			"bin/foo/framis.asm" result in:

				IN:	"A:"
				IN:	"bin\foo"

				OUT:	"A:"
				OUT:	"bin\"

stolower(s)	Convert a string to all lower case

stoupper(s)	Convert a string to all upper case

atoi(s)		Convert a string of digits into a numeric value; stops
		when the first non-digit character is found.



str_time(s,t)	Generates a static string expression of date or time, and
str_date(s,t)	returns a pointer to it. Through a kludge it can be 
		called up to 4 times without overwriting a previous 
		invokation.

same_node(n1,n2) Returns true if the two nodes are the same.

cpy_node(&dest,&src)
		Copies one node into another.

delim(c)	Returns true if the character is a delimiter.

char *str_node(&node)
		Generates a static string expression of the node, and 
		returns a pointer to it. Through a kludge it can be 
		called up to 4 times without overwriting a previous 
		invokation.

set_nn(string,&node)
		Parse the string into various zone:net/node numbers. 
		This accepts "zone:net/node" format. If zone or net 
		isnt specified, it is left untouched.

*/

static int same_node(struct _node *n1, struct _node *n2);
static void cpy_node(struct _node *d, struct _node *s);


/* Display the date. */

char *str_date(WORD t)
{
	static char work[2][sizeof("31 Dec 143 ")];	/* where we keep them */
	static int k;					/* which one as are on */

	k = ++k % 2;		/* select next ... */
	*work[k]= NUL;		/* empty it */

	sprintf(work[k],"%u %s %02u",		/* the format, */
	    t & 0x1f,				/* the day, */
	    months[(t >> 5) & 0x0f],		/* the month name */
	    ((t >> 9) & 0x3f) + 80);		/* the year */
	return(work[k]);
}

/* Display the time. */

char *str_time(WORD t)
{
	static char work[2][sizeof("23:59PM ")];	/* where we keep them */
	static int k;					/* which one as are on */
	char hour;

	k = ++k % 2;		/* select next ... */
	*work[k]= NUL;		/* empty it */

	hour= t >> 11; if (!hour) hour= 12;
	sprintf(work[k],"%d:%02d",hour,(t >> 5) & 0x3f);
	if (hour >= 12) strcat(work[k],"PM"); else strcat(work[k],"AM");
	return(work[k]);
}

/* Open a file for appending, creating it if necessary. Return the
open handle, or -1 if error. */

int append(char *s)
{
	int h;
	char c;

	h= open(s,2);				/* open or create the */
	if (h == -1) h= creat(s,2);		/* file, if opened OK */
	else lseek(h,0L,2);			/* seek to the end */
	return(h);				/* handle or error */
}

/* Log a caller off the system; disconnect and force a termination. */

void logoff(int code, int disc)
{
	int f;
	char buff[SS];
	long p;

	limit= 0;				/* disable time limits */
	doscode= code;				/* set result code, */
	if (!test && disc) discon();		/* do disconnect, */
	frc_abort();				/* force rturn to main */
}

/* Copy the path prefix, append one filename from the input string. */

void makefname(char *d, char *s)
{
	strcpy(d,pup.filepref);		/* the path prefix, */
	d= next_arg(d);			/* point to its end, */
	cpyarg(d,s);			/* add one filename */
}

/* Display a text file. */

int dspfile(char *filename)
{
	int f;

	f= open(filename,0);
	if (f == -1) return(0);
	dumptext(f);
	close(f);
	return(1);
}

/* Output the contents of a text file to the console. The handle of 
an open file is passed. Output ceases when EOF or a Control-Z is found. */

void dumptext(int file)
{
	char lastc,c,buff[512];				/* local buffering */
	unsigned index,count;

	if (file == -1) return;				/* be serious */

	index= count= 0;				/* buffer is empty */
	while (1) {
		if (index >= count) {			/* read some if */
			index= 0;			/* the local buffer */
			count= read(file,buff,sizeof(buff)); /* is empty */
		}
		if (! count) break;			/* stop if empty file */
		if (_abort) break;			/* stop if aborted */

		c= buff[index++];			/* get a character, */
		if (c == LF) continue;			/* ignore LFs */
		if (c == CR + 128) {			/* special case soft CRs */
			if (lastc == ' ') continue;
			c= ' ';
		}

		lastc= c;				/* remember it, */
		if (c == CR) mputs("\r\n");		/* CR becomes CR/LF */
		else fmconout(c);			/* else output text */
		if (c == SUB) break;			/* ^Z is end of file */
	}
	mputs("\r\n");
}

/* Write the system file out to disk. */

void putsys()
{
	int f;

	f= open("puppy.sys",2);
	if (f == -1) {
		printf("Can't open PUPPY.SYS!\r\n");
		return;
	}
	write(f,&pup,sizeof(struct _pup));
	close(f);
}

/* Read a line of text from the file, null terminate it.  Function returns
zero if EOF. Deletes all CRs and Control-Zs from the stream. Lines are
terminated by LFs. */

char rline(int file, char *buf, int len)
{
	int i;
	char notempty,c;

	i= 0; notempty= 0;
	--len;						/* compensate for added NUL */
	while (i < len) {
		if (! read(file,&c,1)) break;		/* stop if empty */
		if (c == 0x1a) continue;		/* totally ignore ^Z, */
		notempty= 1;				/* not empty */
		if (c == '\r') continue;		/* skip CR, */
		if (c == '\r' + 128) continue;		/* skip soft CR, */
		if (c == '\n') break;			/* stop if LF */
		buf[i++]= c;
	}
	buf[i]= '\0';
	return(notempty);
}

/* Return the number of args left in the string. */

int num_args(char *s)
{
	int count;

	count= 0;
	s= skip_delim(s);			/* skip leading blanks, */
	while (*s) {
		++count;			/* count one, */
		s= next_arg(s);			/* find next, */
	}
	return(count);
}

/* Return a pointer to the next argument in the string. */

char *next_arg(char *s)
{
	while ((!delim(*s)) && *s)		/* skip this one, */
		++s;				/* up to delim, */
	s= skip_delim(s);			/* then skip delims, */
	return(s);
}

/* Skip over the leading delimiters in a string. */

char *skip_delim(char *s)
{
	while (delim(*s) && *s) {
		++s;
	}
	return(s);
}

/* Copy the string to the destination array, stopping if we find one
of our delimiters. */

void cpyatm(char *to, char *from)
{
	while ( (!delim(*from)) && *from) 
		*to++= *from++;
	*to= '\0';
}

/* Copy the string to the destination array, stopping if we find one
of our delimiters. */

void cpyarg(char *to, char *from)
{
	while (*from) {
		if (delim(*from)) break;
		*to++= *from++;
	}
	*to= '\0';
}

/* Strip the pathname or disk specifier from a filename, return it in a
seperate array. We do this by initially copying the entire name in, then
searching for the colon or slash. Right after the last one we find,
stuff a null, removing the name part. 

Also return a pointer to the name part in the input name. */

char *strip_path(char *out, char *in)
{
	char *name;
	char *endpath;

	strcpy(out,in);			/* duplicate, for working, */
	name= in;			/* point to name, */
	endpath= out;			/* and end of path part, */

	while (*in) {			/* look for slashes or colons, */
		if (*in == ':')	{	/* if a colon, */
			endpath= ++out;	/* point to name, */
			name= ++in;

		} else if ((*in == '/') || (*in == '\\')) {
			endpath= ++out;	/* move the pointer up, */
			name= ++in;
		} else {
			++in;
			++out;
		}
	}
	*endpath= '\0';			/* delete the name part, */
	return(name);			/* return ptr to name part. */
}

/* Convert a string to lower case. */

void stolower(char *s)
{
	while (*s) {
		*s= tolower(*s);
		++s;
	}
}

/* Convert a string to upper case. */

void stoupper(char *s)
{
	while (*s) {
		*s= toupper(*s);
		++s;
	}
}

/* atoi() function missing from Lattice C. From Kernighan and Richie. */

int atoi(char *s)
{
	int n;
	n= 0;
	while ((*s >= '0') && (*s <= '9')) {
		n *= 10;
		n += *s - '0';
		++s;
	}
	return(n);
}

/* Return true if the two nodes are the same. */

static int same_node(struct _node *n1, struct _node *n2)
{
	return(
		(n1-> number == n2-> number) &&
		(n1-> net == n2-> net) &&
		(n1-> zone == n2-> zone)
	);
}

/* Copy one node structure into another. */

static void cpy_node(struct _node *d, struct _node *s)
{
	d-> zone= s-> zone;
	d-> net= s-> net;
	d-> number= s-> number;
}

/* Return a pointer to a string of the zone:net/node. */

char *str_node(struct _node *n)
{
#define KLUDGE 4		/* strings we can make at once */
	static char work[KLUDGE][40];	/* where we keep them */
	static int k;			/* which one as are on */

	k = ++k % KLUDGE;	/* select next ... */
	*work[k]= NUL;		/* empty it */
	sprintf(work[k],"%d:",n-> zone);
	sprintf(&work[k][strlen(work[k])],"%d/%d",n-> net,n-> number);
	return(work[k]);
}


/* Return true if the character is a delimiter. */

int delim(char c)
{
	int i;
	switch (c) {
		case ';': 
		case ' ': 
		case ',': 
		case ':': 
			return(1);
		default: return(0);
	}
}
