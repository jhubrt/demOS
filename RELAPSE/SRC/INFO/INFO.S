*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2024 J.Hubert
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
* and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation the rights to use, 
* copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
* and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies 
* or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*--------------------------------------------------------------------------------------------------------

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\INFO.PRX

	opt		o+


cpu:		set		0
pitch:		set		168
vspacing:	set		11
*-------------------------------------------------------------------------*

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
	dc.l	init
	dc.l	display
	dc.l	hibordervbl

init:
	move.l	#hbl,$68.w
	rts

display:
	movem.l	d0-d7/a0-a6,-(sp)

	move.l	(a0),a2			; text frame buffer
	move.l  8(a0),a1		; font
	move.l  4(a0),a0		; text

	move.l	a2,a3
	
	moveq.l	#0,d0

.loop:
	move.b	(a0)+,d0
	beq 	.end

	cmp.b	#13,d0
	bgt.s	.noCR
	lea		168*vspacing(a3),a3
	move.l	a3,a2
	bra.s	.loop
.noCR:

	sub.b	#' ',d0
	bne.s	.nospace
	move.w	#64*39,d1
	bra.s	.draw
.nospace:

	subq.b	#'!'-' ',d0  
	bne.s	.noexclamation
	move.w	#64*38,d1
	bra.s	.draw
.noexclamation:

	sub.b	#','-'!',d0
	bne.s	.nocomma
	move.w	#64*37,d1
	bra.s	.draw
.nocomma:

	subq.b	#'.'-',',d0
	bne.s	.nodot
	move.w	#64*36,d1
	bra.s	.draw
.nodot:

	subq.b	#'0'-'.',d0 
	cmp.b	#9,d0	
	bgt.s	.nonumber
	move.w	d0,d1
	add.w	#26,d1
	lsl.w	#6,d1
	bra.s	.draw
.nonumber:
	
	sub.b	#'A'-'0',d0
	move.w	d0,d1
	lsl.w	#6,d1

.draw:
	lea		(a1,d1.w),a4

	movep.l	0(a4),d2
	movep.l	d2,(a2)

	movep.l	8(a4),d2
	movep.l	d2,168(a2)

	movep.l	16(a4),d2
	movep.l	d2,2*168(a2)

	movep.l	24(a4),d2
	movep.l	d2,3*168(a2)

	movep.l	32(a4),d2
	movep.l	d2,4*168(a2)

	movep.l	40(a4),d2
	movep.l	d2,5*168(a2)

	movep.l	48(a4),d2
	movep.l	d2,6*168(a2)

	movep.l	56(a4),d2
	movep.l	d2,7*168(a2)

	move.w	a2,d2
	and.w	#1,d2
	bne.s	.byte2
	addq.l	#1,a2
	bra		.loop
.byte2:
	addq.l	#7,a2
	bra		.loop

.end:

	movem.l	(sp)+,d0-d7/a0-a6

	rts

;-------------------------------------------------------------------------
; High border
;-------------------------------------------------------------------------

timerb1:
	move.l	a5,-(sp)
	move.w	#$2100,sr			; \
	stop	#$2100				;  > Synchro HBL
	move.w	#$2700,sr			; /
	lea	$ffff820a.w,a5			; A5 : adr modes de synchro
	rept 	82					; \
	nop							;  > Attend 82 nops
	endr						; /
	move.b	#0,(a5)				; Passe en 60 Hz interne
	rept	15					; \
	nop							;  > Attend 15 nops
	endr						; /
	move.b	#2,(a5)				; Passe en 50 Hz externe
	clr.b	$fffffa1b.w			; Arret Timer B
	move.b	#228,$fffffa21.w	; 228 lignes
	move.b	#8,$fffffa1b.w		; mode compteur d'evenements
	move.l	#timerb2,$120.w		; Revectorise Timer B
	move.l	(sp)+,a5

	rte							; Retour s/p it

;-------------------------------------------------------------------------
; Low border
;-------------------------------------------------------------------------

timerb2:
	move.l	a5,-(sp)
	move.w	d6,-(sp)
	lea	$fffffa21.w,a5			; Adr data register timer B
	move.b	(a5),d6				; D6 = valeur dans TBDR
.sync:	
	cmp.b	(a5),d6				; Synchronisation sur la
	beq.s	.sync				; ligne 199

	clr.b	$ffff820a.w			; Passe en 60 Hz
	rept	16					; \ 
	nop							;  > Attend 10 nops   
	endr						; /  
	move.b	#2,$ffff820a.w		; Passe en 50 Hz     
	bclr.b	#0,$fffffa07.w		; Stop it timer B
	move.w	(sp)+,d6
	move.l	(sp)+,a5

hbl:
	rte							; Fin routine it

;-------------------------------------------------------------------------
hibordervbl:
		
	clr.b	$fffffa1b.w			; Arret Timer B
	move.b	#$64,$fffffa21.w	; $65 dans registre de donn‚es
	move.b	#4,$fffffa1b.w		; 200 pr‚-divisions
	bset.b	#0,$fffffa07.w		; Autorise it Timer B
	move.l	#timerb1,$120.w		; Revectorise Timer B

	move.l  (a0)+,a1
	jmp		(a1)

