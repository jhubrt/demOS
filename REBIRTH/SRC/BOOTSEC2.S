*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2017 J.Hubert
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
system:		set	0
cpu:        set	0
d:			set	6
dep:		set	127

sectorspertrack:	set 10

_Y			EQU	8
_NEXTCHAR	EQU 128
_NEXTWORD	EQU 64

CURRENTZ:   EQU 0
WORKDATA:   EQU 2
SCREEN0:    EQU 65536 + 1000
*-------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\BOOTSEC2.BOT
	
	ifne	system
	clr.w	-(sp)			* Resolution
	move.l	#-1,-(sp)
	move.l	#-1,-(sp)
	move.w	#5,-(sp)
	trap	#14
	lea	12(sp),sp
	
	clr.l	-(sp)			* Superviseur
	move.w	#32,-(sp)
	trap	#1
	addq.w	#6,sp

	move.l	$fffffa06.w,oldtim	* Sauvegarde timers
	move.l	$70.w,oldvbl		* & vbl
	move.b	#2,$ffff820a.w		* Synchro … 50 hz
	endc

	; -----------------------------------------------

	opt	o+,a+,p+
	
	clr.l	$fffffa06.w		; Timers off
	lea	    160.w,a5        ; preload register

    bra.s       bootsectorstart
    dc.b	    0
    
    ;ds.b        6        ; @$02 OEM      
    ;dc.w        $1234    ; @$08 Serial       
    ;dc.b        $56   
    
    dc.b        0   ; @$0B BPC: bytes per sector (little endian)
    dc.b        2

    dc.b        2   ; @$0D SPC: sectors per cluster

    dc.b        1   ; @$0E RESSEC: reserved sectors before FAT (little endian)
    dc.b        0   ; 

    dc.b        0   ; @$10 NFATS: number of FAT

    dc.b        0   ; @$11 NDIRS: number of files entries for root dir (little endian)
    dc.b        0   

    dc.b        (82*2*sectorspertrack)&255  ; @$13 NSECTS: total number of sectors (little endian)
    dc.b        (82*2*sectorspertrack)/256  ; 

    dc.b        $F9   ; @$15 MEDIA: media type descriptor

    dc.b        0   ; @$16 SPF: nb sectors per FAT   (little endian)
    dc.b        0   

    dc.b        sectorspertrack   ; @$18 SPT: nb sectors per track (little endian)
    dc.b        0

    dc.b        2   ; @$1A NHEADS: nb heads (little endian) => 2 for two sided
    dc.b        0

    ;dc.b        0   ; @$1C NHID: nb hidden sectors (little endian) => not used on ST 
    ;dc.b        0   ; as not used on ST => optimize using it for code...

bootsectorstart:

	lea	    vbl(pc),a1		* Fixe new vbl
	move.l	a1,$70.w		*

	lea	data(pc),a3
	lea	base+WORKDATA(pc),a1
	move.w	#-2000,d7
	moveq.l	#dataend-data-1,d2
calcul:	
	move.b	(a3)+,d0
	bpl.s	minus
	add.w	#34*8,d7
	cmp.b	#-64,d0
	beq.s	saut
minus:
	move.w	d0,d1
	and.w	#7,d0
	asl.w	#5,d0
	add.w	d7,d0
	move.w	d0,(a1)+

	and.w	#$78,d1
	asl.w	#3,d1
	sub.w	#64*2,d1
	move.w	d1,(a1)+

saut:	
	dbra.w	d2,calcul
	clr.l	(a1)+

    move.l  a3,sp		; do not need data anymore => cool place to have the stack

    add.l   #SCREEN0,a1
    move.l  a1,d5
    sub.w   d5,d5
    move.l  d5,a1

	;moveq.l	#-1,d0			* -1 available in d2
eff:	
	clr.b	(a1)+			* Efface ecran 2
	dbra.w	d2,eff			*


    ; MAIN LOOP ----------------------------------------------

main:					    
    ; Vsync
	stop	#$2300			    

	ifne	cpu
	move.w	#7,$ffff8240.w
	endc

    ; Manage Z	    
	moveq.l	#dep,d0             
    lea	    base+CURRENTZ(pc),a1	
	subq.w	#1,(a1)
	bgt.s	okprof
	move.w	d0,(a1)
	eor.w	#$111,(a3)
okprof:
	move.w	#dep+dep/6*2,d0
	sub.w	(a1)+,d0
	divu.w	#dep/6,d0
	mulu.w	(a3),d0
	move.w	d0,$ffff8242.w

    ; Set screen address
	lea	$ffff8203.w,a0
	movep.l	d5,(a0)
    eor.w   #$8000,d5

    ; Small blue gradient (very short opcode)
	moveq.l	#15,d1
gradient:
    moveq.l	#42+42+41,d0
line:
    dbf.w   d0,line
    move.w  d1,$ffff8240.w
    dbf.w   d1,gradient
							
    ; Erase screen
	moveq.l	#-1,d0          
	moveq.l	#8,d7           * Erase screen
	lea	    $ffff8a28.w,a0	
 	move.l	d0,(a0)+		* Init blitter
 	move.w	d0,(a0)+		* Init blitter
	move.w	d7,(a0)+		* Inc X
	move.w	d7,(a0)+		* Inc Y dest
	move.l	d5,(a0)+		*
	move.l	#20*65536+200,(a0)+	
	clr.w	(a0)+			
	move.w	#%11000000*256,(a0)
							* Go
	move.b	d0,-(a0)		    * Hog + combi log (Bits '1')

repeatrect:
	ifne	cpu
	move.w	#$4,$ffff8240.w
	endc

	move.l 	d5,a0			* Adresse ecran
    moveq.l #100,d7

	bsr.s	calc
	move.w	d4,d0
	add.w	a5,d0

	bsr.s	calc
	move.w	d4,d1
	add.w	d7,d1

	bsr.s	calc
	move.w	d4,d2
	add.w	a5,d2

	bsr.s	calc
	add.w	d7,d4
	
	ifne	cpu
	move.w	#$6,$ffff8240.w
	endc

	bsr.s	rect1plane		* Draw rectangle

	tst.l	(a1)			
	bne.s	repeatrect		* Loop
    
	ifne	cpu
	clr.w	$ffff8240.w
	endc
	
	ifeq	system

	bra 	main
	
    else

	cmp.b	#185,$fffffc02.w
	bne 	main
    
    move.l	#$0FFF0555,$ffff8240.w
	move.l	#$02220000,$ffff8244.w
	move.b	#0,$ffff820a.w			* Synchro … 50 hz
	move.l	oldtim(pc),$fffffa06.w	* Restaure timers
	move.l	oldvbl(pc),$70.w		* Restaure vbl
	
    clr.w	-(sp)				    * Fin
	trap	#1

oldtim:	        dc.l	0
oldvbl:	        dc.l	0

	endc

;-------------------------------------------------------------------------
calc:	move.w	(a1)+,d4
		muls.w	#d,d4
	    divs.w	base+CURRENTZ(pc),d4
rts_:	rts

;-------------------------------------------------------------------------
rect1plane:
	tst.w	d0				* Si x1 < 0
	bpl.s	.okh1			*
 	tst.w	d2				*   Si x2 < 0 Alors no rectangle
	bmi.s	rts_			*   Sinon
	moveq.l	#0,d0			*     x1 = 0
.okh1: 						*   Finsi
							* Finsi
					
	move.w	#319,d7			* Precharge #319 dans d7
	cmp.w	d7,d2			* Si x2 > 319
	ble.s	.okh2			*
	cmp.w	d7,d0			*   Si x1 > 319 Alors
	bgt.s	rts_			*     no rectangle
	move.w	d7,d2			*   Sinon x2 = 319
							*   Finsi
.okh2:						* Finsi

	tst.w	d1				* Si y1 < 0
	bpl.s	.okv1			*
 	tst.w	d4				*   Si y2 < 0 Alors no rectangle
	bmi.s	.norect			*   Sinon
	moveq.l	#0,d1			*     x1 = 0
.okv1: 						*   Finsi
							* Finsi
			
	move.w	#199,d7			* Precharge #199 dans d7
	cmp.w	d7,d4			* Si y2 > 199
	ble.s	.okv2			*
	cmp.w	d7,d1			*   Si y1 > 199 Alors
	bgt.s	.norect			*     no rectangle
	move.w	d7,d4			*   Sinon y2 = 199
							*   Finsi
.okv2:						* Finsi

	sub.w	d1,d4			
	addq.w	#1,d4			* Hauteur
	
	mulu.w	#160,d1			* Adr affichage + y1 * 160
	add.w	d1,a0			
	
	moveq.l	#$fffffff0,d7
	move.w	d0,d6			* Adr affichage + offset horizontal
	and.w	d7,d6			
	lsr.w	#1,d6			
	add.w	d6,a0			
	
	move.w	d2,d3			* Adr affichage + offset horizontal
	and.w	d7,d3			
	lsr.w	#1,d3			
	sub.w	d6,d3			
	move.w	a5,d6			* Calcul 
	sub.w	d3,d6			
	move.w	d6,$ffff8a30.w	* Inc Y dest
	lsr.w	#3,d3			* Nombre de mots	
	
	moveq.l	#0,d6			
	moveq.l	#15,d1			
	and.w	d1,d0			
	sub.w	d0,d1			
	bset.l	d1,d6			
	subq.l	#1,d6			

	moveq.l	#0,d7			
	moveq.l	#15,d1			
	and.w	d1,d2			
	sub.w	d2,d1			
	bset.l	d1,d7			
	subq.l	#1,d7			
	not.w	d7				
	move.w	d7,$ffff8a2c.w	* Masque droit

	move.l	a0,$ffff8a32.w	* Adresse dest
	tst.w	d3
	bne.s	.oklarg			* Si largeur en nb mots = 0 Alors
	and.w	d7,d6			*   Masque gauche and masque droit
.oklarg:					*   blitter de masque gauche
	
	move.w	d6,$ffff8a28.w		* Masque gauche
	addq.w	#1,d3				* Finsi	
	movem.w	d3-d4,$ffff8a36.w	* Hori size & Vert size
	move.w	#%11000000*256,$ffff8a3c.w	* No partage & Go !
	
.norect:
	rts

vbl:    
	rte

;-------------------------------------------------------------------------
data:	
	dc.b				0*_Y+3, 7*_Y+4		* 'I'
	dc.b				0*_Y+1, 1*_Y+6
	dc.b				6*_Y+1, 7*_Y+6

	dc.b	  _NEXTCHAR+0*_Y+0, 7*_Y+1		* 'N'
	dc.b				0*_Y+6, 7*_Y+7
	dc.b				1*_Y+1, 2*_Y+2
	dc.b				2*_Y+2, 3*_Y+3
	dc.b				3*_Y+3, 4*_Y+4 
	dc.b				4*_Y+4, 5*_Y+5
	dc.b				5*_Y+5, 6*_Y+6
	
	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		* 'S'
	dc.b				1*_Y+0, 3*_Y+1
	dc.b				3*_Y+0, 4*_Y+7
	dc.b				4*_Y+6, 6*_Y+7
	dc.b				6*_Y+0, 7*_Y+7
	
	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		* 'E'
	dc.b				1*_Y+0, 6*_Y+1
	dc.b				6*_Y+0, 7*_Y+7
	dc.b				3*_Y+1, 4*_Y+6

	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		* 'R'
	dc.b				1*_Y+0, 7*_Y+1
	dc.b				3*_Y+0, 4*_Y+7
	dc.b				4*_Y+5, 5*_Y+6
	dc.b				5*_Y+6, 7*_Y+7
	dc.b				1*_Y+6, 3*_Y+7
	
	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		* 'T'
	dc.b				0*_Y+3, 7*_Y+4
	
	dc.b	_NEXTWORD-_NEXTCHAR
	
	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+6		* 'D'
	dc.b				1*_Y+0, 6*_Y+1
	dc.b				6*_Y+0, 7*_Y+6
	dc.b				1*_Y+6, 6*_Y+7

	dc.b	  _NEXTCHAR+0*_Y+3, 7*_Y+4		* 'I'
	dc.b				0*_Y+1, 1*_Y+6
	dc.b				6*_Y+1, 7*_Y+6

	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		* 'S'
	dc.b				1*_Y+0, 3*_Y+1
	dc.b				3*_Y+0, 4*_Y+7
	dc.b				4*_Y+6, 6*_Y+7
	dc.b				6*_Y+0, 7*_Y+7

	dc.b	  _NEXTCHAR+0*_Y+0, 7*_Y+1		* 'K'
	dc.b				3*_Y+1, 4*_Y+3
	dc.b				2*_Y+2, 3*_Y+3
	dc.b				1*_Y+3, 2*_Y+4
	dc.b				0*_Y+4, 1*_Y+6
	dc.b				4*_Y+3, 5*_Y+4
	dc.b				5*_Y+4, 6*_Y+5
	dc.b				6*_Y+5, 7*_Y+6
	
	dc.b	_NEXTWORD-_NEXTCHAR

	dc.b	  _NEXTCHAR+0*_Y+1, 7*_Y+2		* '1'
	dc.b				1*_Y+0, 2*_Y+1
	
dataend:	

	dc.w	$100

base:	
    dc.w	dep
	
	ifne	system

    Section	bss

	ds.b	1000
    ds.b    65536
    ds.b	65536
	
    endc
