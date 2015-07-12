*------------------------------------------------------------------------------  -----------------
*  Copyright J.Hubert 2015
*
*  This file is part of rebirth demo
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

	output  D:\PROJECTS\DEMOS\OUTPUT\SLIDES.O

	opt	o+

    xdef SLIinit,SLItc,SLItunX1,SLItunX2,SLItcBuf
    xdef SLIfadetogrey,SLIconvertToGrey
    xdef SLIinitMorph,SLIstartMorph,SLImorphFunc,SLIcodepattern,SLIendcodepattern,SLIdisplayMorph

; -----------------------------------------------
;  void SLIinit(void* _tcbuf, u16 _TCheight);
; -----------------------------------------------
SLIinit:

	add.w	d0,d0
	add.w	d0,d0						; 64 colors per scanlines

	move.w	d0,SLIh                     ; 4 pixels hight
	move.l	a0,SLItcBuf

	move.l	#-1,$ffff8a28.w				; Mask 1 - 2
	move.w	#-1,$ffff8a2c.w				; Mask 3
	move.l	#$0002ffe2,$ffff8a2e.w		; Inc X & Y dest	(0 / -30)
	move.w	#16,$ffff8a36.w				; Width				(16 colors)
	move.l	#$20002,$ffff8a20.w			; Inc X & Y source
	move.w	#$203,$ffff8a3a.w			; op

	rts

; -----------------------------------------
;  SLItc display interruption
; -----------------------------------------
SLItc:
    move.w  #$2700,sr		; cut all interrupts

	; move.w	#$70,$ffff8240.w  ; for test

	move.l	a6,-(sp)
	move.w	d0,-(sp)
	move.w	d1,-(sp)

    lea		$ffff8209.w,a6

    ; synchro lsl to be perfectly synchronized with display

    move.b  (a6),d0
SLItc_lslSync:		
	move.b	(a6),d1
    cmp.b   d0,d1
	beq.s	SLItc_lslSync

    sub.b	d1,d0
	add.w	#31,d0
	lsl.w	d0,d1

    clr.b	$fffffa1b.w		; stop timer b

    ; re-param blitter 

    move.l	SLItcBuf(pc),$ffff8a24.w    , Adr source
    move.l	#$ffff8240,$ffff8a32.w		; Adr dest
    move.w	SLIh(pc),$ffff8a38.w	    ; Height

    move.w  SLItunX1(pc),d0

SLItc_tunLoop:
	dbra.w	d0,SLItc_tunLoop
    
	move.w	SLItunX2(pc),d0
	lsl.w	d0,d1
   
	move.w	#$C000,$ffff8a3c.w

	move.w	(sp)+,d1
	move.w	(sp)+,d0
	move.l	(sp)+,a6

	; move.w	#$7,$ffff8240.w

	rte


; -----------------------------------------
; void SLIfadetogrey (u16* _src, u16* _dest, u32 _converter, u16 _h)
; -----------------------------------------
SLIfadetogrey:

	movem.w	d2-d4,-(sp) ; split movems to avoid color glitch with HC colors pictures
	movem.w	d5-d7,-(sp)

	move.l	d0,a2		; a2   = convert table
	move.w	d1,-(sp)		; (sp) = h counter

SLIfadetogrey_y:

	addq.l	#2,a1

	rept 5

	movem.w	(a0)+,d0-d3
	movem.w	(a0)+,d4-d7

	add.w	d0,d0
	add.w	d1,d1
	add.w	d2,d2
	add.w	d3,d3
	add.w	d4,d4
	add.w	d5,d5
	add.w	d6,d6
	add.w	d7,d7

	move.w	(a2,d0.w),(a1)+
	move.w	(a2,d1.w),(a1)+
	move.w	(a2,d2.w),(a1)+
	move.w	(a2,d3.w),(a1)+
	move.w	(a2,d4.w),(a1)+
	move.w	(a2,d5.w),(a1)+
	move.w	(a2,d6.w),(a1)+
	move.w	(a2,d7.w),(a1)+

	endr

	movem.w	(a0)+,d0-d3
	movem.w	(a0)+,d4-d6

	add.w	d0,d0
	add.w	d1,d1
	add.w	d2,d2
	add.w	d3,d3
	add.w	d4,d4
	add.w	d5,d5
	add.w	d6,d6

	move.w	(a2,d0.w),(a1)+
	move.w	(a2,d1.w),(a1)+
	move.w	(a2,d2.w),(a1)+
	move.w	(a2,d3.w),(a1)+
	move.w	(a2,d4.w),(a1)+
	move.w	(a2,d5.w),(a1)+
	move.w	(a2,d6.w),(a1)+

	addq.l	#2,a1

	movem.w	(a0)+,d0-d3
	movem.w	(a0)+,d4-d7

	add.w	d0,d0
	add.w	d1,d1
	add.w	d2,d2
	add.w	d3,d3
	add.w	d4,d4
	add.w	d5,d5
	add.w	d6,d6
	add.w	d7,d7

	move.w	(a2,d0.w),(a1)+
	move.w	(a2,d1.w),(a1)+
	move.w	(a2,d2.w),(a1)+
	move.w	(a2,d3.w),(a1)+
	move.w	(a2,d4.w),(a1)+
	move.w	(a2,d5.w),(a1)+
	move.w	(a2,d6.w),(a1)+
	move.w	(a2,d7.w),(a1)+

	move.w	(a0)+,d0
	add.w	d0,d0
	move.w	(a2,d0.w),(a1)+

	lea		12(a1),a1

	subq.w	#1,(sp)
	bne		SLIfadetogrey_y
	
	addq.l	#2,sp

    movem.w	(sp)+,d5-d7
	movem.w	(sp)+,d2-d4

	rts

; -----------------------------------------
;  SLIfadetogrey (u16* _src, u16* _dest, u32 _rotate4bTable, u32 _greyscale, u32 _multable)
; -----------------------------------------

; real time computation is not fast enought => let's go for big convertion table 

;SLIfadetogrey:

;        movem.l a2-a5/d3-d7,-(sp)

;        move.l  d0,a2       ; rotate left 4b table
;        move.l  d1,a3       ; greyscale table
;        move.l  d2,a4       ; multable
;        lea     16(a2),a5   ; rotate right 4b table

;        moveq.l #0,d1       ; preload 0 for optimization

;        move.w  #198,-(sp)  ; h

;SLIfadetogrey_y:            ; for (y = 0 ; y < _h ; y++)

;        move.w   d1,(a1)+   ;        *_dest++ = 0;

;        moveq.l  #46,d0
;        bsr.s    SLIfadetogrey_x1

;        move.w   d1,(a1)+   ;        *_dest++ = 0;

;        moveq.l  #8,d0
;        bsr.s    SLIfadetogrey_x1

;        move.l  d1,(a1)+   ;        *_dest++ = 0; * 6
;        move.l  d1,(a1)+
;        move.l  d1,(a1)+

;        subq.w  #1,(sp)
;        bne.s   SLIfadetogrey_y

;        addq.w  #2,sp

;        movem.l (sp)+,a2-a5/d3-d7

;        rts



;SLIfadetogrey_x1:       ; for (x = 1 ; x < 58 ; x++)

;       move.w  #$FFF,d4
;        and.w  (a0)+,d4     ; u16 c = *_src++;

;        move.w  #$F00,d5
;        and.w   d4,d5
;        lsr.w   #8,d5   ; r

;        move.w  #$F0,d6
;        and.w   d4,d6
;        lsr.w   #4,d6   ; g

;        moveq.l #$F,d7
;        and.w   d4,d7   ; b
        
        ; move.w  #$F0,d6
        ; and.w   d4,d6
        ; lsr.w   #4,d6   ; g

;        move.b  (a2,d5.w),d5    ; rotate 4b r, g & b
;        move.b  (a2,d6.w),d6    ;
;        move.b  (a2,d7.w),d7    ;

        ; s16 grey = g_screens.slides->greyscaler[c];
        
;        move.b  (a3,d4.w),d4    ; d4 = grey
             
;        moveq.l #0,d2
        
;        move.b  d4,d2           ; u16 r = mul[grey - sr] + sr;
;        sub.b   d5,d2
;        ext.w   d2
;        move.b  (a4,d2.w),d2
;        add.b   d5,d2           ; r
;        ext.w   d2
;        move.b  (a5,d2.w),d2    ; rotate 4b r

;        lsl.w   #4,d2

;        move.b  d4,d3           ; u16 g = mul[grey - sg] + sg;
;        sub.b   d6,d3
;        ext.w   d3
;        move.b  (a4,d3.w),d3
;        add.b   d6,d3           ; g
;        ext.w   d3
;        move.b  (a5,d3.w),d3    ; rotate 4b g
        
;        or.b    d3,d2
;        lsl.w   #4,d2

;        move.b  d4,d3           ; u16 b = mul[grey - sb] + sb;
;       sub.b   d7,d3
;        ext.w   d3
;        move.b  (a4,d3.w),d3
;        add.b   d7,d3           ; b
;        ext.w   d3
;        move.b  (a5,d3.w),d3    ; rotate 4b b

;        or.b    d3,d2

        ; r = COL4b2ST[r];
        ; g = COL4b2ST[g];
        ; b = COL4b2ST[b];

        ; USE only one table instead of 3 ror 4b

;        move.w  d2,(a1)+
        
;        dbra.w  d0,SLIfadetogrey_x1

;        rts




; -----------------------------------------
; void SLIconvertToGrey (u16* _srcbit, u16* _srcpal, u32 _dest, u16 _h)
; -----------------------------------------
SLIconvertToGrey:

		movem.l	d2-d7/a2-a3,-(sp)

		lea		SLIpal(pc),a3

		move.l	d0,a2
		move.w	d1,-(sp)
    
SLIconvertToGrey_y:

        ; STDmcpy(pal, _srcpal, 16 * sizeof(u16));
        ; _srcpal += 16;

		movem.l	(a1)+,d0-d3		; use two transfers instead of one to avoid inteferring with timerb interuption stabilisation
		movem.l	d0-d3,(a3)
		movem.l	(a1)+,d0-d3
		movem.l	d0-d3,16(a3)

		moveq.l	#0,d6

		moveq.l	#20,d5

SLIconvertToGrey_x:		;for (x = 0 ; x < 336 ; x += 16)

        ; srcbit[0] = *_srcbit++;
        ; srcbit[1] = *_srcbit++;
        ; srcbit[2] = *_srcbit++;
        ; srcbit[3] = *_srcbit++;

		movem.w	(a0)+,d0-d3

		rept	4

		moveq.l	#0,d7

		add.w	d3,d3
		addx.w	d7,d7
		add.w	d2,d2
		addx.w	d7,d7
		add.w	d1,d1			
		addx.w	d7,d7
		add.w	d0,d0
		addx.w	d7,d7

		add.w	d7,d7
		move.w	(a3,d7.w),d7

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		add.b	d7,d7
		addx.w	d0,d0
		add.b	d7,d7
		addx.w	d3,d3
		add.b	d7,d7
		addx.w	d2,d2
		add.b	d7,d7
		addx.w	d1,d1

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		endr

		move.w	(a1)+,(a3,d6.w)
		addq.w	#2,d6
		and.w	#31,d6

		rept	8

		moveq.l	#0,d7

		add.w	d3,d3
		addx.w	d7,d7
		add.w	d2,d2
		addx.w	d7,d7
		add.w	d1,d1			
		addx.w	d7,d7
		add.w	d0,d0
		addx.w	d7,d7

		add.w	d7,d7
		move.w	(a3,d7.w),d7

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		add.b	d7,d7
		addx.w	d0,d0
		add.b	d7,d7
		addx.w	d3,d3
		add.b	d7,d7
		addx.w	d2,d2
		add.b	d7,d7
		addx.w	d1,d1

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		endr

		move.w	(a1)+,(a3,d6.w)
		addq.w	#2,d6
		and.w	#31,d6

		rept	4

		moveq.l	#0,d7

		add.w	d3,d3
		addx.w	d7,d7
		add.w	d2,d2
		addx.w	d7,d7
		add.w	d1,d1			
		addx.w	d7,d7
		add.w	d0,d0
		addx.w	d7,d7

		add.w	d7,d7
		move.w	(a3,d7.w),d7

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		add.b	d7,d7
		addx.w	d0,d0
		add.b	d7,d7
		addx.w	d3,d3
		add.b	d7,d7
		addx.w	d2,d2
		add.b	d7,d7
		addx.w	d1,d1

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		endr

		swap	d0
		swap	d1
		swap	d2
		swap	d3

		movem.w	d0-d3,(a2)			
		addq.l	#8,a2	

		dbra.w	d5,SLIconvertToGrey_x

		lea		12(a1),a1

		subq.w	#1,(sp)
		bne		SLIconvertToGrey_y

		addq.l	#2,sp

    	movem.l	(sp)+,d2-d7/a2-a3

		rts

		
;-----------------------------------------------------------------------------------------------
;   void SLIinitMorph (void* _verttable, void* _horitable, u16 _h, u16 _pitch, u16 _w)
;-----------------------------------------------------------------------------------------------

SLIinitMorph:

        move.l  d3,-(sp)

		move.w	d0,SLImorphH
		move.w	d2,d3
		lsr.w	#4,d3
		move.w	d3,SLImorphWW

		;--------------
	    moveq.l	#0,d3

		subq.w	#1,d0

SLIinitMorph_loopy:
	    move.w	d3,(a0)+
	    add.w	d1,d3
	    dbf.w	d0,SLIinitMorph_loopy

		;--------------
    	subq.w	#1,d2

	    move.w	#128,d1
	    moveq.l	#0,d0

SLIinitMorph_loopx:
	    move.w	d1,(a1)+
	    move.w	d0,(a1)+
	    
        ror.b	#1,d1
	    bcc.s	SLIinitMorph_oddadr
	    addq.w	#1,d0

	    btst.l	#0,d0
	    bne.s	SLIinitMorph_oddadr
	    addq.w	#6,d0

SLIinitMorph_oddadr:

	    dbf.w	d2,SLIinitMorph_loopx

        move.l  (sp)+,d3

        rts


;-----------------------------------------------------------------------------------------------
;   void SLIstartMorph (void* _startPos, void* _endPos, u32 _morphCode, u32 _pos, u16 _nbpoints)
;-----------------------------------------------------------------------------------------------

SLIstartMorph:	

    move.l  a2,-(sp)
    move.l  a3,-(sp)

	move.l  d0,a2
	move.l  d1,a3

    subq.w  #1,d2

SLIstartMorph_loop:	

	move.w	(a1)+,d0			* X dest
	move.w	(a0)+,d1			* X start
	sub.w	d1,d0				* X dest - X start
    
    ext.l   d0                  ; replace muls * morphlen (set to 64)
    lsl.l   #8,d0
    lsl.l   #4,d0

	;muls.w	#65536/morphlength,d0
	;lsl.l	#2,d0

	move.l	d0,slicodepattern_automo1+2-SLIcodepattern(a2)	    * coef
	lsl.w	#2,d1
	addq.w	#2,d1
	swap	d1
*	move.w	#32768,d1
	sub.w	d1,d1	
	move.l	d1,2(a3)

	move.w	(a1)+,d0			* Y dest
	move.w	(a0)+,d1			* Y start
	sub.w	d1,d0				* Y dest - Y start

    add.w   d0,d0               ; replace muls * morphlen (set to 64)
    add.w   d0,d0

	; muls.w	#65536/morphlength,d0
	; asr.l	#8,d0

	move.w	d0,slicodepattern_automo2+2-SLIcodepattern(a2)	    * coef
	asl.w	#8,d1
	move.b	#128,d1
	move.w	d1,(a3)

	addq.l	#6,a3

	lea	SLIendcodepattern-SLIcodepattern(a2),a2
	
	dbra.w	d2,SLIstartMorph_loop

    move.l  (sp)+,a3
    move.l  (sp)+,a2

	rts


;-------------------------------------------------------------------------
; void SLImorphFunc(void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable)
;-------------------------------------------------------------------------

SLImorphFunc:

    movem.l a2-a3/a6,-(sp)

	lea	$ffff8a20.w,a6		
	clr.l	(a6)+		    ; increment X / Y source	
	move.l	a0,(a6)+		; source address
 	move.l	#-1,(a6)+		; 
	move.w	#-1,(a6)+		; masks
	move.l	#$80008,(a6)+	; inc X / Y
	move.l	a0,(a6)+        ; dest address
	move.w	SLImorphWW(pc),(a6)+
	move.w	SLImorphH(pc),(a6)+
	move.l	#%11000000*256,(a6)	

    move.l  d0,a2           ; transfer to the right register
    move.l  d1,a3
    move.l  d2,a6
    moveq.l #0,d1
	moveq.l	#-4,d2

    jsr     (a6)

    movem.l (sp)+,a2-a3/a6

    rts

;-------------------------------------------------------------------------
; void SLIdisplayMorph(void* _screenbase, void* _pos, u32 _voffsettable, u32 _htable, u16 _nbPoints)
;-------------------------------------------------------------------------
SLIdisplayMorph:

    movem.l a2-a3/a6/d7,-(sp)

	lea	$ffff8a20.w,a6		
	clr.l	(a6)+			
	move.l	a0,(a6)+		
 	move.l	#-1,(a6)+		; 
	move.w	#-1,(a6)+		; masks
	move.l	#$80008,(a6)+	; inc X / Y
	move.l	a0,(a6)+
	move.w	SLImorphWW(pc),(a6)+
	move.w	SLImorphH(pc),(a6)+
	move.l	#%11000000*256,(a6)	

    move.l  d0,a2           ; transfer to the right register
    move.l  d1,a3
    moveq.l #0,d1
	moveq.l	#-4,d7
    subq.w  #1,d2

.SLIdisplayMorph_loop:
	move.l	a0,a6				
	addq.l	#2,a1		        * Y 
	move.b	-2(a1),d1
	add.w	d1,d1				* Do not need to clear high byte of word  it is cleared by pixel value (rolling on 8 bits) 
	add.w	(a2,d1.w),a6		* vertical adr offset

	addq.l	#4,a1       		* X
	move.w	-4(a1),d0
	and.w	d7,d0				* table stores .l (preshifted on error coef => just mask)	
	move.l	(a3,d0.w),d1
	add.w	d1,a6
	swap	d1
	or.b	d1,(a6)				* display pixel

    dbf.w   d2,.SLIdisplayMorph_loop

    movem.l (sp)+,a2-a3/a6/d7

    rts

;-------------------------------------------------------------------------
; code pattern used for autogeneration
;-------------------------------------------------------------------------

SLIcodepattern:	
	move.l	a0,a6				
slicodepattern_automo2:
	add.w	#$abcd,(a1)+		* Y
	move.b	-2(a1),d1
	add.w	d1,d1				* Do not need to clear high byte of word  it is cleared by pixel value (rolling on 8 bits) 
	add.w	(a2,d1.w),a6		* vertical adr offset

slicodepattern_automo1:
	add.l	#$abcd,(a1)+		* X
	move.w	-4(a1),d0
	and.w	d2,d0				* table stores .l (preshifted on error coef => just mask)	
	move.l	(a3,d0.w),d1
	add.w	d1,a6
	swap	d1
	or.b	d1,(a6)				* display pixel
SLIendcodepattern:

; -----------------------------------------

SLImorphH:	dc.w	0		;
SLImorphWW:	dc.w	0

SLIh:		dc.w	0		; d0
SLItunX1:	dc.w	6       ; d6
SLItunX2:	dc.w	2

SLItcBuf:	dc.l	0

SLIpal:		dc.l	0
			dc.l	0
			dc.l	0
			dc.l	0
			dc.l	0
			dc.l	0
			dc.l	0
			dc.l	0
