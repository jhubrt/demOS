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
*	ROTATING SHADES PLAYER 												  *
*																		  *
*	CYBERNETICS 1992													  *
*-------------------------------------------------------------------------*

	output	D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\SHADE.PRX

	opt	o+

	xdef	modulebootstrap

cpu:		set	0
filelen:	set	300
nbetape:	equ	512
nbpal:		set	12
ncur:		set	13

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
	dc.l	init
	dc.l	main
	dc.l	shutdown
	dc.l	firstpart
	dc.l	secondpart

*------------------------------------------------------------------------*
*
*	INITS
*
*------------------------------------------------------------------------*

init:
	move.l	(a0),ecran2
	move.l	4(a0),colorsdest
	rts

*------------------------------------------------------------------------*
*
*	MAIN LOOP
*
*------------------------------------------------------------------------*

main:
	movem.l	d0-a6,-(sp)

	addq.w	#1,cpt			* Changement d'objet
	move.w	cpt(pc),d0
	and.w	#512-1,d0
	bne	.okobject	

	lea		palettes(pc),a0	* Gestion Palette
	add.w	cptpal(pc),a0
	lea		zero(pc),a2
	moveq.l	#-1,d1
	bsr		light			* (fade)

	clr.w	nexton			* Anim off

	add.w	#32,cptpal
	cmp.w	#32*nbpal,cptpal
	bne.s	.okpal
	clr.w	cptpal
.okpal:
	
	bsr		initcurve		* Init courbe

	clr.w	cptfile			* Reset compteur file
	lea		file,a1			* Renit file
	move.l	ecran2(pc),d2
	add.l	#32000,d2
	move.w	#filelen-1,d0
.fill:
	move.l	#masque1,(a1)+
	move.l	d2,(a1)+
	clr.w	(a1)+
	dbra.w	d0,.fill

	move.l	ecran2(pc),a1		* Efface ecran
	move.w	#7999,d0
.eff:	
	clr.l	(a1)+
	dbra.w	d0,.eff

	lea		palettes,a0		* Fixe nouvelle palette
	add.w	cptpal(pc),a0
	move.l	colorsdest(pc),a1
	moveq.l	#7,d0	
.copypl:	
	move.l	(a0)+,(a1)+
	dbra.w	d0,.copypl

	clr.l	rotcpt			* Reset compteur rotation

	move.w	object(pc),d0	* Gestion numero object
	lea		nbetap(pc),a1
	add.w	d0,a1
	move.l	(a1),actetap
	lea		masque(pc),a1
	add.w	d0,a1
	move.l	(a1),actmasque
	addq.w	#4,object
	cmp.w	#16,object
	bne.s	.okobject
	clr.w	object
.okobject:

	move.w	#-1,nexton	 * Anim on

	movem.l	(sp)+,d0-a6

shutdown:

	rts

;---------------------------------------------------- Interrupts on/off --
*
*	ZIK ON/OFF
*
*------------------------------------------------------------------------*

*----------------------------- FADE ROUTINE ------------------------------*
*	A0 = adr palette depart						  *
* 	A2 = adr palette arrivee					  *
* 	D1 = masque							  *
*-------------------------------------------------------------------------*
light:
	move.w		d1,a5			* Sauve le mot de test couleurs
	clr.w		rd4				* Compteur de boucles: nbre d'etapes
looplight:						* Boucle principale
	move.l		a0,a1			* Adresse palette
	move.l		a2,a3			* Adresse palette
	move.w		rd4(pc),d2		* Compteur boucle (‚tape fade)
	lsl.w		#8,d2			*
 	lsr.w		#4,d2			* Etape * 256 / nb etapes
	move.l		colorsdest(pc),a6	* Adr palette
	move.w		a5,d1
	move.w		#15,rd6
	
light_colors:
	move.w  	(a1)+,d4		* Couleur de depart
	move.w		(a3)+,d5 		* Couleur d'arrivee
	
	add.w		d1,d1			* Test si il faut modifier
	bcc.s		nocol			* ce registre de couleur
	
	moveq.l		#0,d3			* Registre pour couleur finale
	moveq.l		#0,d0			* Indice boucle: 0-4-8 (decalage composante r-v-b)

compo:	
	move.w		d4,d7			* Color dep
	bsr.s		rol4bits		*  
	move.w		d7,d6			* => valeur composante de 0 … 15
	move.w		d5,d7			* Color arr
	bsr.s		rol4bits		* => valeur composante de 0 … 15
	sub.w		d6,d7			* delta composante: Dc
	muls.w		d2,d7			* Dc * 256 / nb ‚tape 
	asr.w		#8,d7			* Dc / 256
	add.w		d6,d7			* Dc + color dep 
	bsr.s		ror4bits		* composante de 0 … 15 => ste color 
	lsl.w		d0,d7
	or.w		d7,d3			* D3 : resultat couleur

	lsr.w		#4,d4			* Decalage pour composante
	lsr.w		#4,d5
	
	addq.w		#4,d0			* Boucle composante
	cmp.w		#12,d0
	bne.s		compo
	
	move.w		d3,(a6)			* Fixe couleur
nocol:	
	addq.w		#2,a6	
	subq.w		#1,rd6			* Boucle couleur
	bge.s 		light_colors

	move.w		#2,d5
.temp:	
	move.w		#-1,vsync
.sync2:	
	tst.w		vsync
	bne.s		.sync2

	dbra.w	d5,.temp

	addq.w		#1,rd4			* Boucle etape
	cmp.w		#16,rd4
	bne.s		looplight

	rts

rd4:	dc.w	0
rd6:	dc.w	0


rol4bits:
	add.w		d7,d7		* D7: couleur shift‚e (selon Composante voulue)
	btst.l		#4,d7
	beq.s		nobit1_1
	or.w		#1,d7	
nobit1_1:
	and.w		#15,d7		* D7: compsante de 0 … 15
	rts

	
ror4bits:
	ror.w		#1,d7
	bcc.s		nobit1_2
	or.w		#8,d7
nobit1_2:
	and.w		#15,d7
	rts

*------------------------------------------------------------------------*
*
*	CALCUL COURBES
*
*------------------------------------------------------------------------*

initcurve:
	lea		curves(pc),a0
	add.w	nbcurve(pc),a0
	add.w	#30,nbcurve
	cmp.w	#30*ncur,nbcurve
	bne.s	.okcur
	clr.w	nbcurve
.okcur:	
	lea		curve(pc),a1
	move.w	#30/2-1,d0
.copycr:	
	move.w	(a0)+,(a1)+
	dbra.w	d0,.copycr

	rts


vblcalc:
	tst.w	curve
	beq.s	.nolisa
	
	lea		cosinus(pc),a0			
	move.w	pcos(pc),d0		* angle actuel 
	adda.w	d0,a0			* add. angle actuel
	add.w	pas(pc),d0		* pas de l'angle
	andi.w	#nbetape*2-1,d0
	move.w	d0,pcos

	move.w	ray(pc),d0		* calcule du 
	muls	(a0),d0			* cosinus
	add.l	d0,d0
	add.l	d0,d0
	sub.w	d0,d0
	swap	d0
	add.w	centx(pc),d0	* ajoute coordonne en x du centre
	move.w	d0,x			* coordonne en x

	lea		sinus(pc),a0
	move.w	psin(pc),d0		* angle actuel 
	adda.w	d0,a0			* add. angle actuel
	add.w	pas2(pc),d0		* pas de l'angle
	andi.w	#nbetape*2-1,d0
	move.w	d0,psin

	move.w	ray2(pc),d0		* calcule du sinus
	muls	(a0),d0
	add.l	d0,d0
	add.l	d0,d0
	sub.w	d0,d0
	swap	d0
	add.w	centy(pc),d0	* ajout de la coordonnes en y 
	move.w	d0,y			* coord. en y
	
	rts
	
.nolisa:
	move.w	icx(pc),d0			
	add.w	d0,x
	move.w	x,d0
	cmp.w	#4,d0
	bge.s	.ok1
	neg.w	icx
.ok1:
	cmp.w	#316-48,d0
	ble.s	.ok2
	neg.w	icx
.ok2:
	move.w	icy(pc),d0
	add.w	d0,y
	move.w	y,d0
	cmp.w	#4,d0
	bge.s	.ok3
	neg.w	icy
.ok3:
	cmp.w	#196-48,d0
	ble.s	.ok4
	neg.w	icy
.ok4:
	rts

;------------------------------------------------------------------------
;
;	SHADED DRAWER
;
;------------------------------------------------------------------------

firstpart:

	ifne cpu
	move.w	#$70,$ffff8240.w
	endc

	clr.w	vsync

	move.w	nexton(pc),on
	beq		.end

	movem.l	d0-a6,-(sp)

	lea		file,a3
	add.w	cptfile(pc),a3

	bsr		vblcalc

	move.w	x(pc),d0			* Coordonnees d'affichage
	moveq.l	#15,d1
	and.w	d0,d1
	move.b	d1,deca
	and.w	#$fff0,d0
	lsr.w	#1,d0
	move.w	d0,off

	move.w	y(pc),d0
	lsl.w	#5,d0
	move.w	d0,d1
	lsl.w	#2,d1
	add.w	d1,d0
	add.w	d0,off
	
	move.l	actmasque(pc),a1	* Deplacement,espacement & shiftage
	add.l	rotcpt(pc),a1		* du masque
	moveq.l	#0,d0
	move.w	rotsnap(pc),d0
	add.l	d0,rotcpt
	move.l	rotcpt(pc),d0
	cmp.l	actetap(pc),d0
	blt.s	.okrot
	clr.l	rotcpt
.okrot:
	move.l	a1,(a3)+

	lea	work,a2	

	move.l	#-1,$ffff8a28.w				* Masque
	move.w	#-1,$ffff8a2c.w

	move.l	a2,$ffff8a32.w				* Adr dest
	move.b	#0,$ffff8a3a.w
	move.l	#$30001,$ffff8a36.w
	move.b	#%11000000,$ffff8a3c.w		* Go

	move.l	a2,$ffff8a32.w				* Adr dest
	move.l	#$20002,$ffff8a20.w			* Inc x source	& inc y source
	move.l	#$80008,$ffff8a2e.w			* Inc x dest & inc y dest
	move.l	a1,$ffff8a24.w				* Adr source
	move.l	#48*4*65536+1,$ffff8a36.w	* Taille vert & hori
	move.w	#$203,$ffff8a3a.w			* Combi log & mode hog
	move.b	deca(pc),$ffff8a3d.w		* Decalage
	move.b	#%11000000,$ffff8a3c.w		* Go
	
	move.l	ecran2(pc),a1
	add.w	off(pc),a1
	move.l	a1,(a3)+
	move.b	deca(pc),(a3)+

*----------------------------------------------------------------------*
hauteur:	equ	48
largeur:	equ	4
*---------------------------recopie-----------------------------------*
	
	clr.b	$ffff8a3d.w								* Decalage a zero
	move.l	#8*65536+168-2*4*largeur,$ffff8a20.w	* incs source
	move.l	#$80008,$ffff8a2e.w						* x inc dest

var: set     0

	rept	3
	lea		var(a1),a6
	move.l	a6,$ffff8a24.w			* source

var: set 	var+2

	lea		var(a2),a6
	move.l	a6,$ffff8a32.w			* destination
	move.l	#largeur*65536+hauteur,$ffff8a36.w  * tailles
	move.b	#3,$ffff8a3b.w			* replace
	move.b	#$c0,$ffff8a3c.w		* blitter on
	endr
*------------------------------------and----------------------------------*
	 	
	move.l	#$20004,$ffff8a20.w		* incs source
	move.l	a2,$ffff8a24.w			* source=masque
	move.l	#$20004,$ffff8a2e.w		* incs dest
	lea		2(a2),a6
	move.l	a6,$ffff8a32.w			* destination
	move.l	#$30000+largeur*hauteur,$ffff8a36.w	* tailles 
	move.b	#1,$ffff8a3b.w			* and
	move.b	#$c0,$ffff8a3c.w		* blitter on

*-------------------------------xor---------------------------------------*
	move.l	#$20002,$ffff8a20.w		* x inc source
	move.l	a2,$ffff8a24.w			* source
	move.l	#$20000+162-2*4*largeur,$ffff8a2e.w  * incs dest
	move.l	a1,$ffff8a32.w			* destination
	move.l	#largeur*4*65536+hauteur,$ffff8a36.w * tailles 
	move.b	#6,$ffff8a3b.w			* xor
	move.b	#$c0,$ffff8a3c.w		* blitter on

	movem.l	(sp)+,d0-a6

.end:

	ifne cpu
	clr.w	$ffff8240.w
	endc

    move.l  (a0)+,a1
    jmp     (a1)

*------------------------------------------------------------------------*
*
*	SHADED ERASER
*
*------------------------------------------------------------------------*

secondpart:

	clr.b	$fffffa1b.w

	tst.w	on
	beq		.end

	ifne cpu
	move.w	#$30,$ffff8240.w
	endc
	
	movem.l	d0-a6,-(sp)

	lea		file,a3
	move.w	cptfile(pc),d0
	add.w	#10,d0
	cmp.w	flen(pc),d0
	blt.s	.inf
	sub.w	flen(pc),d0
.inf:	
	add.w	d0,a3
	add.w	#10,cptfile
	move.w	flen(pc),d0
	cmp.w	cptfile(pc),d0
	bne.s	.okfile
	clr.w	cptfile
.okfile:
	lea		work,a2	
	move.l	a2,$ffff8a32.w			* Adr dest
	move.b	#0,$ffff8a3a.w
	move.l	#$30001,$ffff8a36.w
	move.b	#%11000000,$ffff8a3c.w	* Go

	move.l	a2,$ffff8a32.w			* Adr dest
	move.l	#$20002,$ffff8a20.w		* Inc x source	& inc y source
	move.l	#$80008,$ffff8a2e.w		* Inc x dest & inc y dest
	move.l	(a3),$ffff8a24.w		* Adr source
	move.l	#48*4*65536+1,$ffff8a36.w * Taille vert & hori
	move.w	#$203,$ffff8a3a.w		* Combi log & mode hog
	move.b	8(a3),$ffff8a3d.w		* Decalage
	move.b	#%11000000,$ffff8a3c.w	* Go
	
	move.l	4(a3),a1

*---------------------------recopie-----------------------------------*
	
	clr.b	$ffff8a3d.w				* Decalage a zero
	move.l	#8*65536+168-2*4*largeur,$ffff8a20.w * incs source
	move.l	#$80008,$ffff8a2e.w		* x inc dest

var:	set     0
	rept	3
	lea		var(a1),a6
	move.l	a6,$ffff8a24.w			* source
var:	set 	var+2
	lea		var(a2),a6
	move.l	a6,$ffff8a32.w			* destination
	move.l	#largeur*65536+hauteur,$ffff8a36.w  * tailles
	move.b	#3,$ffff8a3b.w			* replace
	move.b	#$c0,$ffff8a3c.w		* blitter on
	endr
*------------------------------------and----------------------------------*
	 	
	move.l	#$20004,$ffff8a20.w		* incs source
	move.l	a2,$ffff8a24.w			* source=masque
	move.l	#$20004,$ffff8a2e.w		* incs dest
	lea		2(a2),a6
	move.l	a6,$ffff8a32.w			* destination
	move.l	#$30000+largeur*hauteur,$ffff8a36.w	* tailles 
	move.b	#2,$ffff8a3b.w			* and not destination
	move.b	#$c0,$ffff8a3c.w		* blitter on

*-------------------------------xor---------------------------------------*
	move.l	#$20002,$ffff8a20.w		* x inc source
	move.l	a2,$ffff8a24.w			* source
	move.l	#$20000+162-2*4*largeur,$ffff8a2e.w * incs dest
	move.l	a1,$ffff8a32.w			* destination
	move.l	#largeur*4*65536+hauteur,$ffff8a36.w * tailles 
	move.b	#6,$ffff8a3b.w			* xor
	move.b	#$c0,$ffff8a3c.w		* blitter on
*-----------

	movem.l	(sp)+,d0-a6	

.end:
	ifne cpu
	clr.w	$ffff8240.w
	endc

	rte



	Section	data

vsync:		dc.w	0
nexton:		dc.w	0
on:			dc.w	0
cptfile:	dc.w	0

off:		dc.w	0
deca:		dc.w	0			* OBJET

cpt:		dc.w	-1
object:		dc.w	0
actetap:	dc.l	120*48*8
actmasque:	dc.l	masque1
rotcpt:		dc.l	0
nbcurve:	dc.w	0
cptpal:		dc.w	-32

curve:		dc.w	1
x:			dc.w	160			* DEPLACEMENT	
y:			dc.w	100
icx:		dc.w	1
icy:		dc.w	1	
flen:		dc.w	200*10
rotsnap:	dc.w	48*8*8
centx:		dc.w	160
centy:		dc.w	100
pas:		dc.w	2
pas2:		dc.w	2
ray:	 	dc.w	100
ray2:	 	dc.w	50
pcos:		dc.w	0
psin:	 	dc.w	0

colorsdest:	dc.l	0
ecran2:		dc.l	0

curves:		include	curves.s

zero:	rept	8
		dc.l	0
		endr
palettes:	
		incbin	pal1.pal
		incbin	pal2.pal
		incbin	pal11.pal
		incbin	pal3.pal
		incbin	pal4.pal
		incbin	pal5.pal
		incbin	pal6.pal
		incbin	pal7.pal
		incbin	pal8.pal
		incbin	pal9.pal
		incbin	pal10.pal
		incbin	pal12.pal

nbetap:		
		dc.l	120*48*8
		dc.l	90*48*8
		dc.l	72*48*8		
		dc.l	60*48*8

masque:		
		dc.l	masque1
		dc.l	masque2
		dc.l	masque3
		dc.l	masque4

cosinus:	incbin	youpi1.kra
sinus:		incbin	youpi2.kra

masque1:	incbin	rot3.dat
masque2:	incbin	rot4.dat
masque3:	incbin	rot5.dat
masque4:	incbin	rot6.dat

file:	rept	filelen
		dc.l	0
		dc.l	0
		dc.w	0
		endr

work:		ds.b	32*48
