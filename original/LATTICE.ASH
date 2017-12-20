;
;Assembly insert file to setup segments
;and access args on the stack for the various
;8086 models.
;
;Macros and equates defined here:
;
;CSEG		Create a code segment and type.
;ENDC		Close a code segment
;
;DSEG		Create a data segment
;ENDD		Close a data segment
;
;FUNC 		Use to declare a function. Defines
;		it as far or near, appropriately. Also
;	pushes BP, sets BP to SP, and saves DS and ES.
;	(Requirements changed in v2)
;
;ENDF		Declare the end of a function. Restore
;		registers saved by FUNC.
;
;ARG0..9	Defined args from the stack, since
;		the offset depends on the size of
;	the return address: mov ax,ARG0 always gets
;	the right one.
;
;LONG		Defined as true if long pointers
;		are used. Use to conditionally 
;	assemble ES:[BX] or [BX] for pointer args.
;
; --- SMALL MODEL ---
;
if (_large_code eq 0) and (_large_data eq 0)

 _rgf	equ	2
 long	equ	0

 cseg macro
  pgroup group prog
  prog segment byte public 'prog'
  assume cs:pgroup
 endm

 endc macro
  prog ends
 endm

 dseg macro
  dgroup group data
  data segment byte public 'data'
  assume ds:dgroup
 endm

 endd macro
  data ends
 endm

  %out Small Model
;
;Define a function as near or far.
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
endif
;
; --- MEDIUM MODEL ---
;
if (_large_code eq 1) and (_large_data eq 0)

 _rgf	equ	4
 long	equ	0

 cseg macro
  _code segment byte
  assume cs:_code
 endm

 endc macro
  _code ends
 endm

 dseg macro
  dgroup group data
  data segment byte public 'data'
  assume ds:dgroup
 endm

 endd macro
  data ends
 endm

  %out Medium Model
;
;Define a function as near or far.
;
 func macro procname
  public procname
  procname proc far
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
endif
;
; --- LARGE MODEL ---
;
if (_large_code eq 0) and (_large_data eq 1)

 _rgf	equ	2
 long	equ	1

 cseg macro
  cgroup group code
  code segment byte public 'code'
  assume cs:cgroup
 endm

 endc macro
  code ends
 endm

 dseg macro
  dgroup group data
  data segment byte public 'data'
  assume ds:dgroup
 endm

 endd macro
  data ends
 endm

  %out Large Model
;
;Define a function as near or far.
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
endif
;
; --- EXTRA LARGE MODEL ---
;
if (_large_code eq 1) and (_large_data eq 1)

 _rgf	equ	4
 long	equ	1
 cseg macro
  _code segment byte
  assume cs:_code
 endm

 endc macro
  _code ends
 endm

 dseg macro
  dgroup group data
  data segment byte public 'data'
  assume ds:dgroup
 endm

 endd macro
  data ends
 endm

  %out Extra Large Model
;
;Define a function as near or far.
;
 func macro procname
  public procname
  procname proc far
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
endif
;
;Define the args on the stack.
;
arg0	equ	[bp+_rgf+2]
arg1	equ	[bp+_rgf+4]
arg2	equ	[bp+_rgf+6]
arg3	equ	[bp+_rgf+8]
arg4	equ	[bp+_rgf+10]
arg5	equ	[bp+_rgf+12]
arg6	equ	[bp+_rgf+14]
arg7	equ	[bp+_rgf+16]
arg8	equ	[bp+_rgf+18]
arg8	equ	[bp+_rgf+20]
