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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\LIQUID.PRX

	opt	o+


nbdoublelines:	set 38
scrollnblines:  set 7
cpu:			set 0


modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
    
    ; common
    dc.l	LIQinit
    dc.l	LIQupdate
    dc.l	LIQshutdown

    ; specific
    dc.l	LIQvbl
LIQexit:
	dc.b	0
	dc.b	0

miniec:	macro	* Affichage de 2 ecrans digits

	rept 70
	movem.l	(a0)+,d0-d7/a5-a6
	movem.l	d0-d7/a5-a6,(a1)
	movem.l	d0-d7/a5-a6,(a2)
	lea	40(a1),a1
	lea	40(a2),a2
	movem.l	(a0)+,d0-d7/a5-a6
	movem.l	d0-d7/a5-a6,(a1)
	movem.l	d0-d7/a5-a6,(a2)
	lea	256-40(a1),a1
	lea	256-40(a2),a2
	endr

	endm

; param structure :
;   - frame buffers base address
;   - water data address
;   - font address

LIQinit:

    movem.l d0-d1/a0-a1,-(sp)

    move.l  4(a0),water
    move.l  8(a0),fnt

	move.b	#(256-160)>>1,$ffff820f.w

	move.l	(a0),d0
	sub.w	d0,d0
   	move.l	d0,ec			* ADRESSE ECRAN	

	lea	$ffff8240.w,a0		* PALETTE A ZERO
	rept	8			
	clr.l	(a0)+
	endr

	clr.b	$ffff820d.w		* FIXE ADRESSE ECRAN
	lsr.w	#8,d0	
	move.b	d0,$ffff8203.w
	swap	d0
	move.b	d0,$ffff8201.w

	lea 	scrtxt(pc),a1
	move.w	#335,d0
eff:	
    clr.l	(a1)+
	dbra.w	d0,eff

	lea		txt(pc),a0
	lea		txtend(pc),a1

buildtext:
	moveq.l	#0,d1
	move.b	(a0)+,d1
	cmp.w	#32,d1			* Espace
	bne.s	.nospace
	move.w	#39*8*8,d1
	bra.s	.movecar
.nospace:
	cmp.w	#33,d1			* Point d'exclamation 				
	bne.s	.noexcla
	move.w	#38*8*8,d1
	bra.s	.movecar
.noexcla:
	cmp.w	#42,d1
	bne.s	.noreset
	move.w	#-1,d1
	bra.s	.movecar
.noreset:
    cmp.w	#44,d1		    * Virgule
	bne.s	.novirg
	move.w	#37*8*8,d1
	bra.s	.movecar
.novirg: 
    cmp.w	#46,d1		    * Point
	bne.s	.nopoint		
	move.w	#36*8*8,d1
	bra.s	.movecar
.nopoint:
	sub.w	#48,d1			
	cmp.w	#9,d1	
	bgt.s	.nonummer
	lsl.w	#6,d1
	add.w	#26*8*8,d1
	bra.s	.movecar
.nonummer:
	sub.w	#65-48,d1
	lsl.w	#6,d1
.movecar:
    move.w	d1,(a1)+
	bge.s	buildtext

    movem.l (sp)+,d0-d1/a0-a1
					
    rts

			
LIQupdate:

    movem.l a0-a6/d0-d7,-(sp)

	lea		off(pc),a1			* GESTION STROBOSCOPIE
	movem.l	(a1),d0-d7/a2-a5
	movem.l	d0-d7/a2-a5,4(a1)

	add.l	#5600,(a1)
	cmp.l	#5600*45,(a1)
	bne.s	.oko
	clr.l	(a1)
.oko:
								* AFFICHAGE 3*2 ECRANs
	move.l	ec(pc),a1	
	add.l	#70*256*2,a1
	move.l	a1,a2
	lea		80(a2),a2
	move.l	water(pc),a3
	move.l	a3,a0
	add.l	off+48(pc),a0 
	miniec						* AFFICHAGE 2 ECRANs DU BAS

	lea	scrtxt(pc),a3			* AFFICHAGE TEXTE
	move.l	ec(pc),a4
	add.l	#200*256-2560,a4

	moveq.l	#scrollnblines-1,d0
.loop:
	rept	5
	movem.l	(a3)+,d1-d7
	or.l	d1,(a4)+
	or.l	d2,(a4)+
	or.l	d3,(a4)+
	or.l	d4,(a4)+
	or.l	d5,(a4)+
	or.l	d6,(a4)+
	or.l	d7,(a4)+
	endr
	movem.l	(a3)+,d1-d5
	or.l	d1,(a4)+
	or.l	d2,(a4)+
	or.l	d3,(a4)+
	or.l	d4,(a4)+
	or.l	d5,(a4)+

	addq.l	#8,a3
	lea		256-160(a4),a4
	dbra	d0,.loop

	lea	scrtxt(pc),a3		* GESTION SCROLLTEXT
	rept	scrollnblines	* DECALAGE 1 PIXEL SUR 4 PLANS
	roxl.w	160(a3)			* 7 LIGNES
	roxl.w	152(a3)		
	roxl.w	144(a3)		
	roxl.w	136(a3)		
	roxl.w	128(a3)		
	roxl.w	120(a3)		
	roxl.w	112(a3)		
	roxl.w	104(a3)		
	roxl.w	96(a3)		
	roxl.w	88(a3)		
	roxl.w	80(a3)		
	roxl.w	72(a3)		
	roxl.w	64(a3)		
	roxl.w	56(a3)		
	roxl.w	48(a3)		
	roxl.w	40(a3)		
	roxl.w	32(a3)		
	roxl.w	24(a3)		
	roxl.w	16(a3)		
	roxl.w	8(a3)		
	roxl.w	(a3)+		
	roxl.w	160(a3)	
	roxl.w	152(a3)	
	roxl.w	144(a3)		
	roxl.w	136(a3)		
	roxl.w	128(a3)		
	roxl.w	120(a3)		
	roxl.w	112(a3)		
	roxl.w	104(a3)		
	roxl.w	96(a3)		
	roxl.w	88(a3)		
	roxl.w	80(a3)		
	roxl.w	72(a3)		
	roxl.w	64(a3)		
	roxl.w	56(a3)		
	roxl.w	48(a3)		
	roxl.w	40(a3)		
	roxl.w	32(a3)		
	roxl.w	24(a3)		
	roxl.w	16(a3)		
	roxl.w	8(a3)		
	roxl.w	(a3)+		
	roxl.w	160(a3)	
	roxl.w	152(a3)	
	roxl.w	144(a3)		
	roxl.w	136(a3)		
	roxl.w	128(a3)		
	roxl.w	120(a3)		
	roxl.w	112(a3)		
	roxl.w	104(a3)		
	roxl.w	96(a3)		
	roxl.w	88(a3)		
	roxl.w	80(a3)		
	roxl.w	72(a3)		
	roxl.w	64(a3)		
	roxl.w	56(a3)		
	roxl.w	48(a3)		
	roxl.w	40(a3)		
	roxl.w	32(a3)		
	roxl.w	24(a3)		
	roxl.w	16(a3)		
	roxl.w	8(a3)		
	roxl.w	(a3)+		
	roxl.w	160(a3)	
	roxl.w	152(a3)	
	roxl.w	144(a3)		
	roxl.w	136(a3)		
	roxl.w	128(a3)		
	roxl.w	120(a3)		
	roxl.w	112(a3)		
	roxl.w	104(a3)		
	roxl.w	96(a3)		
	roxl.w	88(a3)		
	roxl.w	80(a3)		
	roxl.w	72(a3)		
	roxl.w	64(a3)		
	roxl.w	56(a3)		
	roxl.w	48(a3)		
	roxl.w	40(a3)		
	roxl.w	32(a3)		
	roxl.w	24(a3)		
	roxl.w	16(a3)		
	roxl.w	8(a3)		
	roxl.w	(a3)+		
	lea	160(a3),a3
	endr

	move.w	cpt(pc),d1		* Main compteur
	moveq.l	#7,d0
	and.w	d1,d0
	bne.s	.nochar
	lea    	txtend(pc),a3	* On prend le nouveau caractere 
	lsr.w	#2,d1			* 1 caractere = offset par rapport
	move.w	(a3,d1.w),d1	* au fichier fnt
	bge.s	.nowrap	
	move.w	#39*8*8,d1	; wrap text
	clr.w	cpt
.nowrap:
	move.l	fnt(pc),a3		* Adr caractere
	add.w	d1,a3			
	lea		scrtxt+160(pc),a4	* Endroit a reafficher ds buffer
	
	rept	scrollnblines   * 7 Lignes
	move.l	(a3)+,(a4)+		* Charge caractere plan 1 & 2
	move.l	(a3)+,(a4)+		* Charge caractere plan 1 & 2
	lea		160(a4),a4
	endr
.nochar:

	addq.w	#1,cpt			* Gestion main pointer pour le scrolltext 

	;-----------------------------------------------------------------------
	; water

	move.l	water(pc),a3
	move.l	a3,a0
	add.l	off(pc),a0
	move.l	ec(pc),a1
	lea		80(a1),a2
	miniec				    * AFFICHAGE 2 ECRANs DU HAUT

	move.l	a3,a0
	add.l	off+24(pc),a0
	miniec				    * AFFICHAGE 2 ECRANs DU BAS 

	cmp.l	#32,gen			* FADE IN COLORS 
	bge.s	.okl
	lea		$ffff8240.w,a0
	lea		pal2(pc),a1
	move.l	gen(pc),d0
	and.w	#%111110,d0
	lsl.w	#4,d0
	add.w	d0,a1
	rept	8
	move.l	(a1)+,(a0)+
	endr	
.okl:
	addq.l	#1,gen	

    tst.b   LIQexit
    beq.s   .noexit

    tst.w   ext
    beq.s   .colorfadeend
	subq.w	#1,ext
	lea		$ffff8240.w,a0
	lea		pal2(pc),a1
	move.w	ext(pc),d0
	and.w	#%1111100,d0
	add.w	d0,d0
	add.w	d0,d0
	add.w	d0,d0
	add.w	d0,a1	
	rept	8
	move.l	(a1)+,(a0)+
	endr	

.colorfadeend:

.noexit:

    movem.l (sp)+,a0-a6/d0-d7

    rts


LIQshutdown:
	clr.b	$ffff820f.w
    rts


					******* INTERs (graphic)
	
	  				* TIMER B

hbl1:	
	ifne	cpu
	move.w	#$700,$ffff8240.w
	endc

	clr.b	$FFFFFA1B.w			* Hlb1: revectorise sur Hbl2
	move.b	#2,$FFFFFA21.w		* qui s'occupera du grossissement
	move.l	#hbl2,$120.w		* vertical pendant 80 lignes en  
	move.b	#8,$FFFFFA1B.w		* agissant sur le compteur video
	rte	

hbl2:						* Hbl2:		
	subq.w	#1,cptbig		* Si on est a plus de 40 appels de 
	beq.s	.end			* overscan qui gere le no border

.temp3:	
	tst.b	$FFFF8209.w		
	bne.s	.temp3			

	subq.b	#1,$FFFF8207.w	* on soustrait 256 au compteur:

	rte

.end:
	move.l	d0,-(sp)		* bas
	move.w	#200-2-2*nbdoublelines,d0
	sub.w	vert(pc),d0
	add.w	ic2(pc),d0
	clr.b	$FFFFFA1B.w
	move.b	d0,$FFFFFa21.w
	move.l	#overscan,$120.w
	move.b	#8,$FFFFFA1B.w
	move.l	(sp)+,d0

	rte						* fin inter si plus de 40 appels


overscan:					* no border bas

	clr.b	$FFFFFA1B.w		* arrete le timer B

	ifne	cpu
	move.w	#$70,$ffff8240.w
	endc

.wait:
	cmp.b	#200-nbdoublelines,$FFFF8207.w
	bls.s	.wait

	ifne	cpu
	move.w	#$7,$ffff8240.w
	endc

	clr.b	$ffff820a.w		* 60 Hz
	
	rept	20
	nop						* on attend un peu
	endr

	move.b	#2,$ffff820a.w	* 50 Hz

	rte


LIQvbl:
	ifne	cpu
	move.w	#0,$ffff8240.w
	endc

    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$FFFFFA1B.w		* arrete le timer B
	move.l	#hbl1,$120.w	* vectorise hbl1  

	move.w	vert(pc),$FFFFFA20.w	* mvt vertical du grossissement

	or.b	#1,$fffffa07.w
	move.b	#8,$FFFFFA1B.w	

	move.w	d0,-(sp)		*	( sauvegarde registre )
	move.w	ic(pc),d0		*
	add.w	d0,vert			*
	move.w	(sp)+,d0		* 	( restaure registre )
	move.w	ic(pc),ic2		*
	cmp.w	#115,vert		*
	bne.s	.oka			*
	neg.w	ic			*
.oka:	
	cmp.w	#3,vert			*
	bne.s	.okb			*
	neg.w	ic			*
.okb:	
	move.w	#nbdoublelines,cptbig			* flag a zero du nombre d'appels de hbl2 effectues 

	move.l  (a0)+,a1
    jmp     (a1)

	************** ZONES POUR MES VARS

ec:	        dc.l	0
ic:	        dc.w	1
vert:	    dc.w	5
ic2:	    dc.w	1
cptbig:     dc.w	0
car:	    dc.w	0	
cpt:	    dc.w	0
gen:	    dc.l	0
ext:	    dc.w	64

fnt:	dc.l    0   * FONTE 8X8 - 4 PLANS
water:  dc.l    0

off:	
    dc.l	44*5600		*	<-
	dc.l	44*5600	
	dc.l	44*5600	
	dc.l	44*5600	
	dc.l	44*5600	
	dc.l	44*5600	
	dc.l	44*5600		*	<-
	dc.l	44*5600
	dc.l	44*5600	
	dc.l	44*5600	
	dc.l	44*5600
	dc.l	44*5600
	dc.l	44*5600		*	<-

pal:	
    dc.w	$0008,$0001,$0009,$0002,$008A,$0083,$008B,$0084
	dc.w	$001C,$0095,$0025,$003D,$00B6,$004E,$00CE,$08DF

workspc:
    DS.W	1

scrtxt:	ds.w	1344/2

txt:	
    dc.b	' OPEN YOUR CHAKRAS TO THE POSITIVE ENERGY OF THE LIQUID '
	dc.b	'OSMOSIS. LET THE CYBERNETICS SPIRIT ENTER YOUR MIND.'
	dc.b	' YOU FEEL MECHANICAL. YOU ARE NOW A LIQUID BIOMECHANOID'
	dc.b	', IN COMPLETE OSMOSIS WITH WATER. DO NOT BE AFRAID, YOUR '
	dc.b	'MIND BELONGS TO LIQUID. YOU CAN NOW AFFORD THE ELEMENTS.'
	dc.b	'    CALM... WARM FEELINGS... YOU ARE SWIMMING IN THE '
	dc.b	'ELECTRONIC WATER OF THIS SCREEN.          NOW IT IS TIME TO...*'
	
txtend:

	ds.w	txtend-txt

	even

pal1:
		dc.w	$0000,$0000,$0000,$0000,$0008,$0008,$0008,$0008
		dc.w	$0008,$0008,$0008,$0008,$0001,$0001,$0001,$0001
		dc.w	$0000,$0000,$0000,$0008,$0008,$0008,$0008,$0008
		dc.w	$0001,$0081,$0081,$0081,$0081,$0089,$0089,$0089
		dc.w	$0000,$0000,$0008,$0088,$0888,$0881,$0881,$0881
		dc.w	$0819,$0819,$0819,$0812,$0112,$0192,$019A,$019A
		dc.w	$0000,$0000,$0088,$0888,$0881,$0881,$0811,$0819
		dc.w	$0119,$0112,$0192,$0192,$019A,$099A,$0923,$0923
		dc.w	$0000,$0000,$0888,$0888,$0881,$0811,$0119,$0119
		dc.w	$0192,$0192,$099A,$092A,$0923,$0923,$02A3,$02AB
		dc.w	$0000,$0008,$0888,$0881,$0811,$0119,$0119,$0192
		dc.w	$0992,$092A,$092A,$0223,$02A3,$02AB,$0A3B,$0A34
		dc.w	$0000,$0008,$0888,$0881,$0111,$0119,$0192,$0992
		dc.w	$092A,$022A,$02A3,$02AB,$0A3B,$0A34,$0334,$03BC
		dc.w	$0000,$0088,$0888,$0811,$0119,$0199,$0992,$092A
		dc.w	$022A,$02A3,$0AAB,$0A3B,$0334,$03BC,$03BC,$0B45
		dc.w	$0000,$0888,$0888,$0111,$0119,$0992,$0922,$022A
		dc.w	$02A3,$0AAB,$0A3B,$03B4,$03BC,$0B4C,$0B45,$04CD
		dc.w	$0000,$0888,$0881,$0111,$0199,$0992,$022A,$02A3
		dc.w	$0AA3,$0A3B,$03B4,$0BBC,$0B45,$04C5,$04CD,$0C56
		dc.w	$0000,$0888,$0881,$0119,$0999,$0922,$022A,$0AA3
		dc.w	$0A3B,$03B4,$0BBC,$0B4C,$04C5,$0CCD,$0C56,$05DE
		dc.w	$0000,$0888,$0811,$0119,$0992,$022A,$02A3,$0A3B
		dc.w	$033B,$0BB4,$0B4C,$04C5,$0C5D,$0C56,$05DE,$0D67
		dc.w	$0000,$0888,$0111,$0199,$0992,$022A,$0AA3,$033B
		dc.w	$03B4,$0B4C,$04C5,$0CCD,$0556,$05DE,$0D67,$06EF
		dc.w	$0000,$0888,$0111,$0199,$0992,$022A,$0AA3,$033B
		dc.w	$03B4,$0B4C,$04C5,$0C5D,$0556,$05DE,$0D67,$06EF
pal2:
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0008,$0008,$0008,$0008,$0001,$0001,$0001,$0001
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0008,$0001,$0001,$0001,$0001,$0009,$0009,$0009
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0008,$0001,$0001,$0001,$0009,$0009,$0002,$0002
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0001,$0001,$0009,$0009,$0082,$0082,$008A,$008A
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0001,$0081,$0089,$0089,$0082,$001A,$001A,$0013
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0001,$0089,$0089,$0082,$001A,$0013,$0093,$009B
		dc.w	$0008,$0008,$0008,$0008,$0008,$0008,$0008,$0008
		dc.w	$0081,$0089,$0012,$0012,$009A,$0093,$002B,$0024
		dc.w	$0008,$0008,$0008,$0008,$0001,$0001,$0001,$0001
		dc.w	$0089,$0082,$001A,$001A,$0093,$002B,$0024,$00AC
		dc.w	$0008,$0008,$0008,$0008,$0001,$0001,$0001,$0001
		dc.w	$0089,$0082,$001A,$001A,$0093,$002B,$0024,$00AC
		dc.w	$0008,$0008,$0001,$0001,$0001,$0001,$0009,$0009
		dc.w	$0082,$001A,$0013,$0093,$002B,$00A4,$00AC,$0035
		dc.w	$0008,$0008,$0001,$0001,$0009,$0009,$0002,$0002
		dc.w	$008A,$0013,$009B,$009B,$0024,$00AC,$0035,$00BD
		dc.w	$0008,$0001,$0001,$0009,$0009,$0002,$0002,$000A
		dc.w	$0083,$001B,$0094,$0024,$00AC,$0035,$00BD,$0046
		dc.w	$0008,$0001,$0001,$0009,$0002,$000A,$000A,$0003
		dc.w	$008B,$0014,$009C,$002C,$0035,$00BD,$0046,$00CE
		dc.w	$0008,$0008,$0001,$0009,$0002,$000A,$0003,$000B
		dc.w	$0084,$001C,$009C,$00A5,$003D,$00B6,$0046,$0057
		dc.w	$0008,$0001,$0009,$0002,$008A,$0083,$008B,$0084
		dc.w	$001C,$0095,$0025,$003D,$00B6,$004E,$00CE,$08DF
                                                                                                                                