#include "ascii.h"
#include "puppy.h"
#include "pupmem.h"
#include "ms-asm.h"
#include "sched.h"

/* Scheduler for Fido, etc. Various scheduling and support routines.
These use the MSDOS time and date, and assumes a continuous seven
day schedule. Resolution is one minute.

til_sched(tag,m)
		Returns the index of the soonest event within M
		minutes, or -1 if none found. This ignores events that
		are runnable now but have already been run. (ie. completed
		an External event early.) 

		This returns -1 for all events marked "PREEMPT", unless
		the time til the event is zero.

		If ? is passed as the search tag, then any runnable
		event A - W is allowed, else the tag must match exactly.

til_event(n)	Return the number of minutes until this event number
		should be run, or 0 if it should be running now. 

*/

static WORD til_event(int n);

/* Find the soonest runnable event within M minutes, and return it's 
index, or -1 if none. Ignore events that have already been run. */

int til_sched(char tag, int m)
{
	int n,i;
	unsigned next_time;
	int next_event;

	next_time= MINS_DAY;				/* oldest possible */
	next_event= -1;					/* none of them */

	for (i= 0; i < SCHEDS; i++) {
		if (! pup.sched[i].tag) break;		/* NUL == end of table */
		if ((tag != '?') && (pup.sched[i].tag != tag))
			continue;

/* If we are in the middle of this events window, (time til event == 0)
see if its already run (SCHED_COMPLETE); if so, ignore it, otherwise
run it immediately. (markevt(event#) must be called explicitly to flag an 
event as already run.) */

		n= til_event(i);			/* time til this event runs, */
		if (n == 0) {				/* if its runnable NOW, */
			if (pup.sched[i].bits & SCHED_COMPLETE) continue;
			next_event= i;
			next_time= n;			/* not run yet */
			break;				/* so run it now */
		}

		pup.sched[i].bits &= ~SCHED_COMPLETE;	/* clear it */
		
/* Remember the next-soonest event, so we can return it when we terminate the
loop. OPTIONAL events are not checked, since are only interested in them
when their time comes up. */

		if ((n < next_time) && !(pup.sched[i].bits & SCHED_OPTIONAL)) {
			next_event= i;			/* this is soonest */
			next_time= n;			/* so far, */
		}
	}

/* If we found one within the desired time, return it, else return -1. */

	if (next_time <= m) return(next_event);		/* one we found */
	else return(-1);
}

/* Mark this event as completed */

void markevt(unsigned n)
{
	pup.sched[n].bits |= SCHED_COMPLETE;
}

/* Return the number of minutes until this event can be run, or 0 if it
should be running now. */

static WORD til_event(int n)
{
	WORD now,start,end;

	now= gtime();				/* get current time */
	now= ((now >> 11) & 0x3f) + ((now >> 5) & 0x1f); /* in abs. minutes */
	start= pup.sched[n].hr * 60 + pup.sched[n].min;	/* start time */
	end= start + pup.sched[n].len;		/* when it ends */


	if ((now >= start) && (now < end)) 	/* should be running now */
		return(0);			/* zero mins until start ... */
	int tmp = (int)start - (int)now;
	while (tmp < 0)
		tmp += MINS_DAY;
	return(tmp);
}
