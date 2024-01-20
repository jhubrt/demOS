*-------------------------------------------------------------------------*
* Pro-Tracker Replay by Wizzcat/Delta Force
*
* Optimized by Metal Ages 1993
*-------------------------------------------------------------------------*
* - Move from timer to VBL
* - Automodified code to transpose and mix
* - Interlace instead of adding (25khz mix with DMA at 50khz)
* - Use less than 45%, rather constant execution time
* - Routine does not take too much memory (43kb)
*-------------------------------------------------------------------------*

	output 	D:\PROJECTS\DEMOS\OUTPUT\PRTRKSTE.O

	xdef 	WIZinit,WIZmodInit,WIZplay,WIZrundma,WIZstereo,WIZfrontbuf,WIZbackbuf,WIZgetInfo,WIZjump

*-------------------------------------------------------------------------*
cpu:		set	0	* Time cpu	
*-------------------------------------------------------------------------*
nbre_div:	set	4	* Len/nbre_div doit etre un entier
mvol:		equ	$40	* Main volume				
*-------------------------------------------------------------------------*
LEN			set 500
INC			set $008EFB4E			; 3579546/25035*65536

*-------------------------------------------------------------------------*
*	REPLAY ROUT
*-------------------------------------------------------------------------*
	opt	o+
*-------------------------------------------------------------------------*

WIZinit:	

	movem.l	a0-a6/d0-d7,-(sp)

	bsr		vol
	bsr		incrcal			* Calcul table increments
	
	movem.l	(sp)+,a0-a6/d0-d7
	
	rts

*-------------------------------------------------------------------------*
WIZmodInit:

	movem.l	a2-a6/d0-d7,-(sp)
	
	move.l	a0,data
	move.l	a1,workspc

	move.l	#sample1,wiz1lc		
	clr.l	wiz1len			
	clr.w	wiz1rpt	
	clr.w	wiz1pos	
	clr.w	wiz1frc

	move.l	#sample1,wiz2lc
	clr.l	wiz2len
	clr.w	wiz2rpt	
	clr.w	wiz2pos	
	clr.w	wiz2frc

	move.l	#sample1,wiz3lc
	clr.l	wiz3len
	clr.w	wiz3rpt	
	clr.w	wiz3pos	
	clr.w	wiz3frc

	move.l	#sample1,wiz4lc
	clr.l	wiz4len
	clr.w	wiz4rpt	
	clr.w	wiz4pos	
	clr.w	wiz4frc

	move.l	#dummy,aud1lc
	clr.w	aud1len
	clr.w	aud1per
	clr.l	aud1vol
	clr.l	aud1vol+4

	move.l	#dummy,aud2lc
	clr.w	aud2len
	clr.w	aud2per
	clr.l	aud2vol
	clr.l	aud2vol+4

	move.l	#dummy,aud3lc
	clr.w	aud3len
	clr.w	aud3per
	clr.l	aud3vol
	clr.l	aud3vol+4

	move.l	#dummy,aud4lc
	clr.w	aud4len
	clr.w	aud4per
	clr.l	aud4vol
	clr.l	aud4vol+4
	
	move.l	#sample1,samp1
	move.l	#sample2,samp2
	
	move.w	#6,speed
	clr.w	counter
	clr.b	songpos
	clr.b	break	
	clr.w	pattpos	
	clr.w	dmacon

	lea	voice1(pc),a0
	move.w	#(voice_end-voice1)/2-1,d0
.clear_vox:
	clr.w	(a0)+
	dbf.w	d0,.clear_vox

	move.w	#1,voice1+20
	move.w	#2,voice2+20
	move.w	#4,voice3+20
	move.w	#8,voice4+20

	lea	samplestarts(pc),a0
	moveq.l	#30,d0
.starts:	clr.l	(a0)+
	dbf.w	d0,.starts	

	bsr		init			* Initialise routine music
	bsr		prepare			* Prepare module (samples)
		
	movem.l	(sp)+,a2-a6/d0-d7

	rts

WIZgetInfo:
	move.w	d0,-(sp)
	move.b	songpos(pc),(a0)
	move.w	pattpos(pc),d0
	lsr.w	#4,d0
	move.b	d0,1(a0)
	move.w	(sp)+,d0
	rts

WIZjump:
	subq.b	#1,d0
	move.b	d0,songpos
	st		break
	rts


*----------------------------------------------------------VOLUME TABLE
vol:	
	moveq.l	#64,d0			* approxim�
	
	move.l  #vtabend,d2		* Align table on 256 bytes
	and.l	#$FFFFFF00,d2
	move.l	d2,a0
	sub.l	#vtabend-vtab,d2
	move.l  d2,vtabptr
.ploop:	
	move.w	#255,d1
.mloop:	
	move.w	d1,d2
	ext.w	d2
	muls.w	d0,d2
	divs.w	#mvol,d2		; <---- Master volume
	move.b	d2,-(a0)
	dbra.w	d1,.mloop
	dbra.w	d0,.ploop
	rts

*-------------------------------------------------------INCREMENT TABLE
incrcal:
	lea	stab(pc),a0
	move.w	#$30,d1
	move.w	#$039F-$30,d0
	move.l	#INC,d2			* Frequence de ref (selon freq replay)
	lea	stab2(pc),a1		* Pointe sur la table de correction
	lea	stabvar(pc),a3		* Pointe sur la table de correction 2
	
recalc:	
	swap	d2
	moveq.l	#0,d3			
	move.w	d2,d3			
	divu.w	d1,d3
	move.w	d3,d4
	move.w	d3,a2			* Coef
	swap	d4

	swap	d2
	move.w	d2,d3			* Taux d'erreur
 	divu.w	d1,d3
	move.w	d3,d4
	move.l	d4,(a0)+

	exg.l	d3,a2
	moveq.l	#0,d6			
	moveq.l	#0,d7
	move.w	#LEN-1,d5		* On calcule l'offset auquel on arriverait
calc10:	
	add.w	a2,d6	
	addx.w	d3,d7		
	dbra.w	d5,calc10
	move.w	d7,(a1)+		* Offset auquel on parvient
	move.w	d6,(a1)+		* Taux d'erreur auquel on parvient
					* dans Stab2 (freq * 4 & .w + .w)
	moveq.l	#0,d6			
	move.l	a3,a4			* Stab var + freq *2 dans a4
	move.w	#nbre_div-2,d4
calc4:	
	move.w	#LEN/nbre_div-1,d5	* On calcule l'offset auquel on arriverait
	moveq.l	#0,d7
calc3:	
	add.w	a2,d6			*\
	addx.w	d3,d7			* \ Calcul de l'offset theorique
	dbra.w	d5,calc3		* /
	move.w	d7,(a4)			* On le note	
	lea	$03a0*2(a4),a4
	dbra.w	d4,calc4		* nbre_div-1 fois
	addq.w	#2,a3			* Table + 2 * freq
		
	ifne	cpu
	bchg.b	#1,$ffff8240+1.w
	endc

	addq.w	#1,d1
	dbra.w	d0,recalc
	
	rts


WIZplay:
	movem.l		d0-d1/a0,-(sp)

	moveq.l		#0,d0
	move.w		#((sample2_end-sample1)/4)-1,d1
	lea			sample1(pc),a0
.eff:
	move.l		d0,(a0)+
	dbra.w		d1,.eff

	movem.l		(sp)+,d0-d1/a0

	move.w		#3,$FFFF8920.w		; DMA 50khz stereo

	rts


;-------------------------------------------------------------------------
; VBL interupt
;-------------------------------------------------------------------------
WIZrundma:
	move.b	#1,$FFFF8901.w		* DMA run once
    move.l  (a0)+,a1
    jmp     (a1)

WIZstereo:

	ifne	cpu
	move.w	#$7,$ffff8240.w			
	endc

	movem.l	d0-d7/a0-a6,-(sp)

	; Flip sample buffer
	; -----------------------------------------------------

	move.l	samp2(pc),d0
	move.l	samp1(pc),samp2
	move.l	d0,samp1

	move.l	d0,d1					; d0 = d1 = dma sound buffer to set for next vbl 
	moveq.l	#8,d2

	; Set DMA sound registers
	; -----------------------------------------------------

	lea		$FFFF8907.w,a0
	move.b	d0,(a0)
	lsr.w	d2,d0
	move.l	d0,-5(a0)			* Start
	
	add.l	#LEN*4,d1
	move.b	d1,12(a0)
	lsr.w	d2,d1
	move.l	d1,7(a0)			* End

	move.w	#$2300,sr

	; Score management
	bsr		music

	; LEFT --------------------------------------------------

	lea		itab(pc),a5
	lea		itab2(pc),a6

	* d3, d7, d0, d1 transient
	* automod params : 
	* 	- ch 2 & 1 : d2 = ratio a4 = error d1 = last error d0 = last offset
	*	- ch 3 & 4 : d6 = ratio a2 = error d5 = last error d4 = last offset
	*	- a3: mix routine address to automodify
	* resulting byte order 2134

	moveq.l	#0,d0
	move.l  wiz2lc(pc),a0
	move.w	wiz2pos(pc),d0		* Dernier offset sur sample
	move.w	wiz2frc(pc),d1		* Dernier taux d'erreur
	add.l   d0,a0
	move.l  a0,cursampl2
	
	move.w	aud2per(pc),d7		* Selon la note
	add.w	d7,d7				* on choisit
	add.w	d7,d7
	move.l  (a5,d7.w),d2		* d2 = ratio
	move.w  d2,a4				* a4 = taux d'erreur
	swap 	d2
	; move.w	0(a5,d7.w),d2		
	; movea.w	2(a5,d7.w),a4		

	; manage loop	
    moveq.l	#0,d3				
	move.w	0(a6,d7.w),d3		* Correction offset
	add.l	d0,d3
	cmp.l	wiz2len(pc),d3		* Test si on depasse la longueur du sample
	blt.s	.ok2
	sub.w	wiz2rpt(pc),d3		* dans lequel cas => gestion boucle
.ok2:
	move.w	d3,wiz2pos			* On note l'offset atteint
	move.w	2(a6,d7.w),d3		* Correction du taux d'erreur
	add.w	d3,wiz2frc

	; ---------------------
	moveq.l	#0,d4	
	move.l  wiz3lc(pc),a0
	move.w	wiz3pos(pc),d4
	move.w	wiz3frc(pc),d5
	add.l   d4,a0
	move.l  a0,cursampl3

	move.w	aud3per(pc),d7
	add.w	d7,d7
	add.w	d7,d7
	move.l  (a5,d7.w),d6		* d6 = ratio
	move.w  d6,a2				* a2 = taux d'erreur
	swap	d6
	; move.w	0(a5,d7.w),d6
	; movea.w	2(a5,d7.w),a2

	; manage loop	
	moveq.l	#0,d3				* Table de correction VBL
	move.w	0(a6,d7.w),d3
	add.l	d4,d3
	cmp.l	wiz3len(pc),d3
	blt.s	.ok3
	sub.w	wiz3rpt(pc),d3
.ok3:
	move.w	d3,wiz3pos
	move.w	2(a6,d7.w),d3
	add.w	d3,wiz3frc
	
	lea		computesampleloop+2+8(pc),a3
	bsr		automodrout

	; RIGHT --------------------------------------------------

	moveq.l	#0,d0
	move.l  wiz1lc(pc),a0
	move.w	wiz1pos(pc),d0
	move.w	wiz1frc(pc),d1
	add.l   d0,a0
	move.l  a0,cursampl1

	move.w	aud1per(pc),d7
	add.w	d7,d7
	add.w	d7,d7
	move.l  (a5,d7.w),d2		* d2 = ratio
	move.w  d2,a4				* a4 = taux d'erreur
	swap 	d2
	;move.w	0(a5,d7.w),d2
	;movea.w	2(a5,d7.w),a4
	
    moveq.l	#0,d3
	move.w	0(a6,d7.w),d3
	add.l	d0,d3
	cmp.l	wiz1len(pc),d3
	blt.s	.ok1
	sub.w	wiz1rpt(pc),d3
.ok1:
	move.w	d3,wiz1pos
	move.w	2(a6,d7.w),d3
	add.w	d3,wiz1frc

	;-------------------------
	moveq.l	#0,d4
	move.l  wiz4lc(pc),a0
	move.w	wiz4pos(pc),d4
	move.w	wiz4frc(pc),d5
	add.l   d4,a0
	move.l  a0,cursampl4

	move.w	aud4per(pc),d7
	add.w	d7,d7
	add.w	d7,d7
	move.l  (a5,d7.w),d6		* d6 = ratio
	move.w  d6,a2				* a2 = taux d'erreur
	swap	d6
	;move.w	0(a5,d7.w),d6
	;movea.w	2(a5,d7.w),a2

    moveq.l	#0,d3				* Table de correction voix 4
	move.w	0(a6,d7.w),d3
	add.l	d4,d3
	cmp.l	wiz4len(pc),d3
	blt.s	.ok4
	sub.w	wiz4rpt(pc),d3
.ok4:
	move.w	d3,wiz4pos
	move.w	2(a6,d7.w),d3
	add.w	d3,wiz4frc

	lea		computesampleloop+2(pc),a3
	bsr		automodrout

	; Init correction table addresses into pointers storage
	; -----------------------------------------------------

	lea		offsetsptr(pc),a6

	lea		itabvar(pc),a4

	move.w	aud1per(pc),d3
	add.w	d3,d3
	lea		(a4,d3.w),a5
	move.l	a5,(a6)+

	move.w	aud2per(pc),d3
	add.w	d3,d3
	lea		(a4,d3.w),a5		
	move.l	a5,(a6)+

	move.w	aud4per(pc),d3
	add.w	d3,d3
	lea		(a4,d3.w),a5
	move.l	a5,(a6)+

	move.w	aud3per(pc),d3
	add.w	d3,d3
	lea		(a4,d3.w),a5
	move.l	a5,(a6)+

	; Init tables
	; -----------------------------------------------------

	move.l	vtabptr(pc),a3

	move.w	aud1vol(pc),d7		* aud2vol : d3 - a0
	asl.w	#8,d7
	lea		0(a3,d7.w),a6
	move.l	a6,d3
	movea.l	cursampl1(pc),a0

	move.w	aud2vol(pc),d7		* aud3vol : d4 - a1
	asl.w	#8,d7
	lea		0(a3,d7.w),a6
	move.l	a6,d4
	movea.l	cursampl2(pc),a1
	 
	move.w	aud4vol(pc),d7		* aud1vol : d5 - a2
	asl.w	#8,d7
	lea		0(a3,d7.w),a6
	move.l	a6,d5
	movea.l	cursampl4(pc),a2

	move.w	aud3vol(pc),d7		* aud4vol : d6 - a3
	asl.w	#8,d7
	lea		0(a3,d7.w),a6
	move.l	a6,d6
	movea.l	cursampl3(pc),a3

	movea.l	samp1(pc),a6

	; compute samples routine parameters :
	; - a6 = dma buffer destination
	; - ch2 a0 source d3 volume table range
	; - ch1 a1 source d4 volume table range
	; - ch3 a2 source d5 volume table range
	; - ch4 a3 source d6 volume table range
	; locals :
	; - nbloops d7
	; - offsetsptr table address d2
	; - transient a4

	moveq.l	#nbre_div-1,d7
	lea		offsetsptr(pc),a4
	move.l  a4,d2

computesampleloop:

	REPT LEN/nbre_div
	
	move.b	2(a0),d3		;	12		ch2 
	move.l	d3,a4			;	4
	move.b	(a4),(a6)+		;	12
	
	move.b	2(a1),d4		;			ch1
	move.l	d4,a4
	move.b	(a4),(a6)+

	move.b	2(a2),d5		; 			ch3
	move.l	d5,a4
	move.b	(a4),(a6)+
	
	move.b	2(a3),d6		;			ch4
	move.l	d6,a4
	move.b	(a4),(a6)+
		
	ENDR

	move.l		d2,a5

	move.l		(a5),a4				* ch2
	add.w		(a4),a0				* Correction 
	lea			$3a0*2(a4),a4		* offsets sample
	move.l		a4,(a5)+

	move.l		(a5),a4				* ch1
	add.w		(a4),a1				* Correction 
	lea			$3a0*2(a4),a4		* offsets sample
	move.l		a4,(a5)+

	move.l		(a5),a4				* ch3
	add.w		(a4),a2				* Correction 
	lea			$3a0*2(a4),a4		* offsets sample
	move.l		a4,(a5)+

	move.l		(a5),a4				* ch4
	add.w		(a4),a3				* Correction 
	lea			$3a0*2(a4),a4		* offsets sample
	move.l		a4,(a5)+

	dbra.w	d7,computesampleloop
	
	; end ------------------------

	movem.l	(sp)+,d0-d7/a0-a6	* Restore les registres

	ifne cpu
	clr.w	$ffff8240.w
	endc

    move.l  (a0)+,a1
    jmp     (a1)

*------------------------------------- AUTOMOD & SAMPLE CALC ROUTINES 

var:	set	0

automodrout:
	moveq.l	#0,d0
	moveq.l	#0,d4

	rept	LEN/nbre_div

	add.w	a4,d1
	addx.w	d2,d0
	add.w	a2,d5
	addx.w	d6,d4

	move.w	d0,var(a3)
	move.w	d4,var+16(a3)

var:	set var+32	
	ENDR

	rts

offsetsptr:
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0


	opt	o-

*-------------------------------------------- HARDWARE-REGISTERS & DATA
cursampl1:  DC.l 0
cursampl2:  DC.l 0
cursampl3:  DC.l 0
cursampl4:  DC.l 0

wiz1lc		DC.L sample1
wiz1len		DC.L 0
wiz1rpt		DC.W 0
wiz1pos		DC.W 0
wiz1frc		DC.W 0

wiz2lc	DC.L sample1
wiz2len	DC.L 0
wiz2rpt	DC.W 0
wiz2pos	DC.W 0
wiz2frc	DC.W 0

wiz3lc	DC.L sample1
wiz3len	DC.L 0
wiz3rpt	DC.W 0
wiz3pos	DC.W 0
wiz3frc	DC.W 0

wiz4lc	DC.L sample1
wiz4len	DC.L 0
wiz4rpt	DC.W 0
wiz4pos	DC.W 0
wiz4frc	DC.W 0

aud1lc	DC.L dummy
aud1len	DC.W 0
aud1per	DC.W 0
aud1vol	DC.W 0
	DS.W 3

aud2lc	DC.L dummy
aud2len	DC.W 0
aud2per	DC.W 0
aud2vol	DC.W 0
	DS.W 3

aud3lc	DC.L dummy
aud3len	DC.W 0
aud3per	DC.W 0
aud3vol	DC.W 0
	DS.W 3

aud4lc	DC.L dummy
aud4len	DC.W 0
aud4per	DC.W 0
aud4vol	DC.W 0

dummy	DC.L 0

WIZbackbuf:
samp1:	DC.L sample1
WIZfrontbuf:
samp2:	DC.L sample2

sample1:	DS.W 	LEN*2
			DS.W	8	
sample2:	DS.W	LEN*2
			DS.W	8	
sample2_end:
*--------------------------------------------------- INIT MODULE
prepare:
	move.l	workspc(pc),a6
	movea.l	samplestarts(pc),a0
	movea.l	end_of_samples(pc),a1

tostack:move.w	-(a1),-(a6)
	cmpa.l	a0,a1			; Move all samples to stack
	bgt.s	tostack

	lea	samplestarts(pc),a2
	move.l	data(pc),a1		; Module
	movea.l	(a2),a0			; Start of samples
	movea.l	a0,a5			; Save samplestart in a5

	moveq	#30,d7

roop:	move.l	a0,(a2)+		; Sampleposition

	tst.w	$2A(a1)
	beq.s	samplok			; Len=0 -> no sample

	tst.w	$2E(a1)			; Test repstrt
	bne.s	repne			; Jump if not zero


repeq:	move.w	$2A(a1),d0		; Length of sample
	move.w	d0,d4
	subq.w	#1,d0

	movea.l	a0,a4
fromstk:
	move.w	(a6)+,(a0)+		;  Move all samples back from stack => keep on 8 bits as we mix by interlacing		
	dbra	d0,fromstk

	bra.s	rep

repne:	move.w	$2E(a1),d0
	move.w	d0,d4
	subq.w	#1,d0

	movea.l	a6,a4
get1st:	
	move.w	(a4)+,(a0)+		;  Move all samples back from stack => keep on 8 bits as we mix by interlacing		
	dbra	d0,get1st

	adda.w	$2A(a1),a6		; Move a6 to next sample
	adda.w	$2A(a1),a6

rep:	movea.l	a0,a5
	moveq	#0,d1
toosmal:movea.l	a4,a3
	move.w	$30(a1),d0
	subq.w	#1,d0
moverep:
	move.w	(a3)+,(a0)+		; Move all samples back from stack

	addq.w	#2,d1
	dbra	d0,moverep
	cmp.w	#320*5,d1			; Must be > 320
	blt.s	toosmal

	move.w	#320*5/2-1,d2
last320:
	move.w	(a5)+,(a0)+			; Move all samples back from stack
	dbra.w	d2,last320

done:	add.w	d4,d4

	move.w	d4,$2A(a1)		; length
	move.w	d1,$30(a1)		; Replen
	clr.w	$2E(a1)

samplok:lea	$1E(a1),a1
	dbra.w	d7,roop

	cmp.l	workspc(pc),a0
	bgt.s	.nospac

	rts

.nospac:	illegal

end_of_samples:	DC.L 0

*--------------------------------------------------- INIT SAMPLE ADR TABLE
init:	
	move.l	data(pc),a0
	lea	$03B8(a0),a1

	moveq.l	#$7F,d0
	moveq.l	#0,d1
loop:	move.l	d1,d2
	subq.w	#1,d0
lop2:	move.b	(a1)+,d1
	cmp.b	d2,d1
	bgt.s	loop
	dbra.w	d0,lop2
	addq.b	#1,d2

	lea	samplestarts(pc),a1
	asl.l	#8,d2
	asl.l	#2,d2
	add.l	#$043C,d2
	add.l	a0,d2
	movea.l	d2,a2

	moveq.l	#$1E,d0
lop3:	clr.l	(a2)
	move.l	a2,(a1)+
	moveq	#0,d1
	move.w	42(a0),d1
	add.l	d1,d1
	adda.l	d1,a2
	adda.l	#$1E,a0
	dbra.w	d0,lop3

	move.l	a2,end_of_samples	;
	rts

*-------------------------------------------------------------------------*
*	MUSIC		Optimized version 0.5				  *
*-------------------------------------------------------------------------*

	opt	a+,o+,p+

music:
	move.l	data(pc),a0
	lea		counter(pc),a6
	addq.w	#$01,(a6)
	move.w	(a6),d0
	cmp.w	speed(pc),d0
	blt.s	nonew
	clr.w	(a6)
	bra	getnew

nonew	lea	voice1(pc),a4
	lea	aud1lc(pc),a3
	bsr	checkcom
	lea	voice2(pc),a4
	lea	aud2lc(pc),a3
	bsr	checkcom
	lea	voice3(pc),a4
	lea	aud3lc(pc),a3
	bsr	checkcom
	lea	voice4(pc),a4
	lea	aud4lc(pc),a3
	bsr	checkcom
	bra	endr

arpeggio
	moveq	#0,d0
	move.w	(a6),d0
	divs	#$03,d0
	swap	d0
	tst.w	d0
	beq.s	arp2
	cmp.w	#$02,d0
	beq.s	arp1

	moveq	#0,d0
	move.b	$03(a4),d0
	lsr.b	#4,d0
	bra.s	arp3

arp1	moveq	#0,d0
	move.b	$03(a4),d0
	and.b	#$0F,d0
	bra.s	arp3

arp2	move.w	$10(a4),d2
	bra.s	arp4

arp3	add.w	d0,d0
	moveq	#0,d1
	move.w	$10(a4),d1
	lea	periods(pc),a0
	moveq	#$24,d4
arploop	move.w	0(a0,d0.w),d2
	cmp.w	(a0),d1
	bge.s	arp4
	addq.l	#2,a0
	dbra	d4,arploop
	rts

arp4	move.w	d2,$06(a3)
	rts

getnew	move.l	data(pc),a0
	lea	$43c(a0),a0
	lea	-$043C+$0C(a0),a2
	lea	-$043C+$03B8(a0),a1

	moveq	#0,d0
	move.l	d0,d1
	move.b	songpos(pc),d0
	move.b	0(a1,d0.w),d1
	asl.l	#8,d1
	asl.l	#2,d1
	add.w	pattpos(pc),d1
	clr.w	dmacon-counter(a6)

	lea	aud1lc(pc),a3
	lea	voice1(pc),a4
	bsr.s	playvoice
	lea	aud2lc(pc),a3
	lea	voice2(pc),a4
	bsr.s	playvoice
	lea	aud3lc(pc),a3
	lea	voice3(pc),a4
	bsr.s	playvoice
	lea	aud4lc(pc),a3
	lea	voice4(pc),a4
	bsr.s	playvoice
	bra	setdma

playvoice
	move.l	0(a0,d1.l),(a4)
	addq.l	#4,d1
	moveq	#0,d2
	move.b	$02(a4),d2
	and.b	#$F0,d2
	lsr.b	#4,d2
	move.b	(a4),d0
	and.b	#$F0,d0
	or.b	d0,d2
	tst.b	d2
	beq.s	setregs
	moveq	#0,d3
	lea	samplestarts(pc),a1
	move.l	d2,d4
	subq.l	#$01,d2
	asl.l	#2,d2
	mulu	#$1E,d4
	move.l	0(a1,d2.l),$04(a4)
	move.w	0(a2,d4.l),$08(a4)
	move.w	$02(a2,d4.l),$12(a4)
	move.w	$04(a2,d4.l),d3
	tst.w	d3
	beq.s	noloop
	move.l	$04(a4),d2
	add.w	d3,d3
	add.l	d3,d2
	move.l	d2,$0A(a4)
	move.w	$04(a2,d4.l),d0
	add.w	$06(a2,d4.l),d0
	move.w	d0,8(a4)
	move.w	$06(a2,d4.l),$0E(a4)
	move.w	$12(a4),$08(a3)
	bra.s	setregs

noloop	move.l	$04(a4),d2
	add.l	d3,d2
	move.l	d2,$0A(a4)
	move.w	$06(a2,d4.l),$0E(a4)
	move.w	$12(a4),$08(a3)
setregs	move.w	(a4),d0
	and.w	#$0FFF,d0
	beq	checkcom2
	move.b	$02(a4),d0
	and.b	#$0F,d0
	cmp.b	#$03,d0
	bne.s	setperiod
	bsr	setmyport
	bra	checkcom2

setperiod
	move.w	(a4),$10(a4)
	andi.w	#$0FFF,$10(a4)
	move.w	$14(a4),d0
	clr.b	$1B(a4)

	move.l	$04(a4),(a3)
	move.w	$08(a4),$04(a3)
	move.w	$10(a4),d0
	and.w	#$0FFF,d0
	move.w	d0,$06(a3)
	move.w	$14(a4),d0
	or.w	d0,dmacon-counter(a6)
	bra	checkcom2

setdma	move.w	dmacon(pc),d0

	btst	#0,d0			;-------------------
	beq.s	wz_nch1			;
	move.l	aud1lc(pc),wiz1lc-counter(a6)	;
	moveq	#0,d1			;
	moveq	#0,d2			;
	move.w	aud1len(pc),d1		;
	move.w	voice1+$0E(pc),d2	;
	add.l	d2,d1			;
	move.l	d1,wiz1len-counter(a6)		;
	move.w	d2,wiz1rpt-counter(a6)		;
	clr.w	wiz1pos-counter(a6)			;

wz_nch1	btst	#1,d0			;
	beq.s	wz_nch2			;
	move.l	aud2lc(pc),wiz2lc-counter(a6)	;
	moveq	#0,d1			;
	moveq	#0,d2			;
	move.w	aud2len(pc),d1		;
	move.w	voice2+$0E(pc),d2	;
	add.l	d2,d1			;
	move.l	d1,wiz2len-counter(a6)		;
	move.w	d2,wiz2rpt-counter(a6)		;
	clr.w	wiz2pos-counter(a6)			;

wz_nch2	btst	#2,d0			;
	beq.s	wz_nch3			;
	move.l	aud3lc(pc),wiz3lc-counter(a6)	;
	moveq	#0,d1			;
	moveq	#0,d2			;
	move.w	aud3len(pc),d1		;
	move.w	voice3+$0E(pc),d2	;
	add.l	d2,d1			;
	move.l	d1,wiz3len-counter(a6)	;
	move.w	d2,wiz3rpt-counter(a6)	;
	clr.w	wiz3pos-counter(a6)	;

wz_nch3	btst	#3,d0			;
	beq.s	wz_nch4			;
	move.l	aud4lc(pc),wiz4lc-counter(a6)	;
	moveq	#0,d1			;
	moveq	#0,d2			;
	move.w	aud4len(pc),d1		;
	move.w	voice4+$0E(pc),d2	;
	add.l	d2,d1			;
	move.l	d1,wiz4len-counter(a6)	;
	move.w	d2,wiz4rpt-counter(a6)	;
	clr.w	wiz4pos-counter(a6)	;-------------------

wz_nch4	addi.w	#$10,pattpos-counter(a6)
	cmpi.w	#$0400,pattpos-counter(a6)
	bne.s	endr
nex	clr.w	pattpos-counter(a6)
	clr.b	break-counter(a6)
	addq.b	#1,songpos-counter(a6)
	andi.b	#$7F,songpos-counter(a6)
	move.b	songpos(pc),d1
	move.l	data(pc),a5
	cmp.b	$03B6(a5),d1
	bne.s	endr
	move.b	$03B7(a5),songpos-counter(a6)
endr:	tst.b	break-counter(a6)
	bne.s	nex
	rts

setmyport
	move.w	(a4),d2
	and.w	#$0FFF,d2
	move.w	d2,$18(a4)
	move.w	$10(a4),d0
	clr.b	$16(a4)
	cmp.w	d0,d2
	beq.s	clrport
	bge.s	rt
	move.b	#$01,$16(a4)
	rts

clrport	clr.w	$18(a4)
rt	rts

myport	move.b	$03(a4),d0
	beq.s	myslide
	move.b	d0,$17(a4)
	clr.b	$03(a4)
myslide	tst.w	$18(a4)
	beq.s	rt
	moveq	#0,d0
	move.b	$17(a4),d0
	tst.b	$16(a4)
	bne.s	mysub
	add.w	d0,$10(a4)
	move.w	$18(a4),d0
	cmp.w	$10(a4),d0
	bgt.s	myok
	move.w	$18(a4),$10(a4)
	clr.w	$18(a4)

myok	move.w	$10(a4),$06(a3)
	rts

mysub	sub.w	d0,$10(a4)
	move.w	$18(a4),d0
	cmp.w	$10(a4),d0
	blt.s	myok
	move.w	$18(a4),$10(a4)
	clr.w	$18(a4)
	move.w	$10(a4),$06(a3)
	rts

vib	move.b	$03(a4),d0
	beq.s	vi
	move.b	d0,$1A(a4)

vi	move.b	$1B(a4),d0
	lea	sin(pc),a1
	lsr.w	#$02,d0
	and.w	#$1F,d0
	moveq	#0,d2
	move.b	0(a1,d0.w),d2
	move.b	$1A(a4),d0
	and.w	#$0F,d0
	mulu	d0,d2
	lsr.w	#$06,d2
	move.w	$10(a4),d0
	tst.b	$1B(a4)
	bmi.s	vibmin
	add.w	d2,d0
	bra.s	vib2

vibmin	sub.w	d2,d0
vib2	move.w	d0,$06(a3)
	move.b	$1A(a4),d0
	lsr.w	#$02,d0
	and.w	#$3C,d0
	add.b	d0,$1B(a4)
	rts

nop:	move.w	$10(a4),$06(a3)
	rts

checkcom
	move.w	$02(a4),d0
	and.w	#$0FFF,d0
	beq.s	nop
	move.b	$02(a4),d0
	and.b	#$0F,d0
	tst.b	d0
	beq	arpeggio
	cmp.b	#$01,d0
	beq.s	portup
	cmp.b	#$02,d0
	beq	portdown
	cmp.b	#$03,d0
	beq	myport
	cmp.b	#$04,d0
	beq	vib
	cmp.b	#$05,d0
	beq	port_toneslide
	cmp.b	#$06,d0
	beq	vib_toneslide
	move.w	$10(a4),$06(a3)
	cmp.b	#$0A,d0
	beq.s	volslide
	rts

volslide
	moveq	#0,d0
	move.b	$03(a4),d0
	lsr.b	#4,d0
	tst.b	d0
	beq.s	voldown
	add.w	d0,$12(a4)
	cmpi.w	#$40,$12(a4)
	bmi.s	vol2
	move.w	#$40,$12(a4)
vol2	move.w	$12(a4),$08(a3)
	rts

voldown	moveq	#0,d0
	move.b	$03(a4),d0
	and.b	#$0F,d0
	sub.w	d0,$12(a4)
	bpl.s	vol3
	clr.w	$12(a4)
vol3	move.w	$12(a4),$08(a3)
	rts

portup	moveq	#0,d0
	move.b	$03(a4),d0
	sub.w	d0,$10(a4)
	move.w	$10(a4),d0
	and.w	#$0FFF,d0
	cmp.w	#$71,d0
	bpl.s	por2
	andi.w	#$F000,$10(a4)
	ori.w	#$71,$10(a4)
por2	move.w	$10(a4),d0
	and.w	#$0FFF,d0
	move.w	d0,$06(a3)
	rts

port_toneslide
	bsr	myslide
	bra.s	volslide

vib_toneslide
	bsr	vi
	bra.s	volslide

portdown
	clr.w	d0
	move.b	$03(a4),d0
	add.w	d0,$10(a4)
	move.w	$10(a4),d0
	and.w	#$0FFF,d0
	cmp.w	#$0358,d0
	bmi.s	por3
	andi.w	#$F000,$10(a4)
	ori.w	#$0358,$10(a4)
por3	move.w	$10(a4),d0
	and.w	#$0FFF,d0
	move.w	d0,$06(a3)
	rts

checkcom2
	move.b	$02(a4),d0
	and.b	#$0F,d0
	cmp.b	#$0D,d0
	beq.s	pattbreak
	cmp.b	#$0B,d0
	beq.s	posjmp
	cmp.b	#$0C,d0
	beq.s	setvol
	cmp.b	#$0F,d0
	beq.s	setspeed
	rts

pattbreak
	st	break-counter(a6)
	rts

posjmp	move.b	$03(a4),d0
	subq.b	#$01,d0
	move.b	d0,songpos-counter(a6)
	st	break-counter(a6)
	rts

setvol	moveq	#0,d0
	move.b	$03(a4),d0
	cmp.w	#$40,d0
	ble.s	vol4
	move.b	#$40,$03(a4)
vol4	move.b	$03(a4),$09(a3)
	move.b	$03(a4),$13(a4)
	rts

setspeed
	cmpi.b	#$1F,$03(a4)
	ble.s	sets
	move.b	#$1F,$03(a4)
sets	move.b	$03(a4),d0
	beq.s	rts2
	move.w	d0,speed-counter(a6)
	clr.w	(a6)
rts2	rts

*-------------------------------------------------------------------------*
*	Section DATA
*-------------------------------------------------------------------------*

sin:	
	DC.B $00,$18,$31,$4A,$61,$78,$8D,$A1,$B4,$C5,$D4,$E0,$EB,$F4,$FA,$FD
	DC.B $FF,$FD,$FA,$F4,$EB,$E0,$D4,$C5,$B4,$A1,$8D,$78,$61,$4A,$31,$18

periods:
	DC.W $0358,$0328,$02FA,$02D0,$02A6,$0280,$025C,$023A,$021A,$01FC,$01E0
	DC.W $01C5,$01AC,$0194,$017D,$0168,$0153,$0140,$012E,$011D,$010D,$FE
	DC.W $F0,$E2,$D6,$CA,$BE,$B4,$AA,$A0,$97,$8F,$87
	DC.W $7F,$78,$71,$00,$00

speed:	DC.W $06
counter:DC.W $00
songpos:DC.B $00
break:	DC.B $00
pattpos:DC.W $00

dmacon:			DC.W $00
samplestarts:	DS.L $1F

voice1	
	DS.W 10
	DC.W $01
	DS.W 3
voice2	
	DS.W 10
	DC.W $02
	DS.W 3
voice3	
	DS.W 10
	DC.W $04
	DS.W 3
voice4	
	DS.W 10
	DC.W $08
	DS.W 3
voice_end:

data:		dc.l	0					* Adresse module
workspc:	dc.l	0					* Adresse fin workspc
vtabptr: dc.l 0

*-------------------------------------------TABLE VOLUME & INCREMENTS
itab:	ds.l $30
stab:	DS.L $03A0-$30
itab2:	ds.l $30
stab2:	DS.L $03A0-$30

itabvar:	ds.w	$30					* Table de correction
stabvar:	ds.w	(nbre_div-1)*$3a0	* dynamique

	DS.B 256
vtab:
	DS.B 65*256
vtabend:

