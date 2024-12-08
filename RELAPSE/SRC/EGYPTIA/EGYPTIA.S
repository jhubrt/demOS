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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\EGYPTIA.PRX

	opt	o+

nbr:		equ	30
hauteur:	equ	16
largeur:	equ	17	* ATTENTION SI >16
nbcourbe:	equ	6
nbcouleur:	equ	8
ste:		set	1
timemach:	set	0

;-------------------------------------------------------------------------
;	FLEXI	par KRAG			       	  -02/06/1992-
;
;   Merci a METALAGES pour son aide
;   ( optimisation par des movem, Fonte tourn‚e de 90 degres )
;-------------------------------------------------------------------------

modulebootstrap:
		lea		.exporttable(pc),a0
		rts

.exporttable:
		dc.l	EGYPTinit
		dc.l	EGYPTupdate
		dc.l	EGYPTshutdown
		dc.l	EGYPTvbl15
		dc.l	EGYPTcopy
rasters:
		dc.l	0
fade:
        dc.w    1
ecran1: dc.l	0
ecran2: dc.l	0


EGYPTvbl15:
    move.l  rasters(pc),a1

	move.l	(a1)+,$ffff8242.w
	move.l	(a1)+,$ffff8246.w
	move.l	(a1)+,$ffff824A.w
	move.l	(a1)+,$ffff824E.w
	move.l	(a1)+,$ffff8252.w
	move.l	(a1)+,$ffff8256.w
	move.l	(a1)+,$ffff825A.w
	move.w	(a1)+,$ffff825E.w

    move.l  (a0)+,a1
    jmp     (a1)


EGYPTinit:		
		movem.l	d0-d7/a1-a6,-(sp)

		move.l	(a0),d0				* Framebuffer
		move.l	4(a0),image
		move.l	8(a0),font
		move.l	12(a0),code
		moveq.l	#0,d1
		move.w	16(a0),d1

		add.l	d1,d0
		add.l	#$100,d0
		sub.b	d0,d0
		move.l	d0,ecran2

		add.l	#32000,d0
		add.l	d1,d0
		add.l	#$100,d0
		sub.b	d0,d0
		move.l	d0,ecran1

		move.l	ecran2,d0

		clr.b	$ffff820d.w			* Fixe phy base
		lsr.w	#8,d0
		move.b	d0,$ffff8203.w
		swap	d0
		move.b	d0,$ffff8201.w
		
		bsr		displayimage		* affiche l'image

		bsr		genere				* genere le code pour chaque lettre
	
		lea		tabcou(pc),a0		* tableau contenant les ad. des courbes
		move.l	a0,coucour			* position courante dans ce tableau
		move.l	#courbe6,(a0)+		* Courbe 1: triangle
		move.l	#courbe2,(a0)+		* Courbe 2: lacet
		move.l	#courbe1,(a0)+		* Courbe 3: cercle
		move.l	#courbe4,(a0)+		* Courbe 4: ellipse oblique
		move.l	#courbe5,(a0)+		* Courbe 5: huit a plat
		move.l	#courbe3,(a0)+		* Courbe 6: hank

        lea	tableau(pc),a0		
		lea	baratin(pc),a4

		move.l	a4,currentbarratin
        move.l	a0,currenttableau
        
        bsr initlettre

		movem.l	(sp)+,d0-d7/a1-a6

		rts

;------------------------------------------------------------------------------
EGYPTupdate:
		movem.l	d0-d7/a0-a6,-(sp)

        move.l	currenttableau(pc),a0
		move.l	currentbarratin(pc),a4

		lea	ou(pc),a5

		move.l	ecran1(pc),d2
		move.l  ecran2(pc),ecran1
		move.l	d2,ecran2
		
		move.l	d2,a1
		lsr.w	#$8,d2
		move.b	d2,$ffff8203.w
		swap	d2
		move.b	d2,$ffff8201.w
		
		cmp.l	#tableau,a0				* Nouvelle lettre
		bge		.affiche
		
		lea		lettre(pc),a3		
	    rept	nbr-1					* Decale les autres
		move.l	4(a3),(a3)+
		endr
		
.retour:	
		moveq.l #0,d3
		move.b	(a4)+,d3				* Boucle si fin texte
		cmp.b	#'$',d3	
		bne.s	.suite
		
		move.b	(a4)+,d3
		cmp.b	#'1',d3
		bne.s	.ordre2
		move.w	#1,fade
		bra.s	.retour
		
.ordre2:	
		cmp.b	#'2',d3					* fade out
		bne.s	.ordre3
		move.w	#-1,fade
		bra.s	.retour
		
.ordre3:	
		cmp.b	#'3',d3					* prochaine courbe
		bne.s	.ordre4
		bsr		next
		bra		.end
				
.ordre4:	
		lea		baratin(pc),a4			* boucle texte
		bra.s	.retour

.suite:
		sub.w	#32,d3					* Introduit nouvelle lettre
		add.b	d3,d3
		add.b	d3,d3	
		move.l	(a5,d3.w),(a3)
		
		lea	(hauteur*largeur*4)(a0),a0	* Initialise debut du tableau
		
.affiche:
	 	ifgt	timemach
		move.w	#$567,$ffff8240.w		
		endc

		movem.l	d0-d7/a1/a5,-(sp)
		lea		lettre(pc),a3	
		
		rept	nbr						* Saute a la routine
		move.l	(a3)+,a5				* Correspondant a la lettre
		jsr		(a5)
		endr
	
		movem.l	(sp)+,d0-d7/a1/a5
		lea		-((nbr*4*hauteur*largeur)+$40)(a0),a0	* On se remet au debut
		
.end:
		move.l	a0,currenttableau
		move.l	a4,currentbarratin

		movem.l	(sp)+,d0-d7/a0-a6
		
		rts		

;-------------------------------------------------------------------------------------
EGYPTshutdown:	
		rts
		
;-------------------------------------------------------------------------------------
initlettre:	
		lea	ou(pc),a5
		lea	lettre(pc),a3		
		moveq.l	#nbr-1,d0		* Met des espaces
.int:	
    	move.l	(a5),(a3)+			
		dbra.w	d0,.int
		
		rts
		
*-------------------------PROCHAINE COURBE--------------------------------*
next:	
		movem.l	d0-a6,-(sp)

		move.l	ecran2(pc),-(sp)
		bsr		efface
		addq.w	#4,sp
		
		move.l	ecran1(pc),-(sp)
		bsr		efface
		addq.w	#4,sp			

		move.l	ecran2(pc),a4
		lea		tableau(pc),a3
		move.w	#255,d1

.ready:	
		move.l	#$7d00,(a3)+
		dbra	d1,.ready

		move.l	coucour(pc),a1
		move.l	(a1),a2
		jsr		(a2)
		
.ready2:	
		move.l	#$7d00,(a3)+
		cmp.l	#tableau+$7e90,a3
		blt.s	.ready2

		move.l	ecran2(pc),-(sp)
		bsr		efface
		addq.w	#4,sp

		addq.l	#4,coucour
		move.l	a1,d0
		
		cmp.l	#tabcouend,coucour
		blt.s	.peffet
		move.l	#tabcou,coucour
		
.peffet:	

		movem.l	(sp)+,d0-a6

        bsr     initlettre
		
		rts

*-------------------------GENERATEUR DE CODE------------------------------*

genere:	
	movem.l	d0-a6,-(sp)

	move.l	font(pc),a0		 * Gene
	move.l	code(pc),a1	

	lea		buffer(pc),a3
	lea		.suivant(pc),a2
	lea		ou(pc),a4
	lea		text(pc),a5

	moveq.l	#3,d6			* Nombre de colonnes
	
.db3:	
	moveq.l	#9,d4			* Nombre de lettres/colonnes
	
.db2:	
	moveq.w	#-1,d7			* Compteur d'instructions
	moveq.l	#0,d3
	move.w	#largeur-1,d1
	moveq.l	#0,d5
	move.b	(a5)+,d5		* Calcule place de la lettre
	sub.w	#32,d5			* dans le tableau
	add.w	d5,d5
	add.w	d5,d5
	move.l	a1,(a4,d5.w)	* Sauve addresse de la rout 
	
	move.w	#hauteur-1,d0	* Initialise tableau
.init2:	
	move.w	#$2,(a3)+		
	dbra.w	d0,.init2
	lea		-hauteur*2(a3),a3	* Place debut du tableau
	
.debut1:	
	move.w	#hauteur-1,d0
	
.debut2:	
	move.w	d0,d2			
	add.w	d2,d2			* Position dans le tableau
	rol.w	(a0)			* Rotation
	bcc.s	.rien
	
.inf:
	cmp.b	#1,(a3,d2.w)	* Si bit set on saute
	beq.s	.test2
	move.w	#$100,(a3,d2.w)	* Met a 1 la sauvegarde de l'etat du bit
	
.test2:
	cmp.b	#2,1(a3,d2.w)	* Si deja 2 bit mis 
	bne.s	.ok				* Alors on saute
	jmp		(a2,d3.w)		
	
.ok:
	addq.b	#1,1(a3,d2.w)	* Sinon ajoute 1 au nombre de bits
	addq.w	#1,d7			* Ajoute 1 au compteur
	cmp.w	#3,d7			* Si d7>=3 alors 
	ble.s	.inf1			* d7=0
	move.w	#0,d7			

.inf1:	
	tst.w	d7				* Si d7>0 alors
	ble.s	.normal			* Branche sur les movem
	jsr		movm
	bra.s	.instruc		* Sinon normal
	
.normal:	
	moveq.l	#0,d5
	move.l	#$30183218,(a1)+ * Move.w (a0)+,d0/move.w (a0)+,d1
	
.instruc:
	move.l	#$81711000,(a1)+ * Or.w	d0,(a1,d1.w)
	add.l	d5,-4(a1)		* Modifie l'instruction
	move.b	#0,d3			* 0 pour se brancher sur suivant
	bra.s	.boucle			* Suivant

.rien:	
	tst.b	(a3,d2.w)		* Si bit a 0
	beq.s	.test3			* Alors on saute
	clr.w	(a3,d2.w)		* Met a 0 la sauvegarde de l'etat du bit
	
.test3:	
	cmp.b	#2,1(a3,d2.w)	* Si 2 bit a 0
	bne.s	.ok2			* Alors on saute
	jmp		(a2,d3.w)
	
.ok2:	
	addq.b	#1,1(a3,d2.w)	* Sinon ajoute 1 au nombre de bits
	addq.w	#1,d7			* Ajoute 1 au compteur
	cmp.w	#3,d7			* Si d7>=3 alors 
	ble.s	.inf2			* d7=0
	move.w	#0,d7			

.inf2:	
	tst.w	d7				* Si d7>0
	ble.s	.normal2		* Branche sur les movem
	jsr		movm
	bra.s	.instruc2		* Sinon normal

.normal2:
	moveq.l	#0,d5
	move.l	#$30183218,(a1)+ * Move.w (a0)+,d0/move.w (a0)+,d1

.instruc2:
	move.l	#$b1711000,(a1)+ * Eor.w	d0,(a1,d1.w)
	add.l	d5,-4(a1)		* Modifie instruction
	move.b	#0,d3			* 0 pour se brancher sur suivant
	bra.s	.boucle			* Suivant

.suivant:
	move.w	#-1,d7			* rupture compteur d7=-1
	move.w	#$5888,(a1)+	* Addq.w #4,a0
	move.w	#14,d3			* 10 pour saut variant sur s1
	bra.s	.boucle			* Suivant
	
.s1:	
	move.w	#$5088,-2(a1)	* Addq.w #8,a0
	move.w	#26,d3			* 22 pour saut variant sur s2			
	bra.s	.boucle			* Suivant
	
.s2:	
	move.l	#$41e8000c,-2(a1) * Modifie ancienne instruction
	addq.w	#2,a1			* Lea $10(a0),a0
	move.w	#42,d3			* 38 pour saut variant sur s3
	bra.s	.boucle			* Suivant
	
.s3:
	addq.w	#4,-2(a1)		* Ajoute 4 au lea
	
.boucle:	
	dbra.w	d0,.debut2		* Hauteur-1
	lea		160(a0),a0		* Prochaine ligne
	
	dbra.w	d1,.debut1		* Largeur-1
	move.w	#$4e75,(a1)+	* Rts
	
	dbra.w	d4,.db2			* Repeter colonnes-1 fois

	lea	-(160*largeur*10-8)(a0),a0	* va en haut colonne suivante
	
	dbra.w	d6,.db3			* Repeter nbr colonnes-1fois
	
	movem.l	(sp)+,d0-a6

	rts

*-----------------------CAS DES MOVEMS--------------------------------*

movm:	
	cmp.w	#1,d7				* Si d7=1
	bne.w	.tpts
	move.l	#$4c98000f,-8(a1)	* Movem.w (a0)+,d0-d3
	bra.s	.sortie
	
.tpts:	
	cmp.w	#2,d7				* Si d7=2
	bne.w	.qpts
	move.l	#$4c98003f,-12(a1)	* Movem.w (a0)+,d0-d5
	bra.s	.sortie				* Sinon
	
.qpts:	
	move.l	#$4c9800ff,-16(a1)	* Movem.w (a0)+,d0-d7

.sortie:	
	move.l	#$2001000,d5		* Calcule la modif
	cmp.w	#3,d7				* Si d7=3  
	bne.s 	.vasy				* Calcule different
	subq.w	#1,d7
	lsl.l	d7,d5
	add.l	#$4002000,d5
	addq.l	#1,d7
	bra.s	.fini	
.vasy:
	lsl.l	d7,d5				* de l'instruction
.fini:

	rts

*-------------------------------------------------------------------*
*			effacement												*
*-------------------------------------------------------------------*
efface:		
		move.l	d0,-(sp)
		move.l	a1,-(sp)

		move.w	#199,d0
		move.l	12(sp),a1
		addq.w	#6,a1
		
.topo:	rept	20
		clr.w	(a1)
		addq.w	#8,a1
		endr
		dbra	d0,.topo
		
		move.l	(sp)+,a1
		move.l	(sp)+,d0
		rts

*---------------------------------------------------------------------------------*		
theme:		dc.b	0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15
version:	dc.b	0,2,4,6,8,10,12,14,1,3,5,7,9,11,13,15
offcoul:	dc.l	0
offetap:	dc.l	0
*----------------------------------------------------------------------------*

*-------------------------------------------------------------------*
courbe1:	
		move.w	#-1,ray
		move.w	#99,d6
		move.w	#60,centx
		move.w	#139,centy
		move.w	#1,pasr
		move.w	#1,pasd
		
		bsr		obli

		move.w	#1,ray
		move.w	#99,d6
		move.w	#160,centx
		move.w	#40,centy

		bsr		obli

		move.w	#139,centy
		move.w	#61,centx
		move.w	#1,pasr
		move.w	#1,pasd
		move.w	#198,d6
		
		bsr		hori
		
		rts

*-------------------------------------------------------------------------*
courbe2:	
		move.w	#50,depha1
		move.w	#700,depha2
		move.w	#100,centy
		move.w	#145,centx
		move.w	#100,ray
		move.w	#2,pasr
		move.w	#8,pasd
		move.w	#6,pasd2
		move.w	#473,d6

		bsr		cercle
		
		rts

*-------------------------------------------------------------------------*
courbe3:	
		move.w	#0,depha1
		move.w	#0,depha2
		move.w	#100,centy
		move.w	#160,centx
		move.w	#99,ray
		move.w	#2,pasr
		move.w	#6,pasd
		move.w	#6,pasd2
		move.w	#478,d6

		bsr		cercle
		
		rts

*-------------------------------------------------------------------------*
courbe4:	
		move.w	#320,depha1
		move.w	#0,depha2
		move.w	#100,centy
		move.w	#160,centx
		move.w	#99,ray
		move.w	#2,pasr
		move.w	#6,pasd
		move.w	#6,pasd2
		move.w	#478,d6

		bsr		cercle
		
		rts

*-------------------------------------------------------------------------*
courbe5:
		move.w	#2520,depha1
		move.w	#1800,depha2
		move.w	#100,centy
		move.w	#160,centx
		move.w	#99,ray
		move.w	#2,pasr
		move.w	#6,pasd
		move.w	#12,pasd2
		move.w	#478,d6

		bsr.s	cercle
		
		rts

*-------------------------------------------------------------------------*
courbe6:	
		move.w	#0,depha1
		move.w	#0,depha2
		move.w	#100,centy
		move.w	#160,centx
		move.w	#95,ray
		move.w	#2,pasr
		move.w	#12,pasd
		move.w	#12,pasd2
		move.w	#239,d6

		bsr.s	cercle
		bsr		wave

		rts

;-----------------------AFFICHE L'IMAGE----------------------------

displayimage:	
	movem.l	d0/a1-a3,-(sp)

	move.l	ecran1(pc),a1
	move.l	ecran2(pc),a2

	move.l	image(pc),a3
	lea		32(a3),a3

	move.w	#7999,d0
.mlo:
	move.l	(a3),(a1)+
	move.l	(a3)+,(a2)+
	dbra.w	d0,.mlo

	movem.l	(sp)+,d0/a1-a3
	
	rts


;-------------------------------------------------------------------------
;                              CERCLE
;-------------------------------------------------------------------------
cercle:	
	lea		cosinus(pc),a0		* tableau de cosinus
	lea		sinus(pc),a1		* tableau de sinus
	lea		cosinus+2878(pc),a5
	lea		sinus+2878(pc),a6
	lea		point(pc),a2		* tableau des bits set

	add.w	depha1(pc),a0		* angle
	add.w	depha2(pc),a1
	
.angle:	
	move.w	ray(pc),d1		* rayon dans d2
	move.w	#15,d5

.rayon:	
	move.w	d1,d2			* calcule du 
	muls	(a0),d2			* cosinus
	add.l	d2,d2	
	add.l	d2,d2	
	sub.w	d2,d2
	swap	d2
	
	add.w	centx(pc),d2	* ajoute coordonne en x du centre
	move.w	d2,d3
	and.w	#-16,d3			* masque les 4 derniers bits
	sub.w	d3,d2		
	add.w	d2,d2			* calcule la position du masque
	move.w	(a2,d2.w),d4	* recuper le masque
	lsr.w	#1,d3
	
	move.w	d1,d2			* calcule du cosinus
	muls	(a1),d2
	add.l	d2,d2
	add.l	d2,d2
	sub.w	d2,d2
	swap	d2
	add.w	centy(pc),d2	* ajout de la coordonnes en y 
	muls	#160,d2
	add.w	d2,d3			* offset addresse ecran
	addq.w	#6,d3
	
	move.w	(a4,d3.w),d2	* a4 ad ecran
	and.w	d4,d2
	tst.w	d2
	bne.s	.dplace
	move.w	d4,(a3)+		* a3 ad tableau
	move.w	d3,(a3)+
	or.w	d4,(a4,d3.w)
	bra.s	.rayon2
	
.dplace:	
	move.w	d4,(a3)+
	move.w	#$7d00,(a3)+
		
.rayon2:	
	sub.w	pasr(pc),d1		* pas du rayon
	dbra	d5,.rayon
	
	add.w	pasd(pc),a0		* pas de l'angle
	cmp.l	a0,a5
	bge.s	.ici
	
	move.l	a0,d7
	sub.l	a5,d7
	lea		cosinus(pc),a0
	lea		(a0,d7.w),a0
	
.ici:	
	add.w	pasd2(pc),a1
	cmp.l	a1,a6
	bge.s	.ici2
	
	move.l	a1,d7
	sub.l	a6,d7
	lea		sinus(pc),a1
	lea		(a1,d7.w),a1
	
.ici2:	
	dbra	d6,.angle
	
	rts

*-------------------------------------------------------------------------*
*                         SINIUS                               		  *
*-------------------------------------------------------------------------*
wave:
	lea		sinus+120(pc),a0	* se place sur degres 12
	lea		sinus+2878(pc),a1	* fin du tableau
	lea		point(pc),a2		* tableau des bits set
	move.w	#194,d6	
	
.angle2:	
	move.w	#168,d1		
	move.w	#15,d5

.rayon3:	
	move.w	(a0),d2		
	muls 	#15,d2		
	sub.w	d2,d2
	swap	d2
	
	add.w	d1,d2		
	move.w	d2,d3
	and.w	#-16,d3			* masque les 4 derniers bits
	sub.w	d3,d2		
	add.w	d2,d2			* calcule la position du masque
	move.w	(a2,d2.w),d4	* recuper le masque
	lsr.w	#1,d3
	
	move.w	d6,d2
	sub.w	#196,d2
	neg.w	d2
	muls	#160,d2
	add.w	d2,d3			* offset addresse ecran
	addq.w	#6,d3
	
	move.w	(a4,d3.w),d2	* a4 ad ecran
	and.w	d4,d2
	tst.w	d2
	bne.s	.dplace2
	move.w	d4,(a3)+		* a3 ad tableau
	move.w	d3,(a3)+
	or.w	d4,(a4,d3.w)
	bra.s	.rayon4
	
.dplace2:
	move.w	d4,(a3)+
	move.w	#$7d00,(a3)+

.rayon4:	
	subq.w	#1,d1			* pas du rayon
	dbra	d5,.rayon3
	
	lea		36(a0),a0		* pas de l'angle
	cmp.l	a0,a1
	bge.s	.la
	
	move.l	a0,d7
	sub.l	a1,d7
	lea		sinus(pc),a0
	lea		(a0,d7.w),a0
	
.la:
	dbra	d6,.angle2
	
	rts

*-------------------------------------------------------------------------*	
*                            droites                                      *
*-------------------------------------------------------------------------*
hori:
	lea		point(pc),a0	* tableau des bits set
	move.w	centx(pc),d0
	
.dx:
	sub.w	d1,d1			* rayon dans d2
	move.w	#15,d5

.dy:	
	move.w	d0,d2			* ajoute coordonne en x du centre
	move.w	d2,d3
	and.w	#-16,d3			* masque les 4 derniers bits
	sub.w	d3,d2		
	add.w	d2,d2			* calcule la position du masque
	move.w	(a0,d2.w),d4	* recuper le masque
	lsr.w	#1,d3
	
	move.w	d1,d2
	add.w	centy(pc),d2
	muls	#160,d2
	add.w	d2,d3			* offset addresse ecran
	addq.w	#6,d3
	
	move.w	(a4,d3.w),d2	* a4 ad ecran
	and.w	d4,d2
	tst.w	d2
	bne.s	.dplace3
	move.w	d4,(a3)+		* a3 ad tableau
	move.w	d3,(a3)+
	or.w	d4,(a4,d3.w)
	bra.s	.rayon5
	
.dplace3:
	move.w	d4,(a3)+
	move.w	#$7d00,(a3)+

.rayon5:	
	add.w	pasr(pc),d1		* pas du rayon
	dbra	d5,.dy
	
	add.w	pasd(pc),d0
	dbra	d6,.dx
		
	rts
	
*-----------------------------------------------------------------------*

verti:
	lea		point(pc),a0	* tableau des bits set
	move.w	centy(pc),d0
	
.dx2:	
	sub.w	d1,d1		
	move.w	#15,d5

.dy2:	
	move.w	d1,d2
	add.w	centx(pc),d2	* ajoute coordonne en x du centre
	move.w	d2,d3
	and.w	#-16,d3			* masque les 4 derniers bits
	sub.w	d3,d2		
	add.w	d2,d2			* calcule la position du masque
	move.w	(a0,d2.w),d4	* recuper le masque
	lsr.w	#1,d3
	
	move.w	d0,d2
	muls	#160,d2
	add.w	d2,d3			* offset addresse ecran
	addq.w	#6,d3	

	move.w	(a4,d3.w),d2	* a4 ad ecran
	and.w	d4,d2
	tst.w	d2
	bne.s	.dplace4
	move.w	d4,(a3)+		* a3 ad tableau
	move.w	d3,(a3)+
	or.w	d4,(a4,d3.w)
	bra.s	.rayon6
	
.dplace4:
	move.w	d4,(a3)+
	move.w	#$7d00,(a3)+

.rayon6:	
	add.w	pasr(pc),d1		* pas du rayon
	dbra	d5,.dy2
	
	add.w	pasd(pc),d0
	dbra	d6,.dx2
	
	rts

*----------------------------------------------------------------------*

obli:	
	lea		point(pc),a0	* tableau des bits set
	move.w	centx(pc),d0
	
.dx3:	
	sub.w	d1,d1		
	move.w	#15,d5

.dy3:
	move.w	d0,d2
	move.w	d0,d3
	and.w	#-16,d3			* masque les 4 derniers bits
	sub.w	d3,d2		
	add.w	d2,d2			* calcule la position du masque
	move.w	(a0,d2.w),d4	* recuper le masque
	lsr.w	#1,d3
	
	move.w	d0,d2
	sub.w	centx(pc),d2
	muls	ray(pc),d2
	add.w	d1,d2
	add.w	centy(pc),d2
	muls	#160,d2
	add.w	d2,d3			* offset addresse ecran
	addq.w	#6,d3
	

	move.w	(a4,d3.w),d2	* a4 ad ecran
	and.w	d4,d2
	tst.w	d2
	bne.s	.dplace5
	move.w	d4,(a3)+		* a3 ad tableau
	move.w	d3,(a3)+
	or.w	d4,(a4,d3.w)
	bra.s	.rayon7
	
.dplace5:
	move.w	d4,(a3)+
	move.w	#$7d00,(a3)+

.rayon7:	
	add.w	pasr(pc),d1		
	dbra	d5,.dy3
	
	add.w	pasd(pc),d0
	dbra	d6,.dx3

	rts


EGYPTcopy:

	movem.l a2-a3,-(sp)

	move.l	d0,a2
	subq.w	#1,d1

	moveq.l	#-1,d0

	lea		$ffff8a20.w,a3
	move.w	#$8,(a3)+ 			 ; xinc source
	move.w	#-152+2,(a3)+        ; yinc source
	move.l	a0,(a3)+			 ; adr source
	move.l	d0,(a3)+			 ; end masks
	move.w	d0,(a3)+
	move.l	$ffff8a20.w,(a3)+	 ; xinc + yinc dest
	move.l	a1,(a3)+			 ; adr dest
	move.w	#20,(a3)+			 ; x size
	addq.l	#2,a3				 ; skip y size
	move.w	#$203,(a3)+			 ; HOG + opt

	moveq.l	#1,d0

.loop:
	move.w	d0,$ffff8a38.w		; ysize	
	move.w	#$C000,(a3)			; go

	move.w	d0,$ffff8a38.w		; ysize	
	move.w	#$C000,(a3)			; go

	move.w	d0,$ffff8a38.w		; ysize	
	move.w	#$C000,(a3)			; go

	add.w	(a2)+,a0
	move.l	a0,$ffff8a24.w		; adr source
	lea		160(a1),a1
	move.l	a1,$ffff8a32.w		; adr dest

	dbra	d1,.loop
	
	movem.l (sp)+,a2-a3

	rts
	
;-------------------------------------------------------------------------------------------------
	Section	data
;-------------------------------------------------------------------------------------------------
	
cosinus:	incbin	cosinus.kra
sinus:		incbin	sinus.kra
point:		dc.w	32768,16384,8192,4096,2048,1024,512,256,128,64,32,16,8,4,2,1	

;-------------------------------------------------------------------------------------------------
; 	VARIABLES & BUFFERS						  
;-------------------------------------------------------------------------------------------------

text:			 dc.b	"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.': "
				 
baratin:		 dc.b	"$3   $1  EGYPTIA  THE 7700 POINTS NEAR 50 KHZ MODULE"
				 dc.b	" PLAYED FLEXISCROLL    $2     $3   $1 THANX TO MARC"
				 dc.b	" FOR LENDING HIS STE TO KRAG TO REALISE THIS SCREEN"
				 dc.b	" DURING HOLIDAYS.  $2     $3   $1"
				 dc.b   "LET'S JOIN IN OUR EGYPTIAN FEELINGS THE WORLD OF OSIRIS AND RHA    $2     $3   $1" 
				 dc.b	"ENTER IN OUR DEMO TEMPLE AND GET READY TO BE MUMMIFIED BY THE SHORTNESS OF THIS TEXT    $2     $4      $0"
 				 
		even

currentbarratin: dc.l	0
currenttableau:  dc.l	0
image:	         dc.l	0
font:	         dc.l	0
code:	         dc.l   0
tabcou:			 ds.l	nbcourbe
tabcouend:
coucour:		 ds.l	1	
depha1:			 ds.w	1
depha2:			 ds.w	1
pasr:			 ds.w	1
pasd:			 ds.w	1
pasd2:			 ds.w	1
ray:			 ds.w	1
centy:			 ds.w	1
centx:			 ds.w	1
buffer:			 ds.w	16
ou:				 ds.l	60
lettre:			 ds.l	nbr+2

    Section bss

tableau:         ds.b	40000
