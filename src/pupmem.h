/*
	pup's memory usage declarations.
*/

extern struct _pup pup;		/* our main system file */
extern struct _clr caller;	/* currently logged in caller */

/* Message base memory resident index */

extern struct _msg *msg;	/* msg index file contents */

extern FLAG test;		/* 1 == test mode, (no modem) */
extern FLAG localin;		/* 1 == simultaneous keyboards */
extern char lmtstate;		/* 0 - 2, caller time limit warning state */
extern int limit;		/* time limit in force, or 0 for no limit */
extern int klimit;		/* download limit in force */

/* Local system shit */

extern char months[13][4];	/* table of month names */
extern FLAG abort;		/* True if ^C typed. */
extern FLAG doscode;		/* DOS error code */
extern char column;		/* column number, */
extern char line;

/* XMODEM protocol module */

extern int totl_files;		/* how many failed, */
extern int totl_errors;		/* error count, soft errors incl */
extern int totl_blocks;		/* number blocks sent, */
extern char crcmode;		/* 1 if CRC mode, */
extern char filemode;		/* transfer type; XMODEM, MODEM7, TELINK */

/* Modem variables */

extern FLAG cd_flag;		/* true == ignore CD line */
extern WORD linkrate;		/* baud rate to/from modem */
extern WORD datarate;		/* baud rate to/from caller */
extern WORD cd_bit;		/* MSDOS driver: bit to test for Carrier Detect, */
extern WORD iodev;		/* MSDOS driver: serial channel number */	

/* Local text buffer */

extern char *text;		/* work buffer */
extern int textsize;		/* and its size */

extern LONG millisec;		/* MSDOS driver: G.P. milliseconds */
extern LONG millis2;		/* MSDOS driver */
extern WORD seconds,minutes,hours; /* MSDOS driver */

