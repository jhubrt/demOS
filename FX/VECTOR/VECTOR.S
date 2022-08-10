*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2022 J.Hubert
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

	output  D:\PROJECTS\DEMOS\OUTPUT\FX\VECTOR.O

	opt	o+

	xdef VECprecompute,VECclipline
    xdef VECloop,VECclr,VECclrpass,VECxor,VECxorpass,VECcircle

*-------------------------------------------------------------------------
pitch:			set	336/2	; should be multiple of two
xmax:			set	335
ymax:			set	199
*-------------------------------------------------------------------------



*-------------------------------------------------------------------------
* void VECcircle(void* screen_, u16 centerx_, u16 centery_, u16 radius_)
*-------------------------------------------------------------------------
VECcircle:  

	movem.l d3-d6/a4-a6,-(sp)

    move.w	d2,d5 			* Delta = r

    lea		pitchmul(pc),a6
	moveq.l #$FFFFFFF0,d3
    and.w	d0,d3			* xc And $fff0 + adr Affichage
    lsr.w	#1,d3
    add.w	d3,a0
    add.w	d1,d1
    add.w	(a6,d1.w),a0

    move.w	d2,d3
    add.w	d3,d3
    move.w	(a6,d3.w),d3            
    lea		(a0,d3.w),a4
	sub.w	d3,a0	

    move.l	a0,a1
    move.l	a4,a5

	moveq.l #15,d3
	and.w	d0,d3
    neg.w	d3
    add.w	#15,d3

    moveq.l	#0,d6
    bset.l	d3,d6

    move.w	d6,d1

    moveq.l	#-1,d3			* X: d3 = -1 * Y: d2 = r
	moveq.l	#1,d0

	or.w	d6,(a0)
	or.w	d6,(a4)

.loop:
    move.w	d5,d4			* delta: D5
    add.w	d4,d4			* d: D4  	d = delta * 2

    bge.s	.else			* Si d < 0
        
	subq.w	#1,d2		
    blt     .end

    add.w	d3,d4			* d = d + x
	bge.s	.else1			* Si d < 0           
    add.w	d2,d5			* delta = delta + y	
    lea		pitch(a0),a0			
    lea		pitch(a1),a1
    lea		-pitch(a4),a4			
    lea		-pitch(a5),a5

    bra.s	.loop

.else1:						* Sinon ------------------------------

    addq.w	#1,d3			* Inc x

    ror.w	d0,d6
	bcc.s	.nocarry1
    addq.w	#8,a4
    addq.w	#8,a0
.nocarry1:       

    add.w	d1,d1
    bne.s  .noreload1
         
	moveq.l	#1,d1                
    subq.w	#8,a1
    subq.w	#8,a5

.noreload1:

    or.w	d6,(a0)			* Affichage
    or.w	d6,(a4)			* Affichage
    or.w	d1,(a1)			* Affichage
    or.w	d1,(a5)			* Affichage

    add.w	d2,d5			* 
    sub.w	d3,d5			* delta = delta + y - x                                

    lea		pitch(a0),a0			
    lea		pitch(a1),a1
    lea		-pitch(a4),a4			
    lea		-pitch(a5),a5

    bra.s	.loop

.else:  * ---------------------------------------------------------------------
    
    addq.w	#1,d3	* Inc x
        
    ror.w   d0,d6
	bcc.s	.nocarry2
    addq.w	#8,a4
    addq.w	#8,a0
.nocarry2:

    add.w	d1,d1
    bne.s	.noreload2    
    moveq.l	#1,d1                
    subq.w	#8,a1
    subq.w	#8,a5
.noreload2:

    or.w	d6,(a0)			* Affichage
    or.w	d6,(a4)			* Affichage
    or.w	d1,(a1)			* Affichage
    or.w	d1,(a5)			* Affichage
    
	sub.w	d2,d4			* d = d - y
	blt.s	.else2 * Si y >= 0    
	sub.w	d3,d5			* delta = delta - x

	bra.s	.loop

.else2:						* Sinon
	subq.w	#1,d2			* Dec y
	blt.s	.end

    lea		pitch(a0),a0			
    lea		pitch(a1),a1
    lea		-pitch(a4),a4			
    lea		-pitch(a5),a5

    sub.w	d3,d5			* delta = delta + y - x
    add.w	d2,d5

    bra 	.loop

.end:
	movem.l (sp)+,d3-d6/a4-a6
    rts

*----------------------------------------------*
* PRECOMPUTE COORDINATES
*  
* a1 --> 
*   w : nbedges
*   x * u16(x ; y)
* a0 --> 
*   w : minx
*   w : maxx
*   w : miny
*   w : maxy
*----------------------------------------------*
VECprecompute:
	
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

		lea		vecprecompute_minx(pc),a6

		move.w	#$7FFF,(a6)	    ; minx
		move.w	#$8000,2(a6)	; maxx
		move.w	#$7FFF,4(a6)	; miny
		move.w	#$8000,6(a6)	; maxy

		move.w	(a0),.edgescount
		
		move.w	(a0)+,(a1)
		subq.w	#1,(a1)+

.precalcedges:
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
		bge.s	.vecprecompute_minxok		* if x2 < min
		move.w	d6,(a6)             		*	min = x2
.vecprecompute_minxok:

		cmp.w	2(a6),d6
		ble.s	.vecprecompute_maxxok		* if x2 > max
		move.w	d6,2(a6)             		*   max = x2
.vecprecompute_maxxok:

		cmp.w	4(a6),d7	
		bge.s	.vecprecompute_minyok		* if y2 < min
		move.w	d7,4(a6)					*	min = y2
.vecprecompute_minyok:

		cmp.w	6(a6),d7
		ble.s	.vecprecompute_maxyok		* if y2 > max
		move.w	d7,6(a6)					*   max = y2
.vecprecompute_maxyok:

		subq.w	#1,.edgescount
		bne.s	.precalcedges

		move.l	a1,a0

		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+
		move.w	(a6)+,(a1)+

		movem.l (sp)+,d3-d7/a2-a6

		rts

.edgescount:    
        dc.w    0

*-------------------------------------------------------------------------*
*	PRECOMPUTE LINE CLIPPING AND PARAMETERS
*-------------------------------------------------------------------------*
*   IN:
*	  a0......precomputed data
*	  a1......coords s16 x1,y1,x2,y2
*     d0......nbedges adr
*   OUT:
*     a0
*-------------------------------------------------------------------------*
VECclipline:	   
    movem.l d3-d7/a2-a3,-(sp)

    move.l  d0,a3

    move.w  (a1)+,d0
    move.w  (a1)+,d1
    move.w  (a1)+,d2
    move.w  (a1)+,d3

    bsr.s   .cliplineroutine

    movem.l (sp)+,d3-d7/a2-a3

    rts


.cliplineroutine:
	cmp.w	d1,d3			* On ordonne les coordonnees
	bge.s	.line_yordered	* verticales provisoirement y2 > y1
	exg.l	d0,d2			* pour le clip vertical
	exg.l	d1,d3			*
.line_yordered:
	tst.w	d3				* Si l'ordonnee la plus grande
	bge.s	.line_y2positive * est negative (y2 < 0) on trace la droite

	moveq.l	#0,d3			* entre x1 et x2 (passe xor oblige) 
	moveq.l	#0,d1			* On se branche au niveau du clip 
	bra.s	.clipx			* horizontal

.line_y2positive:
	tst.w	d1				* Si l'ordonnee la plus petite
	bge.s	.clipy2			* negative et l'autre positive
							*
							* On calcule l'intersection avec le
							* bord superieur
							* clipy
	move.w	d0,d5			*
	sub.w	d2,d5			* d5 = x1-x2
							*
	move.w	d3,d6			*
	sub.w	d1,d6			* d6 = y2-y1

	muls.w	d1,d5			* (x1-x2) * y1
	divs.w	d6,d5			* (x1-x2) * y1 / (y2-y1)

	add.w	d0,d5			* (x1-x2) * y1 / (y2-y1) + x1
							* d5: abscisse d'intersection
	movem.w	d0-d3/d5/d6,-(sp)
	moveq.l	#0,d1			*
	moveq.l	#0,d3			*
	move.w	d5,d2			*
							*
    addq.w  #1,(a3)         * Inc nb edges
	bsr.s	.clipx	    	* On trace le bout de droite en haut
							* de l'ecran horizontalement
	movem.w	(sp)+,d0-d3/d5/d6 * (passe xor oblige)

	move.w	d5,d0			* On trace le bout de droite 
	moveq.l	#0,d1			* restant sur l'ecran
							* coordonnees non inversees

.clipy2:	
	cmp.w	#ymax,d1		* Si l'ordonnee la plus petite
	bgt.s	.nothing		* est superieure … ymax

	cmp.w	#ymax,d3		* Si l'ordonnee la plus grande>ymax
	ble.s	.clipx			* et l'autre <ymax
					
							* On calcule l'intersection avec le
							* bord superieur
	sub.w	d0,d2			* d2 = x2-x1
	sub.w	d1,d3			* d3 = y2-y1
							*
	move.w	#ymax,d5		*
	sub.w	d1,d5			* d1 = y1-ymax 

	muls.w	d5,d2			* (x2-x1) * (y1-ymax)
	divs.w	d3,d2			* (x2-x1) * (y1-ymax) / (y2-y1)

	add.w	d0,d2			* (x2-x1) * (y1-ymax) / (y2-y1)+x1
							* d2: abscisse d'intersection
	move.w	#ymax,d3		*

	*----------------------------
	* clipx
	*----------------------------

	* clipx1 --------------------

.clipx:
	cmp.w	d2,d0			*
	ble.s	.clipx_ok1		* Si x1 > x2: Echange x1 x2 & y1 y2
	exg.l	d2,d0			*
	exg.l	d3,d1			*
.clipx_ok1:
	tst.w	d2				* if d2 < 0 => do nothing
	bge.s	.clipx_ok2		*

.nothing:	
    *-------------------------------------------------------------------------*
    * STORAGE: nothing 
    *-------------------------------------------------------------------------*
    addq.w  #1,2(a3)        *
	rts					    *

.clipx_ok2:
	move.w	d0,d7			* if x1 < 0 => store to fix length when displaying
	blt.s	.clipx_ok3
	moveq.l	#0,d7
.clipx_ok3:

	* clipx2 --------------------
	cmp.w	#xmax,d0		* Si l'abscisse la plus petite
	bgt.s	.nothing		* est superieure à xmax
							* => on trace pas
	cmp.w	#xmax,d2		* Si l'abscisse la plus grande > xmax
	ble.s	.preparedraw	* 					
							* On calcule l'intersection avec le
							* bord superieur
	sub.w	d0,d2			* d2 = x2-x1
	sub.w	d1,d3			* d3 = y2-y1
							*
	move.w	#xmax,d5		*
	sub.w	d0,d5			* d0 = xmax-x1 

	muls.w	d5,d3			* (y2-y1) * (xmax-x1)
	divs.w	d2,d3			* (y2-y1) * (xmax-x1) / (x2-x1)

	add.w	d1,d3			* (y2-y1) * (xmax-x1) / (x2-x1)+y1
							* d3: ordonnee d'intersection
	move.w	#xmax,d2		*
	
	*----------------------------
	* prepare draw line
	*----------------------------

	* here we have d0,d1,d2,d3,d7 => x1, y1, x2, y2, lengthfix from x1 clip

.preparedraw:	
	lea		-pitch.w,a2 	 *
    move.w  #$8080,d5		 * d5 bit 0 = sign of vertical increment
 	sub.w	d2,d0			 * d0: largeur									
	beq.s	.nothing		 * if dx = 0 => do nothing
	neg.w	d0				 *
	sub.w	d3,d1			 * d1: hauteur
	beq		h_line			 * if dy = 0 => hline							
	blt.s	.prepare_draw_ok		 *
	lea		pitch.w,a2  	 * if dy negative => invert address increment
    moveq.l #0,d5            * 
	neg.w	d1				 * abs (dy)

.prepare_draw_ok:	
	neg.w	d1				*

	lea	pitchmul(pc),a1		* compute start address : a0 += y2 * pitch
	add.w	d3,d3			*
	move.w	(a1,d3.w),d3	* 
	; add.w	d3,d3			* (pitch / 2) * 2
	move.w  d3,(a0)         *                                          ================ STORE VERTICAL OFFSET
                            *                                          ================ will store draw method into bits 1 & 2 of offset (unused by the planes)
	cmp.w	d0,d1			* Compare dx & dy
	bgt  	vertical		* => dy > dx => vertical routine
	beq 	d45				* => equal	 => 45° routine
                            * => dx > dy => horizontal routine

*------------------------------------------------------------------
*	Dx>Dy
*
*   STORAGE :
*      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 0
*      w: enter routine offset
*      w: exit routine offset
*      w: error increment
*------------------------------------------------------------------
horizontal:	

	swap	d1				* dy*65536
	sub.w	d1,d1			*

	divu.w	d0,d1			* d1: increment dy*65536/dx 				

	add.w	d7,d0			* fix length according to x1 clip

	;lea		line_display_horiz(pc),a3	* compute routine adresses range
	;move.l	a3,a4						* 

	add.w	d2,d2						* start address : get values at x2 * 4
	add.w	d2,d2
	move.l	line_display_horiz_tab(pc,d2.w),d7

                            * or.w    #0,(a0)                           ================= LINE TYPE        
    add.w   d7,(a0)+        * set inc pitch sign into high bit

	swap	d7
	sub.b	d5,d5
	or.w    d5,d7
	move.w	d7,(a0)+		* compute offset into routine				================= STORE OPCODE OFFSET (WORD)

	add.w	d0,d0			* end address into routine : x2 * 4 + length * 4
	add.w	d0,d0
	sub.w	d0,d2
	move.w	line_display_horiz_tab(pc,d2.w),(a0)+					*	================== STORE OPCODE OFFSET END D2 (WORD)

    move.w  d1,(a0)+        *                                           ================ STORE INCREMENT (WORD)

    rts

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
*
*   STORAGE:
*      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 2
*      b: rol bit number
*      b: increment error
*      w: vertical offset increment
*      w: offset into routine
*------------------------------------------------------------------
vertical:																
    or.w    #2,(a0)         * ===================== STORE VERTICAL OPERATION SELECTION

	lsl.l	#8,d1
	and.l	#$ffff00,d1

	divu.w	d0,d1			* d0: increment dx*65536/dy 	

	add.w	d7,d0

	add.w	d2,d2
	add.w	d2,d2
	lea	table2(pc),a1
	move.l	(a1,d2.w),d4
	add.w	d4,(a0)+		* ==================== STORE D4 OFFSET

	swap	d4															
	or.b    d5,d4
    move.b  d4,(a0)+        * ==================== STORE D4 ROL BIT

    move.b  d1,(a0)+        * ==================== STORE D1 INCREMENT	(BYTE)
	lsr.w	#8,d1

    move.l  a2,d6
	muls.w	d1,d6														
    move.w  d6,(a0)+        * ==================== STORE D6 ADDRESS VERTICAL INCREMENT LINE

	move.w	d0,d5			* compute jmp distance
	lsl.w	#4,d0
	add.w	d5,d5
	add.w	d5,d0
	neg.w	d0
    move.w  d0,(a0)+        * ===================== STORE OFFSET INTO ROUTINE

    rts

*----------------------------
*   HLINE
*   STORAGE:
*      w: offset address | 4 | ((pitch inc < 0) ? $8000 : 0)
*      b: start mask selection
*      b: end mask selection
*      w: nb words or 0 when same word
*----------------------------
h_line:	
	lea	pitchmul(pc),a1		* 
	add.w	d3,d3			* stored on word expressing pitch / 2		
	move.w	(a1,d3.w),d3	* get the value

	add.w	d7,d0			* fix length according to x1 clip
	move.w	d2,d1			* d2 = x2
	sub.w	d0,d1			* d1 = x1

	moveq.l	#-16,d0		
	and.w	d1,d0
	lsr.w	#1,d0
	
    or.w    #4,d3           * ===================== STORE HLINE OPERATION SELECTION (do not or because nothing writen in a0 before unlike other methods)
    add.w	d0,d3           * d0 = (x1 & FFF0) >> 1						
    move.w  d3,(a0)+	    * ============== STORE OFFSET D3 + D1
    
	moveq.l	#-16,d5
	and.w	d2,d5
	lsr.w	#1,d5			* d5 = (x2 & FFF0) >> 1

	moveq.l	#15,d3
	and.w	d1,d3			* d3 = x1 & 15
	add.w	d3,d3
    move.b  d3,(a0)+
	moveq.l	#15,d6
	and.w	d2,d6			* d6 = x2 & 15
	add.w	d6,d6
    move.b  d6,(a0)+

	sub.w	d0,d5

	bne.s	.h_line_long	* ================ STORE HLINE LONG OR NOT
    move.w  d5,(a0)+        * Store 0 (d5 = 0)

	rts

.h_line_long:

	lsr.w	#3,d5
	addq.w	#1,d5

    move.w  d5,(a0)+        * != 0 => Store nb words

	rts

*------------------------------------------------------------------
*	Dx=Dy
*
*   STORAGE:
*       w: offset address | 6 | ((pitch inc < 0) ? $8000 : 0)
*       b: bit num
*       b: length
*       w: routine offset
*------------------------------------------------------------------
d45:
    or.w    #6,(a0)         * =================== draw method enum
	add.w	d2,d2
	add.w	d2,d2
	lea	table2(pc),a1
	move.l	(a1,d2.w),d4
    add.w   d4,(a0)+        * =================== STORE OFFSET D4

	swap	d4
	or.b	d5,d4	
    move.b  d4,(a0)+        * =================== STORE BIT

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
    move.b  d1,(a0)+        * =================== STORE LENGTH
    move.w  d0,(a0)+        * =================== STORE ROUTINE OFFSET

    rts

*----------------------------------------------------
*	ERASE
*----------------------------------------------------
*   a0: screen
*   a1: coords
*----------------------------------------------------
VECclr:
	movem.l d3-d5/a2-a3,-(sp)

	move.w	6(a1),d4	; maxy
	cmp.w   #ymax,d4
	ble.s	.vecloop_maxyok
	move.w	#ymax,d4
.vecloop_maxyok:
	
	move.w	4(a1),d3	; miny
	bge.s	.vecloop_minyok
	moveq.l	#0,d3
.vecloop_minyok:

	move.w	d4,d5
	sub.w	d3,d5
    addq.w  #1,d5

	lea		pitchmul(pc),a3
	add.w	d3,d3
	move.w	(a3,d3.w),d3
	lea		(a0,d3.w),a3

	lea	$ffff8a20.w,a2

	move.w	2(a1),d3			; word count x
	add.w	(a1),a3	    		; adr offset x

	move.w	#1+pitch/8,d4		
	sub.w	d3,d4
	add.w	d4,d4
	add.w	d4,d4
	add.w	d4,d4

    clr.l   (a2)+               ; inc x source / inc y source

	move.l	a3,(a2)+			; adr source

	addq.l	#8,a2

	move.w	d4,(a2)+			; inc y dest

	move.l	a3,(a2)+			; adr dest 

	move.w	d3,(a2)+			; x count
	move.w	d5,(a2)+			; y count
	move.w	#$C000,2(a2)

	movem.l (sp)+,d3-d5/a2-a3    
    
    rts

VECclrpass:

	lea		$ffff8A28.w,a0
	
	moveq.l #-1,d0

	move.l	d0,(a0)+
	move.w	d0,(a0)+

	move.w  #8,(a0)+			; inc x dest
	
	move.l	#$C000,$ffff8A3A.w

	rts

*----------------------------------------------------
*	DRAW LINES
*----------------------------------------------------
*   a0: screen
*   a1: display list
*   d0: polycount
*----------------------------------------------------
VECloop:
	movem.l d3-d7/a2-a5,-(sp)

	move.w d0,drawpolycount

	move.w	#6,$ffff8a3a.w						* HOP = 0 (bits 1 instead of source) and xor mode

    ;------------------------------------------------
    ;	Run display list
    ;------------------------------------------------
	move.l	a0,-(sp)

poly_loop:

	move.w	(a1)+,drawedgescount
	blt.s	noedges

edges_loop:	

	move.l	(sp),a0

    ; drawline
    move.w  (a1)+,d0
    move.w  d0,d1
    and.l   #$FFF9,d1
    add.l   d1,a0

    lea     pitch.w,a2
    tst.b   (a1)
	bge.s	.pitchpositive
    lea     -pitch.w,a2
.pitchpositive:

	add.w   d0,d0
    and.w   #$C,d0
    move.l	.switchdraw(pc,d0.w),a3
	jmp		(a3)
.switchdraw:
	dc.l	pixhorizontal
    dc.l	pixvert
    dc.l    pixhline
    dc.l    pixd45

noedges:
	subq.w	#1,drawpolycount
	bne.s	poly_loop

enddraw:
    addq.l  #4,sp
	move.l	a1,a0

	movem.l (sp)+,d3-d7/a2-a5

	rts		* return to caller
    
*----------------------------------------------------
*	XOR BLITTER PASS
*----------------------------------------------------

*----------------------------------------------------    
*   d0: HOG / OP mask
*----------------------------------------------------    
VECxorpass:

	lea		$ffff8A28.w,a0
	
	move.w	d0,$FFFF8A3A.w   	; xor / source = $206

	moveq.l #-1,d0

	move.l	d0,(a0)+									
	move.w	d0,(a0)+									

	move.w  #8,(a0)+			; inc x dest			

	rts


*----------------------------------------------------    
*   a0: screen
*   a1: coords
*----------------------------------------------------    
VECxor:
	movem.l d3-d5/a2-a3,-(sp)

	move.w	6(a1),d4	; maxy
	cmp.w   #ymax,d4
	ble.s	.vecloop_maxyok
	move.w	#ymax,d4
.vecloop_maxyok:
	
	move.w	4(a1),d3	; miny
	bge.s	.vecloop_minyok
	moveq.l	#0,d3
.vecloop_minyok:

	move.w	d4,d5
	sub.w	d3,d5

	lea		pitchmul(pc),a3
	add.w	d3,d3
	move.w	(a3,d3.w),d3
	lea		(a0,d3.w),a3

	lea	$ffff8a20.w,a2

	move.w	2(a1),d3			; word count x
	add.w	(a1),a3	    		; adr offset x

	move.w	#1+pitch/8,d4		
	sub.w	d3,d4
	add.w	d4,d4
	add.w	d4,d4
	add.w	d4,d4

	move.w	#8,(a2)+			; inc x source			*
	move.w	d4,(a2)+			; inc y source

	move.l	a3,(a2)+			; adr source

	addq.l  #8,a2				; endmasks + dest offset

	move.w	d4,(a2)+			; inc y dest

	lea		pitch(a3),a3
	move.l	a3,(a2)+			; adr dest 

	move.w	d3,(a2)+			; x count
	move.w	d5,(a2)+			; y count
	
	move.w	#$C000,2(a2)		; start

	movem.l (sp)+,d3-d5/a2-a3

	rts

*------------------------------------------------------------------
*	Dx>Dy
*
*   STORAGE :
*      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 0
*      w: enter routine offset
*      w: exit routine offset
*      w: error increment
*------------------------------------------------------------------
pixhorizontal:

	lea		line_display_horiz(pc),a3	* compute routine adresses range
	move.l	a3,a4						* 
    
	move.w	(a1)+,d0
	and.w	#$7FFF,d0
	add.w	d0,a3			* add x2 address offset to a0
	add.w	(a1)+,a4		* end address into routine : x2 * 4 + length * 4
    move.w  (a1)+,d1        * increment
    
	movem.w	preload(pc),d0/d2-d7										================== INIT => draw pass will do this

	move.w (a4),a5			* backup opcode
	move.w #$4E75,(a4)		* put an rts into the routine
	jsr	(a3)
	move.w	a5,(a4)			* restore overwritten opcode

	subq.w	#1,drawedgescount
	bge 	edges_loop
	subq.w	#1,drawpolycount
	bne 	poly_loop
    bra     enddraw
    	
preload:
	dc.w	1			* d0
	dc.w	4			* d2
	dc.w	8			* d3
	dc.w	32			* d4
	dc.w	-32768		* d5
	dc.w	64			* d6
	dc.w	128			* d7

*------------------------------------------------------------------
*	Dx<Dy
*
*   STORAGE:
*      w: offset address | ((pitch inc < 0) ? $8000 : 0) | 2
*      b: rol bit number
*      b: increment error
*      w: vertical offset increment
*      w: offset into routine
*------------------------------------------------------------------
pixvert:																
    move.b  (a1)+,d0        * SET ROL BIT
    moveq.l #0,d4
    bset.l  d0,d4
    move.b  (a1)+,d1        * ERROR INCREMENT
    move.w  (a1)+,d6        * ADDRESS VERTICAL INCREMENT LINE
    ext.l   d6              *
    move.w  (a1)+,d0        * OFFSET INTO ROUTINE
	move.w	#-32768,d5		* Accumulateur … 0							===================== INIT DRAW
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
	add.l	a2,a0			*
	endr

vertical_display:

	subq.w	#1,drawedgescount
	bge 	edges_loop
	subq.w	#1,drawpolycount
	bne 	poly_loop
    bra     enddraw

*----------------------------
*   HLINE
*   STORAGE:
*      w: offset address | 4 | ((pitch inc < 0) ? $8000 : 0)
*      b: start mask selection
*      b: end mask selection
*      w: nb words or 0 when same word
*----------------------------
pixhline:	

    moveq.l #0,d3
    move.b  (a1)+,d3
    moveq.l #0,d6
    move.b  (a1)+,d6

    move.w  (a1)+,d5
	bne.s	.h_line_long																			================ STORE HLINE LONG OR NOT

	move.w	.mask_start(pc,d3.w),d1				* masque final 1 - 2 - 3 => il faut les setter		================ STORE D3
	and.w	.mask_end(pc,d6.w),d1																	================ STORE D6
	move.w	d1,$ffff8a28.w
	move.l	a0,$ffff8a32.w						* dest adr											================ DRAW INIT A0 with STORED offset
	move.l	#$10001,$ffff8a36.w					* nb word to transfer horizontal + vertical			================ STORE WIDTH = 1
	move.w	#$C000,$ffff8a3c.w					* go exclusive

	subq.w	#1,drawedgescount
	bge	    edges_loop
	subq.w	#1,drawpolycount
	bne 	poly_loop
    bra     enddraw

.mask_start:
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
    
.mask_end:
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

.h_line_long:

	move.w	.mask_start(pc,d3.w),$ffff8a28.w	* masque final 1 - 2 - 3 => il faut les setter		================ STORE D3
	move.w	.mask_end(pc,d6.w),$ffff8a2c.w															================ STORE D6
	move.l	a0,$ffff8a32.w						* dest adr
	move.w	d5,$ffff8a36.w						* nb word to transfer horizontal				 	================ STORE WIDTH = D5
	move.w	#1,$ffff8a38.w						* nb lines
	move.w	#$C000,$ffff8a3c.w					* go exclusive

	subq.w	#1,drawedgescount
	bge 	edges_loop
	subq.w	#1,drawpolycount
	bne 	poly_loop
    bra     enddraw

*------------------------------------------------------------------
*	Dx=Dy
*
*   STORAGE:
*       w: offset address | 6 | ((pitch inc < 0) ? $8000 : 0)
*       b: bit num
*       b: length
*       w: routine offset
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

	subq.w	#1,drawedgescount
	bge 	edges_loop
	subq.w	#1,drawpolycount
	bne 	poly_loop
    bra     enddraw

pixd45:
    moveq.l #0,d4
    move.b  (a1)+,d5
    bset.l  d5,d4           * ROL BIT
    moveq.l #0,d1
    move.b  (a1)+,d1        * =================== LENGTH
    move.w  (a1)+,d0        * =================== ROUTINE OFFSET

	jmp	saut3(pc,d0.w)		

*------------------------------------------------------------------------------------------
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
drawpolycount		dc.w	0
drawedgescount		dc.w	0
vecprecompute_minx:	dc.w	0
vecprecompute_maxx:	dc.w	0
vecprecompute_miny:	dc.w	0
vecprecompute_maxy:	dc.w	0

pitchmul:
var:	set	0
		rept	ymax+2
		dc.w	var
var:	set		var+pitch
		endr	

table2:	
var:	set	0
		rept	pitch/8
		dc.w	$F,var,$E,var,$D,var,$C,var
		dc.w	$B,var,$A,var,$9,var,$8,var
		dc.w    $7,var,$6,var,$5,var,$4,var
		dc.w	$3,var,$2,var,$1,var,$0,var
var:	set	var+8
		endr
