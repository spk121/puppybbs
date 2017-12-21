#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "ascii.h"
#include "edit.h"
#include "modemio.h"
#include "puppy.h"
#include "pupmem.h"

/*
	Miserable Message Editor

	T. Jennings
	Fido Software
	164 Shipley
	San Francisco CA 94107
	(415)-764-1688

	(k) all rights reversed
*/

#define WIDOFF	(sizeof("99: "))	/* size of line number prompt */


/* Enter and edit text. */

int edit(int ln) /* ln is first line to input at, */
{
	int end;			/* maximum line # */
	int lastline;			/* last line number, for detecting when to display */
	char *cp,c;
	char rptchar;			/* repeat-last-char count */
	char *tp;			/* ptr to the text buffer itself */
	char word[SS * 2];		/* word[] must be at least 2X max line len */
	FLAG run;			/* 1 == keep inputting, etc */

	int pw;				/* length of 'word' (index into word[]) */
	int tw;				/* length of 'tp' (index into text line) */

	end= pup.msgsize / caller.cols; /* maximum lines */
	tp= &text[ln * caller.cols];	/* ptr to the text */
	tw= 0;			/* length of current line */

	lastline= -1;		/* trigger line # display */
	rptchar= 0;		/* no character repeat count */
	ins_line(ln,end);	/* incase we're I)nserting */

	run= 1;
	while ( run && (ln < end)) {
		if (ln != lastline) {
			lastline= ln;			/* only once each line */
			line= 0;			/* override "More?" */
			mconout(CR); mconout(LF);	/* prevent autowrap */
			mprintf("%2u: ",ln + 1);	/* display line number */
			for (cp= tp; *cp; ++cp) {	/* display line so far */
				if (*cp >= ' ') mconout(*cp);
			}
		}

/* OK, now build a 'word', character by character. We stop (unbeknownst
to the user) when we get a CR, space, etc. We also "fall out" if the word
becomes too long to fit on the screen; hence the "while strlen + pc < width". */

		pw= 0;
		while (tw + pw + WIDOFF < caller.cols) {/* while it fits the line ... */
			if (rptchar != 0) {		/* if repeated character */
				--rptchar;		/* count one, */
			} else c= mconin();		/* get one word */

			if ((c == SUB) || (c == VT) || (c == ETX)) {
				run= 0;			/* if ^Z, ^K or ^C */
				break;			/* stop editing */
			}
			if (c == CR) break;		/* stop if CR, */
			if (c == DEL) c= BS;		/* DEL becomes BS */

			if (c == TAB) {			/* if we get a tab, */
				rptchar= 8 - ((tw + pw) % 8); /* count = column mod 8 */
				c= ' ';			/* set the character, */
				continue;		/* then go do it */
			}
			if (c == CAN) {			/* if delete line */
				rptchar= tw + pw;	/* for the entire line, */
				c= BS;			/* set as back space, */
				continue;		/* go do it */
			}

/* If a character delete, backspace on the screen. First delete characters 
from the word we are building, if that is empty, then from the line we
are building. */

			if (c == BS) {
				if (pw > 0) --pw;	/* remove chars from the word, */
				else if (tw > 0) --tw; 	/* or the line, */
				else continue;		/* (neither, ignore BS) */
				erase(1); 		/* then erase on screen */
				mconout(BS);
				continue;
			}

			if (c >= ' ') {
				mconout(c);
				word[pw++]= c;		/* store chars in word */
			}
			if (pw >= sizeof(word) - 1) break; /* max length */
			if (c == TAB) break;		/* word terminators */
			if (c == ' ') break;
			if (c == ',') break;
			if (c == ';') break;
		}
		word[pw]= NUL;				/* terminate the word */
		tp[tw]= NUL;				/* and the line so far */

/* We have text on the screen, and a word in the word[] array. If it will
fit within the screen line length, display as-is, if not, do a CR/LF and
do it on the next line. (This gets executed when word[] is zero length, 
ie. only a CR is entered.) */

		if (tw + pw + WIDOFF < caller.cols) {
			strcat(tp,word);		/* add it if it fits, */
			tw += pw;			/* current line length, */

/* When we go to the next line, insert a blank line; if we're entering
new text this is a waste of time, but if we're inserting it moves existing
text down one line. */

		} else {				/* doesnt fit */
			erase(pw);			/* clear last word from screen, */
			if (tw == 0) strcat(tp," ");	/* no 0 length lines */
			tp += caller.cols;		/* go to next line */
			++ln;				/* next line # */
			ins_line(ln,end);		/* insert a blank one, */
			strcpy(tp,word);		/* put word on next line, */
			tw= pw;				/* also its line length */
		}

/* If a CR was entered, then we want to force things to start on the next 
line. Also stuff a CR when the line is ended if its not a blank line by itself. */

		if (c == CR) {				/* if hard return */
			strcat(tp,"\r\n");		/* add a CR/LF */
			tp += caller.cols;		/* next line */
			++ln;				/* next line # */
			ins_line(ln,end);		/* insert a blank one, */
			tw= 0;				/* new line is blank */
		}
	}
	if ( ! *tp) strcpy(tp,"\r\n");

/* Now find the first unused line, and return that as the next line. */

	while (*tp) {					/* find next blank line */
		if (ln >= end) break;			/* dont go past the end */
		tp += caller.cols;
		++ln;
	}
	return(ln);
}

/* Delete a line from the text array. */

int del_line(int line, int end)
{
	char *tp;

	tp= &text[line * caller.cols];			/* ptr to 1st line */
	while (line < end) {
		strcpy(tp,tp + caller.cols);		/* copy the line, */
		tp += caller.cols;			/* next ... */
		++line;
	}
	*tp= NUL;					/* last line is empty */
	return(--end);
}

/* Insert a blank line. Copies all lines down one, dropping
off one at the bottom (which is presumably blank.) */

int ins_line(int line, int end)
{
	char *tp;
	int i;

	tp= &text[end * caller.cols];		/* tp == address of last line, */
	for (i= end - line; i; --i) {
		strcpy(tp,tp - caller.cols);	/* copy line down, */
		tp -= caller.cols;		/* next ... */
	}
	*tp= NUL;				/* inserted line is empty */
	return(end);
}

/* Erase a word. Backspace, then type spaces. */

void erase(int n)
{
	int i;

	for (i= 0; i < n; i++) mconout(BS);
	for (i= 0; i < n; i++) mconout(' ');
}
