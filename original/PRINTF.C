
/* Formatted string processor.

	T. Jennings
	Fido Software
	164 Shipley
	San Francisco CA 94107
	(415)-764-1688

	(k) all rights reversed

Format string options:

	"%-0w.p,l{dux}"

		- 	left justified (else right)
		0 	left zero fill
		w 	field width
		.p	precision
		,	comma separate 1000's (cute, huh?)
		l	long

_spr() is a formatted string processor for printf() etc. Gross. Plagiarized 
from Leor Zolman's STDLIB2.C for BDS C 1.42. Brain Damage Software still 
lives on ... what year is this???

HARD-CODED ASSUMPTIONS:

this uses a pointer to access the variable number of arguments on the
stack instead of complex (to me) structures and unions. Hence, its very
dependent on the ACTUAL SIZE of elements it deals with.

MSDOS:
	chars are two bytes on stack
	ints are two bytes on stack
	longs are four bytes on stack
	char *'s are two bytes

	For the function call(a,b,c,d,e,f), where all are ints (say)

	a is at address N on the stack
	b is at N + 2
	c is at N + 4
	...

It works by keeping a pointer to the arguments on the stack, and using
the %thing string to figure out what kind of arg to pull off.


KNOWN BUGS:

Staunchly refuses to print negative numbers. I've been meaning to get around
to fixing this bug for over four years. It doesnt look like I'll get to it
soon, so if it bugs you fix it yourself. (Id love a copy.) I never could find
a reason to shove negative numbers in the users face anyways.

*/

#define MAXSTR 300		/* size of process buffer */

char _spr_sepchar = ',';	/* 1000's seperator character */

/* Write a string to the console. */

puts(s)
char *s;
{
	while (*s) lconout(*s++);
}
/* Replacements for the overly large Lattice ones. */

printf(f)
char *f;
{
char buf[MAXSTR],*p;

	_spr(buf,&f);
	puts(buf);
}

sprintf(s,f)
char *s,*f;
{
	_spr(s,&f);
}

_spr(line,fmt)
char *line, **fmt;
{
char c;
char base;			/* base is 10, 8, 16, etc */
char *sptr;			/* pointer to string argument */
char *format;			/* pointer to format string */
char **s;
char pf;			/* true == doing precision (dot detector) */
char ljflag;			/* true == left justification */
char zfflag;			/* true == zero fill */
char sepflag;			/* true == comma fill, ie 1,000,000,000,000 */
int width;			/* width of current arg (ie. for padding w blanks/zeros) */
int precision;
int sizearg,islong;
long num;

unsigned li;			/* index into line, above */

char wb[33];			/* room for largest possible digit string (long base 2) */
char sb[MAXSTR];
unsigned si;			/* index to sb[] */
unsigned i,j,l;			/* habits can be hard to break */

int *args;			/* ptr to int/char args */
long *longarg;			/* avoids ugly casts */
char **charstararg;		/* ditto */

	format = (char *)*fmt++;
	args = (int *)fmt;
	li= 0;				/* total output line length */

	while (c = *format++)
		if (c == '%') {
			si= 0;		/* output buffer index */
			precision = 6;
			ljflag = pf = zfflag = sepflag= 0;

			if (*format == '-') {
				format++;
				ljflag++;
			 }

			if (*format == '0') zfflag++;	/* test for zero-fill */

			width = (isdigit(*format)) ? _gv2(&format) : 0;

			c= *format++;
			if (c == '.') {
				c = *format++;
				precision = _gv2(&format);
				pf++;
			}

			if (c == ',') {
				c= *format++;
				sepflag= 1;
			}

			longarg= (long *)args;
			charstararg= (char **)args;
			islong= 0;

			if (c == 'l') {			/* if long, */
				c= *format++;		/* skip the L, */
				sizearg= sizeof(long);	/* for bumping pointer */
				longarg= (long *)args;	/* force confusing typing */
				num= *longarg;		/* get the value, */
				islong= 1;

			} else {
				sizearg= sizeof(int);
				num= *args;
			}
			switch(toupper(c)) {

				case 'D':
					if (num < 0L) {
						sb[si]= '-';
						if (si < MAXSTR) ++si;
						num= - num;
						width--;
					}

				case 'U': base = 10; goto val;
				case 'X': base = 16; goto val;
				case 'O': base = 8; goto val;

/* _uspr() builds the plain digit string in wb, the Work Buffer. If using
comma formatting, then separators are added before the string is moved to
sb, the String Buffer. */

				val:	_uspr(wb,&si,num,base);
					if ((si > 3) && sepflag) {
						l = si % 3;
/*
				si= raw string length
				wb= raw digit string (ie. "12345")
				sb= working string (ie. "12,345")
				l= 100's digits counter, MOD 3
				i= index into sb[]
				j= index into wb[]
*/
						for (i= j= 0; j < si;) {
							sb[i++]= wb[j++];
							if ((--l == 0) && (j < si))
								sb[i++]= _spr_sepchar;
							if (l <= 0) l= 3;
						}
						si= i;		/* see pad: */

					} else for (i= 0; i < si; i++) sb[i]= wb[i];
					width -= i;

					if (islong) args= (int *) ++longarg;
					else ++args;
					goto pad;

/* A char on the stack is the same size as an int. */
				case 'C':
					sb[si]= *(char *)args;
					if (si < MAXSTR) ++si;
					++args;
					width--;
					goto pad;

				case 'S':
					if (! pf) precision = 200;
					sptr = *(char **)args;
					args= (int *) ++charstararg;

					while (*sptr && precision) {
						sb[si]= *sptr++;
						if (si < MAXSTR) ++si;
						precision--;
						width--;
					}

				pad:
					sb[si]= '\0';
					si= 0;
					if (! ljflag) {
						while (width-- > 0) {
							line[li]= (zfflag ? '0' : ' ');
							if (li < MAXSTR) ++li;
						}
					}
					while (sb[si]) {
						line[li]= sb[si++];
						if (li < MAXSTR) ++li;
					}

					if (ljflag) {
						while (width-- > 0) {
							line[li]= ' ';
							if (li < MAXSTR) ++li;
						}
					}
					break;

				 default:
					line[li]= c; if (li < MAXSTR) ++li; break;
			 }

		} else {
			line[li]= c;
			if (li < MAXSTR) ++li;
		}

	line[li]= '\0';
}

/*
	Internal routine used by "_spr" to perform ascii-
	to-decimal conversion and update an associated pointer:
*/

int _gv2(sptr)
char **sptr;
{
int n;
	n = 0;
	while (isdigit(**sptr)) n = 10 * n + *(*sptr)++ - '0';
	return(n);
}

_uspr(buff, buffi, n, base)
char buff[];
unsigned *buffi;
long n;
int base;
{
int length;

	if (n < base) {
		buff[*buffi]= (n < 10L) ? n + 48L : n + 55L;
		if (*buffi < MAXSTR) ++(*buffi);
		buff[*buffi]= '\0';
		return(1);
	}
	length = _uspr(buff, buffi, n / base, base);
	_uspr(buff, buffi, n % base, base);
	return (length + 1);
}
