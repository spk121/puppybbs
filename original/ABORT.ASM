title Program Abort Functions
name abort
include model.ash
include lattice.ash
;
;Multi-level program error return functions.
;
;set_abor()
;
;Set to return to after set_abort() if a
;function calls frc_abort(). This assumes
;that the first thing after set_abort() is
;was_abort(), like so:
;
;	set_abort(0);
;	if(was_abort()) {
;		do whatever ...
;	}
;	function();
;
;Where function() is presumably the actual
;function that invokes frc_abort, or one of
;its called functions.
;
; The argument to set_abort(arg) is necessary,
;otherwise Lattice will not put SP into BP
;(trying to be smart.)
;
;frc_abor()
;
;Does a jump to the abort address presumably
;set by set_abor(). 
;
;was_abor()
;
;Returns true if return from the function was
;done by the frc_abor function. Once read,
;it clears itself to zero.
;
;NOTE:
;	These are currently hardcoded for small
;and large models; 16 bit code addresses only.
;
;	Lattice puts SP into BP before calling
;a function; all we have to do to restore the
;original stack pointer is to save this BP
;value and use it later. 
;
dseg

abflag	dw	(?)	;1 == aborted
rbp	dw	(?)	;BP save
rds	dw	(?)
res	dw	(?)
addr	dw	0	;func address

endd

cseg
;
;Set the abort jump address to after func()
;(Cant use macros FUNC and ENDF since we mush
;BP etc.)
;
public set_abor

set_abor proc
	mov	dx,bp		;orig. BP,
	push	bp		;into DX
	mov	bp,sp
	mov	bx,ds		;orig. DS
	push	ds		;into BX
	mov	ax,data
	mov	ds,ax
	mov	ax,[bp+2]	;get our return
	mov	addr,ax		;address,
	mov	rbp,dx		;save BP,
	mov	rds,bx		;save DS,
	mov	res,es		;save ES,
	mov	abflag,0	;no abort
	pop	ds
	pop	bp
	ret
set_abor endp
;
;Execute an abort.
;
public frc_abor
frc_abor proc 
	mov	ax,data
	mov	ds,ax
	push	addr		;set ret addr
	mov	abflag,1	;flag abort,
	mov	bp,rbp		;get old BP,
	mov	es,res		;DS and ES
	mov	ds,rds
	ret			;return old BP
frc_abor endp
;
;Return true if the error abort was taken.
;
func was_abor
	mov	ax,data
	mov	ds,ax
	mov	ax,0
	xchg	ax,abflag

endf was_abor

endc

	end
