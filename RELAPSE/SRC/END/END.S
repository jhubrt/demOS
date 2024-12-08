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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\END.PRX

	opt	o+

*-------------------------------------------------------------------------
*	END SCREEN 'RELAPSE'			CYBERNETICS 1992 
*									  
*	'BOBS MIGRATION OVER A ZOOM LAND'
*-------------------------------------------------------------------------

cpu:		set	0
nbre_image:	set	12
*-------------------------------------------------------------------------*

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
    
    ; common
    dc.l	init
    dc.l	update
    dc.l	shutdown
exitflag:	
	dc.w	0

init:
		movem.l	a0-a1/d0,-(sp)

		move.l	4(a0),zoom			; ds.b	252000

		move.l	(a0),d0				; buffer_ec:	ds.b	256*132*2
		add.l	#256,d0
		clr.b	d0
		move.l	d0,bufferec
		move.l	d0,ecran1
		add.l	#132*256,d0
		move.l	d0,ecran2
		add.l	#132*256,d0
		move.l	d0,ecran3

		move.l	8(a0),wizcall

		moveq.l #nbre_image-1,d0
		lea		12(a0),a0
		move.l	(a0),actimg
		lea		tabimg(pc),a1
.loop:
		move.l	(a0)+,(a1)+
		dbra	d0,.loop

		move.b	#4,$ffff820f.w

		move.w	#2,$ffff8a20.w		* Inc x source
		move.w	#2,$ffff8a22.w		* Inc y source
		move.w	#8,$ffff8a2e.w		* Inc x dest
		move.w	#-1,$ffff8a28.w		* End masks
		move.w	#-1,$ffff8a2a.w		
		move.w	#-1,$ffff8a2c.w
		move.w	#$207,$ffff8a3a.w	* HOG + OP

		movem.l	(sp)+,a0-a1/d0

		rts
		
	

flip:
		move.l	ecran1(pc),d0
		move.l	ecran2(pc),ecran1
		move.l	d0,ecran2

		lsr.l	#8,d0
		lea		$FFFF8201.w,a0
		movep.w	d0,(a0)

		rts

*-------------------------------------------------------------------------*
*	MAIN LOOP							  
*-------------------------------------------------------------------------*
update:
		movem.l	a0-a6/d0-d7,-(sp)

		bra	construc

main:	stop 	#$2300

		ifne	cpu
		move.w	#7,$ffff8240.w
		endc

		bsr.s	flip

		move.w	cpt(pc),d0
		tst.w	d0
		beq		zoomin
		clr.w	calc
		cmp.w	#1,d0
		beq.s	calctraj
		cmp.w	#210,d0
		blt		anim	
		cmp.w	#210,d0
		beq		construc
		move.w	#-1,calc
		cmp.w	#211,d0
		beq.s	calctraj
		cmp.w	#415,d0
		blt		anim
		bra		zoomout
return:	addq.w	#1,cpt

		ifne	cpu
		move.w	#0,$ffff8240.w
		endc

		tst.w	exitflag
		bne.s	end
		
		tst.w	cptimage
		bne.s	main

end:		
		movem.l	(sp)+,a0-a6/d0-d7

		rts


		;-------------------------------------------------------------
shutdown:	

		rts		


zero:	ds.w	16


rd4:	dc.w	0
rd6:	dc.w	0

*-------------------------------------------------------------------------*
*	Calcul trajectoires: apparition					  *
*-------------------------------------------------------------------------*
calctraj:	
		lea		pospr(pc),a1		* Position des sprites
		lea		arrpos(pc),a2		* Position d'arr des sprites
		lea		tx(pc),a3			* Taux pour deplacement
		lea		tabtext(pc),a4
		move.w	nbimage(pc),d0
		add.w	d0,d0
		move.l	(a4,d0.w),a4		* Texte
		move.l	aleopos(pc),a5		* Table aleatoire
		lea		bobs_tab(pc),a6		* Table de sprites
		
		moveq.l	#0,d5			
		moveq.l	#4,d6
		moveq.l	#10,d7
		move.w	#-1,nb			

traj:	move.b	(a4)+,d0			* Caractere dans le texte
		cmp.b	#32,d0				* Si espace pas de sprite a gerer
		beq		nogest
		cmp.b	#".",d0
		bne.s	okette
		addq.w	#8,d5		
		bra		nogest
okette:		
		cmp.b	#"@",d0			* Si arobace fin generation trajectoire
		beq	fintraj

		addq.w	#1,nb			* Nb de sprite … gerer

		sub.b	#65,d0
		ext.w	d0
		lsl.w	#6,d0
		lea		sprite(pc),a0
		add.w	d0,a0
		move.l	a0,(a6)+

		move.w	(a5)+,d1		* Nb d'etapes

		tst.w	calc
		bne.s	calc2		
						*--------- X
		move.l	d5,d2			* X arrivee
		asl.l	d7,d2
		move.l	d2,(a2)+		

		move.w	(a5)+,d3		* Nombre aleo
		ext.l	d3
		asl.l	d7,d3
		move.l	d3,(a1)+		
		
		sub.l	d3,d2
		divs.w	d1,d2
		ext.l	d2
		move.l	d2,(a3)+

						*--------- Y
		move.l	d6,d2			* Y arrivee
		asl.l	d7,d2
		move.l	d2,(a2)+		

		move.w	(a5)+,d3		* Nombre aleo
		ext.l	d3
		asl.l	d7,d3
		move.l	d3,(a1)+
		
		sub.l	d3,d2
		divs.w	d1,d2
		ext.l	d2
		move.l	d2,(a3)+
		bra.s	endcalc

calc2:
						*--------- X
		move.l	d5,d3			* X arrivee
		asl.l	d7,d3
		move.l	d3,(a1)+		

		move.w	(a5)+,d2		* Nombre aleo
		ext.l	d2
		asl.l	d7,d2
		move.l	d2,(a2)+		
		
		sub.l	d3,d2
		divs.w	d1,d2
		ext.l	d2
		move.l	d2,(a3)+

						*--------- Y
		move.l	d6,d3			* Y arrivee
		asl.l	d7,d3
		move.l	d3,(a1)+		

		move.w	(a5)+,d2		* Nombre aleo
		ext.l	d2
		asl.l	d7,d2
		move.l	d2,(a2)+
		
		sub.l	d3,d2
		divs.w	d1,d2
		ext.l	d2
		move.l	d2,(a3)+

endcalc:	
		cmp.l	#aleo+3000,aleopos
		bne.s	nogest
		move.l	#aleo,aleopos
nogest:		
		add.w	#16,d5	
		cmp.w	#320,d5
		blt.s	noligne
		moveq.l	#0,d5
		add.w	#17,d6
noligne:
		bra	traj
fintraj:
		bra	return

calc:		dc.w	0

*-------------------------------------------------------------------------*
*	ANIMATION DES SPRITES						  *
*-------------------------------------------------------------------------*

anim:		
		move.w	#2,$ffff8a20.w		* Inc x source
		move.w	#2,$ffff8a22.w		* Inc y source
		move.w	#8,$ffff8a2e.w		* Inc x dest
		move.w	#8+8,$ffff8a30.w	* Inc y dest

		move.l	ecran2(pc),d0
		addq.l	#6,d0
		move.l	d0,$ffff8a32.w			* Effacage ecran

		move.w	#20,$ffff8a36.w			* Hori size
		move.w	#200,$ffff8a38.w		* Vert size
 		clr.w	$ffff8a3a.w				* Mode HOG + OP
		move.w	#$C000,$ffff8a3c.w  * GO
noeff:
		cmp.w	#-1,nb
		bne.s	oktrac
		bra		return
		
oktrac:		
		move.b	#2,$ffff8a3a.w			* Mode HOG
		move.b	#7,$ffff8a3b.w			* Or
		move.w	#2,$ffff8a36.w			* Hori size
		move.w	#168-16+8,$ffff8a30.w	* Inc y dest

		move.w	nb(pc),d0				* Nb of sprites
		moveq.l	#10,d7					* Nombre de decalages pour calcul
		lea		pospr(pc),a1			* Position sprites
		lea		tx(pc),a4				* Taux pour trajectoires
		lea		arrpos(pc),a5			* Position d'arrivee
		lea		bobs_tab(pc),a6			* Liste des sprites a afficher
		move.l	ecran2(pc),a2			* Adr affichage
		addq.w	#6,a2
		
aff_spr:	
		move.l	(a1)+,d1				* X pos
		move.l	(a1)+,d2				* Y pos

		move.l	(a5)+,d3
		move.l	(a5)+,d4

		sub.l	d1,d3
		bpl.s	ok1
		neg.l	d3
ok1:		
		sub.l	d2,d4
		bpl.s	ok2
		neg.l	d4
ok2:		
		and.l	#$fffffe00,d3
		bne.s	dep
		and.l	#$fffffe00,d4
 		bne.s	dep  
		
		move.l	-8(a5),d1
		move.l	-4(a5),d2
		
		addq.w	#8,a4
		bra.s	nodep
		
dep:		
		add.l	(a4)+,d1
		move.l	d1,-8(a1)
		add.l	(a4)+,d2
		move.l	d2,-4(a1)
nodep:
		asr.l	d7,d1
		asr.l	d7,d2
		move.l	(a6)+,a3

		moveq.l	#16,d3
		cmp.w	#184,d2
		ble.s	noclipy1
		move.w	#200,d3
		sub.w	d2,d3
noclipy1:
		tst.w	d2
		bge.s	noclipy2
		add.w	d2,d3
		add.w	d2,d2
		add.w	d2,d2
		sub.w	d2,a3
		moveq.l	#0,d2
noclipy2:
		cmp.w	#-16,d1
		bge.s	noclipx1
		moveq.l	#0,d3	
noclipx1:
		cmp.w	#320,d1
		blt.s	noclipx2
		moveq.l	#0,d3
noclipx2:	
		tst.w	d3
		ble.s	noaff
		move.w	d3,$ffff8a38.w		* Vert size
    	move.l	a3,$ffff8a24.w		* Adr source
		mulu.w	#168,d2
		lea	(a2,d2.l),a3
		move.w	d1,d2
		and.w	#$fff0,d1
		asr.w	#1,d1
		add.w	d1,a3
		and.w	#15,d2
		or.w	#$C000,d2
		move.l	a3,$ffff8a32.w		* Adr dest
		move.w	d2,$ffff8a3c.w
noaff:
		dbra.w	d0,aff_spr		

		bra	return

*-------------------------------------------------------------------------
*	ZOOM IN
*-------------------------------------------------------------------------
construc:	
		move.w	tempcpt(pc),d0
		lea		tabtemp(pc),a0
		add.w	d0,a0
		move.w	(a0)+,d0
.temps:		
		stop	#$2300

		tst.w	exitflag
		bne		end

		lea		wizinfo,a0
		move.l	wizcall,a1
		jsr		(a1)
		cmp.w	(a0),d0
		bgt.s	.temps

		addq.w	#2,tempcpt
		
		move.l	actimg(pc),ancimg
		lea		tabimg(pc),a0
		add.w	cptimage(pc),a0
		addq.w	#4,cptimage
		cmp.w	#nbre_image*4,cptimage
		bne.s	.okimg
		clr.w	cptimage	
.okimg:
		move.l	(a0),actimg

		moveq.l	#16,d5
		moveq.l	#10,d6
		moveq.l	#18,d7

		move.l	(a0),a0
		lea		32(a0),a0
		move.l	zoom(pc),a2

zoomloop1:				*** CONSTRUCTION
		move.w	d5,d2
		move.w	d6,d3
			
		move.w	#320,d0
		sub.w	d2,d0
		lsr.w	#1,d0
		move.w	d0,d4
		and.w	#15,d4
		and.w	#$fff0,d0
		lsr.w	#3,d0
		mulu.w	#3,d0
		lea		(a2,d0.w),a1

		move.w	#320,d0
		move.w	#199,d1
		movem.l	a0-a2/d5-d7,-(sp)
		bsr		zoomer
		movem.l	(sp)+,a0-a2/d5-d7

		move.w	d6,d4
		mulu.w	#120,d4
		add.w	d4,a2

		add.w	#16,d5
		add.w	#10,d6
		
		tst.w	exitflag
		bne		end

		dbra.w	d7,zoomloop1

		bra	return

zoomin:
		move.l	ecran1(pc),a0
		move.l	ecran2(pc),a1
		move.w	#199,d0
.clear:	rept	40
		clr.l	(a0)+
		clr.l	(a1)+
		endr
		addq.w	#8,a0
		addq.w	#8,a1
		dbra	d0,.clear

		move.l	actimg(pc),a0
		lea		$ffff8240.w,a1
		rept	8
		move.l	(a0)+,(a1)+
		endr
		move.l	ecran3(pc),a1
		move.w	#199,d0
.copy:
		rept	40
		move.l	(a0)+,(a1)+
		endr
		addq.w	#8,a1
		dbra.w	d0,.copy

		moveq.l	#16,d5
		moveq.l	#10,d6
		moveq.l	#18,d7
		move.l	zoom(pc),a2

		move.w	#6,$ffff8a20.w		* Inc x source
		move.w	#6+12,$ffff8a22.w	* Inc y source
		move.w	#8,$ffff8a2e.w		* Inc x dest
		move.w	#8+8+16,$ffff8a30.w	* Inc y dest
		move.w	#$203,$ffff8a3a.w
		move.w	#18,$ffff8a36.w		* Hori size

.zoomloop2:							* AFFICHAGE
		move.l	ecran2(pc),a1
        bsr     flip
		stop	#$2300

		move.w	#200,d0
		sub.w	d6,d0
		lsr.w	#1,d0
		mulu.w	#168,d0
		lea		8(a1,d0.w),a0

		ifne	cpu
		move.w	#$70,$ffff8240.w		
		endc

		lea		6(a2),a3

		rept	3
		move.w	d6,$ffff8a38.w			* Vert size
		move.l	a0,$ffff8a32.w			* Dest
		move.l	a3,$ffff8a24.w			* Source
		move.w	#$C000,$ffff8a3c.w		* Go
		addq.w	#2,a3
		addq.w	#2,a0
		endr

		move.w	d6,d4
		mulu.w	#120,d4
		add.w	d4,a2
	
		add.w	#16,d5
		add.w	#10,d6

		ifne	cpu
		move.w	#$0,$ffff8240.w		
		endc

		dbra.w	d7,.zoomloop2	

		move.l	ecran3,d0
		lsr.l	#8,d0
		lea		$ffff8201.w,a0
        movep	d0,(a0)
		stop	#$2300

		move.l	actimg(pc),a0
		lea		32(a0),a0
		move.l	ecran1(pc),a1
		move.l	ecran2(pc),a2
		move.w	#199,d0
.copy2:
		rept	40
		move.l	(a0),(a1)+
		move.l	(a0)+,(a2)+
		endr
		addq.w	#8,a1
		addq.w	#8,a2
		dbra.w	d0,.copy2

		bra	return

*-------------------------------------------------------------------------*
*	ZOOM OUT							  *
*-------------------------------------------------------------------------*
zoomout:	
	move.l		ancimg(pc),a0
	move.w		#-1,d0
light:	
	moveq.l		#0,d4			* Compteur de boucles
	cmp.w		#1,d0			* Increment (fade in/out)
	beq.s		oklight
	moveq.l		#15,d4
oklight:
	
looplight:						* Boucle principale
	move.l		a0,a1			* Adresse palette
	move.l		d4,d2
	lsl.w		#4,d2	
	lea			$ffff8240.w,a2
	moveq.l		#15,d6
	
light_colors:
	move.w  	(a1)+,d7		* Couleur de la palette (1 mot)
	
	move.w		d7,d1			* Bleu
	and.w		#15,d1
	add.w		d1,d1		
	btst.l		#4,d1
	beq.s		nobit1_1
	or.w		#1,d1	
nobit1_1:
	and.w		#15,d1
	move.w		d2,d5
	mulu.w		d1,d5
	lsr.w		#8,d5
	ror.w		#1,d5	
	btst.l		#15,d5
	beq.s		nobit1_2
	or.w		#8,d5	
nobit1_2:
	and.w		#15,d5
	move.w		d5,d3	

	
	move.w		d7,d1			* Vert
	lsr.w		#4,d1
	and.w		#15,d1
	add.w		d1,d1		
	btst.l		#4,d1
	beq.s		nobit2_1
	or.w		#1,d1	
nobit2_1:
	and.w		#15,d1
	move.w		d2,d5
	mulu.w		d1,d5
	lsr.w		#8,d5
	ror.w		#1,d5	
	btst.l		#15,d5
	beq.s		nobit2_2
	or.w		#8,d5	
nobit2_2:
	and.w		#15,d5
	lsl.w		#4,d5
	or.w		d5,d3	
	
	
	move.w		d7,d1			* Rouge
	lsr.w		#8,d1
	and.w		#15,d1
	add.w		d1,d1		
	btst.l		#4,d1
	beq.s		nobit3_1
	or.w		#1,d1	
nobit3_1:
	and.w		#15,d1
	move.w		d2,d5
	mulu.w		d1,d5
	lsr.w		#8,d5
	ror.w		#1,d5	
	btst.l		#15,d5
	beq.s		nobit3_2
	or.w		#8,d5	
nobit3_2:
	and.w		#15,d5
	lsl.w		#8,d5
	or.w		d5,d3	
	
	move.w		d3,(a2)+
	
	dbra.w		d6,light_colors

	move.w		#5000,d6
temp:	
	move.l		4(sp),4(sp)  
	dbra.w		d6,temp
	
	add.w		d0,d4
	beq.s		exit
	cmp.w		#17,d4
	beq.s		exit
	bra			looplight
exit:	
	move.l		bufferec(pc),a0
	move.w		#132*256*2/4-1,d0
	moveq.l		#0,d1
eff:	
	move.l		d1,(a0)+
	dbra.w		d0,eff

	move.w		#-1,cpt
	addq.w		#2,nbimage
	cmp.w		#nbre_image*2,nbimage
	bne.s		okimg2
	clr.w		nbimage	
okimg2:
	bra			return

*-------------------------------------------------------------------------
plane:		set	3			* Nombre de plan
endline:	set	1			* Endline creation (on/off)

*-------------------------------------------------------------------------
*      ZOOMER Source>Dest
*      Routine generale (pas d'optimisation selon cas particuliers)	
*-------------------------------------------------------------------------

zoomer:		
	   * Parametrages & autogeneration
		
		;ifne		cpu
		;move.w		#7,$ffff8240.w		* Cpu time
		;endc
		
		move.l		a0,-(sp)			* Sauve adresse de depart
		move.l		a1,-(sp)			* Sauve adresse d'affichage
		
		swap		d2					* Dxdest*65536
		clr.w		d2
		divu.w		d0,d2				* Coef x: Dxdest*65536/Dxsource
		moveq.l		#0,d5				* Accumulateur … z‚ro
		lea			code(pc),a2			* Buffer pour g‚n‚ration
		tst.w		d4					*
		beq.s		.nomoveq			*
		move.w		#$7c00,(a2)+
.nomoveq:	
		moveq.l		#0,d7				* Compteur pour pixels dest
		bset.l		d4,d7				* initialise selon offset pixel
	
		lea			$de47,a3			* Add.w   d7,d7
		lea			$dd46,a4			* Addx.w  d6,d6
		lea			$3e185c48,a5		* Move.w  (a0)+,d7 & Addq.w #6,a0
		lea			$32c65849,a6		* Move.w  d6,(a1)+ & Addq.w #4,a1
		
		add.w		d0,d0				* Calcul adresse de retour
		lea			table(pc),a0		* dans la routine d'autogeneration
		move.w		(a0,d0.w),d0		*
		lea			auto_g1(pc),a1		* Pointe sur debut routine d'autogeneration
		lea			(a1,d0.w),a0		* Position de retour dans la routine d'autogeneration
		move.w		(a0),-(sp)			* Sauve mot dans la routine
		move.w		#$4e75,(a0)			* Place un Rts dans la routine (selon longueur)
		jsr			(a1)				* Saut a la routine d'autogen 
		move.w		(sp)+,(a0)			* Restaure mot dans la routine
		ifne		endline				* Si option activee
		cmp.w		#1,d7				* Teste si le dernier mot de destination
		beq.s		.dadadou			* a ete affiche
		moveq.l		#0,d6				* sinon
.bibili:	
		addq.w		#1,d6				* recherche le decalage
		add.w		d7,d7				* a effectue sur d6
		bcc.s		.bibili				* et le genere:
		move.l		#$7e00ef6e,(a2)		* Moveq.l #x,d7 & Lsl.w	d7,d6 
		move.b		d6,1(a2)			*
		addq.l		#4,a2				*
		move.w		#$32c6,(a2)+		* Move.w d6,(a1)+
.dadadou:	
		endc
		move.w		#$4e75,(a2)+		* On place un Rts … la fin du code g‚n‚r‚
		move.l		(sp)+,a1			* Restaure l'adresse d'affichage 
		move.l		(sp)+,a0			* Restaure l'adresse de depart

		;ifne		cpu		
		;move.w		#$70,$ffff8240.w * Cpu time
		;endc

		swap		d3			* Hauteur dest * 65536
		clr.w		d3
		divu.w		d1,d3		* Coef x: Dxdest*65536/Dxsource
		moveq.l		#0,d5		* Accumulateur … z‚ro
		move.l		a0,a3		* Adresses video dans a3 & a4
		move.l		a1,a4		*

		* Appel routine autogeneree : Zoom 	(non optimisee)

		ifeq	plane-3
.haut3:	add.w	d3,d5			* Taux d'erreur
		bcc.s	.noaff3			* Si taux>=1
		move.l	a3,a0		
		move.l	a4,a1		
		bsr		code
		lea		2(a3),a0		
		lea		2(a4),a1		
		bsr		code
		lea		4(a3),a0		
		lea		4(a4),a1		
		bsr		code
		lea		120(a4),a4
.noaff3:	
		lea		160(a3),a3
		;ifne	cpu
		;not.w	$ffff8240.w
		;endc
		dbra.w	d1,.haut3
		endc

		;ifne	cpu
		;clr.w	$ffff8240.w
		;endc
		
		rts

*-------------------------------------------------------------------------
*     ROUTINE D'AUTOGENERATION
*-------------------------------------------------------------------------

tagada:	macro
		move.w		a3,(a2)+	  * Add.w d7,d7
		add.w		d2,d5		  * Tx d'erreur
		dc.w		$640A         * Si tx>=1
		move.w		a4,(a2)+      * Addx d6,d6
		add.w		d7,d7	      * Si seize pixels destination
		dc.w		$6404		  * Bcc.s + 4
		move.l		a6,(a2)+      * Pose le plan sur l'ecran
		moveq.l		#1,d7		  * compteur
		endm

auto_g1:	
		Rept		20
		move.l		a5,(a2)+ 	  * d7 ... (move & addq)
		move.w		a3,(a2)+	  * Add.w d7,d7
		add.w		d2,d5		  * Tx d'erreur
		dc.w		$640A         * Si tx>=1
		move.w		a4,(a2)+      * Addx d6,d6
		add.w		d7,d7	      * Si seize pixels destination
		dc.w		$6404		  * Bcc.s + 4 
		move.l		a6,(a2)+      * Pose le plan sur l'ecran
		moveq.l		#1,d7		  * compteur

		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		tagada
		endr

		rts							* Au cas ou

*-------------------------------------------------------------------------*
* Buffer pour code genere	
	
code:		DS.B	1600

*-------------------------------------------------------------------------*
	
i:		set	0
table:	rept	20		* Plan de la routine d'autogeneration
		dc.w	i
i:		set	i+18
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		dc.w	i
i:		set	i+16
		endr
		dc.w	i

*-------------------------------------------------------------------------*
ecran1:		dc.l	0
ecran2:		dc.l	0
ecran3:		dc.l	0
bufferec:	dc.l	0
zoom:		dc.l	0
wizcall:	dc.l	0
wizinfo:	dc.b	0
			dc.b	0
			dc.w	0
*-------------------------------------------------------------------------*
cpt:		dc.w	-1
nbimage:	dc.w	0
*-------------------------------------------------------------------------*
sprite:		incbin	conclus.bin
nb:			dc.w	0
pospr:		ds.l	100*2
arrpos:		ds.l	100*2
etap:		ds.w	100
tx:			ds.l	100*2
aleo:		incbin	aleo.bin
aleopos:	dc.l	aleo
bobs_tab:	ds.l	100
tempcpt:	dc.w	0
*-------------------------------------------------------------------------*
tabtemp:	
		dc.w	$0007
		dc.w	$0202
		dc.w	$0402
		dc.w	$0602
		dc.w	$0722
		dc.w	$0A01
		dc.w	$0C02
		dc.w	$0D22
		dc.w	$0F02
		dc.w	$1102
		dc.w	$1314
		dc.w	$1420
		dc.w	$143F

tabtext:	
		dc.l	text1
		dc.l	text2
		dc.l	text3
		dc.l	text4
		dc.l	text5
		dc.l	text6
		dc.l	text7
		dc.l	text8
		dc.l	text9
		dc.l	text10
		dc.l	text11


text1:	dc.b	'    INTRO SCREEN    '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'      .DELOS        '
		dc.b	'                    '
	 	dc.b	'     .GRAPHIX       '
		dc.b	'  .BRAIN BLASTER    '
		dc.b	'     METAL AGES@    '


text2:	dc.b	'   LIQUID OSMOSIS   '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'      .SINIS        '
		dc.b	'                    '
	 	dc.b	'   .VIDEO WORKS     '
		dc.b	'  .BRAIN BLASTER@   '


text3:	dc.b	'     .EGYPTIA       '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b	'        KRAG        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'       .BIP         '
		dc.b	'                    '
	 	dc.b	'     .GRAPHIX       '
		dc.b	'     METAL AGES@    '


text4:	dc.b	'  GRAPHIK SOUND II  '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b	'     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'  BIP   METAL AGES  '
		dc.b	'     OR  SINIS      '
		dc.b	'                    '
	 	dc.b	'     .GRAPHIX       '
		dc.b	'     METAL AGES@    '


text5:	dc.b	'    .INTERLUDE      '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'   .SAMPLE LOOP     '
		dc.b	'    ART OF NOISE    '
		dc.b	'                    '
		dc.b	'     .GRAPHIX       '
		dc.b	'     METAL AGES@    '


text6:	dc.b	'     .CASCADE       '
		dc.b	'                    '
	 	dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'     .GRAPHIX       '
		dc.b	'     .POLARIS       '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'   DELOS OR SINIS@  '


text7:  dc.b	'                    '
		dc.b	'SHADED VISION OF ART'
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b	'        KRAG        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'      .DELOS@       '

text8:	
		dc.b	'    SPACE FILLER    '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        ' 
		dc.b	'      .SINIS        '
		dc.b	'                    '
		dc.b	'   .VIDEO WORKS     '
		dc.b	'  .BRAIN BLASTER@   '

text9:	dc.b	' .BOBS MIGRATIONeee '
		dc.b	'                    '
		dc.b	'        CODE        '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'       .BIP         '
		dc.b	'                    '
		dc.b	'                    '
		dc.b	'       LOADER       '
		dc.b	'     METAL AGES@    '
		

text10:	
		dc.b	'   .INFO SCREEN     '
		dc.b    '     METAL AGES     '
		dc.b	'                    '
		dc.b	'      .MUSIC        '
		dc.b	'    .BIP  REMI      '
		dc.b	'         OR         '
		dc.b	'     BIP  SINIS     '
		dc.b	'                    '
		dc.b	'                    '
		dc.b	'      FASTMENU      '
		dc.b	'        KRAG@       '
		
text11:		dc.b	'@'

*-------------------------------------------------------------------------*
tabimg:		dc.l		0		; image1
			dc.l		0		; image2
			dc.l		0		; image3
			dc.l		0		; image4
			dc.l		0		; image5
			dc.l		0		; image6
			dc.l		0		; image7
			dc.l		0		; image8
			dc.l		0		; image9
			dc.l		0		; image10
			dc.l		0		; image9
			dc.l		0		; image9

cptimage:	dc.w		0
actimg:		dc.l		0
ancimg:		dc.l		0
 