*------------------------------------------------------------------------------  -----------------
*  Copyright J.Hubert 2015
*
* This file is part of rebirth demo
*
*  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
*  the GNU Lesser General Public License as published by the Free Software Foundation, 
*  either version 3 of the License, or (at your option) any later version.
*
*  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
*  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
*  See the GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
*  If not, see <http://www.gnu.org/licenses/>.
*-----------------------------------------------------------------------------------------------

	output  D:\PROJECTS\DEMOS\OUTPUT\FUGIT.O

	opt	o+

	xdef FUGsetData,FUGinitSequence,FUGdtDpack,FUGcopyAux2Screen,FUGxorPass,FUGbufsize

*-------------------------------------------------------------------------
* void FUGsetData(u32 _fontoffs, u32 _fontscan, u32 _fontbitmap, void* _asmbuf)
*-------------------------------------------------------------------------
FUGsetData:
	move.l  d0,fontoffs
    move.l  d1,fontscan
    move.l  d2,font

	move.l	a0,autog
	lea (end_macr_code2-macr_code2+(end_macr_code1-macr_code1)*15)*20+2(a0),a0
	
	move.l	a0,table
	lea		20*16*2(a0),a0
	
	move.l	a0,code

	movem.l	a0-a6/d3-d7,-(sp)
	bsr	init_zoom
	movem.l	(sp)+,a0-a6/d3-d7

    rts

*-------------------------------------------------------------------------
* void FUGinitSequence(void* auxiliaire, void* deltabuffer,u32 _textpos)
*-------------------------------------------------------------------------
FUGinitSequence:
	
	movem.l	a2-a6/d1-d7,-(sp)

    move.l  a0,auxiliaire   * Delta screen … zero
    move.l  a1,delta
    move.l  d0,textpos

	lea		8000(a0),a0
	move.w	#249,d0			*
	moveq.l	#0,d1			*
del_d:
	rept	8	
	move.l	d1,(a0)+		*
	endr
	dbf.w	d0,del_d		*

	move.l	textpos(pc),a0	* Position  sur texte
	move.w	(a0)+,d0		* Numero d'effet
	moveq.l	#0,d1
	cmp.w	#2,d0
	ble.s	loli
	move.l	#26*65536+26,d1
loli:	
    lea     file(pc),a1
	moveq.l	#6*5-1,d2
rempli:	
    move.l	d1,(a1)+
	dbf.w	d2,rempli

	add.w	d0,d0			
	add.w	d0,d0			
	lea	    effects(pc),a1		* Table des effets
	move.l	(a1,d0.w),a3	    * Adresse routine de gestion etapes
	move.w	(a0)+,d7		    * Nombre d'‚tapes
    move.l  a0,textpos			* On reprend la position
	
	move.l	delta(pc),a6		* Pointe sur buffer pour delta animation

etape:					        * Boucle par etape
	move.l	auxiliaire(pc),a0	* Efface le bitmap d'une etape
	moveq.l	#0,d1			
	move.w	#249,d0			
del:	
    rept	8
    move.l	d1,(a0)+		
	endr				    
	dbf.w	d0,del			
					        
	move.l	textpos(pc),a5	* Pointe sur texte

	jsr	(a3)			    * Gestion etape selon effet
	
	moveq.l	#0,d3			* Abscisse lettre courante
	moveq.l	#11,d6			* 12 lettres par message
	lea		file(pc),a4		* Pointe sur file etape

lettre_loop:				* Boucle lettre 
	moveq.l	#0,d0			
	move.b	(a5)+,d0		* Charge caractere courant
	cmp.w	#32,d0			
	beq.s	nothing			* Si espace -> rien

	sub.w	#65,d0

	move.l	font(pc),a0		* Pointe sur bitmap fontes
	move.l  fontoffs(pc),a1
	move.l  fontscan(pc),a2	

	add.w	d0,d0
    add.w   d0,d0
	add.w	2(a1,d0.w),a0	* a0 = font bitmap data
	add.w	0(a1,d0.w),a2	* a2 = scandescription for current letter
				        
	move.l	auxiliaire(pc),a1	* Copie le bloc a la bonne place

	moveq.l	#26,d0			
	sub.w	(a4),d0			
	add.w	d3,d0			* Position dest
	moveq.l	#15,d4			
	and.w	d0,d4			
	and.w	#$fff0,d0		
	lsr.w	#3,d0   		
	add.w	d0,a1			
	
	moveq.l	#53,d0			* Taille de depart horizontale
	move.w	(a2)+,d1		* nb scanlines to zoom

	move.w	(a4),d2			* Etape de la reduction
	add.w	d2,d2			
	movem.l	a3-a6/d3/d6-d7,-(sp)	
	bsr	zoomer			    * Reduit
	movem.l	(sp)+,a3-a6/d3/d6-d7	

nothing:
	add.w	#53,d3			* Position lettre suivante
	lea	10(a4),a4

	dbf.w	d6,lettre_loop

	move.l	auxiliaire(pc),a0
	lea	    8000(a0),a1         * delta_screen

	move.l	a6,a2				* Place pour mettre le nb
	addq.l	#2,a6				* de delta
	moveq.l	#0,d4
    moveq.l	#0,d1
	move.w	#249,d0
pack:	
    rept	8
	move.l	(a0)+,d2
	cmp.l	(a1)+,d2			* DELTA PACK
	beq.s	*+12
	move.w	d1,(a6)+			* Offset
	move.l	d2,(a6)+			* Data … cet offset: delta
	move.l	d2,-4(a1)			
	addq.w	#1,d4
	addq.w	#4,d1
	endr
	dbf.w	d0,pack
	
	move.w	d4,(a2)				* Nombre de delta

	dbf.w	d7,etape			* Boucle sur etapes

	move.l	a5,textpos			* Reprend la pos texte courante

	move.w	#-2,(a6)+			* Fin de l'animation delta

	move.l  auxiliaire(pc),a0	* Efface ecran auxiliaire
	moveq.l	#0,d1				
	move.w	#249,d0			
del2:	
    rept    8
    move.l	d1,(a0)+		
    endr
	dbf.w	d0,del2				

    movem.l	(sp)+,a2-a6/d1-d7
    
    rts


*-------------------------------------------------------------------------
*	u8* FUGdtDpack(u8* _deltabuffer, u8* _auxiliaire)
*   returns new position into delta buffer or NULL is end reached
*-------------------------------------------------------------------------
FUGdtDpack:

	move.w	(a0)+,d0			* DELTA DEPACKING
	cmp.w	#-1,d0				
	beq.s	.empty_frame
	cmp.w	#-2,d0				
	beq.s   .fin_anim
	
    move.w  d1,-(sp)

    btst.l   #0,d0
    beq.s   .depack1
    move.w	(a0)+,d1		    * Charge offset ecran
	move.l	(a0)+,(a1,d1.w)		* Change data sur ecran 
.depack1:
    lsr.w   #1,d0
    subq.w  #1,d0
    blt.s   .depackend

.depack:
    move.w	(a0)+,d1		    * Charge offset ecran
	move.l	(a0)+,(a1,d1.w)		* Change data sur ecran 
    move.w	(a0)+,d1		    * Charge offset ecran
	move.l	(a0)+,(a1,d1.w)		* Change data sur ecran 
	dbf.w	d0,.depack			* Boucle

.depackend:

    move.w  (sp)+,d1

.empty_frame:

    rts

.fin_anim:
    move.l #0,a0
    
    rts


*-------------------------------------------------------------------------
*	void FUGcopyAux2Screen(u8* _auxiliaire, u8* _screenlineadr)
*-------------------------------------------------------------------------
FUGcopyAux2Screen:

    move.l  a2,-(sp)

	lea	$ffff8a20.w,a2			* Copie du deltapack‚ sur l'ecran
	move.l	#$20002,(a2)+		* Increment X source
	move.l	a0,(a2)+	        * Adresse source
	move.l	#-1,(a2)+			* Masques 1 & 2
	move.w	#-1,(a2)+			* Masque 3
	move.l	#$40004,(a2)+		* Increment X dest
	move.l	a1,(a2)+			
	move.l	#40*100*65536+1,(a2)+ * Hori size
	move.l	#$203*65536+%11000000*256,(a2)+	* Source

    move.l  (sp)+,a2

    rts


*-------------------------------------------------------------------------
*	void FUGxorPass(u8* _screenlineadr)
*-------------------------------------------------------------------------
FUGxorPass:
    
    move.l  a2,-(sp)

	lea	$ffff8a20.w,a2    		    * Passe xor pour obtenir la fonte
	move.l	#$40004,(a2)+			* Increment X source
	move.l	a0,(a2)+			    *
	move.l	#-1,(a2)+			    * Masques 1 & 2
	move.w	#-1,(a2)+			    * Masque 3
	move.l	#$40004,(a2)+			* Increment X dest
	lea	    160(a0),a0			    * Adresse dest
	move.l	a0,(a2)+			    *
	move.l	#40*100*65536+1,(a2)+	* Hori size
	move.l	#$206*65536+%11000000*256,(a2)+	* Go & ...

    move.l  (sp)+,a2

    rts


*-------------------------------------------------------------*
*	X dep:		d0
*	Y dep:		d1
*	adr dep:	a0
*	adr arr:	a1


apparition1:
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	cmp.w	#26,d0			
	beq.s	okette			
	addq.w	#1,d0			
	addq.w	#1,etap_c		
okette:	
    lea	file+12*5*2-2(pc),a0	
	moveq.l	#12*5-2,d1		
decal:	
    move.w	-(a0),2(a0)	
	dbf.w	d1,decal		
	move.w	d0,file										
	rts

apparition2:
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	cmp.w	#26,d0			
	beq.s	.okette			
	addq.w	#1,d0			
	addq.w	#1,etap_c		
.okette:lea	file(pc),a0		
	moveq.l	#12*5-1,d1		
.decal:	move.w	d0,(a0)+	
	dbf.w	d1,.decal		
	rts

apparition3:
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	cmp.w	#26,d0			
	beq.s	.okette			
	addq.w	#1,d0			
	addq.w	#1,etap_c		
.okette:
    lea	file(pc),a0		
	moveq.l	#12*5-2,d1		
.decal:	move.w	2(a0),(a0)+	
	dbf.w	d1,.decal		
	move.w	d0,file+12*5*2-2								
	rts

disparition1:	
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	beq.s	.okette			
	subq.w	#1,d0			
	subq.w	#1,etap_c		
.okette:
    lea	file+12*5*2-2(pc),a0	
	move.w	#12*5-2,d1		
.decal:	move.w	-(a0),2(a0)	
	dbf.w	d1,.decal		

	move.w	d0,file			

	rts

disparition2:
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	beq.s	.okette			
	subq.w	#1,d0			
	subq.w	#1,etap_c		
.okette:
    lea	file(pc),a0		
	moveq.l	#12*5-1,d1	
.decal:	move.w	d0,(a0)+
	dbf.w	d1,.decal
	rts

disparition3:	
	move.w	etap_c(pc),d0	* Etape de reduction suivante
	beq.s	.okette			
	subq.w	#1,d0			
	subq.w	#1,etap_c		
.okette:lea	file(pc),a0		
	moveq.l	#12*5-2,d1		
.decal:	move.w	2(a0),(a0)+	
	dbf.w	d1,.decal		
	move.w	d0,file+12*5*2-2	
	rts

*-------------------------------------------------------------------------
*     ZOOMER (comes from Relapse demo 1992)
*-------------------------------------------------------------------------

endline:	set	1			* Endline creation (on/off)

************ Inits

init_zoom:	
        move.l	table(pc),a0		* Calcul de la table
		move.l	autog(pc),a1		
		moveq.l	#19,d0				* de sauts
		moveq.l	#0,d7		
loop1:	
        move.w	d7,(a0)+	
		add.w	#18,d7		
		lea	macr_code1(pc),a6
		move.w	#(end_macr_code1-macr_code1)/2-1,d2
copy_cod1:	
        move.w	(a6)+,(a1)+	
		dbf.w	d2,copy_cod1
						    
		moveq.l	#14,d1		
loop2:		
        move.w	d7,(a0)+	
		add.w	#16,d7

		lea	macr_code2(pc),a6
		move.w	#(end_macr_code2-macr_code2)/2-1,d2
copy_cod2:	
        move.w	(a6)+,(a1)+	
		dbf.w	d2,copy_cod2		

		dbf.w	d1,loop2
		dbf.w	d0,loop1

		move.w	#$4e75,(a1)+

		rts

*-------------------------------------------------------------------------
*
*	Infos autogeneration						  
*
*		add.w		d7,d7	 :	$DE47			  
*		addx.w		d6,d6	 :	$DD46			  
*		move.w		(a0)+,d7 :	$3E18			  
*		or.w		d6,(a1)+ :	$8D59  			  
*		addq.l		#2,a0	 :	$5488			  
*		addq.l		#2,a1	 :	$5489			  
*		moveq.l		#0,d6	 :	$7C00			  
*		moveq.l		#0,d7	 :	$7E00			  
*		lsl.w		d7,d6	 :	$EF6E			  
*		rts			 :	$4E75			  
*									 
*-------------------------------------------------------------------------

*-------------------------------------------------------------------------
*      ZOOMER Source>Dest						  
*									  
*	d0:	X size depart
*	d1:	Y size
*	d2:	X size arrivee
*	d4:	offset pixel	
*	a0:	adr depart
*	a1:	adr arrive
*	a2: adr table offset lignes
*
*-------------------------------------------------------------------------
zoomer:								* Parametrages & autogeneration
		subq.w	#1,d1

		tst.w	d2
		bne.s	.suite		
		rts	
.suite:	
		cmp.w	d2,d0
		bne.s	.reduit

.ly:	add.w	(a2)+,a1
		move.l	(a0)+,(a1)
		move.l	(a0)+,4(a1)
		dbf.w	d1,.ly

		rts

.reduit:
		move.l		a0,-(sp)    	    * Sauve adresse de depart
		move.l		a1,-(sp)	        * Sauve adresse d'affichage
		move.l		a2,-(sp)	        * Sauve adresse d'affichage
		
		swap		d2		        * Dxdest*65536
		clr.w		d2
		divu.w		d0,d2		    * Coef x: Dxdest*65536/Dxsource
		moveq.l		#0,d5		    * Accumulateur … zero
		move.l		code(pc),a2	    * Buffer pour generation
		tst.w		d4		
		beq.s		.nomoveq		
		move.w		#$7c00,(a2)+	
.nomoveq:	
        moveq.l		#0,d7		    * Compteur pour pixels dest
		bset.l		d4,d7		    * initialise selon offset pixel
	
		lea		$de47,a3	        * Add.w   d7,d7
		lea		$dd46,a4	        * Addx.w  d6,d6
		lea		$3e18.w,a5	        * Move.w  (a0)+,d7
		lea		$8d59,a6	        * Or.w  d6,(a1)+
		
		add.w		d0,d0		    * Calcul adresse de retour
		move.l	    table(pc),a0	* dans la routine d'autogeneration
		move.w		(a0,d0.w),d0	
		move.l		autog(pc),a1	* Pointe sur debut routine d'autogeneration
		lea		    (a1,d0.w),a0    * Position de retour dans la routine d'autogeneration
		move.w		(a0),-(sp)	    * Sauve mot dans la routine
		move.w		#$4e75,(a0)	    * Place un Rts dans la routine (selon longueur)
		jsr		    (a1)		    * Saut a la routine d'autogen 
		move.w		(sp)+,(a0)	    * Restaure mot dans la routine
		ifne		endline		    * Si option activee
		cmp.w		#1,d7		    * Teste si le dernier mot de destination
		beq.s		.dadadou		* a ete affiche
		moveq.l		#0,d6		    * sinon
.bibili:		
        addq.w		#1,d6		    * recherche le decalage
		add.w		d7,d7		    * a effectue sur d6
		bcc.s		.bibili		    * et le genere:
		move.l		#$7e00ef6e,(a2) * Moveq.l #x,d7 & Lsl.w	d7,d6
		move.b		d6,1(a2)	
		addq.l		#4,a2		
		move.w		#$8d59,(a2)+    * Or.w d6,(a1)+
.dadadou:	
		endc
		move.w		#$4e75,(a2)+	* On place un Rts … la fin du code genere
		move.l		(sp)+,a2	    * Restaure l'adresse d'affichage 
		move.l		(sp)+,a1	    * Restaure l'adresse d'affichage 
		move.l		(sp)+,a0	    * Restaure l'adresse de depart

.nogen:		
		move.l		a0,a3		    * Adresses video dans a3 & a4
		move.l		a1,a4		

		* Appel routine autogeneree : Zoom 	(non optimisee)

		move.l		code(pc),a5

.haut1:		
		add.w		(a2)+,a4
        move.l		a3,a0	
		move.l		a4,a1		
		jsr		    (a5)
		lea		    8(a3),a3
		dbra.w		d1,.haut1
		
		rts

*-------------------------------------------------------------------------*
*     ROUTINE D'AUTOGENERATION						  *
*-------------------------------------------------------------------------*

macr_code2:	
        move.w		a3,(a2)+	    * Add.w  d7,d7
		add.w		d2,d5		    * Tx d'erreur
		dc.w		$640A           * Si tx>=1
		move.w		a4,(a2)+        * Addx d6,d6
		add.w		d7,d7	        * Si seize pixels destination
		dc.w		$6404		    * Bcc.s + 4
		move.w		a6,(a2)+        * Pose le plan sur l'ecran
		moveq.l		#1,d7		    * compteur
end_macr_code2:

macr_code1:	
        move.w		a5,(a2)+        * d7 ... move 
		move.w		a3,(a2)+	    * Add.w d7,d7
		add.w		d2,d5		    * Tx d'erreur
		dc.w		$640A           * Si tx>=1
		move.w		a4,(a2)+        * Addx d6,d6
		add.w		d7,d7	        * Si seize pixels destination
		dc.w		$6404		    * Bcc.s + 4 
		move.w		a6,(a2)+        * Pose le plan sur l'ecran
		moveq.l		#1,d7		    * compteur
end_macr_code1:

*-------------------------------------------------------------------------*
effects:	
        dc.l	apparition1		* 0
		dc.l	apparition2		* 1
		dc.l	apparition3		* 2
		dc.l	disparition1	* 3
		dc.l	disparition2	* 4
		dc.l	disparition3	* 5
	
textpos:		dc.l   0
etap_c:			ds.w	    1

fontoffs:	    dc.l    0
fontscan:	    dc.l    0
font:		    dc.l    0

auxiliaire:	    dc.l    0
delta:		    dc.l	0

FUGbufsize:		dc.w	(end_macr_code2-macr_code2+(end_macr_code1-macr_code1)*15)*20+2 + 20*16*2 + 400

autog:			dc.l	0
table:			dc.l	0
code:			dc.l	0

file:			ds.w	12*5
