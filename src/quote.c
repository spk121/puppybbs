#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#define open(FN,FLAG) (_open((FN),(FLAG)))
#define close(H) (_close((H)))
#define read(H,BUF,SIZ) (_read((H),(BUF),(SIZ)))
#define lseek(H,X,Y) (_lseek((H),(X),(Y)))
#endif
#include <string.h>
#include "modemio.h"
#include "puppy.h"
#include "pupmem.h"
#include "quote.h"

/* Pull the next quote from the file, output to the screen. */

void quote()
{
	int i,f;
	char mark[5];
	char c;

	f= open("quotes.pup", _O_RDONLY);		/* open it, */
	if (f == -1) return;			/* doesnt exist, */
	lseek(f,pup.quote_pos,0);		/* seek to correct place, */
	strcpy(mark,"abcd");			/* anything but CR LF CR LF */

	mputs("\r\n");
	while (1) {
		if (! read(f,&c,1)) {		/* read a character, */
			lseek(f,0L,0);		/* rewind if we hit EOF */
			pup.quote_pos= 0L;	/* (current position) */
			if (! read(f,&c,1)) break; /* try again */
		}
		++pup.quote_pos;		/* current actual position */
		for (i= 0; i < 4; i++)		/* ASCII shift register */
			mark[i]= mark[i + 1];	/* slide 'em through */
		mark[3]= c;			/* add to the end */
		if (strcmp(mark,"\r\n\r\n") == 0) break;

		fmconout(c);
	}
	mputs("\r\n");
	close(f);
}
