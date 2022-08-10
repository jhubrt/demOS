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

	output	D:\PROJECTS\DEMOS\OUTPUT\FX\STARFILD.O

    xdef STARasminit,STARdraw,STARerase
    xdef STARloop,STARreset
    xdef STARpix0,STARpix1,STARpixloop,STARinc,STARpixdraw

;-------------------------------------------------------------------
;   void STARasminit (STARparam* _param);
;-------------------------------------------------------------------
STARasminit:

	move.l  (a0)+,STARprex
    move.l  (a0)+,STARprey
    move.l  (a0),STARrndx
    move.l  (a0)+,STARrndxcurrent
    move.l  (a0),STARrndy
    move.l  (a0)+,STARrndycurrent

	rts

;-------------------------------------------------------------------
;   void STARdraw(void* framebuffer, void* z, u16 nbstars, u32 erasebuffer)	
;
;   a0: framebuffer
;   a1: z[star] table
;   d0: nbstars
;   d1: erase buffer
;-------------------------------------------------------------------
STARdraw:

    movem.l a2-a6,-(sp)
    movem.w d2-d7,-(sp)

    movem.w preload(pc),d2-d7

    move.l  d1,a3
    move.l  STARrndxcurrent(pc),a4
    move.l  STARrndycurrent(pc),a5
    lea     STARpixdraw(pc),a6

    bsr     STARloop

    move.l  a4,STARrndxcurrent
    move.l  a5,STARrndycurrent

    movem.w (sp)+,d2-d7
    movem.l (sp)+,a2-a6

    rts

STARprex:	        dc.l	0
STARprey:	        dc.l	0
STARrndx:	        dc.l	0
STARrndy:	        dc.l	0
STARrndxcurrent:    dc.l    0
STARrndycurrent:    dc.l    0

preload:
    dc.w    1
    dc.w    4
    dc.w    8
    dc.w    32
    dc.w    64
    dc.w    128

STARloop:
	move.l	(a1),a2			; a1 = z[star].x		  12
	addq.l	#2,(a1)+		; z++					  16
	move.w	(a2),d1			; stardraw offset		  8

	move.l	(a1),a2			; a1 = z[star].y		  12
	addq.l	#2,(a1)+		; z++					  16
	move.w	(a2),a2			; a3 = &prey[star][z]	  8

	add.l	a0,a2		    ; a0 = framebuffer => a2  8
    
	jmp		(a6,d1.w)       ; a6 = stardraw table	  16
	
							; 96

STARreset:
    move.l  a2,(a3)+

    move.l  (a4)+,d1	  ; a4 : rndx
	bge.s	.ok1
	move.l	STARrndx(pc),a4
    moveq.l #0,d1
.ok1:
	add.l	STARprex(pc),d1
	move.l	d1,-8(a1)

    move.l  (a5)+,d1	  ; a5 : rndy
	bge.s	.ok2
	move.l  STARrndy(pc),a5
    moveq.l #0,d1
.ok2:
	add.l	STARprey(pc),d1
	move.l	d1,-4(a1)

    dbra.w  d0,STARloop
	rts

STARpixdraw:
    dcb.w    16384,0

;-------------------------------------------------------------------
; templates
; or.b    d2,1(a2)
; or.b    d2,3(a2)
; dbra.w  d0,.loop
; rts
;-------------------------------------------------------------------
	
STARpix0:
	or.b	d7,(a2)	    ; 128
	or.b	d6,(a2)	    ; 64
	or.b	d5,(a2)	    ; 32
	bset.b	d3,(a2)	    ; 16
	or.b	d4,(a2)	    ; 8
	or.b	d3,(a2)	    ; 4
	bset.b	d2,(a2)	    ; 2
	or.b	d2,(a2)	    ; 1

STARpix1:
	or.b	d7,1(a2)	; 128
	or.b	d6,1(a2)	; 64
	or.b	d5,1(a2)	; 32
	bset.b	d3,1(a2)	; 16
	or.b	d4,1(a2)	; 8
	or.b	d3,1(a2)	; 4
	bset.b	d2,1(a2)	; 2
	or.b	d2,1(a2)	; 1

STARpixloop:
    dbra.w  d0,*+2
    rts

STARinc:
    lea     1(a2),a2
    move.l  a2,(a3)+

;-------------------------------------------------------------------
;   void STARerase (void* _framebuffer, u16 _nbstars / 6 - 1);
;-------------------------------------------------------------------
STARerase:
    moveq.l #0,d1

	movem.l	a2-a6,-(sp)
	
.loop
    movem.l (a0)+,a1-a6
    movep.l d1,(a1)
    movep.l d1,(a2)
    movep.l d1,(a3)
    movep.l d1,(a4)
    movep.l d1,(a5)
    movep.l d1,(a6)

    dbra    d0,.loop

	movem.l	(sp)+,a2-a6

    rts


;--------------------------------------------------------------------
; other posibilities...

;	move.w	(a1),d6			; z++					16
;	addq.w	#2,(a1)+								12

;	move.l	(a1)+,a6		; a1 = z[star]			12
;	add.w	(a6,d6.w),a2	; a6 = &prex[starx][z]	20

;	move.l	(a1)+,a6		; a1 = z[star]			12
;	move.w	(a6,d6.w),d6	; stardraw offset		16
;	jmp		(a5,d6.w)       ; a5 = stardraw table	16
	
							;104
	

 ;  move.w  (a1),d6         8	; a1 = z[star] * 2 => d6
 ;  add.w	d6,d6			4
;    addq.w	#2,(a1)+		12

;	add.w	(a3,d6.w),a2	20	; a3 = prey table

;   move.w  (a1),d6         8		; a1 = z[star] * 2 => d6
;	add.w	d6,d6			4
;    addq.w	#2,(a1)+		12
	
;	move.w	(a4,d6.w),d6    16	; a4 = prey table
;	jmp		(a5,d6.w)       16	; a5 = stardraw table
	
							;100

