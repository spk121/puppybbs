title MSDOS dependent functions
name msasm

comment %

	T. Jennings
	13 Oct 87

	(k) all rights reversed


These are support routines for Puppy that are
currently implemented in MSDOS MASM.EXE assembler.
Most of them are pretty simple; some are annoying.

Some, such as time & date, you can dummy for systems
that dont support those functions, like CP/M; best to
return a 0 or something rather than remove the code; 
if you ever get a clock/calendar you can add it later.

Some are arbitrary 8086isms; byte order in longs. Motorola
people will just have to shut up and implement! :-)

And some are just because when you invoke some supposedly
simple thing like read() or open() the stupid library
drags a whole bag of shit with it. There is a very simple
but effective Level 1 File I/O set of calls here.


handle= open(pathname,access);
int handle,access;
char *pathname;

	Open a file. handle returns either 
the DOS handle, or -1 if error (file not 
found). Access is: 0 == read only, 1 == write
only, 2 == read/write.

handle= creat(pathname,access);

	Create a new file, delete any existing
copy. Access is not used: use 0. Returns the
handle or -1 if error.

v= read(handle,buffer,count);
v= write(handle,buffer,count);
int v,handle,count;
char *buffer;

	File read or write to an opened or
created file. reads or writes 'count' bytes
to the file 'handle', to or from 'buffer'.
Returns the number of bytes processed: equal
to 'count' if sucessful.

error= close(handle)
int error;

	Close an open file. Returns -1 if 
error. Any buffers are flushed at this point.

pos= lseek(handle,distance,flag);
long pos;	new position, else -1
int handle;
long distance;	displacement
int flag;	0 == from BOF, 
		1 == from current,
		2 == from EOF,

	Seek to a position within a file.


Get the time & date from the system, store 
in the provided array. Unsupported items 
should be returned as 0's.

See PUPPY.H for the definition of 'time' and
date.

date= gdate();
time= gtime();


GET and SET MSDOS file creation times of the open
file handle. If not supported, GET a 0 and
make SET do nothing.

SET is done just before the file is closed.

	_ftime(1,handle,timedate) set time/date
	_ftime(0,handle,timedate) get time/date
	char *timedate;
	int handle;



	val= _ioctl(func,handle,data,dummy)
	val= _ioctl(func,handle,buff,count)
	int val;	ret value or flag
	int func,handle,dummy,count;
	char *buff;

This is the MSDOS ioctl() function, used only
to support the function badname(), in MS-C.C. The
description below is only for informational 
purposes; if not MSDOS, just dont implement any 
of it. Refer to badname().


RETURN:	See below
	VAL returns as -1 if any error.

DESCRIPTION:
	IOCTL get/set device information. The 
	args and return values depend on the 
	function:

func == 0	get
func == 1	set
	Get or set device info on (handle).
	0x80 Is a device (else file)
	0x01	Console input
	0x02	Console out
	0x04	Null device
	0x08	Clock device
	0x10	Special (fast) device
	0x20	Raw mode
	0x40	EOF on input
	0x4000	Accepts control strings

func == 2	read
func == 3	write
	Read or write (count) bytes from
	the control channel.

func == 4	read
func == 5	write
	Same as 2,3, except (handle) is drive
	number; 0 == default, 1 == A:, etc.

func == 6	input status
func == 7	output status
	if (handle) is a device, returns 0 if
	not ready, else non-zero if device is 
	ready. For files, output always ready, 
	input always ready until EOF hit.



Search for file first/next function. 

found= _find(pathname,n,&xfbuf);
int n;
char *pathname;
struct _xfbuf *xfbuf;

The structure xfbuf holds search information 
inbetween calls to _find(); MSDOS, unlike CP/M,
allows other file operations between successive
_find() calls. It is absolutely necessary to 
support this. The CP/M implementation of a 
Unix level 1 file system (CPMFILES.C) supports
this.

This function returns 0 if there is no (more)
matching filenames.

pathname is the pathname to search for; it may
contain wildcards and drive specs. 

N indicates first or "next" searches; it is
0 for the first time. For systems like CP/M you
should build a list of all matching files and
return the first, if any. Each successive call 
will return N + 1, ie. 1 2 3 ... N. You
can use this to index the table built when N
was 0.

xfbuf is the address of the structure used to
hold information to perform the search. The
contents are preserved in between calls, so
you can store anything you want in it. It is 
completely uninitialized space, so you should
init it during the N == 0 pass.


title XMODEM CRC Algorithm 8086 / Lattice C Compiler

I did not write this. I dont know who did. I munged an 8080
M80 source into 8086 MASM.

clrcrc();
CLRCRC - A call to this entry resets the CRC accumulator.
 It must be called at the start of each message.

 Entry Parameters:	None.
 Exit Conditions: 	CRC accumulator cleared.

udcrc(c);
char c;
UPDCRC - A call to this entry updates the CRC accumulator.
 It must be called once for each byte in the
 message for which the CRC is being calculated.

 Entry Parameters:	a byte to be included
 			in the CRC calculation.
 Exit Conditions:	CRC accumulator updated.

crc= fincrc();
unsigned crc;
FINCRC - A call to this entry finishes the CRC calculation
 for a message which is to be TRANSMITTED. It must
 be called after the last byte of the message has
 been passed thru UPDCRC. It returns the calculated
 CRC bytes, which must be transmitted as the final
 two bytes of the message.

Note that this returns a single 16 bit value; 
if transmitting bytes, it must be transmitted 
upper half first then lower half:

	output (crc >> 8);
	output (crc);

 Entry Parameters: None.
 Exit Conditions:  calculated CRC bytes.


error= chkcrc();
int error;
CHKCRC - A call to this routine checks the CRC bytes of
 a RECEIVED message and returns a code to indicate
 whether the message was received correctly. It must
 be called after the message AND the two CRC bytes
 have been received AND passed thru UPDCRC.

 Entry Parameters: None.
 Exit Conditions:  0 if message ok.
		   non-zero if message garbled.

end of comment ------> %

;
;Define a (paranoid) near function.
;
 func macro procname
  public procname
  procname proc near
	push	bp
	mov	bp,sp
	push	ds
	push	es
	mov	ax,ds
	mov	es,ax
 endm
;
;Close a function declaration.
;
 endf macro procname
	pop	es
	pop	ds
	pop	bp
	ret
  procname endp
 endm
;
;Define the args on the stack.
;
arg0	equ	[bp+4]
arg1	equ	[bp+6]
arg2	equ	[bp+8]
arg3	equ	[bp+10]
arg4	equ	[bp+12]
arg5	equ	[bp+14]
arg6	equ	[bp+16]
arg7	equ	[bp+18]
arg8	equ	[bp+20]
;
;dgroup just generates a runtime accessible 
;constant 'dataseg' so that drivers and other
;such oddities can set DS: to access data 
;during interrupt service routines.
;
dgroup group data
data segment byte public 'data'
assume ds:dgroup

public dataseg
dataseg dw dgroup	;so drivers can find it

data ends

pgroup group prog		;MASM mumbo jumbo
prog segment byte public 'prog'
assume cs:pgroup
page
;
;Return the time or date components.
;
func gdate
	mov	ah,2ah
	int	21h		;get date
	mov	bx,cx		;build it here
	sub	bx,1980		;CX = year
	mov	cl,9
	shl	bx,cl		;shift it in,

	mov	al,dh		;DH = month
	and	ax,0fh
	mov	cl,5
	shl	ax,cl
	or	bx,ax

	mov	al,dl		;DL = day
	and	ax,1fh
	or	ax,bx
endf gdate

func gtime
	mov	ah,2ch
	int	21h		;get time

	mov	bl,cl		;CL = mins
	and	bx,3fh
	mov	cl,5
	shl	bx,cl

	mov	al,ch		;CH = hour
	xor	ah,ah
	mov	cl,11
	shl	ax,cl
	or	bx,ax

	mov	al,dh		;DH = secs
	and	ax,1fh		;only 2 sec
	add	ax,ax		;resolution
	or	ax,bx
endf gtime
page
;
;Convert a long to a character array.
;
func _ltoc 
	cld
	mov	di,arg2
	mov	ax,arg0		;LSW,
	stosw
	mov	ax,arg1		;MSW
	stosw
endf _ltoc 
;
;Convert a character array to a long.
;
func _ctol 
	cld
	mov	si,arg0
	lodsw		;get LSW,
	mov	bx,ax	;MSW to AX,
	lodsw		;return AX:BX.
endf _ctol 
page
;
;Get and set File times.
;
func _ftime 
	mov	al,arg0		;1=set, 0=get,
	and	al,1
	mov	bx,arg1		;file handle,
	mov	si,arg2		;ptr to array,
	mov	cx,[si]		;CX= time,
	mov	dx,[si+2]	;DX= date,
	push	si
	mov	ah,87
	int	33
	pop	si
	test byte ptr arg0,1	;if get time,
	jnz	_ft1
	mov	[si],cx		;return time,
	mov	[si+2],dx	;return date,
_ft1:
endf _ftime 
page
;
;Perform IOCTL() functions.
;
func _ioctl 
	mov	al,arg0		;func
	mov	bx,arg1		;handle
	mov	dx,arg2		;data/buff
	mov	cx,arg3		;dummy/count
	mov	ah,68
	int	33

	jnc	_io1		;if error,
	mov	ax,-1		;return -1
	jmp	short _ioz

_io1:	cmp	word ptr arg0,2	;functions 0,1
	jae	_io2		;return from DX
	mov	ax,dx
	jmp	short _ioz

_io2:	cmp	word ptr arg0,6	;functions 2-5
	jb	_ioz		;return from AX
	mov	ah,0		;6,7 from AL
_ioz:
endf _ioctl 
page
;
;Do search first/next.
;
func _find 
	mov	dx,arg2		;XFBUF ptr,
	mov	bx,dx
	mov	ah,26		;set DMA addr
	int	21h		;to buffer,
	mov	cx,0		;CX == attrib
	mov	dx,arg0		;pathname
	mov	ax,arg1		;iteration flag
	or	ax,ax
	jz	f0
	mov	al,1		;make 0,1

f0:	mov	ah,78		;search first
	cmp	al,0		;if AL == 0,
	je	f1
	mov	ah,79		;else search next
f1:	int	21h		;DOS call,
	mov	ax,-1		;make 0 == not found
	adc	ax,0
endf _find 
page
;
;Our very own little bit of data space.
;
crc	dw	0	;in CS space

;
;Initialize the CRC.
;
func clrcrc
	mov	cs:crc,0
endf clrcrc

;
;Update the CRC with the new byte. The method used is 
;the CCITT polynomial:
;
;	x^16 + x^12 + x^5 + 1
;
;An alternate method often used in synchronous
;protocols is:
;
;	x^16 + x^15 + x^2 + 1
;
;Which can be generated by changing the XOR pattern
;from 1021 hex to 8005 hex.
;
func updcrc
	mov	bx,cs:crc	;BX == CRC a reg for speed,
	mov	cx,8		;CX == bits in a byte,
	mov	ax,arg0		;AL == msg byte,

u1:	rcl	al,1		;MSB -> carry,
	rcl	bx,1		;    -> CRC LSB,
	jnc	u2
	xor	bx,1021h
u2:	loop	u1

	mov	cs:crc,bx
endf updcrc

;
;Finish off the CRC.
;
func fincrc
	xor	ax,ax		;do two zeros
	push	ax
	call	updcrc
	call	updcrc
	pop	ax
	mov	ax,cs:crc	;return finished CRC
endf fincrc

;
;Check the calculated CRC. Return 0 if OK.
;
func chkcrc
	mov	ax,cs:crc
endf chkcrc
page
;
;File functions
;
func open 
	mov	ah,61
	call	opncrt
endf open

func creat 
	mov	ah,60
	call	opncrt
endf creat

opncrt:
	mov	dx,arg0		;pathname,
	mov	al,arg1		;access,
	xor	bx,bx
	xor	cx,cx
	int	21h		;do it,
	jnc	opncrt1
	mov	ax,-1		;error!
opncrt1:ret
;
;Close a file
;
func close 
	mov	ah,62
	mov	bx,arg0		;handle,
	int	21h
endf close 
;
;File read & write
;
func read 
	mov	ah,63
	call	rdwrt
endf read

func write 
	mov	ah,64
	call	rdwrt
endf write

rdwrt:	mov	bx,arg0		;handle,
	mov	dx,arg1		;buffer,
	mov	cx,arg2		;count,
	int	21h
	ret
;
;File seek
;
func lseek 
	mov	bx,arg0		;BX == handle,
	mov	dx,arg1		;CX:DX == disp,
	mov	cx,arg2
	mov	ah,42h
	mov	al,arg3		;unless error,
	int	21h		;DX:AX == pos,
	mov	bx,ax
	mov	ax,dx
	jnc	_x1
	mov	ax,-1
	mov	bx,-1
_x1:
endf lseek 

prog ends

end
