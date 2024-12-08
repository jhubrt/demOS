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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\CASCADE.PRX

	opt		o+


cpu:		set		0
pitch:		set		168
fontwbyte:	set		16
fonth:		set		54
fontspace:	set		4
*-------------------------------------------------------------------------*

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
	dc.l	init
	dc.l	update
	dc.l	shutdown
	dc.l	hibordervbl

init:

	move.l	(a0),cascade
	move.l  4(a0),font
	move.l  8(a0),pal

	move.l	#hbl,$68.w

	rts

update:
	ifne		cpu
	move.w		#$70,$ffff8240.w
	endc
	
	movem.l	d0-a6,-(sp)

	not.w	flip_ec
	move.w	flip_ec(pc),d0
	ext.l	d0
	and.l	#274*pitch,d0
	add.l	cascade(pc),d0
	move.l	d0,physb

	swap	d0
	move.b	d0,$ffff8201.w
	swap	d0
	ror.w	#8,d0
	move.b	d0,$ffff8203.w
	lsr.w	#8,d0
	move.b	d0,$ffff820d.w

	move.l	physb(pc),a1			*---- SCROLLTEXT ----*
	lea		pitch-fontwbyte(a1),a2
	lea		pitch(a1),a0
	move.w	#272,d0
.copyvert:	
	rept	fontwbyte/4
	move.l	(a0)+,d1
	move.l	d1,(a1)+
	move.l	d1,(a2)+
	endr
	lea		pitch-fontwbyte(a0),a0
	lea		pitch-fontwbyte(a1),a1
	lea		pitch-fontwbyte(a2),a2
	dbra.w	d0,.copyvert

	cmp.w	#(fonth+fontspace)*fontwbyte,cptfont
	bne.s	.okcptext
	clr.w	cptfont

	addq.w	#1,cptext
	move.w	cptext(pc),d0
	lea		text(pc),a0
	move.b	(a0,d0.w),d0
	cmp.b	#48,d0
	bne.s	.oktext
	clr.w	cptext
	move.l	#26*fonth*fontwbyte,adrfont
	bra.s	.okcptext
.oktext:
	cmp.b	#32,d0
	bne.s	.other
	move.l  #26*fonth*fontwbyte,adrfont
	bra.s	.okcptext
.other:
	sub.b	#65,d0
	ext.w	d0
	mulu.w	#fonth*fontwbyte,d0
	move.l	d0,adrfont
.okcptext:

	move.l	font(pc),a0

	cmp.w	#fonth*fontwbyte,cptfont
	blt.s	.nospacing
	add.l	#fonth*26*fontwbyte,a0
	bra.s	.displayfont
.nospacing:
	add.l	adrfont(pc),a0
	add.w	cptfont(pc),a0
.displayfont:

	move.l	physb(pc),a1
	add.l	#273*pitch,a1
	rept	6
	move.l	(a0)+,(a1)+
	endr

	add.w	#fontwbyte,cptfont

	movem.l	(sp)+,d0-a6

	ifne	cpu
	move.w	#0,$ffff8240.w
	endc

shutdown:
	
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

;-------------------------------------------------------------------------
text:	dc.b	'  '
		dc.b	'NO VIOLENCE     '
		dc.b	'NO COMPUTER     '
		dc.b	'JUST SILENCE     '
		dc.b	'AND THE SOUND OF WATER     '
		dc.b	'         '
		dc.b	'SOME CRIES IN THE NIGHT     '
		dc.b	'BUT YOU NEED NOT FEEL BLUE     '
		dc.b	'SOMEWHERE IS THE LIGHT     '
		dc.b	'AND SOMEONE WAITS FOR YOU     '
		dc.b	'                0'

		even	

zero:		ds.l	8
flip_ec:	dc.w	0
physb:		dc.l	0
cptext:		dc.w	0
cptfont:	dc.w	0
adrfont:	dc.l	26*fontwbyte*fonth
font:		dc.l	0
pal:		dc.l	0
cascade:	dc.l	0
