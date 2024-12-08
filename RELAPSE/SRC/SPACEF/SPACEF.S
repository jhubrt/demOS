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

*-------------------------------------------------------------------------
*									  
*    - SLIDE SHOW -			       -CYBERNETICS 1992 -
*									  
*-------------------------------------------------------------------------

		opt	o+
	
		output	D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\SPACEF.PRX


fonth:  set 35


modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
	dc.l	init
	dc.l	main
	dc.l	shutdown
	dc.l	vbl
exitflag:	
	dc.w	0		
haslooped2: dc.b	0
haslooped1: dc.b	0

init:	move.l	(a0),d0
		clr.b	d0
		add.l	#256,d0
		move.l	d0,ecran
		
		lea		$ffff8201.w,a1
		lsr.l	#8,d0
		movep.w d0,(a1)
		clr.b	$ffff820d.w

		move.l	4(a0),depack
		move.l	8(a0),p2c
		move.l	12(a0),tamp
		move.l	16(a0),cloud
		move.l	20(a0),font

		lea		24(a0),a0
		lea		tabimg(pc),a1
		moveq.l #(tabimgend-tabimg)/4-1,d0
.loop:	move.l	(a0)+,(a1)+
		dbra	d0,.loop

		rts

*------------------------- BOUCLE PRINCIPALE -----------------------------*

main:	
		movem.l	d0-d7/a0-a7,-(sp)

endless:
		move.w	#200,d0
.wait:
		stop    #$2300
		tst.w	exitflag
		bne.s  	end
		dbra.w	d0,.wait

		move.b	haslooped1(pc),haslooped2

		addq.w	#1,flipimg
		cmp.w	#24,flipimg
		bne.s	.ok3
		move.w	#1,flipimg	
.ok3:	btst.b	#0,flipimg+1
		bne	text
		bra	app
end:
		movem.l	(sp)+,d0-d7/a0-a7
		rts	

*------------------------------- FIN -------------------------------------*

shutdown:	
		movem.l	d0-d7/a0-a7,-(sp)

		move.w	#14*4,vblcpt
		move.w	#-1,action

		move.l	ecran(pc),a1
		lea		45*160(a1),a1
		lea		31840(a1),a3
		move.w	#99,d2
.fceffb1:	
		moveq.l	#39,d1
.fccop1:	
		clr.l	(a1)+
		clr.l	(a3)+
		dbra.w	d1,.fccop1
		lea		160(a1),a1
		lea		-480(a3),a3
		move.w	#2000,d4
.fczzz:	dbra.w	d4,.fczzz
		dbra.w	d2,.fceffb1

		movem.l	(sp)+,d0-d7/a0-a7
		rts	
			
*---------------------------- ROUTINE VBL --------------------------------*

vbl:	movem.l	d0-d7/a0-a5,-(sp)
		
		lea		pal1(pc),a0
		move.w	vblcpt(pc),d0
		and.w	#$ffc,d0
		lsl.w	#3,d0
		add.w	d0,a0
		
		move.w	action(pc),d1
		beq.s	.okp
		blt.s	.negative
		add.w	d1,vblcpt
		cmp.w	#15*4,vblcpt
		blt.s	.okp
		clr.w	action
		bra.s	.okp	
.negative:
		add.w	d1,vblcpt
		tst.w	vblcpt
		bgt.s	.okp
		clr.w	action
.okp:
		lea		$ffff8240.w,a1
		rept	8
		move.l	(a0)+,(a1)+
		endr

		clr.b	$fffffa1b.w		* PARAMETRAGES TIMER B
		move.b	#42,$fffffa21.w
		move.l	#timerb,$120.w
		or.b	#1,$fffffa07.w
		move.b	#8,$fffffa1b.w		

		not.w	flip
		beq.s	.noaff
		move.l	cloud(pc),a0
		add.l	nuage(pc),a0
		add.l	#42*160,nuage
		cmp.l	#42*160*27,nuage
		bne.s	.noreset
		clr.l	nuage
.noreset:	
		move.l	ecran(pc),a1		* NUAGES
		moveq.l	#41,d0
.copy2:	rept	40
		move.l	(a0)+,(a1)+
		endr	
		dbra.w	d0,.copy2
.noaff:	
		movem.l	(sp)+,d0-d7/a0-a5

		move.l  (a0)+,a1
		jmp		(a1)

*----------------------------- ROUTINE TIMER B ---------------------------*
timerb:		
		move.l	a0,-(sp)		* CHANGEMENT DE PALETTE
		move.l	a1,-(sp)
		lea		$ffff8240.w,a1
		lea		pal2(pc),a0
		rept	8
		move.l	(a0)+,(a1)+
		endr
		move.l	(sp)+,a1
		move.l	(sp)+,a0
	
		clr.b	$fffffa1b.w		* PARAMETRAGES TIMER B
		move.b	#156,$fffffa21.w
		move.b	#8,$fffffa1b.w
		move.l	#overscan,$120.w
		bclr.b	#0,$fffffa0f.w
		rte

overscan:	
		move.w	d0,-(sp)		* SAUVEGARDE REGISTRES
		move.w	d1,-(sp)
		move.w	ecran+2(pc),d0
		add.w	#199*160,d0
att:		cmp.b	$ffff8209.w,d0		* SYNCHRO SUR FIN 199 EM LIGNE
		bgt.s	att
att2:		move.b	$ffff8209.w,d1		* SYNCHRO SUR DEBUT 200 EM LIGNE
		cmp.b	d1,d0
		ble.s	att2
		neg.b	d1			* SYNCHRO LSL
		add.b	#16,d1
		lsl.w	d1,d1
		rept	55			* NO BORDER BAS
		nop
		endr		
		clr.b	$ffff820a.w		
		rept	20
		nop
		endr
		move.b	#$2,$ffff820a.w
		move.w	(sp)+,d1		* RESTAURE REGISTRES
		move.w	(sp)+,d0
		bclr.b	#0,$fffffa0f.w
		clr.b	$fffffa1b.w		* STOP

		rte				* FIN D'INTERS

*-------------------------------- TEXTE ----------------------------------*

textshift:	dc.w	0

text:	move.l	font(pc),a1			* DEPACK LA FONTE
		addq.l	#4,a1
		move.l	tamp(pc),a0
		move.l	depack(pc),a2
		jsr		(a2)

		tst.w	exitflag
		bne  	end

		move.w		#200,d0
		move.w		#20,d1
		clr.w		d2
		move.l		tamp(pc),a0
		lea			32(a0),a0
		move.l		a0,a1
		move.l		p2c(pc),a2
		jsr			(a2)

		tst.w	exitflag
		bne  	end

		bsr		clear

		move.l	tamp(pc),a0			* FIXE LA PALETTE
		lea		pal2(pc),a1			* DE LA FONTE
		rept	8
		move.l	(a0)+,(a1)+
		endr
		
		lea		tabtext(pc),a0		
		move.w	nbtext(pc),d0
		add.w	d0,d0
		add.w	d0,d0
		move.l	(a0,d0.w),a0		* ADRESSE DU TEXTE 

		moveq.l	#0,d0				* COMPTEUR
print:		
		tst.w	exitflag
		bne		end

		move.l	ecran(pc),a2		* CALCUL ADRESSE AFFICHAGE LETTRE
		lea		(45+20)*160(a2),a2
		moveq.l	#0,d1
		move.w	d0,d1
		divu.w	#10,d1
		swap	d1
		lsl.w	#4,d1
		add.w	d1,a2
		swap	d1
		mulu.w	#(fonth+5)*160,d1
		add.w	d1,a2		

		move.l	tamp(pc),a1
		lea		32(a1),a1			* CALCUL ADRESSE LETTRE
		move.b	(a0)+,d1
		
		cmp.b	#'<',d1
		bne.s	.dontshift0
		move.b	(a0)+,d1
		clr.w	textshift
		bra.s	.dontshift8
.dontshift0:
		cmp.b	#'>',d1
		bne.s	.dontshift8
		move.b	(a0)+,d1
		move.w	#8,textshift
.dontshift8:

		ext.w	d1
		sub.w	#65,d1
		cmp.w	#26,d1
		bge 	.noaff2
		ext.l	d1
		divu.w	#10,d1
		swap	d1
		lsl.w	#4,d1
		add.w	d1,a1
		swap	d1
		mulu.w	#fonth*160,d1
		add.w	d1,a1
		lea	(fonth-2)*160(a2),a4
		lea	(fonth-2)*160(a1),a5

		move.w	textshift(pc),d2
		add.w	d2,a2
		add.w	d2,a4

		moveq.l	#(fonth/2)-1,d3			* AFFICHAGE LETTRE

.let:   
		move.l	(a1)+,(a2)+
		move.l	(a1)+,(a2)+
		move.l	(a1)+,(a2)+
		move.l	(a1)+,(a2)+
		move.l	(a5)+,(a4)+
		move.l	(a5)+,(a4)+
		move.l	(a5)+,(a4)+
		move.l	(a5)+,(a4)+

		lea	160+144(a1),a1
		lea	160+144(a2),a2
		lea	-(160+176)(a4),a4
		lea	-(160+176)(a5),a5

		move.w	#1400,d2
.zzz:	dbra.w	d2,.zzz

		dbra.w	d3,.let

.noaff2:
		addq.w	#1,d0			* COMPTEUR DE BOUCLE
		cmp.w	#40,d0
		bne		print
		
		addq.w	#1,nbtext
		cmp.w	#12,nbtext
		bne.s	.ok2
		clr.w	nbtext	
.ok2:
		bra	endless
		
*------------------------------ APPARITIONs ------------------------------*

app:	
		move.w		nbimg(pc),d0	* ADRESSE IMAGE COMPACTEE
		add.w		d0,d0		
		add.w		d0,d0
		lea			tabimg(pc),a0
		move.l		(a0,d0.w),a1

		addq.w		#1,nbimg		* COMPTEUR IMAGES
		cmp.w		#11,nbimg
		bne.s		.ok1
		move.b		#-1,haslooped1
		clr.w		nbimg
.ok1:
		move.l		tamp(pc),a0		* DECOMPACTAGE IMAGE DANS LE TAMPON
		move.l		depack(pc),a2
		addq.l		#4,a1
		jsr			(a2)
	
		tst.w	exitflag
		bne  	end

		move.w		#200,d0
		move.w		#20,d1
		clr.w		d2
		move.l		tamp(pc),a0
		lea			32(a0),a0
		move.l		a0,a1
		move.l		p2c(pc),a2
		jsr			(a2)

		tst.w	exitflag
		bne  	end

		bsr			clear		
	
		move.l		tamp(pc),a0		* APPARITIONS
		move.l		ecran(pc),a1
		lea			45*160(a1),a1
		lea			pal2(pc),a2		* PALETTE
		rept		8
		move.l		(a0)+,(a2)+
		endr

		addq.w		#1,effect		* CHOIX EFFET	
		cmp.w		#5,effect
		bne.s		.ok4
		clr.w		effect
.ok4:

		cmp.w		#0,effect		* SELECT EFFECT
		beq.s		eff1
		cmp.w		#1,effect
		beq.s		eff2
		cmp.w		#2,effect
		beq.s		eff3
		cmp.w		#3,effect
		beq			eff4
		cmp.w		#4,effect
		beq			eff5

	*------------ Effet 1 : Double balayage vertical
	
eff1:	lea		31840(a0),a2
		lea		31840(a1),a3
		move.w	#99,d2
.effb1:	moveq.l	#39,d1
.cop1:	move.l	(a0)+,(a1)+
		move.l	(a2)+,(a3)+
		dbra.w	d1,.cop1
		lea		160(a0),a0
		lea		160(a1),a1
		lea		-480(a2),a2
		lea		-480(a3),a3
		move.w	#2000,d4
.zzz:	dbra.w	d4,.zzz

		dbra.w	d2,.effb1
		bra		endless

	*------------ Effet 2 : Simple balayage vertical

eff2:	move.w	#199,d1
.copy:	moveq.l	#39,d2
.cop2:	move.l	(a0)+,(a1)+
		dbra.w	d2,.cop2
		move.w	#1500,d4
.zzz:	dbra.w	d4,.zzz

		dbra.w	d1,.copy
		bra	endless
 	
 	*------------ Effet 3 : Double balayage horizontal

eff3:	lea		16000(a0),a0
		lea		16000(a1),a1
		lea		-160(a0),a2
		lea		-160(a1),a3
		move.w	#99,d0
.cop3:	move.w	#39,d1
.cop4:	move.l	(a0)+,(a1)+
		move.l	(a2)+,(a3)+
		dbra.w	d1,.cop4
		lea		-320(a2),a2
		lea		-320(a3),a3
		move.w	#1000,d1
.zzz:	dbra.w	d1,.zzz

		dbra.w	d0,.cop3
		bra		endless

 	*------------ Effet 4 : Damier
eff4:
		move.w		#1,icx
		move.w		#1,icy
		move.w		#-1,icx2
		move.w		#1,icy2
		move.w		#600,d4
		moveq.l		#13,d0
		moveq.l		#8,d1
		moveq.l		#7,d5
		moveq.l		#7,d6

.cop5:
		cmp.w		#20,d0
		beq			.noaff4
		move.l		a0,a2
		move.l		a1,a3	
		move.w		d0,d2
		lsl.w		#3,d2
		move.w		d1,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a2
		add.w		d3,a3

		rept	10
		move.l		(a2)+,(a3)+
		move.l		(a2)+,(a3)+		
		lea			152(a2),a2
		lea			152(a3),a3
		endr
.noaff4:
		cmp.w		#20,d5
		beq			.noaff5
		move.l		a0,a2
		move.l		a1,a3	
		move.w		d5,d2
		lsl.w		#3,d2
		move.w		d6,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a2
		add.w		d3,a3

		rept	10
		move.l		(a2)+,(a3)+
		move.l		(a2)+,(a3)+		
		lea			152(a2),a2
		lea			152(a3),a3
		endr
.noaff5:		
		
		add.w	icx(pc),d0
		cmp.w	#20,d0
		bne.s	.ok6
		bsr.s	pause
		neg.w	icx
.ok6:	tst.w	d0
		bne.s	.ok62
		bsr.s	pause
		neg.w	icx
.ok62:
		add.w	icy(pc),d1
		cmp.w	#19,d1
		bne.s	.ok7
		bsr.s	pause
		neg.w	icy
.ok7:	tst.w	d1
		bne.s	.ok72
		bsr.s	pause
		neg.w	icy
.ok72:
		add.w	icx2(pc),d5
		cmp.w	#20,d5
		bne.s	.ok26
		bsr.s	pause
		neg.w	icx2
.ok26:	tst.w	d5
		bne.s	.ok262
		bsr.s	pause
		neg.w	icx2
.ok262:
		add.w	icy2(pc),d6
		cmp.w	#19,d6
		bne.s	.ok27
		bsr.s	pause
		neg.w	icy2
.ok27:	tst.w	d6
		bne.s	.ok272
		bsr.s	pause
		neg.w	icy2
.ok272:
		dbra.w	d4,.cop5
		bra		endless

pause:	move.w	#500,d7
.zzz:	dbra.w	d7,.zzz
		rts

 	*------------ Effet 5 : Escargot
eff5:		
		move.w		#450,d7
		moveq.l		#0,d0
		moveq.l		#0,d1
		moveq.l		#0,d2
		moveq.l		#0,d3
		moveq.l		#19,d4
		moveq.l		#19,d5
		moveq.l		#0,d6
.cop6:
		move.l		d2,-(sp)
		move.l		d3,-(sp)
		
		move.w		#100,d2	
.temp10: 
		dbra.w		d2,.temp10

		move.l		a0,a2
		move.l		a1,a3	
		move.w		d0,d2
		lsl.w		#3,d2
		move.w		d1,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a2
		add.w		d3,a3

		move.l		(sp)+,d3
		move.l		(sp)+,d2

		rept	10
		move.l		(a2)+,(a3)+
		move.l		(a2)+,(a3)+		
		lea			152(a2),a2
		lea			152(a3),a3
		endr
		
		tst.w	d6
		beq.s	.phase0		
		cmp.w	#1,d6
		beq.s	.phase1
		cmp.w	#2,d6
		beq.s	.phase2
		cmp.w	#3,d6
		beq.s	.phase3

.bl:	dbra.w	d7,.cop6
		bra	endless

.phase0:	
		addq.w	#1,d0
		cmp.w	d0,d4
		bne.s	.sok0
		addq.w	#1,d6
		subq.w	#1,d4
.sok0:	bra.s	.bl	

.phase1:	
		addq.w	#1,d1
		cmp.w	d1,d5
		bne.s	.sok1
		addq.w	#1,d6
		subq.w	#1,d5
.sok1:	bra.s	.bl

.phase2:		
		subq.w	#1,d0
		cmp.w	d0,d2
		bne.s	.sok2
		addq.w	#1,d6
		addq.w	#1,d2
.sok2:	bra.s	.bl

.phase3:		
		subq.w	#1,d1
		cmp.w	d1,d3
		bne.s	.sok3
		addq.w	#1,d3
		moveq.l	#0,d6
.sok3:	bra.s	.bl

icx:		dc.w	1
icy:		dc.w	0
icx2:		dc.w	1
icy2:		dc.w	0

*----------------------------- EFFACEMENT --------------------------------*

clear:		
		
		move.l	ecran(pc),a1
		lea		45*160(a1),a1

		addq.w	#1,effect2
		cmp.w	#5,effect2
		bne.s	.ok20
		clr.w	effect2
.ok20:
		cmp.w	#0,effect2
		beq		clear4
		cmp.w	#1,effect2
		beq		clear5
		cmp.w	#2,effect2
		beq.s	clear2
		cmp.w	#3,effect2
		beq.s	clear3
		cmp.w	#4,effect2
		beq		clear1

clear1:
		lea	31840(a1),a3
		move.w	#99,d2
.ceffb1:
		moveq.l	#39,d1
.ccop1:	clr.l	(a1)+
		clr.l	(a3)+
		dbra.w	d1,.ccop1
		lea	160(a1),a1
		lea	-480(a3),a3
		move.w	#2000,d4
.zzz:	dbra.w	d4,.zzz
		dbra.w	d2,.ceffb1
		rts
		
clear2:
		move.w	#199,d1
.ccopy:	moveq.l	#39,d2
.ccop2:	clr.l	(a1)+
		dbra.w	d2,.ccop2
		move.w	#1500,d4
.zzz:	dbra.w	d4,.zzz
		dbra.w	d1,.ccopy
		rts
	
clear3:		
		lea	31840(a1),a3
		move.w	#99,d0
.ccop3:	move.w	#39,d1
.ccop4:	clr.l	(a1)+
		clr.l	(a3)+
		dbra.w	d1,.ccop4
		lea	-320(a3),a3
		move.w	#1000,d1
.ctemp2:	
		dbra.w	d1,.ctemp2
		dbra.w	d0,.ccop3
		rts

clear4:		
		move.w		#1,icx
		move.w		#1,icy
		move.w		#-1,icx2
		move.w		#1,icy2
		move.w		#600,d4
		moveq.l		#13,d0
		moveq.l		#8,d1
		moveq.l		#7,d5
		moveq.l		#7,d6

.ccop5:
		cmp.w		#20,d0
		beq.s		.cnoaff4
		move.l		a1,a3	
		move.w		d0,d2
		lsl.w		#3,d2
		move.w		d1,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a3

		rept	10
		clr.l	(a3)+
		clr.l	(a3)+		
		lea		152(a3),a3
		endr
.cnoaff4:
		cmp.w		#20,d5
		beq.s		.cnoaff5
		move.l		a1,a3	
		move.w		d5,d2
		lsl.w		#3,d2
		move.w		d6,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a3

		rept	10
		clr.l	(a3)+
		clr.l	(a3)+		
		lea	152(a3),a3
		endr
.cnoaff5:		
		
		add.w	icx(pc),d0
		cmp.w	#20,d0
		bne.s	.cok6
		bsr		pause
		neg.w	icx
.cok6:	tst.w	d0
		bne.s	.cok62
		bsr		pause
		neg.w	icx
.cok62:
		add.w	icy(pc),d1
		cmp.w	#19,d1
		bne.s	.cok7
		bsr		pause
		neg.w	icy
.cok7:	tst.w	d1
		bne.s	.cok72
		bsr		pause
		neg.w	icy
.cok72:
		add.w	icx2(pc),d5
		cmp.w	#20,d5
		bne.s	.cok26
		bsr		pause
		neg.w	icx2
.cok26:	tst.w	d5
		bne.s	.cok262
		bsr		pause
		neg.w	icx2
.cok262:
		add.w	icy2(pc),d6
		cmp.w	#19,d6
		bne.s	.cok27
		bsr		pause
		neg.w	icy2
.cok27:	tst.w	d6
		bne.s	.cok272
		bsr		pause
		neg.w	icy2
.cok272:
		dbra.w	d4,.ccop5
		rts

clear5:
		move.w		#450,d7
		moveq.l		#0,d0
		moveq.l		#0,d1
		moveq.l		#0,d2
		moveq.l		#0,d3
		moveq.l		#19,d4
		moveq.l		#19,d5
		moveq.l		#0,d6
.ccop6:	
		move.l		d2,-(sp)
		move.l		d3,-(sp)
		
		move.w		#100,d2	
.ctemp10:
		dbra.w		d2,.ctemp10

		move.l		a1,a3	
		move.w		d0,d2
		lsl.w		#3,d2
		move.w		d1,d3
		mulu.w		#1600,d3
		add.w		d2,d3
		add.w		d3,a3

		move.l		(sp)+,d3
		move.l		(sp)+,d2

		rept	10
		clr.l		(a3)+
		clr.l		(a3)+		
		lea			152(a3),a3
		endr
		
		tst.w		d6
		beq.s		.cphase0		
		cmp.w		#1,d6
		beq.s		.cphase1
		cmp.w		#2,d6
		beq.s		.cphase2
		cmp.w		#3,d6
		beq.s		.cphase3

.cbl:	dbra.w	d7,.ccop6
		rts

.cphase0:	
		addq.w	#1,d0
		cmp.w	d0,d4
		bne.s	.csok0
		addq.w	#1,d6
		subq.w	#1,d4
.csok0:	bra.s	.cbl

.cphase1:	
		addq.w	#1,d1
		cmp.w	d1,d5
		bne.s	.csok1
		addq.w	#1,d6
		subq.w	#1,d5
.csok1:	bra.s	.cbl

.cphase2:
		subq.w	#1,d0
		cmp.w	d0,d2
		bne.s	.csok2
		addq.w	#1,d6
		addq.w	#1,d2
.csok2:	bra.s	.cbl

.cphase3:	
		subq.w	#1,d1
		cmp.w	d1,d3
		bne.s	.csok3
		addq.w	#1,d3
		moveq.l	#0,d6
.csok3:	bra.s	.cbl
		
*------------------------------ VARIABLES --------------------------------*

action:		dc.w	1
vblcpt:		dc.w	0
nbtext:		dc.w	0
nbimg:		dc.w	0
flipimg:	dc.w	0
nuage:		dc.l	0
flip:		dc.w	0	
effect:		dc.w	0
effect2:	dc.w	0
ecran:		dc.l	0
pal1:		incbin	"pal2.dat"
pal2:		rept	8
			dc.l	0
			endr

tabtext:	dc.l	text1
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
			dc.l	text12

tabimg:		dc.l	0	; img5
			dc.l	0	; img6
			dc.l	0	; img4
			dc.l	0	; img7
			dc.l	0	; img8
			dc.l	0	; img2
			dc.l	0	; img9
			dc.l	0	; img12
			dc.l	0	; img10
			dc.l	0	; img1
			dc.l	0	; img11
tabimgend:

*-------------------- ROUTINES SOUNDTRACK & BUFFERS ----------------------*

	Section data

depack:		dc.l	0
p2c:		dc.l	0
cloud:		dc.l	0	; incbin	"nuages2"
font:		dc.l	0		
tamp:		dc.l	0

		even
	
text1:	
	dc.b	'>THE^SPACE^'
	dc.b	'<^^FILLER^^'
	dc.b	'>THE^FINAL^'
	dc.b	'<^FRONTIER^'
	even
text2:	
	dc.b	'<^^^^NO^^^^'
	dc.b	'>^FUCKING^^'
	dc.b	'<^^^^NO^^^^'
	dc.b	 '^GREETING^'
	even
text3:	
	dc.b	'^^^JUST^^^'
	dc.b   '>^^^^A^^^^^'
	dc.b	'^CERTAIN^^'
	dc.b	'^FEELING^^'
	even
text4:	
	dc.b   '>^WELCOME^^'
	dc.b   '<^^^^IN^^^^'
	dc.b   '>^^^OUR^^^^'
	dc.b   '>CYBERZONE^'
	even
text5:	
	dc.b   '<^^^JUST^^^'
	dc.b   '>FOLLOW^US^'
	dc.b   '<^^IN^OUR^^'
	dc.b	'^^TRAVEL^^'
	even
text6:	
	dc.b   '>^TO^MEET^^'
	dc.b   '<^^^SOME^^^'
	dc.b   '>^STRANGE^^'
	dc.b    'CREATURES^'
	even
text7:	
	dc.b   '>^THROUGH^^'
	dc.b	'^^^THE^^^^'
	dc.b	'^ENDLESS^^'
	dc.b	'^^SPACE^^^'
	even
text8:	
	dc.b   '>TECHNICAL^'
	dc.b   '<^LIFE^FOR^'
	dc.b	'^ELECTRIC^'
	dc.b	'^^PEOPLE^^'
	even
text9:	
	dc.b   '<^^^IN^A^^^'
	dc.b	'^^BETTER^^'
	dc.b	'^^FUTURE^^'
	dc.b	'^^^^^^^^^^'
	even
text10:	
	dc.b   '<ARTIFICIAL'
	dc.b	'^^WORLDS^^'
	dc.b   '>FOR^GIANT^'
	dc.b   '<^^CITIES^^'
	even
text11:	
	dc.b   '>^^AND^A^^^'
	dc.b	'PLACE^FOR^'
	dc.b	'THE^SPACE^'
	dc.b   '<^^LOVERS^^'
	even
text12:	
	dc.b   '<^^RENDEZ^^'
	dc.b	'^^^VOUS^^^'
	dc.b	'^^IN^THE^^'
	dc.b	'^^FUTURE^^'
	even
