#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "compat.h"
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

	f= xopen2("QUOTES.PUP", XO_RDONLY);		/* open it, */
	if (f == -1) return;			/* doesnt exist, */
	xseek(f,pup.quote_pos,0);		/* seek to correct place, */
	strcpy(mark,"abcd");			/* anything but CR LF CR LF */

	mputs("\r\n");
	while (1) {
		if (! xread(f,&c,1)) {		/* read a character, */
			xseek(f,0L,0);		/* rewind if we hit EOF */
			pup.quote_pos= 0L;	/* (current position) */
			if (! xread(f,&c,1)) break; /* try again */
		}
		++pup.quote_pos;		/* current actual position */
		for (i= 0; i < 4; i++)		/* ASCII shift register */
			mark[i]= mark[i + 1];	/* slide 'em through */
		mark[3]= c;			/* add to the end */
		if (strcmp(mark,"\r\n\r\n") == 0) break;
		if (mark[2] == '\n' && mark[3] == '\n') break;

		fmconout(c);
	}
	mputs("\r\n");
	xclose(f);
}
