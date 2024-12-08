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

*-------------------------------------------------------------------------*
*
*	GRAFIK SOUND II								- CYBERNETICS 1992 -
*
*-------------------------------------------------------------------------*

	opt		o+

	output D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\GRAFIK_S.PRX	


cpu:		set		0


modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:		dc.l	init
					dc.l	update
					dc.l	shutdown
					dc.l	vblroutine

exit:				dc.w	0
WIZfrontbufptr:		dc.l	0
bufferec:			dc.l	0
colors3P:			dc.l	0
playerInfos:		dc.l	0
					dc.w	0
					dc.l	0
					dc.w	0
wizinfocall:		dc.l	0
periodColors:		dc.l	0
equalizerPeriods:	dc.l	0
wait:				dc.w	12
wait2:				dc.w	10
starttimer:			dc.b	29
haslooped:			dc.b	0

OffsetTableY: set 32000

*-------------------------------------------------------------------------*
*	EFFETS:	- Logo Curve	
*			- 3D Curve		
*			- Color Equalizer
*			- Sonic Plasma	
*			- Psyche Vu Meters
*			- Circular curves
*			- Full images	
*			- Sonic vectors	
*			- Circular echo	
*			- Sound valley	
*-------------------------------------------------------------------------*

ismegaste:			dc.b	0
	even

init:
	movem.l		d0-d7/a0-a6,-(sp)

	move.b		(a0),d0
	add.w		d0,d0
	move.b		d0,ismegaste

	move.l		#prg2,prg
	tst.b		1(a0)
	bne.s		.altmuzik
	move.l		#prg1,prg
.altmuzik:

	move.l		bufferec(pc),d0		
	move.l		d0,physb
	move.l		d0,ecran

	move.l		d0,a0
	move.w		#74240/4-1,d0
.effbss:
	clr.l		(a0)+
	dbra.w		d0,.effbss

	clr.b		$fffffa1b.w		* Init timer B
	bset.b		#0,$fffffa07.w		
	bset.b		#0,$fffffa13.w

	*------------------------*
	* Inits Blitter		 *
	*------------------------*
		
	move.w		#-1,$ffff8a2a.w		* Masque
	move.w		#-1,$ffff8a28.w		* Masque
	move.w		#-1,$ffff8a2c.w		* Masque

	jsr			create4				* Blitter test & autogeneration

	lea			$ffff8240.w,a0		* Met les couleurs … zero
	rept		8
	clr.l		(a0)+
	endr

	movem.l		(sp)+,d0-d7/a0-a6

shutdown:

	rts

*-------------------------------------------------------------------------*
*  Boucle Principale
*-------------------------------------------------------------------------*
update:		
	movem.l		d0-d7/a0-a6,-(sp)

main:
	move.w		#-1,vsync			* Vsynchro
.temp:	
	tst.w		vsync
	bne.s		.temp

	lea			wizinfo,a0
	move.l		wizinfocall(pc),a1
	jsr			(a1)

	ifne		cpu					* Cpu time
	move.w		#4,$ffff8240.w
	endc

	move.w		change(pc),d0
	;blt.s		.forcechange
	cmp.w		(a0),d0				* On teste si il faut 
	bne.s		.nochange			* changer d'effet 

;.forcechange:
	lea			$ffff8240.w,a0		* Zero colors lut
	rept		8		
	clr.l		(a0)+
	endr

	clr.b		$fffffa1b.w			* Timer B off
	clr.l		vbl					* Vbl rout off
	clr.w		isinit				* Screen isInit = False
	move.l		prg(pc),a0			* Base programme +
	add.w		instruct(pc),a0		* Offset instruction
	move.w		(a0)+,d1			* Instruction: nb effect * 4
	move.w		(a0)+,change		* Nb vbl pour effect
	move.w		d1,effect			* Nb effect note pour appel
	lea			tabinit,a1		
	move.l		(a1,d1.w),a1
	jsr			(a1)				* run instruction: jsr rout init

.nochange:

	move.w		effect(pc),d0		* Choix de l'effet & go
	lea			tabeffect,a0
	move.l		(a0,d0.w),a0
	jsr			(a0)

	ifne		cpu				* Cpu time
	clr.w		$ffff8240.w
	endc

	tst.w		exit
	bne.s		fin

	bra			main
	
	*--------------------------------------------*
	* Reinit & fin 
	*--------------------------------------------*

fin:
	clr.b		$fffffa1b.w

	lea			$ffff8240.w,a0
	rept		8
	clr.l		(a0)+	
	endr

	movem.l		(sp)+,d0-d7/a0-a6
	
	rts


prg			dc.l	0
instruct:	dc.w	0
change:		dc.w	$0001
param:		dc.w	0
effect:		dc.w	10

*-------------------------------------------------------------------------*
*	EFFET 1:	Curve logo
*-------------------------------------------------------------------------*
curve:	macro
	move.b		(a0)+,d4
	ext.w		d4
	add.w		d4,d4
	move.w		(a2,d4.w),d4
	endm

*-------------------------------------------------------------------------*	
effect1:	
	move.l		bufferec(pc),a0		* Flip ec
	not.w		flip_ec
	move.w		flip_ec,d0
	and.w		#32000,d0	
	add.w		d0,a0
	move.l		a0,ecran
	move.l		a0,physb
	
	tst.w		isinit				* INITIALISATION 
	bne			init1madend

	tst.w		param
	bne.s		.ok111	
	move.w		#5,y1
	move.w		#45,y2
.ok111:

	move.l		physb,physb1
	move.w		y1(pc),d0
	mulu.w		#160,d0
	add.l		d0,physb
	move.w		y2(pc),d0
	add.w		#100,d0
	mulu.w		#160,d0
	add.l		d0,physb1

	lea         physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	lea			multab+128*2,a1
	move.l		a1,a2
	moveq.l		#128/4-1,d0
	moveq.l		#0,d1
	move.w		#-160,d2
.maketab:
	rept	4
	move.w		d1,(a1)+		
	move.w		d2,-(a2)
	endr
	add.w		#160,d1
	sub.w		#160,d2
	dbra.w		d0,.maketab

	move.w		#60*160,multab+128*2
	
	lea			img1+32(pc),a0			* Decompacte image
	move.l		bufferec(pc),a1
	jsr			BIT4RLEUncompress

	lea			32000(a1),a2				* Copie sur les deux ecrans
	move.w		#7999,d0
.copy2:
	move.l		(a1)+,(a2)+
	dbra.w		d0,.copy2

	move.w		#1999,d0	
effend:	
	clr.l		(a2)+
	dbra.w		d0,effend

	move.l		#vbl1,vbl
	not.w		isinit						* Init ok
	
	rts										* Return


init1madend:	

	addq.w		#6,a0						* EFFACAGE 3 EM PLAN AU BLITTER
	move.l		a0,$ffff8a32.w				* Adresse dest
	move.l		#$80008,$ffff8a2e.w			* Increment horizontal dest
	move.w		#40,$ffff8a36.w				* Horizontal size
	clr.w		$ffff8a3a.w					* Mode demi-teinte & combinaison logique
	moveq.l		#-1,d0
	move.w		d0,$ffff8a28.w					; restore endmasks
	move.l		d0,$ffff8a2a.w
	
	move.w		#99,d0
.loop:			; let the interupt exec
	move.w		#1,$ffff8a38.w				* Vertical size
	move.w		#$C000,$ffff8a3c.w			* Pas de partage de bus
	dbf			d0,.loop

	ifne		cpu
	move.w		#$30,$ffff8240.w
	endc									* TRACE DES COURBES
	
	move.l		WIZfrontbufptr,a0			* Adr sample courant
	move.l		(a0),a0
	
	move.l		ecran,a1					* Adr affichage
	lea			160*75+6(a1),a1				* (sur 3em plan)
	lea			100*160(a1),a3				* 
	lea			multab+128*2,a2				
	lea			160(a1),a4
	lea			160(a3),a5
	
	moveq.l		#19,d1						* Longueur courbe

	move.w		#%11000000,d0
	move.w		#%00110000,d5
	move.w		#%00001100,d6
	move.w		#%00000011,d7

aff:
var:	set		0

	rept		2
	
	curve
	move.b		d0,var(a1,d4.w)
	move.b		d0,var(a4,d4.w)
	curve
	move.b		d0,var(a3,d4.w)
	move.b		d0,var(a5,d4.w)

	curve
	or.b		d0,var(a1,d4.w)
	or.b		d0,var(a4,d4.w)
	curve
	or.b		d0,var(a3,d4.w)
	or.b		d0,var(a5,d4.w)

	addq.l		#8,a0

	;---

	curve
	or.b		d5,var(a1,d4.w)
	or.b		d5,var(a4,d4.w)
	curve
	or.b		d5,var(a3,d4.w)
	or.b		d5,var(a5,d4.w)
	
	curve
	or.b		d5,var(a1,d4.w)
	or.b		d5,var(a4,d4.w)
	curve
	or.b		d5,var(a3,d4.w)
	or.b		d5,var(a5,d4.w)

	addq.l		#8,a0

	;----

	curve
	or.b		d6,var(a1,d4.w)
	or.b		d6,var(a4,d4.w)
	curve
	or.b		d6,var(a3,d4.w)
	or.b		d6,var(a5,d4.w)
	
	curve
	or.b		d6,var(a1,d4.w)
	or.b		d6,var(a4,d4.w)
	curve
	or.b		d6,var(a3,d4.w)
	or.b		d6,var(a5,d4.w)

	addq.l		#8,a0

	;----

	curve
	or.b		d7,var(a1,d4.w)
	or.b		d7,var(a4,d4.w)
	curve
	or.b		d7,var(a3,d4.w)
	or.b		d7,var(a5,d4.w)
	
	curve
	or.b		d7,var(a1,d4.w)
	or.b		d7,var(a4,d4.w)
	curve
	or.b		d7,var(a3,d4.w)
	or.b		d7,var(a5,d4.w)

	addq.l		#8,a0

var:	set		1
	endr
	
	addq.w		#8,a1
	addq.w		#8,a3
	addq.w		#8,a4
	addq.w		#8,a5

	dbra.w		d1,aff
	
	rts


*-------------------------------------------------------------------------*
*	ROUTINE VBL LOGO CURVE
*-------------------------------------------------------------------------*

vbl1:	
	clr.b	$fffffa1b.w
	move.b	#100,$fffffa21.w
	move.l	#timer1,$120.w
	move.b	#8,$fffffa1b.w

	lea		img1(pc),a0	* Fixe la palette
	lea		$ffff8240.w,a1	
	rept	8
	move.l	(a0)+,(a1)+
	endr

	lea		physb+1,a0
	move.b	(a0)+,$ffff8205.w
	move.b	(a0)+,$ffff8207.w
	move.b	(a0)+,$ffff8209.w

	rts

*-------------------------------------------------------------------------*
*	ROUTINES TIMER B LOGO CURVE
*-------------------------------------------------------------------------*

timer1:
	move.l	d0,-(sp)

	move.b	$fffffa21.w,d0
.sync:
	cmp.b	$fffffa21.w,d0		;wait for right border
	beq.s	.sync

	ifne	cpu
	move.w	#$770,$ffff8240.w
	endc

	move.l	a0,-(sp)

	lea		physb1+1(pc),a0
	move.b	(a0)+,$ffff8205.w
	move.b	(a0)+,$ffff8207.w
	move.b	(a0)+,$ffff8209.w
	clr.b	$fffffa1b.w

	move.l	a1,-(sp)
	move.l	physb,physb1
	move.w	y1(pc),d0
	mulu.w	#160,d0
	add.l	d0,physb
	move.w	y2(pc),d0
	add.w	#100,d0
	mulu.w	#160,d0
	add.l	d0,physb1
	
	tst.w	param
	beq.s	.fintimer1
	move.w	icy1(pc),d0
	add.w	d0,y1
	cmp.w	#1,y1
	bne.s	.ok1111
	neg.w	icy1
.ok1111:	
	cmp.w	#49,y1
	bne.s	.ok1112
	neg.w	icy1
.ok1112:
	move.w	icy2(pc),d0
	add.w	d0,y2
	cmp.w	#1,y2
	bne.s	.ok1113
	neg.w	icy2
.ok1113:	
	cmp.w	#49,y2
	bne.s	.ok1114
	neg.w	icy2
.ok1114:
	
.fintimer1:
	
	move.l	(sp)+,a1
	move.l	(sp)+,a0
	move.l	(sp)+,d0

	rte

*----------------------- Datas & Buffer Logo Curve
physb1:		dc.l	0
icy1:		dc.w	-1
icy2:		dc.w	1
y1:			dc.w	5
y2:			dc.w	45
img1:		incbin	..\..\databin\grafik_s\grafiks3.c4v
	even
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 2:	3D Curve 					  *
*-------------------------------------------------------------------------*
effect2:
	move.l		bufferec(pc),a0		*********** Flip ec
	not.w		flip_ec
	move.w		flip_ec,d0
	ext.l		d0
	and.l		#32000+28*160,d0
	add.l		d0,a0
	move.l		a0,ecran
	move.l		a0,physb	
	addq.w		#6,a0			

	tst.w		isinit			*********** Init
	bne.s		initmad2
	lea			img2+32(pc),a0		* Decompacte image
	move.l		bufferec(pc),a1
	jsr			BIT4RLEUncompress

	move.l		a1,a2
	add.l		#32000+28*160,a2 * Copie sur les deux ecrans
	move.w		#7999,d0
.copy3:	
	move.l		(a1)+,(a2)+
	dbra.w		d0,.copy3

	move.l		#vbl2,vbl		

	not.w		isinit			* Valide l'initialisation

	rts							* Return


initmad2:
						********** EFFECT 2
	* EFFACAGE AU BLITTER
	
	move.w		#8,$ffff8a2e.w		* Increment horizontal dest
	move.w		#32+8,$ffff8a30.w	* Increment vertical dest
	move.w		#-1,$ffff8a2a.w		* Masque
	clr.w		$ffff8a3a.w			* Mode demi-teinte & combinaison logique
	lea			16(a0),a0
	move.l		a0,$ffff8a32.w		* Adresse dest
	move.l		#16*65536+200,$ffff8a36.w	* Horizontal size & Vertical size
	move.w		#$C000,$ffff8a3c.w	* Go

	ifne		cpu
	move.w		#$30,$ffff8240.w
	endc

	move.l		WIZfrontbufptr,a0	* a0: Adresse du buffer sample
	move.l		(a0),a0
	move.l		ecran,a1			* Adresse ecran
	lea			16000+6(a1),a1		* A1: adresse courbe 1
	move.l		a1,a5

	move.w		ic(pc),d0			* Gestion  zoom
	add.w		d0,x
	move.w		x(pc),d1
	cmp.w		#250,d1
	bne.s		.okzoom1
	neg.w		ic	
.okzoom1:	
	cmp.w		#2,d1
	bne.s		.okzoom2
	neg.w		ic
.okzoom2:	
	move.w		#250,d3			* Adr affichage courbe
	sub.w		d1,d3			* selon zoom
	add.w		#66,d3
	lsr.w		#1,d3
	move.w		d3,d2
	and.w		#15,d2
	and.w		#$fff0,d3
	lsr.w		#1,d3
	add.w		d3,a1

	move.l		a1,a3				* A3: adresse courbe 2
	lea			80(a5),a5
	move.l		a5,a6
	move.l		bufferec(pc),a2		* A2: table d'offsets 160 pour affichage
	add.l		#73256,a2
	move.l		a2,a4
	move.w		x(pc),d0		* On modifie la hauteur selon le zoom
	lsr.w		#2,d0
	mulu.w		#160,d0
	add.w		d0,a3
	add.w		d0,a6
	sub.w		d0,a1
	sub.w		d0,a5

	move.w		#1000,d0		* Size Depart
	
	moveq.l		#0,d7			* Init bit tournant
	neg.w		d2
	add.w		#15,d2
	bset.l		d2,d7			* D7: bit tournant

	move.l		#65536,d6
	divu.w		d0,d6
	ext.l		d0
	divu.w		d1,d0			* D0: coef de reduction
	swap		d0
	mulu.w		d0,d6			* D6: taux d'erreur
	swap		d0
	add.w		d0,d0
	
	move.w		d7,-(sp)
	move.w		d1,-(sp)
	
	; apply zoom on y offsets scale
	moveq.l		#0,d3			* D3: accumulateur
	moveq.l		#0,d1
	move.w		#160,d4
	move.w		d4,d7
	neg.w		d7
	moveq.l		#127,d2
	move.w		d0,d5
.calc2:
	move.w		d1,(a2)+
	move.w		d7,-(a4)
	add.w		d6,d3
	bcc.s		.ok2_4
	subq.w		#1,d5
.ok2_4:	
	subq.w		#1,d5
	bgt.s		.ok2_5
	move.w		d0,d5
	add.w		d4,d1
	sub.w		d4,d7
.ok2_5:	
	dbra.w		d2,.calc2
	
	move.w		(sp)+,d1
	move.w		(sp)+,d7

	;
	move.l		bufferec(pc),a2
	add.l		#73256,a2
	moveq.l		#0,d3			* D3: accumulateur
	move.w		#$2000,d4		* D4: offset adresse
	moveq.l		#1,d2			* D2: prechargement
	add.w		d0,d0

	ifne	cpu
	move.w	#$300,$ffff8240.w
	endc

	lsr.w		#1,d1

	subq.w		#4,d0

.aff2:	
	add.w		d0,a0
	add.w		d6,d3			* tx d'erreur
	dc.w		$6402		
	addq.w		#4,a0	

	move.b		(a0)+,d5
	ext.w		d5
	add.w		d5,d5
	move.w		(a2,d5.w),d5
	or.w		d7,(a1,d5.w)

	move.b		(a0)+,d5
	ext.w		d5
	add.w		d5,d5
	move.w		(a2,d5.w),d5
	or.w		d4,(a6,d5.w)

	move.b		(a0)+,d5
	ext.w		d5
	add.w		d5,d5
	move.w		(a2,d5.w),d5
	or.w		d4,(a5,d5.w)

	move.b		(a0)+,d5
	ext.w		d5
	add.w		d5,d5
	move.w		(a2,d5.w),d5
	or.w		d7,(a3,d5.w)

	ror.w		d2,d7
	bcc.s		.rorb1
	addq.w		#8,a1
	addq.w		#8,a3
.rorb1:

	ror.w		d2,d4
	bcc.s		.rorb2
	addq.w		#8,a5
	addq.w		#8,a6
.rorb2:

	dbra.w		d1,.aff2
	
	lea			tableyp(pc),a0
	lea			tabley_0+5*2,a1
	move.w		x(pc),d0
	lsr.w		#1,d0
	add.w		d0,d0
	add.w		d0,a1
	mulu.w		#20,d0
	add.w		d0,a0

	ifne		cpu
	move.w		#$50,$ffff8240.w
	endc

	* AFFICHAGE GRILLE VERTICALE AU BLITTER

	lea			$ffff8a00+20.w,a5
	rept		10
	move.w		(a0)+,-(a5)
	endr

	lea			$ffff8a2e.w,a5
	move.w		#160,(a5)+					* Inc X dest
	move.w		#-199*160+8,(a5)+			* Inc Y dest
	move.l		ecran,a4
	addq.w		#4,a4
	move.l		a4,(a5)+					* Adr dest
	move.w		#200,(a5)+					* X size
	move.w		#10,(a5)+					* Y size
	move.w		#$0103,(a5)+				* Hog + Op
	move.w		#$C900,(a5)					* Go

	lea			$ffff8a00+20.w,a5
	rept		10
	move.w		(a0)+,-(a5)
	endr

	move.w		#10,$ffff8a38.w				* Y size

	move.w		#$C900,$ffff8a3c.w			* Go
	
	ifne		cpu
	move.w		#$30,$ffff8240.w
	endc

	* AFFICHAGE GRILLE HORIZONTALE AU BLITTER

	move.l		#$80008,$ffff8a2e.w			* Inc X & Y dest
	move.w		#20,$ffff8a36.w				* Y size
	move.w		#$000F,$ffff8a3a.w			* Hog + Op
	
	move.l		ecran,a4
	lea			16004(a4),a4
	move.w		(a1),d2
	moveq.l		#0,d0
	moveq.l		#0,d1
	moveq.l		#1,d3
	move.w		#$C000,d4
.grillex:
	lea			(a4,d0.w),a3
	move.l		a3,$ffff8a32.w				* Adr dest
	move.w		d3,$ffff8a38.w				* Y size
	move.w		d4,$ffff8a3c.w				* Go
	
	lea			(a4,d1.w),a3
	move.l		a3,$ffff8a32.w				* Adr dest
	move.w		d3,$ffff8a38.w				* Y size
	move.w		d4,$ffff8a3c.w				* Go
	
	sub.w		d2,d0
	add.w		d2,d1
	cmp.w		#16000,d1
	blt.s		.grillex

	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	rts

*-------------------------------------------------------------------------*
*	ROUTINE VBL 3D CURVES		 				  *
*-------------------------------------------------------------------------*

vbl2:	
	move.w		x(pc),d5		* Palette
	lsr.w		#4,d5
	add.w		d5,d5
	lea			pal1(pc),a4		* pointe sur couleur grille
	add.w		d5,a4			
	lea			$ffff8240.w,a2	* registres shifter
	move.w		(a4),16(a2)
	move.w		(a4),18(a2)
	move.w		(a4),20(a2)
	move.w		(a4),22(a2)
	move.w		(a4),24(a2)
	move.w		(a4),26(a2)
	move.w		(a4),28(a2)
	move.w		(a4),30(a2)
	lea			32(a4),a4
	move.w		(a4),10(a2)
	move.w		(a4),12(a2)
	move.w		(a4),14(a2)
	move.w		(a4),8(a2)

	move.w		mode2,d0
	beq.s		palgr
	tst.w		appa
	bne.s		.evit
	
	subq.w		#1,d0
	btst		d0,wizinfo+3
	beq.s		palgr
	move.w		#7,appa
.evit:
	subq.w		#1,appa
	lea			deg(pc),a3		* Palette image		
	move.w		appa(pc),d0
	mulu.w		#6,d0
	add.w		d0,a3
	move.l		(a3),2(a2)
	move.w		4(a3),6(a2)
	
	rts

palgr:	
	clr.l		(a2)
	clr.l		4(a2)
	rts

*---------------------------- Datas & variables 3D curves
appa:		dc.w	0
x:			dc.w	100
ic:			dc.w	2
mode2:		dc.w	0
pal1:		incbin	pal2.bin
pal3:		incbin	pal3.bin
deg:		incbin	deg.dat

tableyp:	incbin		tabley.bin
img2:		incbin		..\..\databin\grafik_s\grafiks.c4v
*----------------------------

*-------------------------------------------------------------------------*
*	EFFET 3:	Colors equalizers
*-------------------------------------------------------------------------*

equalize_h:		set 72*2

effect3:

	move.l		bufferec(pc),physb
	
	tst.w		isinit	
	bne.s		.notinit3

	move.l		physb,a0
	moveq.l		#0,d1
	move.w		#15999,d0
.eff3:	
	move.l		d1,(a0)+
	dbra.w		d0,.eff3

	lea			bufferec+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	move.l		#timer3,$120.w
	move.l		#vbl3,vbl

	move.w		#2,$ffff8a20.w					* Inc X source
	moveq.l		#-1,d0
	move.l		d0,$ffff8a28.w
	move.w		d0,$ffff8a2c.w
	move.w		#$203,$ffff8a3a.w				* HOP + OP 

	lea			$ffff8a00.w,a0
	move.w		#$F0F,d0
	rept		16
	move.w		d0,(a0)+
	endr

	not.w		isinit
	rts
.notinit3:

	ifne 	cpu	
	move.w	#$70,$ffff8240.w
	endc

	move.l		playerInfos,a6

	move.l		bufferec(pc),a1
	lea			32000(a1),a1					* Adr affichage barres
	lea			tabcolor(pc),a3
	lea			tabvox(pc),a0					* Table voix
	move.l		equalizerPeriods,a5

	move.w		#128,$ffff8a22.w				* Inc Y source
	move.w		#128,$ffff8a30.w				* Inc Y Dest
	move.w		#1,$ffff8a36.w					* Hori size

	btst		#0,wizinfo+3
	beq.s		.novx1
	move.w		6(a6),d1
	bsr 		aff_rout
.novx1:	
	add.w		playerInfos+4,a6

	btst		#1,wizinfo+3
	beq.s		.novx2
	move.w		6(a6),d1
	bsr 		aff_rout
.novx2:	
	add.w		playerInfos+4,a6

	btst		#2,wizinfo+3
	beq.s		.novx3
	move.w		6(a6),d1
	bsr 		aff_rout
.novx3:	
	add.w		playerInfos+4,a6

	btst		#3,wizinfo+3
	beq.s		.novx4
	move.w		6(a6),d1
	bsr 		aff_rout
.novx4:	

	ifne cpu	
	move.w		#$50,$ffff8240.w
	endc

	move.w		#64*2,d2
	moveq.l		#0,d4

var: set 0
	rept		52
	move.w		(a0)+,d0
	beq.s		*+4 ; nodes
	sub.w		d2,-2(a0)
	; .nodes:
	move.w		d4,var(a1,d0.w)
var: set var+2
	endr

	rts

		;--------------------------------------------------
		; d1 : MOD periods
		;--------------------------------------------------
aff_rout:
		moveq.l		#0,d0
		move.b		(a5,d1.w),d0					* key between 0 and 59 * 2
		
		lea			(a1,d0.w),a2					* Buffer+32000 + nb freq elem dans a2
		lea			(a3,d0.w),a4
		move.w		#equalize_h/2*64*2,(a0,d0.w)

		move.l		a4,$ffff8a24.w					* Adr source
		move.l		a2,$ffff8a32.w					* Adr dest
		move.w		#equalize_h/2,$ffff8a38.w		* Vert size
		move.w		#$C000,$ffff8a3c.w				* No share + no shift

		rts

*-------------------------------------------------------------------------*	
*	ROUTINE TIMER B COLOR EQUALIZER					  *
*-------------------------------------------------------------------------*

timer3:

	move.l		d0,-(sp)

	move.b		$fffffa21.w,d0
.sync0:
	cmp.b		$fffffa21.w,d0
	beq.s		.sync0

	move.l		d1,-(sp)

	move.b		$fffff8209.w,d0
.sync1:	
	move.w		d0,d1
	sub.b		$fffff8209.w,d1
	beq.s		.sync1

	clr.b		$fffffa1b.w						* Stop Timer B 

	add.b		#31,d1
	lsl.w		d1,d1

	moveq.l		#31,d1
	lsl.l		d1,d1

	move.w		#-126-128,$ffff8a22.w			* Inc Y source
	move.l		bufferec,d0
	add.l		#32000+(equalize_h/2-1)*64*2,d0
	move.l		d0,$ffff8a24.w					* Adr source
	sub.l		#(equalize_h/2-1)*64*2,d0
	clr.l		$ffff8a2e.w						* Inc X Dest / Y Dest
	move.l		#$ffff8240,$ffff8a32.w			* Adr dest
	move.w		#64,$ffff8a36.w					* Hori size
	move.w		#equalize_h/2,$ffff8a38.w		* Vert size
	move.w		#$C000,$ffff8a3c.w				* No share + no shift

	move.w		#31,d1
.wait:
	dbf			d1,.wait

	moveq.l		#2,d1
	sub.b		ismegaste(pc),d1
	lsl.w		d1,d1

	move.w		#$303,$ffff8a3a.w				* HOP + OP 

	move.w		#2,$ffff8a22.w					* Inc Y source
	move.l		d0,$ffff8a24.w					* Adr source
	move.w		#equalize_h/2,$ffff8a38.w		* Vert size
	move.w		#$C000,$ffff8a3c.w				* No share + no shift

	move.w		#$203,$ffff8a3a.w				* HOP + OP 

	move.l		(sp)+,d1
	move.l		(sp)+,d0

	rte

*-------------------------------------------------------------------------*	
*	ROUTINE VBL COLOR EQUALIZER					  *
*-------------------------------------------------------------------------*

vbl3:	
	clr.b	$fffffa1b.w
	move.b	#(199-equalize_h)/2,$fffffa21.w
	move.b	#8,$fffffa1b.w
	rts

*------------------- Datas & variables equalizer
tabvox:		ds.w	64
tabcolor:	
	incbin	coltabi.dat
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	BLITTER TEST & AUTOCREATION ROUTINE SONIC PLASMA
*-------------------------------------------------------------------------*
*   Routine d'autogeneration
*-------------------------------------------------------------------------*

plasma_h:		set	156
plasma_voice_h:	set 39

create4:					
	lea			plasma(pc),a1
	move.w		#plasma_h-1,d0

.autogen:
	lea			tramecode(pc),a0
	rept		3
	move.l		(a0)+,(a1)+
	endr
	
	tst.b		ismegaste
	bne.s		.simpleste
	move.w		#$4E71,(a1)+
.simpleste:
	move.w		#$4E71,(a1)+

	dbra.w		d0,.autogen
	
	move.w		#$4e75,(a1)+

	rts


tramecode:	
	move.l	(a0)+,d0			* Charge le decalage
	lsl.w	d0,d0				* Decalage 0/4
	swap	d0					*
	move.w	d0,(a5)				* Inc Y source: deca >=8
	move.w	d6,(a6)				* Vert size
	move.w	d5,(a4)				* No partage & Go


*-------------------------------------------------------------------------*
*	EFFECT 4 : SONIC PLASMA
*-------------------------------------------------------------------------*

effect4:
	tst.w	isinit
	bne 	initmad4

	move.l	bufferec,a1
	move.l	a1,physb
	lea		32000(a1),a0
	lea		plasmatabcolor(pc),a2
		
	move.w	#plasma_h,d0
.plasminit:
	lea		128(a2),a2
	move.l	a2,a3
	moveq.l	#63,d1
.copyx1:	
	move.w	-(a3),(a0)+
	dbra.w	d1,.copyx1
	addq.w	#2,a3
	moveq.l	#63,d1
.copyx2:	
	move.w	(a3)+,(a0)+
	dbra.w	d1,.copyx2
	dbra.w	d0,.plasminit
		
	moveq.l	#0,d1
	move.w	#7999,d0
.eff4:	
	move.l	d1,(a1)+
	dbra.w	d0,.eff4

	clr.w	voice_1
	clr.w	voice_2
	clr.w	voice_3
	clr.w	voice_4

	move.l	#timerb4,$120.w	
	move.l	#vbl4,vbl

	lea			bufferec+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	not.w	isinit

	rts


initmad4:
	lea		wizinfo+3,a0

	btst	#0,(a0)
	beq.s	.novoice1
	move.w	#31*plasma_voice_h*4,voice_1		
.novoice1:
	btst	#1,(a0)
	beq.s	.novoice2
	move.w	#31*plasma_voice_h*4,voice_2		
.novoice2:
	btst	#2,(a0)
	beq.s	.novoice3
	move.w	#31*plasma_voice_h*4,voice_3
.novoice3:
	btst	#3,(a0)
	beq.s	.novoice4
	move.w	#31*plasma_voice_h*4,voice_4
.novoice4:

	lea	voice_1(pc),a2

	tst.w	(a2)
	ble.s	.okv1
	sub.w	#plasma_voice_h*4,(a2)
.okv1:	
	tst.w	2(a2)
	ble.s	.okv2
	sub.w	#plasma_voice_h*4,2(a2)
.okv2:	
	tst.w	4(a2)
	ble.s	.okv3
	sub.w	#plasma_voice_h*4,4(a2)
.okv3:	
	tst.w	6(a2)
	ble.s	.okv4
	sub.w	#plasma_voice_h*4,6(a2)
.okv4:
	
	lea	plasmatab(pc),a0	
	lea	plasmacurve(pc),a1

	; 0 ----

	move.w	(a2)+,d0
	lea		(a1,d0.w),a3

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,44(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,88(a0)

	movem.l	(a3)+,d0-d5
	movem.l	d0-d5,132(a0)

	lea		plasma_voice_h*4(a0),a0

	; 1 ----

	move.w	(a2)+,d0
	lea		(a1,d0.w),a3

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,44(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,88(a0)

	movem.l	(a3)+,d0-d5
	movem.l	d0-d5,132(a0)

	lea		plasma_voice_h*4(a0),a0

	; 2 ----

	move.w	(a2)+,d0
	lea		(a1,d0.w),a3

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,44(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,88(a0)

	movem.l	(a3)+,d0-d5
	movem.l	d0-d5,132(a0)

	lea		plasma_voice_h*4(a0),a0

	; 3 ----

	move.w	(a2)+,d0
	lea		(a1,d0.w),a3

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,44(a0)

	movem.l	(a3)+,d0-d7/a4-a6
	movem.l	d0-d7/a4-a6,88(a0)

	movem.l	(a3)+,d0-d5
	movem.l	d0-d5,132(a0)
	
	rts

*------------------------------*
voice_1:		dc.w	0
voice_2:		dc.w	0
voice_3:		dc.w	0
voice_4:		dc.w	0

plasmacurve:	
	incbin	"plasmacb.dat"

*-------------------------------------------------------------------------*	
*	ROUTINE TIMER B SONIC PLASMA
*-------------------------------------------------------------------------*
timerb4:
	move.l		d0,-(sp)

	move.b		$fffffa21.w,d0
.sync0:
	cmp.b		$fffffa21.w,d0
	beq.s		.sync0

	move.l		d1,-(sp)

	move.b		$fffff8209.w,d0
.sync1:	
	move.w		d0,d1
	sub.b		$fffff8209.w,d1
	beq.s		.sync1

	clr.b		$fffffa1b.w						* Stop Timer B 

	add.b		#31,d1
	lsl.w		d1,d1

	movem.l		d5/d6/a0/a4-a6,-(sp)			* Sauve registres

	move.l		bufferec,a0
	lea			32000(a0),a0
	move.l		a0,$ffff8a24.w		* Adresse source
	lea			plasmatab(pc),a0	* Table plasma (decalages)
	moveq.l		#1,d6
	move.w		#$C000,d5
	lea			$ffff8a22.w,a5		* Adresse Inc Y Dest blitter
	lea			$ffff8a38.w,a6		* Adresse Vert size blitter	
	lea			$ffff8a3c.w,a4		* Adresse Run blitter

	rept		28
	nop
	endr

hori:	set	55
	
	bsr.s		plasma

ret:	
	clr.w		$ffff8240.w
	movem.l		(sp)+,d5/d6/a0/a4-a6 * Restaure registres
	
	move.l		(sp)+,d1
	move.l		(sp)+,d0

	rte

plasma:	ds.b	3300

*-------------------------------------------------------------------------*	
*	ROUTINE VBL SONIC PLASMA
*-------------------------------------------------------------------------*

vbl4:	
	clr.b		$fffffa1b.w
	move.b		#(199-plasma_h)/2,$fffffa21.w
	move.b		#8,$fffffa1b.w

	move.w		#2,$ffff8a20.w				* Inc X source
	clr.l		$ffff8a2e.w					* Inc X & Y Dest
	move.w		#$203,$ffff8a3a.w			* Mode demi teinte
	move.w		#hori,$ffff8a36.w			* Hori size
	move.l		#$ffff8240,$ffff8a32.w		* Adr dest

	rts

*------------------- Datas & variables plasma
plasmatab:	
		rept	plasma_h
		dc.w	-(hori-1)*2+256
		dc.w	4
		endr

plasmatabcolor:	
		incbin	colortab.dat

*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 5:	Psyche Vu Meters
*-------------------------------------------------------------------------*
psyche_vm_height:	set		130

effect5:
	tst.w	isinit
	bne.s	initmad5

	move.l	bufferec,a0
	move.l	a0,ecran
	move.l	a0,physb	
	moveq.l	#0,d0
	move.w	#3999,d1
.eff5_1:	
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	move.l	d0,(a0)+
	dbra.w	d1,.eff5_1

	move.w		#2,$ffff8a20.w			* Inc X source
	move.w		#122,$ffff8a22.w		* Inc Y Dest
	move.w		#2,$ffff8a2e.w			* Inc X Dest
	move.w		#122,$ffff8a30.w		* Inc Y Dest
	move.w		#20,$ffff8a36.w			* Hori size

	clr.l	voice
	clr.l	voice+4
	move.l	#vbl5,vbl

	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	not.w	isinit
	rts

initmad5:
	move.w		wizinfo+2,d7

	move.l		bufferec,a0
	lea			(200-psyche_vm_height)/2*160(a0),a0	
	lea			voice(pc),a2
	lea			crystalm(pc),a3
	lea			tabley_0,a5

	moveq.l		#1,d5
	move.w		#$C000,d6

	moveq.l		#3,d0

.equafish:
	btst		d0,d7
	beq.s		.novoice

	move.l		a3,$ffff8a24.w					* Adr source
	move.l		a0,$ffff8a32.w					* Adr dest
	move.w		#psyche_vm_height,$ffff8a38.w	* Vert size
	move.w		#$203,$ffff8a3a.w				* Mode demi teinte
	move.w		#(psyche_vm_height/2)+1,(a2)+	*
	move.w		d6,$ffff8a3c.w					* No partage & go
	bra.s		.noeffish
.novoice:

	move.w		(a2)+,d1
	ble.s		.noeffish
	subq.w		#1,d1
	move.w		d1,-2(a2)

	add.w		d1,d1
	move.w		(a5,d1.w),d1

	lea			(a0,d1.w),a4
	lea			16000-(200-psyche_vm_height)/2*160(a4),a4
	move.l		a4,$ffff8a32.w			* Adr dest
	move.w		d5,$ffff8a38.w			* Vert size
	clr.w		$ffff8a3a.w				* Mode demi teinte + '0' bits
	move.w		d6,$ffff8a3c.w			* No partage & go

	neg.w		d1
	lea			(a0,d1.w),a4
	lea			16000-(200-psyche_vm_height)/2*160(a4),a4
	move.l		a4,$ffff8a32.w			* Adr dest
	move.w		d5,$ffff8a38.w			* Vert size
	move.w		d6,$ffff8a3c.w			* No partage & go
.noeffish:

	lea			40(a0),a0
	lea			40(a3),a3
	
	dbra.w		d0,.equafish

	rts

*-------------------------------------------------------------------------*
*	VBL	Psyche Vu Meters
*-------------------------------------------------------------------------*

vbl5:	move.w	poscycle(pc),d0
		addq.w	#2,d0
		cmp.w	#30*6,d0
		bne.s	.okcol
		moveq.l	#0,d0
.okcol:	
		move.w	d0,poscycle

		lea		$ffff8242.w,a1
		lea		cycling(pc,d0.w),a0

		rept	7
		move.l	(a0)+,(a1)+
		endr
		move.w	(a0)+,(a1)+

 		rts

*------------------- Datas & variables plasma
poscycle:	dc.w	0

cycling:	incbin	cycling.dat
crystalm:	incbin	optique2.dat

voice:		dc.w	0
			dc.w	0
			dc.w	0
			dc.w	0
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 6:	Circle curves	 				  *
*-------------------------------------------------------------------------*

effect6:
	move.l		bufferec,a0		*********** Flip ec
	move.l		a0,ecran
	move.l		a0,physb

	tst.w		isinit			*********** INIT
	bne.s		init6mad

	lea			bufferec+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	move.l		bufferec,a1
	moveq.l		#0,d1
	move.w		#7999,d0

.clearscreen:
	move.l		d1,(a1)+
	move.l		d1,(a1)+
	dbra.w		d0,.clearscreen

	not.w		isinit

	rts


init6mad:
	move.w		plane6(pc),d0
	addq.w		#1,plane6
	and.w		#3,plane6

	add.w		d0,d0
	add.w		d0,a0

	move.l      colors3P,a1
	lsl.w		#4,d0
	add.w		d0,a1
	lea			$ffff8240.w,a2
	rept		8
	move.l		(a1)+,(a2)+
	endr

	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

											********** EFFACAGE BLITTER
	move.w		#-1,$ffff8a2a.w				* Masque
	move.w		#8,$ffff8a2e.w				* Increment horizontal dest
	move.w		#8+160,$ffff8a30.w			* Increment vertical dest
	lea			22*160(a0),a1
	move.l		a1,$ffff8a32.w				* Adresse dest
	move.l		#20*65536+80,$ffff8a36.w	* Horizontal size + Vertical size
	clr.w		$ffff8a3a.w					* Mode demi-teinte & combinaison logique
	move.b		#%11000000,$ffff8a3c.w		* Go

	ifne	cpu
	move.w	#$40,$ffff8240.w
	endc
	
	move.l		WIZfrontbufptr,a1	* Pointe sur sample
	move.l		(a1),a1
	lea			circurvedata+16*4(pc),a2
	move.l		a0,a3				* Adresse ecran dans A3
	move.w		#239,d0				* Longueur sample
	move.w		#$fffc,d4			* Prechargements
	moveq.l		#1,d5

.affcircle:
	move.b		(a1)+,d1			* Selon sample
	ext.w		d1
	asr.w		d5,d1
	and.w		d4,d1
	move.l		(a2,d1.w),d3		* va chercher pos & bitmap point
	move.l		d3,d6 
	swap		d6
	or.w		d6,(a3,d3.w)		* Affiche point

	move.b		(a1)+,d1			* Selon sample
	ext.w		d1
	asr.w		d5,d1
	and.w		d4,d1
	move.l		(a2,d1.w),d3		* va chercher pos & bitmap point
	move.l		d3,d6 
	swap		d6
	or.w		d6,80(a3,d3.w)		* Affiche point

	move.b		(a1)+,d1			* Selon sample
	ext.w		d1
	asr.w		d5,d1
	and.w		d4,d1
	move.l		(a2,d1.w),d3		* va chercher pos & bitmap point
	move.l		d3,d6 
	swap		d6
	or.w		d6,(a3,d3.w)		* Affiche point

	move.b		(a1)+,d1			* Selon sample
	ext.w		d1
	asr.w		d5,d1
	and.w		d4,d1
	move.l		(a2,d1.w),d3		* va chercher pos & bitmap point
	move.l		d3,d6 
	swap		d6
	or.w		d6,80(a3,d3.w)		* Affiche point

	lea			32*4(a2),a2
	addq.l		#4,a1

	dbra.w		d0,.affcircle

	rts

*------------------- Datas & variables circular curves
plane6:			dc.w	0

circurvedata:	incbin	..\..\databin\grafik_s\circurve.bin
			
			even
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 7:	Full images 					  *
*-------------------------------------------------------------------------*

wizinfo:
	dc.b		0
	dc.b		0
	dc.w		0

effect7:
	lea			overpic+32-160(pc),a0

	not.w		flip_ec
	beq.s		.noflip
	add.l		#41600,a0
.noflip:
	move.l		a0,ecran
	move.l		a0,physb

	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w
	
	tst.w	isinit
	bne.s	.initmad7end

	clr.l	overpic+32-8
	clr.l	overpic+32-4

	move.l	#vbl7,vbl	
	not.w	isinit

.initmad7end:

	move.w		wizinfo+2,d6			; dmacom into d6

	move.l		playerInfos,a0
	move.w		playerInfos+4,d0
	move.l		playerInfos+6,a1
	move.w		playerInfos+10,d1

	lea			overpic+32-8(pc),a2
	move.l		periodColors,a3
	move.w		#3,d7

.updatecolors:

	btst		#0,d6
	beq.s		.nonew
	move.w		#$fff,(a2)+
	bra.s		.bypass

.nonew:
	tst.b		9(a0)				; if volume > 0
	beq.s		.voiceoff

	move.w		4(a0),d2			; if no looping sample
	add.w		#$640,d2
	cmp.w		6(a1),d2
	bne.s		.loopingsample

	move.w		4(a0),d2
	cmp.w		10(a1),d2
	bhi.s		.voiceon			; running = (p2[5] < p[2])
	bra.s		.voiceoff

.loopingsample:
	tst.w		10(a1)				; running = (p2[5] > 0)
	beq.s		.voiceoff
    
	; --

.voiceon:	
	move.w		6(a0),d3			; current period
	add.w		d3,d3
	move.w		(a3,d3.w),(a2)+		; assign colors

	bra.s		.bypass

.deccolors:
	dc.b		0		; 0		0 => 0		0000
	dc.b		8		; 1		2 => 1		1000
	dc.b		9		; 2		4 => 3		1001	
	dc.b		$A		; 3		6 => 5		1010
	dc.b		$B		; 4		8 => 7		1011
	dc.b		$C		; 5		A => 9		1100
	dc.b		$D		; 6		C => B		1101
	dc.b		$E		; 7		E => D		1110
	dc.b		0		; 8		1 => 0		0000
	dc.b		1		; 9		3 => 2		0001
	dc.b		2		; $A	5 => 4	    0010
	dc.b		3		; $B	7 => 6		0011
	dc.b		4		; $C	9 => 8		0100
	dc.b		5		; $D	B => A		0101
	dc.b		6		; $E	D => C		0110
	dc.b		7		; $F	F => E		0111

	; --

.voiceoff:
	
	move.w		(a2),d3

	moveq.l		#$F,d4
	and.w		d3,d4
	move.b		.deccolors(pc,d4.w),d5
	ror.w		#4,d5
	lsr.w		#4,d3

	moveq.l		#$F,d4
	and.w		d3,d4
	move.b		.deccolors(pc,d4.w),d5
	ror.w		#4,d5
	lsr.w		#4,d3

	moveq.l		#$F,d4
	and.w		d3,d4
	move.b		.deccolors(pc,d4.w),d5
	rol.w		#8,d5

	move.w		d5,(a2)+

.bypass:

	lsr.w		#1,d6
	add.w		d0,a0
	add.w		d1,a1

	dbra		d7,.updatecolors

	rts

;--------------------------------------------------
vbl7:	
	clr.b		$fffffa1b.w
	move.b		#1,$fffffa21.w
	move.l 		#timer7_1,$120.w
	move.b		#8,$fffffa1b.w

	rts

;--------------------------------------------------
timer7_1:
	move.l		a0,-(sp)
	move.l		a1,-(sp)

	lea			overpic(pc),a0
	lea			$ffff8240.w,a1
	rept		8
	move.l		(a0)+,(a1)+
	endr

	move.l		(sp)+,a1
	move.l		(sp)+,a0

	clr.b		$fffffa1b.w
	move.b		starttimer,$fffffa21.w
	move.l 		#timer7_2,$120.w
	move.b		#8,$fffffa1b.w
	rte


timer7_2:	
	move.w		d0,-(sp)

	move.b		$fffffa21.w,d0
.sync0:
	cmp.b		$fffffa21.w,d0
	beq.s		.sync0

	move.w		d1,-(sp)

	move.b		$fffff8209.w,d0
.sync1:	
	move.w		d0,d1
	sub.b		$fffff8209.w,d1
	beq.s		.sync1

	add.b		#31,d1
	lsl.w		d1,d1

	clr.b		$fffffa1b.w				* Stop Timer B 
	move.b		#198-1,d0
	sub.b		starttimer,d0
	move.b		d0,$fffffa21.w
	move.b		#8,$fffffa1b.w
	move.l 		#timer7_3,$120.w

	move.w		wait,d0
.wait:
	dbra		d0,.wait

	move.w		wait2,d0
	lsl.w		d0,d0

	move.w		#2,d1		; Prepare le 50 Hz

	move.w		#149,d0

.full1:
	
	move.b		d1,$ffff8260.w
	move.w		d1,$ffff8260.w
	
	rept 90
	nop
	endr

	move.w		d1,$ffff820a.w
	move.b		d1,$ffff820a.w

	rept 23
	nop
	endr
	
	dbra.w		d0,.full1

	move.w	(sp)+,d1
	move.w	(sp)+,d0

	rte


timer7_3:

	clr.l	$ffff8240.w
	clr.l	$ffff8244.w
	clr.l	$ffff8248.w
	clr.l	$ffff824C.w
	clr.l	$ffff8250.w
	clr.l	$ffff8254.w
	clr.l	$ffff8258.w
	clr.l	$ffff825C.w
	
	clr.b		$fffffa1b.w

	rte


*------------------- Datas & full images
overpic:
		incbin	..\..\databin\GRAFIK_S\GS1.bin

*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 8:	Sonic Vectors
*-------------------------------------------------------------------------*

ORIGINE_X:	EQU (320/2)	* -> 160
ORIGINE_Y:	EQU (300/2)	* -> 150

TYPE_ASM: 	EQU 0	* 0=SOUS ASMONE / 1= POUR MEGADEMO
TIME_VBL:	EQU 1	* VISUALISER TEMPS MACHINE 0=OFF / 1=ON
TRACKER:	EQU 0

CLIP_X1:	EQU 0
CLIP_Y1:	EQU 0
CLIP_X2:	EQU 319
CLIP_Y2:	EQU 299


effect8:	
	move.l		bufferec,a0		*********** Flip ec
	lea			16000(a0),a0
	not.w		flip_ec
	beq.s		.flip
	add.l		#48000,a0
.flip:
	move.l		a0,physb	
	move.l		a0,ecran

	tst.w		isinit
	bne			initmad8
	
	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w
	
	lea			linefond+32(pc),a0
	move.l		bufferec,a1
	lea			16000(a1),a1
	move.l		a1,a2
	add.l		#48000,a2

	lea			32000-160(a1),a3
	lea			32000-160(a2),a4

	move.w		#99,d0
.copy81:	
	rept		40
	move.l		(a0)+,d1

	move.l		d1,(a1)+
	move.l		d1,(a2)+

	move.l		d1,(a3)+
	move.l		d1,(a4)+
	endr

	lea			-320(a3),a3
	lea			-320(a4),a4

	dbra.w		d0,.copy81

	lea			linefond(pc),a0
	lea			$ffff8240.w,a1
	rept		8
	move.l		(a0)+,(a1)+
	endr	

	move.w		param,d0
	add.w		d0,d0
	move.w		d0,d1
	add.w		d0,d0
	add.w		d1,d0
	add.w		d0,d0

	lea			tabobject(pc),a0
	move.L		(a0,d0.W),a1
	move.w		(a1)+,SPEED_X
	move.w		(a1)+,SPEED_Y
	move.w		(a1)+,SPEED_Z
	move.l		a1,object
	move.L		4(a0,d0.W),object+4
	move.l		8(a0,d0.W),objectanim
	addq.w		#1,param
	cmp.w		#9,param
	bne.s		.okob
	clr.w		param
.okob:	

	lea			circle_primitives_discs+4(pc),a1
	move.w		#30*512,d0
	move.w		d0,(a1)
	move.w		d0,4(a1)
	move.w		d0,8(a1)
	move.w		d0,12(a1)

	lea			circle_primitives_4keys+4(pc),a1
	move.w		#15*512,d0
	move.w		d0,(a1)
	move.w		d0,4(a1)
	move.w		d0,8(a1)
	move.w		d0,12(a1)

	clr.w		X_ANGLE
	clr.w		Y_ANGLE
	clr.w		Z_ANGLE

	move.l	#-1,$FFFF8A28.w
	move.w	#-1,$FFFF8A2C.w

	not.w		isinit

	rts

initmad8:

  *-----* EFFACEMENT DU PLAN AVEC LE BLITTER *-----*

	MOVE.L	ecran,a0
    
	LEA		32(a0),a2
	lea		$FFFF8A2E.W,a1
	MOVE.L	#8*65536+160-12*8+8,(a1)+
	MOVE.L	a2,(a1)+
	MOVE.L	#12*65536+50,(a1)+
	clr.w	(a1)+
	move.w	#$C000,(a1)

	lea		24+50*160(a0),a2
	MOVE.W	#160-14*8+8,$FFFF8A30.W
	MOVE.L	a2,$FFFF8A32.W
	MOVE.L	#14*65536+100,$FFFF8A36.W
	move.w	#$C000,(a1)

	lea		32+150*160(a0),a2
	MOVE.W	#160-12*8+8,$FFFF8A30.W
	MOVE.L	a2,$FFFF8A32.W
	MOVE.L	#12*65536+50,$FFFF8A36.W
	move.w	#$C000,(a1)

	ifne	cpu
	move.w	#$70,$ffff8240.w
	endc
	
	*-----* *-----*

	move.l	objectanim(pc),a0
	jsr		(a0)

	BSR	CALCULE

	LEA		X_ANGLE(PC),A0		* ROTATION X
;	MOVE.W	SPEED_X(PC),D0
;	ADD.W	D0,(A0)
;	CMPI.W	#720,(A0)
;	BNE.S	.OL1
;	CLR.W	(A0)
;.OL1:
	MOVE.W	SPEED_Y(PC),D0
	ADD.W	D0,2(A0)			* ROTATION Y
	CMPI.W	#720,2(A0)
	BNE.S	.OL2
	CLR.W	2(A0)
.OL2:
	MOVE.W	SPEED_Z(PC),D0
	ADD.W	D0,4(A0)			* ROTATION Z
	CMPI.W	#720,4(A0)
	BNE.S	.OL3
	CLR.W	4(A0)

.OL3:
	* BSR.W	Z_AV			* ANIMATION EN Z !!!

	ifne	cpu
	move.w	#$770,$ffff8240.w
	endc

PASANIM:
	LEA		COOR(PC),A5
	move.l	object+4(PC),A1
					******		
	MOVE.L	ecran,A0
	LEA		-(ORIGINE_Y-100)*160+OffsetTableY(a0),a0

	MOVE.W	(A1)+,D6
	blt.s	.noline

.loopline:
	move.l	a5,a2
	add.W	(A1)+,a2
	MOVE.W	(a2)+,D0
	MOVE.W	(A2)+,D1

	move.l	a5,a2
	add.W	(A1)+,a2
	MOVE.W	(A2)+,D2
	MOVE.W	(A2)+,D3

	MOVEM.L	A0/A1/a5/D6,-(sp)
	BSR.S	DROITE
	MOVEM.L	(SP)+,A0/A1/A5/D6

	dbra	D6,.loopline

.noline:

	move.w	(a1)+,d6
	blt.s	.nocircle

.loopcircle:

	move.l	a5,a2
	add.W	(A1)+,a2

	move.w	512(a2),d7

	MOVE.W	(a2)+,D0
	MOVE.W	(A2)+,D1

	moveq.l	#0,d2
	move.w	(a1)+,d2
	DIVS	D7,D2

	MOVEM.L	A0/A1/a5/D6,-(sp)
	BSR		cercle
	MOVEM.L	(SP)+,A0/A1/A5/D6

	dbra	d6,.loopcircle

.nocircle:

	lea			physb+1,a1
	move.b		(a1)+,$ffff8201.w
	move.b		(a1)+,$ffff8203.w
	move.b		(a1)+,$ffff820d.w

	rts

*******************************************************************************

SPEED_X	  DC.W 0	* VITESSE POUR LA ROTATION (PAIR)
SPEED_Y   DC.W 4
SPEED_Z	  DC.W 6

	*-----* TRACER UNE DROITE SUR UN PLAN *-----*
	
DROITE:
			
*-------------------------------------------------------------------------*
*
* 	LINE ROUT
*
*-------------------------------------------------------------------------*
*	A0: Adr ecran
*	D0: X1
*	D1: Y1
* 	D2: X2
*	D3: Y2
*----------------------------- CUT HERE ----------------------------------*

linemaxh:	set 300

	move.w	d2,d4			* Determination routine a appeler
	sub.w	d0,d4			* d4: largeur
	beq	vert			*
	bpl.s	ok			*
	exg.l	d0,d2			*
	exg.l	d1,d3			*
	neg.w	d4			*
ok:	move.w	d3,d5			* 
	lea	160.w,a6
	sub.w	d1,d5			* d5: hauteur
	* beq	hline			*
	bpl.s	okh			*
	neg.w	d5			*	
	lea	-160.w,a6		*
okh:	
	cmp.w	d4,d5			* Dx>Dy => rout1
	beq	diago			* Dx=Dy => diago
	bgt	rout2			* sinon rout2
	
	****** Dx > Dy

	lea	tabley(pc),a1		* Y2*160 + adr affichage
	add.w	d1,d1			*
	add.w	(a1,d1.w),a0		*
					
	lea	tablex-tabley(a1),a1	* (X2 and $fff0)/2 + adr affichage
	add.w	d0,d0			*
	add.w	(a1,d0.w),a0		*
	and.w	#31,d0			* 

	move.l	d5,d1			* Calcul taux d'erreur
	swap	d1			*
	sub.w	d1,d1			* 
	divu.w	d4,d1			* d1: increment dy*65536/dx 

	lea	map-tablex(a1),a1	* calcul saut variable
	lea	loop1(pc),a2		* en entree: depend de l'offset
	move.l	a2,a3			*
	add.w	(a1,d0.w),a2		* pixel

	add.w	d4,d4			* calcul retour variable
	add.w	d0,d4			* selon longueur ligne
	add.w	(a1,d4.w),a3		*
	move.w	(a3),a5			* sauve instruction 
	move.w	#$4e75,(a3)		* et la remplace par rts
					
	moveq.l	#1,d0			* Registres precharges
	moveq.l	#4,d2			* (plus de bit tournant)		
	moveq.l	#8,d3
	moveq.l	#0,d4
	moveq.l	#32,d5
	moveq.l	#64,d6
	moveq.l	#128,d7

	jsr	(a2)				* go
	move.w	a5,(a3)			* restaure instruction ecrasee
							* par rts
	rts						* go

loop1:	
	rept	20

	or.b	d7,(a0)			* Affiche le point
	add.w	d1,d4			* On ajoute le taux d'erreur (dy*65536/dx)
	dc.w	$6402			* Si bit carry => on depasse l'unite (65356)
	add.l	a6,a0			* la mise a zero se fait automatiquement 

	or.b	d6,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d5,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	bset.b	d2,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d3,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d2,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	bset.b	d0,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d0,(a0)+		
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d7,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d6,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d5,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	bset.b	d2,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d3,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d2,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	bset.b	d0,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			

	or.b	d0,(a0)			
	add.w	d1,d4			
	dc.w	$6402
	add.l	a6,a0			 

	addq.w	#7,a0
	
	endr
	
	**************	DY > DX

rout2:	
	lea	tabley(pc),a1		* Y2*160 + adr affichage
	add.w	d3,d3			*
	add.w	(a1,d3.w),a0		*
					
	lea	tablex2-tabley(a1),a1	* (X2 and $fff0)/2 + adr affichage
	moveq.l	#15,d6			*
	sub.w	d2,d6			*
	add.w	d2,d2			*
	add.w	(a1,d2.w),a0		*
	and.w	#15,d6			*
	moveq.l	#0,d0			*
	bset.l	d6,d0			* Init bit tournant

	move.w	d4,d1			* Calcul taux d'erreur
	swap	d1			*
	sub.w	d1,d1			* 
	divu.w	d5,d1			* d1: increment dy*65536/dx 

	move.w	#linemaxh,d2	*
	sub.w	d5,d2			*
	lsl.w	#4,d2			*
	lea	loop2(pc,d2.w),a1	*
	moveq.l	#0,d2			* erreur de depart a zero
	jmp	(a1)			*
 
loop2:	
	rept	linemaxh
	or.w	d0,(a0)			* Affichage point
	sub.l	a6,a0			* adr affichage-160 (trace a l'envers)
	add.w	d1,d2			* Incremente le taux d'erreur
	dc.w	$6408			* Si bit carry , taux > 1 unite (65536)
	add.w	d0,d0			* on decale de 1 le bit tournant
	dc.w	$6404			* Si abscisse bit > 15
	subq.w	#8,a0			* adr affichage-8 (trace a l'envers)
	moveq.l	#1,d0
	endr		
	
	rts

	*********** DIAGO 		(non optimisee)

diago:	
	lea	tabley(pc),a1		* Adr affichage + Y * 160
	add.w	d3,d3			*
	add.w	(a1,d3.w),a0		*		

	moveq.l	#15,d6			* 
	sub.w	d2,d6			*
	lea	tablex2-tabley(a1),a1	* Adr affichage + (x2 and $fff0)/2
	add.w	d2,d2			*
	add.w	(a1,d2.w),a0		*
	
	and.w	#15,d6			* Bit tournant
	moveq.l	#0,d4			*
	bset.l	d6,d4			*

tdiago:	or.w	d4,(a0)			* Affiche le point
	sub.l	a6,a0			* Mvt vert
	add.w	d4,d4			* Mvt hori
	bcc.s	lsaut			* Incremente adr si abscisse mod 16 = 0
	subq.w	#8,a0			*
	moveq.l	#1,d4			*
lsaut:
	dbra.w	d5,tdiago		* Longueur

	rts	
	
	*********** VERTLINE	(non optimisee)

vert:	
	cmp.w	d3,d1			* Si Y1>Y2
	bgt.s	vsaut
	exg	d3,d1			* Echange Y1 & Y2
vsaut:
	sub.w	d3,d1			* Hauteur

	add.w	d3,d3			* Y1*160 + adr affichage
	lea	tabley(pc),a1
	add.w	(a1,d3.w),a0

	moveq.l	#15,d0
	sub.w	d2,d0
	lea		tablex2-tabley(a1),a1
	add.w	d2,d2			* (X1 and $fff0)/2 + adr affichage
	add.w	(a1,d2.w),a0

	moveq.l	#0,d4			* Init bit 
	and.w	#15,d0
	bset.l 	d0,d4

	move.w	#160,d3			* Precharge registre
	
lvert:	or.w	d4,(a0)			* Affiche point
	add.w	d3,a0			* Mvt vert
	dbra.w	d1,lvert		* Longueur
	
	rts

	*******************************************
	*	TABLES				  *
	*******************************************


tabley:		* Pour eviter les mulu.w ds le calcul de l'offset y*160
			* sur l'adresse

i:	set		-32000
	
	rept	200
	dc.w	i
i:	set 	i+160
	endr

tabley_0:

	rept	200
	dc.w	i
i:	set 	i+160
	endr
	
tablex:		* Calcul offset adresse selon x en octet

i:	set	0
	rept	20
	dc.w	i,i,i,i,i,i,i,i
i:	set	i+1
	dc.w	i,i,i,i,i,i,i,i
i:	set	i+7
	endr

tablex2:	* Calcul offset adresse selon x en mot

i:	set	0
	rept	20
	dc.w	i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i
i:	set	i+8
	endr

map:		

i:	set 	0
	rept	20
	dc.w	i,i+8,i+16,i+24,i+32,i+40,i+48,i+56,i+64,i+72,i+80,i+88,i+96,i+104,i+112,i+120
i:	set	i+8*16+2
	endr

*-------------------------------------------------------------------------*
*
*	CIRCLE ROUT
*
*-------------------------------------------------------------------------*
*	D0:	X centre
*	D1:	Y centre
*	D2:	Rayon
*	A0:	ecran
*------------------------------ CUT HERE ---------------------------------*

bitgauche:	macro
		ror.w	d0,d6
		dc.w	$6404
		addq.w	#8,a4
		addq.w	#8,a0
		endm	
bitdroit:	macro
		add.w	d1,d1
		dc.w	$6406
		moveq.l	#1,d1
		subq.w	#8,a5
		subq.w	#8,a1
		endm	

cercle:	
	move.w	d2,d5 			* Delta = r

	lea		tabley(pc),a5
	move.w	d0,d3			* xc And $fff0 + adr Affichage
	and.w	#$fff0,d3
	lsr.w	#1,d3
	add.w	d3,a0

	add.w	d1,d1
	add.w	(a5,d1.w),a0
	
	move.w	d2,d3

	add.w	d3,d3
	lea		tabley_0-tabley(a5),a5
	move.w	(a5,d3.w),d3
	
	lea		(a0,d3.w),a4
	sub.w	d3,a0	
	move.l	a0,a1
	move.l	a4,a5
	
	and.w	#15,d0			* Init bit tournant
	neg.w	d0
	add.w	#15,d0
	moveq.l	#0,d6
	bset.l	d0,d6
	move.w	d6,d1

	moveq.l	#1,d0
	moveq.l	#-1,d3			* X: d3 = -1
					* Y: d2 = r
.deb:
	tst.w	d2			* While y>= 0
	bge.s	.ok
	rts
.ok:
	or.w	d6,(a0)			* Affichage
	or.w	d6,(a4)			* Affichage
	or.w	d1,(a1)			* Affichage
	or.w	d1,(a5)			* Affichage

	move.w	d5,d4			* delta: D5
	add.w	d4,d4			* d: D4  	d = delta * 2
	
	bge.s	.else			* Si d < 0
	subq.w	#1,d2		
	lea		160(a0),a0			
	lea		160(a1),a1
	lea		-160(a4),a4	
	lea		-160(a5),a5
	add.w	d3,d4			* d = d + x
	bge.s	.else1			* Si d < 0
	add.w	d2,d5			* delta = delta + y	
	bra.s	.deb
.else1:					* Sinon
	addq.w	#1,d3			* Inc x
	bitgauche			
	bitdroit			
	add.w	d2,d5			* 
	sub.w	d3,d5			* delta = delta + y - x
	bra.s	.deb
.else:					* Sinon
	addq.w	#1,d3			* Inc x
	bitgauche			
	bitdroit
	sub.w	d2,d4			* d = d - y
	blt.s	.else2			* Si y >= 0
	sub.w	d3,d5			* delta = delta - x
	bra.s	.deb
.else2:					* Sinon
	subq.w	#1,d2			* Dec y
	lea		160(a0),a0	
	lea		160(a1),a1
	lea		-160(a4),a4	
	lea		-160(a5),a5
	sub.w	d3,d5			* delta = delta + y - x
	add.w	d2,d5
	bra.s	.deb

;-----------------------------------------------------------------------

PONG:	DC.W 1
CPT:	DC.W 0
	
Z_AV:
	TST.W		CPT			* COMPTEUR VIDE ?
	BEQ.S		.OK
	SUBQ.W		#1,CPT
	RTS
.OK:
	LEA			ZDEP(PC),A0
	TST.W		PONG
	BEQ.S		.NN
	ADDI.W		#30,(A0)
	BRA.S		.S
.NN:
	SUBI.W		#-30,(A0)
	CMPI.W		#-900,(A0)
	BGT.S		.RT
	MOVE.W		#1,PONG
	RTS

.S:	
	TST.W		(A0)
	BLT.B		.RT
	CLR.W		PONG
	MOVE.W		#100,CPT

.RT:
	RTS

	*-----* PARTIE DES CALCULS TRIGONOMETRIQUES *-----*
				
CALCULE:
	LEA			COS_TAB(PC),A4
	LEA			SIN_TAB(PC),A5
	LEA			Y_ANGLE(PC),A6

	;MOVE.W		(A6)+,D2	
	MOVE.W		(A6)+,D1	
	MOVE.W		(A6)+,D0

	;MOVE.W		0(A5,D2.W),D5 => 0
	;MOVE.W		0(A4,D2.W),D4 => 1 

	MOVE.W		0(A5,D1.W),D3
	MOVE.W		0(A4,D1.W),D2
	MOVE.W		0(A5,D0.W),D1
	MOVE.W		0(A4,D0.W),D0

	LEA			VAR_3D(PC),A6
	MOVEQ		#9,D7
	MOVE.W		D3,D6	
	NEG.W		D6	
	MOVE.W		D6,(A6)+	
	; MOVE.W	D2,D6	 
	; MULS		D5,D6 => D6	= 0 
	; ASR.L		D7,D6	    = 0
	; MOVE.W		D6,(A6)+	
	clr.w		(a6)+
	MOVE.W		D2,D6
	ext.l		d6
	;MULS		D4,D6
	;ASR.L		D7,D6
	MOVE.W		D6,(A6)+	
	MOVE.W		D0,D6	
	MULS		D2,D6	
	ASR.L		D7,D6	
	MOVE.W		D6,(A6)+	
	MOVE.W		D1,D6	
	ext.l		d6
	;MULS		D4,D6	
	;ASR.L		D7,D6	
	;MOVEA.W	D6,A4	
	;MOVE.W		D0,D6	
	;MULS		D3,D6	
	;ASR.L		D7,D6
	;MULS		D5,D6	
	;ASR.L		D7,D6	
	;SUB.W		A4,D6	
	neg.w		d6
	MOVE.W		D6,(A6)+	
	;MOVE.W		D1,D6	
	;MULS		D5,D6	
	;ASR.L		D7,D6	
	;MOVEA.W	D6,A4	
	MOVE.W		D0,D6	
	MULS		D3,D6	
	ASR.L		D7,D6	
	ext.l		d6
	;MULS		D4,D6	
	;ASR.L		D7,D6	
	;ADD.W		A4,D6	
	MOVE.W		D6,(A6)+	
	MOVE.W		D1,D6	
	MULS		D2,D6	
	ASR.L		D7,D6	
	MOVE.W		D6,(A6)+	
	MOVE.W		D0,D6	
	ext.l		d6
	;MULS		D4,D6	
	;ASR.L		D7,D6	
	MOVEA.W		D6,A4	
	MOVE.W		D1,D6	
	MULS		D3,D6	
	;ASR.L		D7,D6	
	;MULS		D5,D6	
	;ASR.L		D7,D6	
	;ADD.W		A4,D6
	;MOVE.W		D6,(A6)+	
	MOVE.W		A4,(A6)+	
	;MOVE.W		D0,D6	
	;MULS		D5,D6	
	;ASR.L		D7,D6	
	;MOVEA.W		D6,A4	
	MOVE.W		D1,D6
	MULS		D3,D6	
	ASR.L		D7,D6	
	;MULS		D4,D6	
	ext.l		d6
	;ASR.L		D7,D6
	;SUB.W		A4,D6
	MOVE.W		D6,(A6)+	
	
	*---* *---*

	move.l	object(PC),A0
	LEA		COOR(PC),A1
	MOVE.W	(A0)+,d6

.while:
	MOVE.W	(A0)+,D0		* X
	MOVE.W	(A0)+,D1		* Y
	MOVE.W	(A0)+,D2		* Z

    *-----* TRANSFORMATION COORDONNEES 3D EN COORDONNEES PLANAIRES *-----*
	
 	MOVE.L A1,-(SP)
	
	LEA			XDEP(PC),A3
	MOVE.W		(A3)+,D3	
	MOVE.W		(A3)+,D4	
	MOVE.W		(A3)+,D5

	EXT.L		D3
	EXT.L		D4
	EXT.L		D5

	MOVEQ #9,D7
	ASL.L D7,D3 	
	ASL.L D7,D4	
	ASL.L D7,D5	

	;swap		d3
	;swap		d4
	;swap		d5
	;clr.w		d3
	;clr.w		d4
	;clr.w		d5

	MOVEA.L		D3,A1
	MOVEA.L		D4,A2
	MOVEA.L		D5,A3
	
	LEA			VAR_3D(PC),A6

	MOVE.W D0,D3	
	MOVE.W D1,D4	
	MOVE.W D2,D5	

	MULS		(A6)+,D3	
	MULS		(A6)+,D4
	MULS		(A6)+,D5
	ADD.L		D4,D5
	ADD.L		D3,D5
	ADD.L		A3,D5
	MOVE.L		D5,A5

	; SWAP		d5

	LSR.L		D7,D5

	MOVE.W		#512,D7
	SUB.W		D5,D7	
	
	MOVE.W		D0,D3
	MOVE.W		D1,D4	
	MOVE.W		D2,D5
		
	MULS		(A6)+,D3
	MULS		(A6)+,D4
	MULS		(A6)+,D5
	ADD.L		D4,D5	
	ADD.L		D3,D5	
	ADD.L		A1,D5	
	MOVE.L		D5,A4	
	DIVS		D7,D5	
	
	MULS		(A6)+,D0	
	MULS		(A6)+,D1	
	MULS		(A6)+,D2
	ADD.L		D0,D1	
	ADD.L		D1,D2	
	ADD.L		A2,D2

	DIVS		D7,D2
	
	ADDI.W 		#ORIGINE_X,D5
	ADDI.W		#ORIGINE_Y,D2
	
	MOVE.L (SP)+,A1

	*---* *---*
	
	MOVE.W	d7,512(A1)
	MOVE.W	D5,(A1)+
	MOVE.W	D2,(A1)+

	dbra.w	d6,.while

	RTS

.whilenb:	dc.w	0

*----------------------- Datas & Buffer Sonic Vectors

		include		objects.s	

object:		
		dc.l	0
		dc.l	0
objectanim:
		dc.l	0

tabobject:	
		dc.l	vertex_key
		dc.l	primitives_key
		dc.l	anim_none
		
		dc.l	vertex_4keys
		dc.l	primitives_4keys
		dc.l	anim_4keys

		dc.l	vertex_discs
		dc.l	primitives_discs
		dc.l	anim_discs

		dc.l	objet_1
		dc.l	table_1
		dc.l	anim_none

		dc.l	objet_2
		dc.l	table_2
		dc.l	anim_none

COS_TAB:	INCBIN COSINUS.TAB
SIN_TAB:	EQU COS_TAB+720

XDEP	DC.W 0
YDEP	DC.W 0
ZDEP	DC.W -300
X_ANGLE	DC.W 0
Y_ANGLE	DC.W 0
Z_ANGLE	DC.W 0

VAR_3D	DCB.W 9,0
COOR	DCB.W 1024,0		* TAILLE ARBITRAIRE !

*--------------------- Buffers & Datas  3D curves
linefond:	incbin	linefond.dat
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
*	EFFET 9:	CIRCULAR ECHO					  *
*-------------------------------------------------------------------------*

ray2:		set	64
ray2start:	set 8

effect9:
		move.l		bufferec,a0		* Flip ec
		not.w		flip_ec
		move.w		flip_ec,d0
		and.w		#32000,d0
		add.w		d0,a0
		move.l		a0,ecran
		move.l		a0,physb

		tst.w		isinit
		bne.s		initmad9

		lea			physb+1,a1
		move.b		(a1)+,$ffff8201.w
		move.b		(a1)+,$ffff8203.w
		move.b		(a1)+,$ffff820d.w

		move.l		bufferec,a0
		moveq.l		#0,d1
		move.w		#15999,d0
.copy9:	move.l		d1,(a0)+
		dbra.w		d0,.copy9

		move.l		bufferec,a6
		add.l		#64000,a6
		move.w		#1,(a6)
		lea			32000(a6),a6
		move.w		#1,(a6)

		move.l		#vbl9,vbl

		not.w		isinit
		rts

initmad9:	
		; Erase
		;------------------------------------------------

		ifne cpu
		move.w	#$7,$ffff8240.w
		endc

		move.l		bufferec,d0
		move.l		ecran,a6
		add.l		#64000,a6
		moveq.l		#0,d1

.effcircle:	
		move.w		(a6)+,d0
		btst.l		#0,d0
		bne.s		.enderase

		move.l		d0,a1
		move.w		d1,(a1)

		rept		3
		move.w		(a6)+,d0
		move.l		d0,a1
		move.w		d1,(a1)
		endr
			
		bra.s		.effcircle

.enderase:

		lea			oldpos(pc),a0
		lea			final(pc),a1
		moveq.l		#24/4-1,d0
.copye9:
		move.l		(a0)+,(a1)+
		dbra.w		d0,.copye9

		move.l		ecran,a0			* adr ecran
		move.l		a0,a6
		add.l		#64000,a6
		moveq.l		#3,d3				* 4 voix = 4 cercles
		moveq.l		#100,d4				* Position de depart en x
		lea			wizinfo+3,a1		* Pointe sur cpt sndtrack
		lea			poscirc(pc),a2		* Pointe sur buffer etat cercle
		lea			oldpos(pc),a3		* Pointe sur buffer keep ray
		lea			circlecolors(pc),a4
		lea			aleo9(pc),a5
		add.w		aleocpt(pc),a5

		ifne cpu
		move.w		#$70,$ffff8240.w
		endc		

		bra.s		.tracircle

.raycl:	dc.w    0,0,0,$3F,$3F,$FF,$7F,$EF,$6F,$DF,$5F,$CF,$4F,$BF,$3F,$AF,$2F,$9F,$1F,$8F,$7,$E,$6,$D,$5,$C,$4,$B,$3,$A,$2,$9,$1,0

.tracircle:	
		btst		d3,(a1)			* Test la voix
		beq.s		.nothing		* si zero: on laisse
		move.w		#ray2-ray2start,(a2)	* sinon init cercle
		move.l		(a5)+,2(a2)		* abscisse & ordonnee
		addq.w		#4,aleocpt
.nothing:
		clr.w		(a4)+
		move.w		#-1,(a3)+		* oldray = -1
		tst.w		(a2)+			* si rayon cercle <> 0
		ble.s		.db
		
		moveq.l		#ray2,d2		* 
		sub.w		-2(a2),d2		* inverse rayon
		subq.w		#1,-2(a2)		* dec rayon
		move.w		d2,-2(a3)		* note rayon en keep ray
		
		move.w		d2,d7
		and.w		#$FFFE,d7
		move.w		.raycl(pc,d7.w),-2(a4)
		
		movem.w		d3-d4,-(sp)		* sauve les registres
		movem.l		a0-a5,-(sp)

		move.w		(a2)+,d0		* abscisse
		move.w		(a2)+,d1		* ordonnee
		move.w		d0,(a3)+
		move.w		d1,(a3)+

		bsr.s		cercle2			* go : trace cercle
		
		movem.l		(sp)+,a0-a5		* restaure les registres
		movem.w		(sp)+,d3-d4
.db:
		addq.w		#4,a2
		addq.w		#4,a3
		add.w		#40,d4
		addq.w		#2,a0
				
		dbra.w		d3,.tracircle

		move.w		#1,(a6)

		and.w		#128*4-1,aleocpt
		
		lea			circlecolors(pc),a0
		lea			paleff9+2(pc),a1

		move.w		(a0)+,d0		; 1
		move.w		(a0)+,d1		; 2
		move.w		(a0)+,d3		; 4
		move.w		(a0)+,d7		; 8
		
		move.w		d0,d2
		or.w		d1,d2			; 3

		move.w		d3,d4
		or.w		d0,d4			; 5

		move.w		d1,d5
		or.w		d3,d5			; 6

		move.w		d5,d6
		or.w		d0,d6			; 7

		movem.w		d0-d7,(a1)

		or.w		d7,d0
		or.w		d7,d1
		or.w		d7,d2
		or.w		d7,d3
		or.w		d7,d4
		or.w		d7,d5
		or.w		d7,d6

		movem.w		d0-d6,16(a1)

		lea			physb+1,a1
		move.b		(a1)+,$ffff8201.w
		move.b		(a1)+,$ffff8203.w
		move.b		(a1)+,$ffff820d.w

		rts
	
circlecolors:	dcb.w	4,0

		*-------------------------------------------------------------------------*
		*	CIRCLE ROUT with write address storage
		*-------------------------------------------------------------------------*
		*	D0:	X centre
		*	D1:	Y centre
		*	D2:	Rayon
		*	A0:	ecran
		*   A6: adresses
		*------------------------------ CUT HERE ---------------------------------*

bitgauche2:	macro
		ror.w	d0,d6
		dc.w	$6406
		addq.w	#8,a4
		addq.w	#8,a0
		moveq.l	#1,d7
		endm

bitdroit2: macro
		add.w	d1,d1
		dc.w	$6408
		moveq.l	#1,d1
		subq.w	#8,a5
		subq.w	#8,a1
		moveq.l	#1,d7
		endm

cercle2:	
	move.w	d2,d5 			* Delta = r

	lea		tabley_0(pc),a5
	move.w	d0,d3			* xc And $fff0 + adr Affichage
	and.w	#$fff0,d3
	lsr.w	#1,d3
	add.w	d3,a0

	add.w	d1,d1
	add.w	(a5,d1.w),a0
	
	move.w	d2,d3

	add.w	d3,d3
	move.w	(a5,d3.w),d3
	
	lea		(a0,d3.w),a4
	sub.w	d3,a0	
	move.l	a0,a1
	move.l	a4,a5
	
	and.w	#15,d0			* Init bit tournant
	neg.w	d0
	add.w	#15,d0
	moveq.l	#0,d6
	bset.l	d0,d6
	move.w	d6,d1

	moveq.l	#1,d0
	moveq.l	#-1,d3			* X: d3 = -1
							* Y: d2 = r
	moveq.l	#1,d7

.deb:
	tst.w	d7
	beq.s	.donotstoreaddress
	movem.w	a0/a1/a4/a5,(a6)
	addq.l	#8,a6
.donotstoreaddress:

	tst.w	d2			* While y>= 0
	bge.s	.ok
	rts
.ok:
	or.w	d6,(a0)			* Affichage
	or.w	d6,(a4)			* Affichage
	or.w	d1,(a1)			* Affichage
	or.w	d1,(a5)			* Affichage

	moveq.l	#0,d7

	move.w	d5,d4			* delta: D5
	add.w	d4,d4			* d: D4  	d = delta * 2
	
	bge.s	.else			* Si d < 0
	subq.w	#1,d2		
	moveq.l	#1,d7
	lea		160(a0),a0			
	lea		160(a1),a1
	lea		-160(a4),a4	
	lea		-160(a5),a5
	add.w	d3,d4			* d = d + x
	bge.s	.else1			* Si d < 0
	add.w	d2,d5			* delta = delta + y	
	bra.s	.deb
.else1:					* Sinon
	addq.w	#1,d3			* Inc x
	bitgauche2		
	bitdroit2			
	add.w	d2,d5			* 
	sub.w	d3,d5			* delta = delta + y - x
	bra.s	.deb
.else:					* Sinon
	addq.w	#1,d3			* Inc x
	bitgauche2			
	bitdroit2
	sub.w	d2,d4			* d = d - y
	blt.s	.else2			* Si y >= 0
	sub.w	d3,d5			* delta = delta - x
	bra.s	.deb
.else2:					* Sinon
	subq.w	#1,d2			* Dec y
	moveq.l	#1,d7
	lea		160(a0),a0	
	lea		160(a1),a1
	lea		-160(a4),a4	
	lea		-160(a5),a5
	sub.w	d3,d5			* delta = delta + y - x
	add.w	d2,d5
	bra		.deb

vbl9:
		lea		paleff9+2(pc),a0		* Fixe palette
		lea		$ffff8242.w,a1
		
		rept	7
		move.l	(a0)+,(a1)+
		endr
		move.w	(a0)+,(a1)+

		rts

poscirc:	dc.w	-1,160,100,-1,160,100
			dc.w	-1,160,100,-1,160,100

oldpos:		dc.w	10,160,100,10,160,100
			dc.w	10,160,100,10,160,100

final:		dc.w	-1,160,100,-1,160,100
			dc.w	-1,160,100,-1,160,100

paleff9:	dc.w	0
			dcb.w	15,$343

aleocpt:	dc.w	0

aleo9:		incbin	aleoeff9.dat

*-------------------------------------------------------------------------*
*	EFFET 10:	Sound Valley					  *
*-------------------------------------------------------------------------*

sv_step:		set 4
sv_pitch:		set 160
sv_taboffset:	set 73256

effect10:	
		move.l		bufferec,a0		
		lea			sv_pitch(a0),a0
		move.l		a0,ecran
		move.l		a0,physb

		tst.w		isinit
		bne.s		initmad10

		move.l		bufferec,a0		
		move.w		#sv_pitch*(200+sv_step+1)/8,d0
.copy10:
		clr.l		(a0)+
		clr.l		(a0)+
		dbra.w		d0,.copy10

		move.l		bufferec,a1		* Construit la table
		add.l		#sv_taboffset,a1
		moveq.l		#31,d0
		moveq.l		#0,d1
		move.w		#-32*sv_pitch,d2
.maketab2:	
		rept	2
		move.w		d2,128(a1)
		move.w		d1,(a1)+
		endr
		add.w		#sv_pitch,d1
		add.w		#sv_pitch,d2
		dbra.w		d0,.maketab2

		move.l		bufferec,a1		* Construit la table
		add.l		#sv_taboffset,a1
		move.w		#(32+sv_step+2)*sv_pitch,(a1)
			
		lea			physb+1,a1
		move.b		(a1)+,$ffff8201.w
		move.b		(a1)+,$ffff8203.w
		move.b		(a1)+,$ffff820d.w
			
		move.l		#vbl10,vbl
		not.w		isinit
		
		rts

initmad10:	

	ifne	cpu
	move.w	#$700,$ffff8240.w
	endc

	move.l		WIZfrontbufptr,a0			* Adr sample courant
	move.l		(a0),a0
	move.l		bufferec,a2		
	add.l		#sv_taboffset,a2
	move.l		ecran,a1					* Adr affichage
	lea			sv_pitch*(200-32)+6(a1),a1

	moveq.l		#23,d7
	move.w		#$8000,d6
	move.w		#$FE,d4

.aff9_1:
	move.b		(a0)+,d0
	move.b		(a0)+,d1
	move.b		(a0)+,d2
	move.b		(a0)+,d3

	addq.l		#4,a0

	and.w		d4,d0
	and.w		d4,d1
	and.w		d4,d2
	and.w		d4,d3

	move.w		(a2,d0.w),d0
	move.w		(a2,d1.w),d1
	move.w		(a2,d2.w),d2
	move.w		(a2,d3.w),d3

	or.w		d6,(a1,d0.w)
	or.w		d6,24(a1,d1.w)
	or.w		d6,48(a1,d2.w)
	or.w		d6,72(a1,d3.w)
	
	ror.w		#1,d6
	dc.w		$6402
	addq.w		#8,a1

	;-------------

	move.b		(a0)+,d0
	move.b		(a0)+,d1
	move.b		(a0)+,d2
	move.b		(a0)+,d3

	addq.l		#8,a0

	and.w		d4,d0
	and.w		d4,d1
	and.w		d4,d2
	and.w		d4,d3

	move.w		(a2,d0.w),d0
	move.w		(a2,d1.w),d1
	move.w		(a2,d2.w),d2
	move.w		(a2,d3.w),d3

	or.w		d6,(a1,d0.w)
	or.w		d6,24(a1,d1.w)
	or.w		d6,48(a1,d2.w)
	or.w		d6,72(a1,d3.w)
	
	ror.w		#1,d6
	dc.w		$6402
	addq.w		#8,a1

	dbra.w		d7,.aff9_1

.trac:

	ifne	cpu
	clr.w	$ffff8240.w
	endc

	clr.w		screenbottom
.waitbottom:	
	tst.w		screenbottom
	beq.s		.waitbottom

	ifne	cpu
	move.w	#$7,$ffff8240.w
	endc

	move.l		#$80010,$ffff8a20.w
	move.l		#$80010,$ffff8a2e.w
	move.w		#$203,$ffff8a3a.w
	move.w		#19,$ffff8a36.w
	moveq.l		#-1,d0
	
	move.l		ecran,a0
	lea			6+8(a0),a0

	move.l		a0,$ffff8a32.w
	lea			sv_pitch*sv_step(a0),a1
	move.l		a1,$ffff8a24.w
	move.w		#100,$ffff8a38.w
	clr.w		$ffff8a28.w					; endmasks
	move.l		d0,$ffff8a2a.w
	move.w		#$C000+sv_step,$ffff8a3c.w

	lea			sv_pitch*100-8(a0),a0
	move.l		a0,$ffff8a32.w
	lea			sv_pitch*sv_step(a0),a1
	move.l		a1,$ffff8a24.w
	move.w		#100,$ffff8a38.w
	move.w		d0,$ffff8a28.w				; endmasks
	move.l		d0,$ffff8a2a.w
	move.w		#$C000+sv_step,$ffff8a3c.w

	ifne	cpu
	clr.w	$ffff8240.w
	endc

	rts


vbl10:
	clr.b	$fffffa1b.w
	move.b	#12,$fffffa21.w
	move.l	#timer10_1,$120.w
	move.b	#8,$fffffa1b.w

	move.w	#$5,$ffff8250.w

	rts

timer10_1:
	move.w	#$85,$ffff8250.w
	move.l	#timer10_2,$120.w
	rte

timer10_2:
	move.w	#$1D,$ffff8250.w
	move.l	#timer10_3,$120.w
	rte

timer10_3:
	move.w	#$9D,$ffff8250.w
	move.l	#timer10_4,$120.w
	rte

timer10_4:
	move.w	#$2D,$ffff8250.w
	move.l	#timer10_5,$120.w
	rte

timer10_5:
	move.w	#$A6,$ffff8250.w
	move.l	#timer10_6,$120.w
	rte

timer10_6:
	move.w	#$36,$ffff8250.w
	move.l	#timer10_7,$120.w
	rte

timer10_7:
	move.w	#$B6,$ffff8250.w
	move.l	#timer10_8,$120.w
	rte

timer10_8:
	move.w	#$4E,$ffff8250.w
	move.l	#timer10_9,$120.w
	rte

timer10_9:
	move.w	#$CE,$ffff8250.w
	move.l	#timer10_10,$120.w
	rte

timer10_10:
	move.w	#$5E,$ffff8250.w
	move.l	#timer10_11,$120.w
	rte

timer10_11:
	move.w	#$D7,$ffff8250.w
	move.l	#timer10_12,$120.w
	rte

timer10_12:
	move.w	#$67,$ffff8250.w
	move.l	#timer10_13,$120.w
	rte

timer10_13:
	move.w	#$E7,$ffff8250.w
	move.l	#timer10_14,$120.w
	rte

timer10_14:
	move.w	#$7F,$ffff8250.w
	move.l	#timer10_15,$120.w
	rte

timer10_15:
	clr.b	$fffffa1b.w
	move.w	#$FF,$ffff8250.w
	move.w	#-1,screenbottom
	rte
	

screenbottom: dc.w	0



*-------------------------------------------------------------------------*
*	PROCEDURES GENERALES						  *
*-------------------------------------------------------------------------*
vblroutine:
	movem.l	d0-d7/a0/a2-a6,-(sp)

	move.l	vbl(pc),d0
	beq.s	.novbl
	move.l	d0,a1
	jsr		(a1)
.novbl:

	movem.l	(sp)+,d0-d7/a0/a2-a6

	clr.w	vsync

	move.l	(a0)+,a1
	jmp		(a1)

	include		..\..\..\demosdk\bitmap3.s

fadesound:	
	moveq.l		#20,d0
.temp2:	
	move.w		#$4c0,d2
	or.w		d0,d2
	add.w		#20,d2
	move.w		d2,$ffff8922.w
	move.w		#10000,d1
.temp1:	
	move.l		4(sp),4(sp)
	dbra.w		d1,.temp1
	dbra.w		d0,.temp2

	rts

*-------------------------------------------------------------------------*	
*-------------------------------------------------------------------------*	
save:		dc.w		0
vsync:		dc.w		0
ecran:		dc.l		0
flipaff:	dc.w		0
flip_ec:	dc.w		0
mega:		dc.w		0
*-------------------------------------------------------------------------*	

isinit:		dc.w	0
vbl:		dc.l	0

none:		rts

tabeffect:						 
		dc.l	effect1		; 0  logocurve
		dc.l	effect2		; 1  gridcurve
		dc.l	effect3		; 2  equalizer
		dc.l	effect4		; 3  plasmason
		dc.l	effect5		; 4  psyche_vm
		dc.l	effect6		; 5  circlecur
		dc.l	effect7		; 6  fullimage
		dc.l	effect8		; 7  sonicvect
		dc.l	effect9		; 8  circlecho
		dc.l	effect10	; 9  soundvall
		dc.l	none		; 10 endproc
		dc.l	none		; 11 waitstate

tabinit:	
		dc.l	einit7		; logocurve
		dc.l	einit2		; gridcurve
		dc.l	einit1		; equalizer
		dc.l	einit1		; plasmason
		dc.l	einit1		; psyche_vm
		dc.l	einit1		; circlecur
		dc.l	einit1		; fullimage
		dc.l	einit7		; sonicvect
		dc.l	einit1		; circlecho
		dc.l	einit1		; soundvall
		dc.l	finprg		; endproc
		dc.l	einit1		; waitstate

einit1:	addq.w	#4,instruct
		rts

einit2:	add.w	#10,instruct
		move.w	(a0)+,x
		move.w	(a0)+,ic
		move.w	(a0)+,mode2
		rts

einit7:	addq.w	#6,instruct
		move.w	(a0)+,param
		rts

finprg:	clr.w	instruct
		move.w	#$0001,change
		move.b	#-1,haslooped
		rts

physb:	dc.l	0

multab:	ds.w	256


*-------------------------------------------------------------------------*
*
*	NOYAU DE CONTROLE 
*
*-------------------------------------------------------------------------*
				*
				*	PARAMETRES
				*

logocurve:	set	0	*	Nb Vbl,Mode
gridcurve:	set	4	*	Nb Vbl,Prof dep,Sens,Voice (nb voix*2) Logo (0 = logo off) 
equalizer:	set	8	*	Nb Vbl
plasmason:	set	12	*	Nb Vbl
psyche_vm:	set	16	*	Nb Vbl
circlecur:	set	20	*	Nb Vbl
fullimage:	set	24	*	Nb Vbl,Nb of image 
sonicvect:	set	28	*	Nb Vbl,Nb of object
circlecho:	set	32	*	Nb Vbl
soundvall:	set	36	*	Nb Vbl
endproc:	set	40	*       1
waitstate:	set	44	*	Nb vbl

prg1:	; dc.w		soundvall,$1132		
		; dc.w		sonicvect,$1132,2	

		dc.w		logocurve,$0020,0
		dc.w		circlecur,$0100
		dc.w		gridcurve,$010A,10,4,0
		dc.w		fullimage,$011C
		dc.w		plasmason,$013A
		dc.w		gridcurve,$0220,246,-2,0
		dc.w		soundvall,$0301
		dc.w		circlecur,$0308
		dc.w		logocurve,$0310,0
		dc.w		circlecur,$0316
		dc.w		fullimage,$0320
		dc.w		soundvall,$0335
		dc.w		circlecur,$0400
		dc.w		sonicvect,$0420,1	
		dc.w		sonicvect,$0500,2
		dc.w		sonicvect,$0520,0
		dc.w		logocurve,$0600,1
		dc.w		gridcurve,$0613,10,4,0
		dc.w		plasmason,$0623
		dc.w		fullimage,$0701
		dc.w		gridcurve,$0714,10,2,0
		dc.w		logocurve,$0720,0
		dc.w		equalizer,$0800
		dc.w		circlecho,$0820
		dc.w		plasmason,$082E
		dc.w		psyche_vm,$0900
		dc.w		plasmason,$0910
		dc.w		psyche_vm,$0926
		dc.w		circlecho,$0A00
		dc.w		circlecur,$0A1F
		dc.w		circlecho,$0A3A
		dc.w		plasmason,$0A3F
		dc.w		gridcurve,$0B17,10,2,1
		dc.w		fullimage,$0B1F
		dc.w		gridcurve,$0B28,10,2,1
		dc.w		fullimage,$0B2F
		dc.w		gridcurve,$0B3A,10,2,1
		dc.w		plasmason,$0C00
		dc.w		logocurve,$0C20,1
		dc.w		equalizer,$0D00
		dc.w		sonicvect,$0D20,2
		dc.w		fullimage,$0E00
		dc.w		sonicvect,$0E20,0
		;dc.w		fullimage,$0E3A
		dc.w		soundvall,$0E3A
		dc.w		plasmason,$0F00
		dc.w		circlecur,$0F20
		dc.w		logocurve,$0F30,0
		dc.w		circlecur,$0F38
		dc.w		plasmason,$1000
		dc.w		circlecur,$1037
		dc.w		plasmason,$1120
		dc.w		waitstate,$1132
		dc.w		endproc


prg2:	dc.w		logocurve,$0018,0
		dc.w		fullimage,$0020
		dc.w		logocurve,$003F,1
		dc.w		gridcurve,$0128,10,2,1
		dc.w		soundvall,$0200
		dc.w		psyche_vm,$0210
		dc.w		gridcurve,$0220,10,2,0
		dc.w		sonicvect,$0230,0 	
		dc.w		circlecho,$0300
		dc.w		plasmason,$0320
		dc.w		fullimage,$0330
		dc.w		sonicvect,$0400,1
		dc.w		fullimage,$0410
		dc.w		plasmason,$0420
		dc.w		sonicvect,$0500,2
		dc.w		psyche_vm,$0510
		dc.w		circlecur,$0520
		dc.w		logocurve,$0530,0
		dc.w		gridcurve,$0700,246,2,2
		dc.w		equalizer,$0800
		dc.w		sonicvect,$0810,0
		dc.w		sonicvect,$0820,1
		dc.w		sonicvect,$0830,2
		dc.w		sonicvect,$0900,0
		dc.w		logocurve,$0910,0
		dc.w		plasmason,$0920
		dc.w		circlecho,$0A00
		dc.w		logocurve,$0A10,0
		dc.w		plasmason,$0A20
		dc.w		circlecho,$0B00
		dc.w		equalizer,$0C00
		dc.w		gridcurve,$0D00,2,4,0
		dc.w		circlecur,$0E00
		dc.w		circlecho,$0F00
		dc.w		psyche_vm,$1000
		dc.w		plasmason,$1100
		dc.w		fullimage,$1120
		dc.w		logocurve,$1200,1
		dc.w		fullimage,$1220
		dc.w		circlecur,$1300
		dc.w		gridcurve,$1520,246,2,4
		dc.w		waitstate,$1532
		dc.w		endproc
