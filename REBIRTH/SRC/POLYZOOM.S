*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2018 J.Hubert
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

	output  D:\PROJECTS\DEMOS\OUTPUT\REBIRTH\POLYZOOM.O

	opt	o+

	xdef pzloop,pzprecompute,pzsetrasters

    xref RASvbl2,RASmid1

*-------------------------------------------------------------------------
pitch:			set	320/2	; should be multiple of two
xmax:			set	319
ymax:			set	199

benchdiv:	set 0
*-------------------------------------------------------------------------
LPREC:	set	200

*----------------------------------------------*
*	MAIN LOOP
*----------------------------------------------*
pzprecompute:
	
		movem.l d3-d7/a2-a6,-(sp)	* 8 * 4 => 32 offset

		* u16* polyzoomprecompute(u16* poly, s16 cs, s16 sn, u16 coef, s16 offsetx, s16 offsety, s16 centerx, s16 centery, u16* dlist);
		
		* a0 = poly
		* a1 = displist
		* d0 = cs
		* d1 = sn
		* d2 = coef
		* stack = offsetx (offsetx*16+depx)
		* stack = offsety
		* stack = centerx (SCREEN_WIDTH >> 1)
		* stack = centery (SCREEN_HEIGHT >> 1)

		move.w	44(sp),a2	* offsetx
		move.w	46(sp),a3	* offsety
		move.w  48(sp),a4	* centerx
		move.w	50(sp),a5	* centery

		lea		pzprecompute_minx(pc),a6

		move.w	#$7FFF,(a6)	    ; minx
		move.w	#$8000,2(a6)	; maxx
		move.w	#$7FFF,4(a6)	; miny
		move.w	#$8000,6(a6)	; maxy

		move.w	(a0),edgescount
		
		move.w	(a0)+,(a1)
		subq.w	#1,(a1)+

precalcedges:
		move.w  (a0)+,d4	* d4 = x
		lsl.w	#4,d4		* 
		add.w	a2,d4		* a2 = offsetx

		move.w	d4,d6		* copy x into d6
		muls.w	d0,d6		* d6 = x * cs
		muls.w	d1,d4		* d4 = x * sn

		move.w	(a0)+,d5	* d5 = y
		lsl.w	#4,d5		*
		add.w	a3,d5		* a3 = offsety

		move.w	d5,d7		* copy y into d7
		muls.w	d0,d7		* d7 = y * cs
		muls.w	d1,d5		* d5 = y * sn

		sub.l	d5,d6		* d6 -= y * sn => x2
		add.l   d4,d7		* d7 += x * sn => y2 

		swap	d6
		swap	d7

		muls.w	d2,d6		* x2 *= coef
		muls.w	d2,d7		* y2 *= coef

		swap	d6
		swap	d7

		add.w	a4,d6
		add.w	a5,d7

		move.w	d6,(a1)+	* *dlist++ = x2
		move.w	d7,(a1)+	* *dlist++ = y2	
	
		cmp.w	(a6),d6
		bge.s	pzprecompute_minxok			* if x2 < min
		move.w	d6,(a6)             		*	min = x2
pzprecompute_minxok:

		cmp.w	2(a6),d6
		ble.s	pzprecompute_maxxok			* if x2 > max
		move.w	d6,2(a6)             		*   max = x2
pzprecompute_maxxok:

		cmp.w	4(a6),d7	
		bge.s	pzprecompute_minyok			* if y2 < min
		move.w	d7,4(a6)					*	min = y2
pzprecompute_minyok:

		cmp.w	6(a6),d7
		ble.s	pzprecompute_maxyok			* if y2 > max
		move.w	d7,6(a6)					*   max = y2
pzprecompute_maxyok:

		subq.w	#1,edgescount
		bne.s	precalcedges

		move.l	a1,a0

		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+

		movem.l (sp)+,d3-d7/a2-a6

		rts



pzsetrasters:

nbcolors: set 48

	; a0 : rasterop buffer
	; a1 : pal data
	; d0 : miny
	; d1 : maxy
	
	movem.l	d2-d4/a3,-(sp)

	move.w d1,d2		
	sub.w  d0,d2        ; d2 = dy
	
	move.l	#(nbcolors-3)*256,d3
	divu.w	d2,d3		; d3 = inc
	swap	d3
	sub.w	d3,d3
	lsr.l	#7,d3

	moveq.l	#3,d4		; d4 = firstline

	tst.w	d0
	bge.s	pzsetrasters_minypositive

	;	if (miny < 0)

	neg.w	d0
	mulu.w	#nbcolors-3,d0
	divu.w	d2,d0
	bge.s	pzsetrasters_offpositive
	moveq.l	#0,d0
pzsetrasters_offpositive:
	add.w	d0,d0
	add.w	d0,a1

	clr.w	(a0)+           ; background color
	move.w	(a1)+,(a0)+     ; color
	move.w	#2,(a0)+		; scanlinestointerupt
	move.l	#RASmid1,(a0)+  ; next raster routine

	move.w	(a1)+,(a0)      ; color with stop code
	or.w	#$8000,(a0)+
	move.w	#2,(a0)+        ; scanlinestointerupt
	move.l	#RASmid1,(a0)+

	bra.s	pzsetrasters_minywasnegative

pzsetrasters_minypositive:

	addq.w	#1,d4
	tst.w	d0
	beq.s	pzsetrasters_minyis0
	subq.w	#1,d4
	add.w	d0,d4
pzsetrasters_minyis0:

	clr.w	(a0)+
	move.w	(a1)+,(a0)+
	move.w	d4,(a0)+
	move.l	#RASmid1,(a0)+

	move.w	(a1)+,(a0)
	or.w	#$8000,(a0)+
	move.w	#2,(a0)+
	move.l	#RASmid1,(a0)+
	
pzsetrasters_minywasnegative:

	move.w	#ymax-1,d0
	cmp.w	d0,d1
	ble.s	pzsetrasters_maxynotclipped
	move.w	d0,d1
pzsetrasters_maxynotclipped:

	moveq.l	#0,d0		; acc

	sub.w	d4,d1
	ble		pzsetrasters_noloop

	lea		pzsetrasters_loop(pc),a3
	add.w	d1,d1
	move.w	d1,d4
	lsl.w	#3,d1
	add.w	d4,d1
	sub.w	d1,a3

	moveq.l	#0,d4		        ; acc2

	jmp		(a3)
	
	rept	ymax
	move.l	d0,d2
	swap	d2
	add.w	d0,d4		        ; acc2 += acc
	bcc.s	*+4
	addq.w	#1,d2
	add.w	d2,d2
	move.w	(a1,d2.w),(a0)+		; pal + 2 * ( acc + acc2 > 65535 ? 1 : 0 ) 
	add.l	d3,d0		        ; acc += inc
	endr
pzsetrasters_loop:

	swap	d0
	add.w	d0,d0
pzsetrasters_noloop:
	move.w	(a1,d0.w),(a0)
	or.w	#$8000,(a0)+ 

	move.w	#255,(a0)+
	clr.l	(a0)+	

	movem.l	(sp)+,d2-d4/a3

	rts


*----------------------------------------------*
*	dev test for line
*----------------------------------------------*
; void Clinetest(void* _image, u16* _coord)

	xdef Clinetest

Clinetest:

	movem.l	a2-a5/d0-d7,-(sp)
	move.w	(a1)+,d0
	move.w	(a1)+,d1
	move.w	(a1)+,d2
	move.w	(a1)+,d3

	bsr.s	initblitterforhline
	bsr		line

	movem.l	(sp)+,a2-a5/d0-d7

	rts

initblitterforhline:
	
	clr.l	$ffff8a20.w							* inc x / inc y source
	move.l	a0,$ffff8a24.w						* source adr
	move.w	#-1,$ffff8a2a.w						* mask 2
	move.l	#$80008,$ffff8a2e.w					* inc x / inc y dest
	move.w	#6,$ffff8a3a.w						* HOP = 0 (bits 1 instead of source) and xor mode
	
	rts

*----------------------------------------------*
*	MAIN LOOP
*----------------------------------------------*

pzloop_xorpass:
;------------------------------------------------
;	Xor blitter pass
;------------------------------------------------

	;move.w	#$700,$ffff8240.w

	move.w	(sp)+,d4	; maxy
	cmp.w   #ymax,d4
	ble.s	pzloop_maxyok
	move.w	#ymax,d4
pzloop_maxyok:
	
	move.w	(sp)+,d3	; miny
	bge.s	pzloop_minyok
	moveq.l	#0,d3
pzloop_minyok:

	move.w	d4,d5
	sub.w	d3,d5

	lea		pitchmul(pc),a3
	add.w	d3,d3
	move.w	(a3,d3.w),d3
	lea		(a0,d3.w),a3

	lea	$ffff8a20.w,a2

	move.w	(sp)+,d3			; word count x
	add.w	(sp)+,a3			; adr offset x

	move.w	#1+pitch/8,d4		
	sub.w	d3,d4
	add.w	d4,d4
	add.w	d4,d4
	add.w	d4,d4

	move.w	#8,(a2)+			; inc x source 
	move.w	d4,(a2)+			; inc y source

	move.l	a3,(a2)+			; adr source

	move.w	#-1,(a2)+	
	addq.w	#2,a2
	move.w	#-1,(a2)+	

	move.w  #8,(a2)+			; inc x dest
	move.w	d4,(a2)+			; inc y dest

	lea		pitch(a3),a3
	move.l	a3,(a2)+			; adr dest 

	move.w	d3,(a2)+			; x count
	move.w	d5,(a2)+			; y count
	move.l	#$2068000,(a2)

waitblit2:
	bset.b  #7,$FFFF8A3C.w     ; (re)start the BLiTTER
	nop                        ; BLiTTER will need a few cycles
	bne.s   waitblit2          ; Loop if registers shows "busy"

	;move.w	#$0,$ffff8240.w

	movem.l (sp)+,d3-d7/a2-a5

	rts		* return to caller


pzloop:

	movem.l d3-d7/a2-a5,-(sp)

	move.w d0,polycount

;------------------------------------------------
;	Blitter erase
;------------------------------------------------
	bsr		initblitterforhline

	;move.w	#$7,$ffff8240.w

	move.w	#-1,$FFFF8A28.w	
	move.w	#-1,$FFFF8A2C.w
	move.l	a0,$FFFF8A32.w
	
	move.l	#pitch/8*150*65536+1,$FFFF8A36.w
	move.l	#$C000,$FFFF8A3A.w

	move.l	#pitch/8*(ymax+1-150)*65536+1,$FFFF8A36.w
	move.l	#$8000,$FFFF8A3A.w
	
waitblit:
	bset.b  #7,$FFFF8A3C.w     ; (re)start the BLiTTER
	nop                        ; BLiTTER will need a few cycles
	bne.s   waitblit           ; Loop if registers shows "busy"

	move.w	#6,$ffff8a3a.w						* HOP = 0 (bits 1 instead of source) and xor mode

	;move.w	#$0,$ffff8240.w

;------------------------------------------------
;	Run display list
;------------------------------------------------

	move.w	(a1)+,-(sp)	; adr offset x
	move.w	(a1)+,-(sp)	; x word count
	move.w	(a1)+,-(sp)	; miny
	move.w	(a1)+,-(sp)	; maxy

poly_loop:

	move.w	(a1)+,edgescount
	blt.s	noedges

	move.w	(a1)+,d0
	move.w	(a1)+,d1

	move.w	d1,-(sp)
	move.w	d0,-(sp)

edges_loop:	
	move.w	(a1),d2
	move.w	2(a1),d3

	move.l	a0,-(sp)
	move.l	a1,-(sp)
	bsr.s	line
	move.l	(sp)+,a1
	move.l	(sp)+,a0

	move.w  (a1)+,d0
	move.w  (a1)+,d1

	subq.w	#1,edgescount
	bne.s	edges_loop

	move.w	(sp)+,d2	* first point
	move.w	(sp)+,d3

	move.l	a0,-(sp)
	move.l	a1,-(sp)
	bsr.s	line
	move.l	(sp)+,a1
	move.l	(sp)+,a0

noedges:
	subq.w	#1,polycount
	bne.s	poly_loop

	move.l	a1,d0

;	move.w	#$00FF,$ffff8240.w

	bra		pzloop_xorpass	; avoid having bsr.s too large to be short
	
*-------------------------------------------------------------------------*
*	LINES                            
*-------------------------------------------------------------------------*
*	d0......x1
*	d1......y1
*	d2......x2
*	d3......y2
*	a0......ecran
*-------------------------------------------------------------------------*

*-------------------------------------------------------------------------*
line:	
	cmp.w	d1,d3			* On ordonne les coordonnees
	bge.s	line_yordered	* verticales provisoirement y2 > y1
	exg.l	d0,d2			* pour le clip vertical
	exg.l	d1,d3			*
line_yordered:
	tst.w	d3				* Si l'ordonnee la plus grande
	bge.s	line_y2positive	* est negative (y2 < 0) on trace la droite

	moveq.l	#0,d3			* entre x1 et x2 (passe xor oblige) 
	moveq.l	#0,d1			* On se branche au niveau du clip 
	bra.s	clipx			* horizontal

line_y2positive:
	tst.w	d1				* Si l'ordonnee la plus petite
	bge.s	clipy2			* negative et l'autre positive
							*
							* On calcule l'intersection avec le
							* bord superieur
							* clipy
	move.w	d0,d5			*
	sub.w	d2,d5			* d5 = x1-x2
							*
	move.w	d3,d6			*
	sub.w	d1,d6			* d6 = y2-y1

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	muls.w	d1,d5			* (x1-x2) * y1
	divs.w	d6,d5			* (x1-x2) * y1 / (y2-y1)

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	add.w	d0,d5			* (x1-x2) * y1 / (y2-y1) + x1
							* d5: abscisse d'intersection
	movem.w	d0-d3/d5/d6,-(sp)
	move.l	a0,-(sp)			*
	moveq.l	#0,d1			*
	moveq.l	#0,d3			*
	move.w	d5,d2			*
							*
	bsr.s	clipx			* On trace le bout de droite en haut
							* de l'ecran horizontalement
	move.l	(sp)+,a0		*
	movem.w	(sp)+,d0-d3/d5/d6 	* (passe xor oblige)

	move.w	d5,d0			* On trace le bout de droite 
	moveq.l	#0,d1			* restant sur l'ecran
							* coordonnees non inversees

clipy2:	
	cmp.w	#ymax,d1		* Si l'ordonnee la plus petite
	bgt.s	return			* est superieure … ymax

	cmp.w	#ymax,d3		* Si l'ordonnee la plus grande>ymax
	ble.s	clipx			* et l'autre <ymax
					
							* On calcule l'intersection avec le
							* bord superieur
	sub.w	d0,d2			* d2 = x2-x1
	sub.w	d1,d3			* d3 = y2-y1
							*
	move.w	#ymax,d5		*
	sub.w	d1,d5			* d1 = y1-ymax 

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	muls.w	d5,d2			* (x2-x1) * (y1-ymax)
	divs.w	d3,d2			* (x2-x1) * (y1-ymax) / (y2-y1)

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	add.w	d0,d2			* (x2-x1) * (y1-ymax) / (y2-y1)+x1
							* d2: abscisse d'intersection
	move.w	#ymax,d3		*

	*----------------------------
	* clipx
	*----------------------------

	* clipx1 --------------------

clipx:	
	cmp.w	d2,d0			*
	ble.s	clipx_ok1		* Si x1 > x2: Echange x1 x2 & y1 y2
	exg.l	d2,d0			*
	exg.l	d3,d1			*
clipx_ok1:
	tst.w	d2				* if d2 < 0 => do nothing
	bge.s	clipx_ok2		*
return:	
	rts						*
clipx_ok2:
	move.w	d0,d7			* if x1 < 0 => store to fix length when displaying
	blt.s	clipx_ok3
	moveq.l	#0,d7
clipx_ok3:

	* clipx2 --------------------
	cmp.w	#xmax,d0		* Si l'abscisse la plus petite
	bgt.s	return			* est superieure à xmax
							* => on trace pas
	cmp.w	#xmax,d2		* Si l'abscisse la plus grande > xmax
	ble.s	trace			* 					
							* On calcule l'intersection avec le
							* bord superieur
	sub.w	d0,d2			* d2 = x2-x1
	sub.w	d1,d3			* d3 = y2-y1
							*
	move.w	#xmax,d5		*
	sub.w	d0,d5			* d0 = xmax-x1 

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	muls.w	d5,d3			* (y2-y1) * (xmax-x1)
	divs.w	d2,d3			* (y2-y1) * (xmax-x1) / (x2-x1)

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	add.w	d1,d3			* (y2-y1) * (xmax-x1) / (x2-x1)+y1
							* d3: ordonnee d'intersection
	move.w	#xmax,d2		*
	
	*----------------------------
	* display
	*----------------------------

	* here we have d0,d1,d2,d3,d7 => x1, y1, x2, y2, lengthfix from x1 clip

trace:	
	lea		-pitch.w,a2 *
 	sub.w	d2,d0			 * d0: largeur
	beq.s	return			 * if dx = 0 => do nothing
	neg.w	d0				 *
	sub.w	d3,d1			 * d1: hauteur
	beq		h_line			 * if dy = 0 => hline
	blt.s	trace_ok		 *
	lea		pitch.w,a2  * if dy negative => invert address increment
	neg.w	d1				 * abs (dy)

trace_ok:	
	neg.w	d1				*

	lea	pitchmul(pc),a1		* compute start address : a0 += y2 * pitch
	add.w	d3,d3			*
	move.w	(a1,d3.w),d3	* 
	; add.w	d3,d3			* (pitch / 2) * 2
	add.w	d3,a0

	cmp.w	d0,d1			* Compare dx & dy
;	blt.s	horizontal		* => dx > dy => horizontal routine
	bgt		vertical		* => dy > dx => vertical routine
	beq 	d45				* => equal	 => 45° routine

*
*	Dx>Dy
*------------------------------------------------------------------
horizontal:	

	swap	d1				* dy*65536
	sub.w	d1,d1			*

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	divu.w	d0,d1			* d1: increment dy*65536/dx 

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc
	
	add.w	d7,d0			* fix length according to x1 clip

	lea		line_display_horiz(pc),a3	* compute routine adresses range
	move.l	a3,a4						* 

	add.w	d2,d2						* start address : get values at x2 * 4
	add.w	d2,d2
	move.l	line_display_horiz_tab(pc,d2.w),d7

	add.w	d7,a0			* compute offset into routine
	swap	d7
	add.w	d7,a3			* add x2 address offset to a0

	add.w	d0,d0			* end address into routine : x2 * 4 + length * 4
	add.w	d0,d0
	sub.w	d0,d2
	add.w	line_display_horiz_tab(pc,d2.w),a4

	movem.w	precharge(pc),d0/d2-d7

	move.w (a4),a5			* backup opcode
	move.w #$4E75,(a4)		* put an rts into the routine
	jsr	(a3)
	move.w	a5,(a4)			* restore overwritten opcode

	rts

precharge:
	dc.w	1			* d0
	dc.w	4			* d2
	dc.w	8			* d3
	dc.w	32			* d4
	dc.w	-32768		* d5
	dc.w	64			* d6
	dc.w	128			* d7

line_display_horiz_tab:

screen:	set	0
opcode:	set	(pitch/8)*2+(pitch*2)*8

		rept	pitch/8

		; 8 times
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen

screen:	set screen+1

opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen

		; 8 times
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen
opcode:	set		opcode-8
		dc.w	opcode
		dc.w	screen

opcode:	set	opcode-2
screen:	set screen+7

		endr	

*------------------------------------------------------------------
*	Dx<Dy
*------------------------------------------------------------------

vertical:
	move.l	a2,d6
	lsl.l	#8,d1
	and.l	#$ffff00,d1

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	divu.w	d0,d1			* d0: increment dx*65536/dy 	

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	add.w	d7,d0

	add.w	d2,d2
	add.w	d2,d2
	lea	table2(pc),a1
	move.l	(a1,d2.w),d4
	add.w	d4,a0			
	swap	d4

	move.l	d6,d2
	move.l	d1,d3
	lsr.w	#8,d3

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc

	muls.w	d3,d6

	ifne benchdiv
	not.w	$ffff8240.w		*
	endc
	
	move.w	d0,d5			* compute jmp distance
	lsl.w	#4,d0
	add.w	d5,d5
	add.w	d5,d0
	neg.w	d0

	move.w	#-32768,d5		* Accumulateur … 0

	lea		vertical_display(pc),a3
	jmp		(a3,d0.w)

	rept	ymax+1
	eor.w	d4,(a0)			* Affichage point
	add.l	d6,a0			* adr affichage-160 (trace a l'envers)
	add.w	d4,d4			* on decale de 1 le bit tournant
 	bcc.s	*+6				* Si abscisse bit > 15
	subq.l	#8,a0			* adr affichage-8 (trace a l'envers)
	moveq.l	#1,d4
	add.b	d1,d5			* Incremente le taux d'erreur
	bcc.s	*+4				* Si bit carry , taux > 1 unite (65536)
	add.l	d2,a0			*
	endr

vertical_display:

	rts

*----------------------------
*
*----------------------------
h_line:	

	lea	pitchmul(pc),a1		* 
	add.w	d3,d3			* stored on word expressing pitch / 2
	move.w	(a1,d3.w),d3	* get the value
	; add.w	d3,d3			* pitch / 2 * 2
	add.w	d3,a0

	add.w	d7,d0			* fix length according to x1 clip
	move.w	d2,d1			* d2 = x2
	sub.w	d0,d1			* d1 = x1

	moveq.l	#-16,d0		
	and.w	d1,d0
	lsr.w	#1,d0
	add.w	d0,a0			* d0 = (x1 & FFF0) >> 1

	moveq.l	#-16,d5
	and.w	d2,d5
	lsr.w	#1,d5			* d5 = (x2 & FFF0) >> 1

	moveq.l	#15,d3
	and.w	d1,d3			* d3 = x1 & 15
	add.w	d3,d3
	moveq.l	#15,d6
	and.w	d2,d6			* d6 = x2 & 15
	add.w	d6,d6

	sub.w	d0,d5

	bne.s	h_line_long

	move.w	mask_start(pc,d3.w),d1				* masque final 1 - 2 - 3 => il faut les setter
	and.w	mask_end(pc,d6.w),d1
	move.w	d1,$ffff8a28.w
	move.l	a0,$ffff8a32.w						* dest adr
	move.l	#$10001,$ffff8a36.w					* nb word to transfer horizontal + vertical
	move.w	#$C000,$ffff8a3c.w					* go exclusive

	rts

h_line_long:

	lsr.w	#3,d5
	addq.w	#1,d5

	move.w	mask_start(pc,d3.w),$ffff8a28.w		* masque final 1 - 2 - 3 => il faut les setter
	move.w	mask_end(pc,d6.w),$ffff8a2c.w
	move.l	a0,$ffff8a32.w						* dest adr
	move.w	d5,$ffff8a36.w						* nb word to transfer horizontal
	move.w	#1,$ffff8a38.w						* nb lines
	move.w	#$C000,$ffff8a3c.w					* go exclusive

	rts

mask_start:
	dc.w	%0111111111111111
	dc.w	%0011111111111111
	dc.w	%0001111111111111
	dc.w	%0000111111111111
	dc.w	%0000011111111111
	dc.w	%0000001111111111
	dc.w	%0000000111111111
	dc.w	%0000000011111111
	dc.w	%0000000001111111
	dc.w	%0000000000111111
	dc.w	%0000000000011111
	dc.w	%0000000000001111
	dc.w	%0000000000000111
	dc.w	%0000000000000011
	dc.w	%0000000000000001
	dc.w	%0000000000000000

mask_end:
	dc.w	%1000000000000000
	dc.w	%1100000000000000
	dc.w	%1110000000000000
	dc.w	%1111000000000000
	dc.w	%1111100000000000
	dc.w	%1111110000000000
	dc.w	%1111111000000000
	dc.w	%1111111100000000
	dc.w	%1111111110000000
	dc.w	%1111111111000000
	dc.w	%1111111111100000
	dc.w	%1111111111110000
	dc.w	%1111111111111000
	dc.w	%1111111111111100
	dc.w	%1111111111111110
	dc.w	%1111111111111111

*
*	Dx=Dy
*------------------------------------------------------------------
diago:	
	rept	8
	eor.w	d4,(a0)			* Affiche le point
	add.l	a2,a0			* Mvt vert
	add.w	d4,d4			* Mvt hori
	bcc.s	*+6				* Incremente adr si abscisse mod 16 = 0
	subq.l	#8,a0			*
	moveq.l	#1,d4			*
	endr
saut3:	
	dbra.w	d1,diago	* Longueur

	rts
	
d45:
	;moveq.l	#$FFF0,d5
	;and.w	d2,d5
	;lsr.w	#1,d5
	;add.w	d5,a0

	add.w	d2,d2
	add.w	d2,d2
	lea	table2(pc),a1
	move.l	(a1,d2.w),d4
	add.w	d4,a0			
	swap	d4

	add.w	d7,d0
	add.w	d7,d1

	and.w	#7,d0
	neg.w	d0
	
	add.w	d0,d0			* let's do the mul 12 manually
	add.w	d0,d0
	move.w	d0,d7
	add.w	d7,d7
	add.w	d7,d0

	lsr.w	#3,d1
	jmp	saut3(pc,d0.w)


line_display_horiz:

	rept	pitch/8

	subq.l	#7,a0		; decrement address by 7

 	eor.b	d0,(a0)		; bit 0
 	add.w	d1,d5
 	bcc.s	*+4
 	add.l	a2,a0

	bchg.b	d0,(a0)		; bit 1
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d2,(a0)		; bit 2
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d3,(a0)		; bit 3
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	bchg.b	d2,(a0)		; bit 4
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d4,(a0)		; bit 5
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d6,(a0)		; bit 6
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d7,(a0)		; bit 7
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d0,-(a0)		; bit 0 + decrement address by one
	add.w	d1,d5		
	bcc.s	*+4
	add.l	a2,a0

	bchg.b	d0,(a0)		; bit 1
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d2,(a0)		; bit 2
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d3,(a0)		; bit 3
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	bchg.b	d2,(a0)		; bit 4
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d4,(a0)		; bit 5
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d6,(a0)		; bit 6
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	eor.b	d7,(a0)	    ; bit 7
	add.w	d1,d5
	bcc.s	*+4
	add.l	a2,a0

	endr

	rts

*-------------------------------------------------------------------------*
*	Section	data
*-------------------------------------------------------------------------*
polycount			dc.w	0
edgescount			dc.w	0
pzprecompute_minx:	dc.w	0
pzprecompute_maxx:	dc.w	0
pzprecompute_miny:	dc.w	0
pzprecompute_maxy:	dc.w	0

pitchmul:
var:	set	0
		rept	ymax+2
		dc.w	var
var:	set		var+pitch
		endr	

table2:	
var:	set	0
		rept	pitch/8
		dc.w	$8000,var,$4000,var,$2000,var,$1000,var
		dc.w	$0800,var,$0400,var,$0200,var,$0100,var
		dc.w	$0080,var,$0040,var,$0020,var,$0010,var
		dc.w	$0008,var,$0004,var,$0002,var,$0001,var
var:	set	var+8
		endr
