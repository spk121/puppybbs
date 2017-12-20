
#include <puppy.h>

struct _time {
	BYTE year;
	BYTE month;
	BYTE day;
	BYTE hour;
	BYTE minute;
	BYTE second;
};

struct _oldmsg {
	char from[36];		/* who from, */
	char to[36];		/* who to, */
	char subj[36];		/* message subject, */
	struct _time time;
	WORD attr;		/* attribute bits (see below) */
	WORD topic;		/* topic selection(s) */
	WORD topic_map;		/* shared topics */
};

main() {
int i;
struct _oldmsg msg;
struct _msg *m;
long pos;

	printf("converts Pup 2a message base to Pup 2b message base (very quickly)\r\n");
	i= open("puppy.idx",2);
	if (i == -1) return;

	pos= 0L;
	m= (struct _msg *) &msg;
	while (read(i,&msg,sizeof(struct _oldmsg))) {
		if (m-> extra != 0) {
			m-> date= ((msg.time.year - 80) << 9) | ((msg.time.month) << 5) | (msg.time.day);
			m-> time= (msg.time.hour << 11) | (msg.time.minute << 5);
			m-> extra= 0;
		}
		prdate(m-> date); printf(" "); prtime(m-> time); printf("\r\n");
		lseek(i,pos,0);
		write(i,&msg,sizeof(struct _msg));
		pos += sizeof(struct _oldmsg);
	}
	close(i);
}
/* Display the date. */

prdate(t)
WORD t;
{
	printf("%02u-%02d-%02u",		/* the format, */
	    (t >> 5) & 0x0f,			/* the month */
	    t & 0x1f,				/* the day, */
	    ((t >> 9) & 0x3f) + 80);		/* the year */
}

/* Display the time. */

prtime(t)
WORD t;
{
	printf("%d:%02d",t >> 11,(t >> 5) & 0x3f);
}

