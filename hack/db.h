#ifndef PB_DB_H
#define PB_DB_H

// The dynamic global properties
struct _pup_dynamic
{
  uint32_t callers;	/* Cumulative number of callers to the system
			   over all time. */
  uint32_t quote_pos;		/* The index of the current entry in
				   the quote file. */
  uint32_t msgnbr;		/* Cumulative count of all messages to
				   the system over all time. */
};


/* CREATE TABLE Properties (
Callers INT,    ; increments by one each time a call is received
LastQuote INT,  ; last quotation printed
MessageCount INT ; increments by one each time a message is created

*/

/*
CREATE TABLE Users (
ID int NOT NULL,
Name VARCHAR(36) NOT NULL,
Terminal VARCHAR(36),  ; "xterm" or such
Lines INT,       ; default presumed screen size
Columns INT,     ; default presumed screen size
Extra INT,
Calls INT,
Topic1 INT,      ; last read message ID in topic #1
...
Topic16 INT,
PRIMARY KEY (ID)
);
*/

/*
CREATE TABLE Messages (
ID int NOT NULL,
From VARCHAR(36) NOT NULL,
To VARCHAR(36),
Subject VARCHAR(36),
Timestamp DATETIME,
Read TINYINT,     ; true if read by apparent receipient
Topic TINYINT,     ; which if the 16 topic categories this belongs to
Extra INT,
Message VARCHAR(80*20) NOT NULL,
PRIMARY KEY (ID)
*/



struct _clr {
	char name[36];			/* the miserable SOBs name, */
	WORD date[BITSWORD];		/* newest read for each topic */
	WORD topic;			/* last topic(s) selected */
	char lines;			/* number of lines, */
	char cols;			/* number of columns, */
	int calls;			/* sigh ... how many times, */
	int extra;
};



struct _msg {
	char from[36];		/* who from, */
	char to[36];		/* who to, */
	char subj[36];		/* message subject, */
	WORD date;		/* message creation date, */
	WORD time;		/* message creation time, */
	WORD extra;		/* unused space */
	WORD attr;		/* attribute bits (see below) */
	WORD topic;		/* topic selection(s) */
	WORD topic_map;		/* shared topics */
};

/* Message attribute bits */

#define MSGEXISTS 1		/* message slot occupied */
#define MSGREAD 2		/* read by addressee */
#define MSGSENT 4		/* sent OK (remote) */
#define MSGTAG 8		/* general purpose tag bit */		    

#endif
