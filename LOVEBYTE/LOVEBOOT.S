*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2024 J.Hubert
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
cpu:        set	0
perspec:	set	6*8
zstart:		set	255				; necessarily 255 (wrap on byte)
ystart:		set 22
xstart:		set -146
xmiddle:	set 160
ymiddle:	set 100
charstep:	set 38
screenbase: set $20000

_Y			EQU	8
_NEXTCHAR	EQU 128

SOUNDCOUNT: EQU -4
CURRENTZ:   EQU -2

nbsides:			set 1
nbtracks:			set 1
nbsectorspertrack:	set 1
*-------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\LOVEBYTE\LOVEBOOT.BOT
	
	; -----------------------------------------------

	opt	o+,a+,p+
	

	clr.l	$fffffa06.w			; Timers off
	lea	    vbl(pc),a3			* Fixe new vbl

	bra.s       bootsectorstart

	;---------------------------------- start floppy header ---------------------------------------------
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

    dc.b        (nbsides*nbtracks*nbsectorspertrack)&255  ; @$13 NSECTS: total number of sectors (little endian)
    dc.b        (nbsides*nbtracks*nbsectorspertrack)/256  ; 

    dc.b        $F9   ; @$15 MEDIA: media type descriptor

    dc.b        0   ; @$16 SPF: nb sectors per FAT   (little endian)
    dc.b        0   

    dc.b        nbsectorspertrack   ; @$18 SPT: nb sectors per track (little endian)
    dc.b        0

    dc.b        nbsides   ; @$1A NHEADS: nb heads (little endian) => 2 for two sided
    dc.b        0

    ;dc.b        0   ; @$1C NHID: nb hidden sectors (little endian) => not used on ST 
    ;dc.b        0   ; as not used on ST => optimize using it for code...

	;---------------------------------- end floppy header ---------------------------------------------

bootsectorstart: 

	move.l	a3,$70.w			*
	move.l	#screenbase,d5		; set framebuffer to absolute address instead

	move.l	d5,a1
	moveq.l	#-1,d0
erase:	
	clr.b	(a1)+				* Efface 2 ecrans
	dbra.w	d0,erase

	; d5 : current frame buffer
	; a0 : display frame buffer for rectangle
	; a1 : increasing pointer on data
	; a2 : x in rect loop
	; a3 : points on variables area
	; a4 : song pointer
	; a5 : unused
	; a6 : temporary hardware registers address

	lea	    song(pc),a4
	addq.l	#2,a3				* Equivalent to	lea	base(pc),a3
	clr.l	(a3)+				; a3 points on base:

    ; MAIN LOOP ----------------------------------------------

main:					    
    ; Vsync
	stop	#$2300			    

	moveq.l	#15,d1			; preload before to also use it for screen swap (not only gradient)
					
    ; Set screen address
	lea		$ffff8203.w,a6
	movep.l	d5,(a6)
	bchg	d1,d5
	
	; Small blue gradient
.gradient:
    move.w	#42*6+41,d0
.line:
    dbf.w   d0,.line
	move.w	d1,d7
	lsr.w	#1,d7
	bcc.s	.noextended
	addq.w	#8,d7
.noextended:
    move.w  d7,$ffff8240.w
    dbf.w   d1,.gradient

    lea	    SOUNDCOUNT(a3),a1

	lea		$ffff8800.w,a6
	addq.b	#1,(a1)
	blt.s	nosound
sound:
	move.w	(a4)+,d0
	ble.s	.next
	movep.w	d0,(a6)
	bra.s	sound
.next:
	move.w	d0,(a1)
	ext.w	d0
	add.w	d0,a4
nosound
	addq.l	#2,a1

    ; Manage Z and font colors
	subq.b	#1,1(a1)
	move.w	(a1)+,d0
	neg.b	d0
	lsr.w	#5,d0
	move.w	d0,d1
	mulu.w	#$111,d0
	lsl.w	#8,d1
	move.w	d0,$ffff8242.w
	move.w	d1,$ffff8248.w

    ; Erase screen planes with blitter
	moveq.l	#-1,d0          
	moveq.l	#4,d7				* Erase screen
	lea	    $ffff8a28.w,a6		
 	move.l	d0,(a6)+			* End mask 1 & 2
 	move.w	d0,(a6)+			* End mask 3
	move.w	d7,(a6)+			* Inc X dest
	move.w	d7,(a6)+			* Inc Y dest
	move.l	d5,(a6)+			* Dest address
	move.l	#8000*65536+1,(a6)+	* X & Y count
	clr.w	(a6)+				* Hop + op
	move.w	d0,(a6)				* Go approximate #%11000000*256						
	
	; Re-param for rectangles
	move.b	d0,-(a6)			* Hog + combi log (Bits '1')

	clr.w	(a3)				* Counter = 0

	lea		data(pc),a1
	move.w	#xstart,a2

.repeatrect:
	move.l 	d5,a0				* Screen address

	move.w	(a3),d0				* Manage different color for heart
	subq.w	#2,d0
	blt.s	.firstplane
	subq.w	#5,d0
	bge.s	.firstplane
	addq.l	#4,a0
.firstplane:

	bsr.s	compute
	move.w	d2,d0
	move.w  d4,d1

	bsr.s	compute

	bsr.s	rect1plane		* Draw rectangle

	addq.w	#1,(a3)			* Inc counter
	cmp.w	#(dataend-data)/2,(a3)
	bne.s	.repeatrect		* Loop
    
	ifne	cpu
	not.w	$ffff8240.w
	endc
	
	bra 	main
	
;-------------------------------------------------------------------------
compute:	
	move.b	(a1)+,d4
	bpl.s	.minus
	lea		charstep(a2),a2
.minus:
	moveq.l #7,d2
	and.w	d4,d2
	asl.w	#2,d2
	add.w	a2,d2

	moveq.l	#perspec,d7

	muls.w	d7,d2
    divs.w	CURRENTZ(a3),d2
	add.w   #xmiddle,d2

	and.w	#$38,d4
	sub.w	#ystart,d4
	muls.w	d7,d4
    divs.w	CURRENTZ(a3),d4
	add.w	#ymiddle,d4

rts_:	rts

;-------------------------------------------------------------------------
rect1plane:
	tst.w	d0				* If x1 < 0
	bpl.s	.okh1			*
 	tst.w	d2				*   If x2 < 0 => norect
	bmi.s	rts_			*   Else
	moveq.l	#0,d0			*     x1 = 0
.okh1:
					
	move.w	#319,d7			
	cmp.w	d7,d2			* If x2 > 319
	ble.s	.okh2			*
	cmp.w	d7,d0			*   If x1 > 319 
	bgt.s	rts_			*     no rectangle
	move.w	d7,d2			*   Else x2 = 319
.okh2:

	tst.w	d1				* If y1 < 0
	bpl.s	.okv1			*
 	tst.w	d4				*   If y2 < 0 => no rectangle
	bmi.s	.norect			*   Else
	moveq.l	#0,d1			*     x1 = 0
.okv1:
			
	move.w	#199,d7			
	cmp.w	d7,d4			* If y2 > 199
	ble.s	.okv2			*
	cmp.w	d7,d1			*   If y1 > 199
	bgt.s	.norect			*     no rectangle
	move.w	d7,d4			*   Else y2 = 199							
.okv2:

    ; rect
	sub.w	d1,d4			
	addq.w	#1,d4			* Hauteur
	
	mulu.w	#160,d1			* Adr affichage + y1 * 160
	add.w	d1,a0			
	
	moveq.l	#$fffffff0,d7
	move.w	d0,d6			
	and.w	d7,d6			* d6 = x1 & 0xfff0
	lsr.w	#1,d6			* / 2
	add.w	d6,a0			* Adr affichage + offset horizontal
	
	move.w	d2,d3			
	and.w	d7,d3			* d3 = x2 & 0xfff0
	lsr.w	#1,d3			* / 2
	sub.w	d6,d3		    * d3 = nb horiz chunks * 8	
	move.w	#xmiddle,d6
	sub.w	d3,d6	
	lsr.w	#3,d3			* d3 = nb words		
	
	moveq.l	#15,d1			
	and.w	d1,d2			* d2 = x2 & 15
	sub.w	d2,d1			* d1 = 15 - (x2 & 15)
	moveq.l	#0,d7			
	bset.l	d1,d7			
	subq.l	#1,d7			* d7 = (1 << d1) - 1
	not.w	d7				
	
	lea		$ffff8a2c.w,a6
	move.w	d7,(a6)+		* Masque droit	
	move.w	#8,(a6)+
	move.w	d6,(a6)+		* Inc Y dest = 160 - nb horiz chunks * 8
	move.l	a0,(a6)+		* Adresse dest

	moveq.l	#15,d1			
	and.w	d1,d0	        * d0 = x1 & 15		
	sub.w	d0,d1			* d1 = 15 - (x1 & 15)
	moveq.l	#0,d6			
    addq.l  #1,d1
	bset.l	d1,d6			
	subq.l	#1,d6			* d6 = (1 << (d1 + 1)) - 1

	tst.w	d3
	bne.s	.oklarg			* If width nb words = 0
	and.w	d7,d6			*   And left & right mask
.oklarg:					
	move.w	d6,$ffff8a28.w	* Set left mask
	addq.w	#1,d3			*
	movem.w	d3-d4,(a6)		* Hori size & vert size
	move.w	#%11000000*256,$ffff8a3c.w	* No partage & Go !
	
.norect:
	rts

song:
	dc.w	$C00				; Env period hight
	dc.w	$B77				; Env period low
	dc.w	$810				; Level A = 16
	dc.w	$0BD				; Tone period low
	dc.w	$103				; Tone period hight
	dc.w	$7FE				; Tone A on

	dc.w	$D0E				; Triangle
	dc.w	$F500				; Wait
	dc.w	$D08				; Sawtooth
	dc.w	$F5F8				; Wait & loop

;-------------------------------------------------------------------------
data:	
	dc.b				0+0*_Y, 1+7*_Y		; 0		  1
	dc.b				1+6*_Y, 7+7*_Y		; 1		  1
											; 2       1
											; 3		  1
											; 4		  1
											; 5		  1
											; 6		  1222222
													  
	dc.b	  _NEXTCHAR+1+0*_Y, 3+5*_Y		; 0        00 11		(overdraw saves 2 bytes)
	dc.b				4+0*_Y, 6+5*_Y		; 1 	  2002112	
	dc.b				0+1*_Y, 7+4*_Y		; 2       2002112	
	dc.b				2+4*_Y, 5+6*_Y		; 3       2002112 
	dc.b				3+6*_Y, 4+7*_Y		; 4        03331 
											; 5         333	
											; 6          4
	
	dc.b	  _NEXTCHAR+0+0*_Y, 1+7*_Y		;  0      0     1
	dc.b				6+0*_Y, 7+5*_Y		;  1	  0     1
	dc.b				1+6*_Y, 4+7*_Y		;  2	  0     1
	dc.b				4+5*_Y, 6+6*_Y		;  3	  0     1
											;  4	  0	   	1 
											;  5	  0	  33	
											;  6	  0222 
	 
	dc.b	  _NEXTCHAR+0+0*_Y, 7+1*_Y		; 0	      0000000
	dc.b				0+1*_Y, 1+6*_Y		; 1       1
	dc.b				0+6*_Y, 7+7*_Y		; 2       1
	dc.b				1+3*_Y, 6+4*_Y		; 3       133333
											; 4       1
											; 5       1
											; 6       2222222

	dc.b	  _NEXTCHAR+0+0*_Y, 6+1*_Y		; 0	      0000000
	dc.b				0+1*_Y, 1+6*_Y		; 1       1      3
	dc.b				0+3*_Y, 6+4*_Y		; 2	      1      3
	dc.b				6+1*_Y, 7+3*_Y		; 3	      2222222	
	dc.b				6+4*_Y, 7+6*_Y		; 4	      1      4
	dc.b			    0+6*_Y, 6+7*_Y		; 5	      1      4
											; 6       5555555

	dc.b	  _NEXTCHAR+0+0*_Y, 1+3*_Y		; 0	      0      2
	dc.b				1+3*_Y, 6+4*_Y		; 1		  0      2
	dc.b				6+0*_Y, 7+6*_Y		; 2 	  0      2
	dc.b				1+6*_Y, 6+7*_Y		; 3		   1111112
											; 4              2
											; 5              2
											; 6		   333333
	
	dc.b	  _NEXTCHAR+0*_Y+0, 1*_Y+7		; 0		  0000000
	dc.b				0*_Y+3, 7*_Y+4		; 1          1
											; 2          1
											; 3          1
											; 4          1
											; 5          1
											; 6          1
	
	dc.b	  _NEXTCHAR+0+0*_Y, 7+1*_Y		; 0	      0000000
	dc.b				0+1*_Y, 1+6*_Y		; 1       1
	dc.b				0+6*_Y, 7+7*_Y		; 2       1
	dc.b				1+3*_Y, 6+4*_Y		; 3       133333
											; 4       1
											; 5       1
											; 6       2222222
dataend:	

vbl:	rte

base:
	; 1 byte sound counter
	; 1 byte sound counter trash
	; 1 word currentz
	; 1 word rectangle display counter 
