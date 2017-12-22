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

	output  D:\PROJECTS\DEMOS\OUTPUT\VISUALIZ.O

	opt	o+

    xdef VISinit,VISfade,VISfade3,VIScopy,VISfilsampl
	xdef VIStc,VIScolor,VIStrace,VIStunX1,VIStunX2,VIStunX3,VIStunX4
	xdef VISireg,VISload,VISstore
	xdef VISlines


TC_WIDTH:       set 48
SAMPLOFFSET:    set 20

;-----------------------------------------------
; void VISinit(void* _tcbuf, void* _lines, u16 _TCwidth, u16 _TCheight);
;-----------------------------------------------
VISinit:
	move.l	#-1,$ffff8a28.w				; Mask 1 - 2
	move.w	#-1,$ffff8a2c.w				; Mask 3
	clr.l	$ffff8a2e.w					; Inc X & Y dest
	move.l	#$ffff8240,$ffff8a32.w		; Adr dest
	move.w	d0,$ffff8a36.w				; Width
    move.w  d0,VISw
	add.w	d0,d0
	move.w	d0,VISpitch
	move.w	#2,$ffff8a20.w				: Inc X source
	neg.w	d0
    addq.w  #2,d0
	move.w	d0,$ffff8a22.w				: Inc Y source
	move.w	#$203,$ffff8a3a.w			; op
    subq.w  #1,d1
	move.w	d1,VISh                     ; 4 pixels hight
	move.l	a0,VIStcBuf
	move.l	a1,VISlines

	rts

; -----------------------------------------
; void VIScopy(void* _source, void* _dest, u16 _nbwords);
; -----------------------------------------
VIScopy:	
    move.l	a0,$ffff8a24.w              ; Adr source
    move.w	#2,$ffff8a2e.w				; Inc X
	move.l	a1,$ffff8a32.w		        ; Adr dest
	move.w	d0,$ffff8a36.w				; Width
    move.w  #1,$ffff8a38.w              ; Height
    move.w  #$C000,$ffff8a3c.w

	rts

; -----------------------------------------
; VIStc display interruption
; -----------------------------------------
VIStc:
    move.w  #$2700,sr		; cut all interrupts

	move.w	d0,-(sp)
    move.w  d1,-(sp)
    move.l  a6,-(sp)

    lea		$ffff8209.w,a6

    ; synchro lsl to be perfectly synchronized with display

    move.b  (a6),d0
VIStc_lslSync:		
	move.b	(a6),d1
    cmp.b   d0,d1
	beq.s	VIStc_lslSync

    sub.b	d1,d0
	add.w	#31,d0
	lsl.w	d0,d1

    clr.b	$fffffa1b.w		; stop timer b
  
    ; re-param blitter 

    move.l	VIStcBuf(pc),$ffff8a24.w    , Adr source
    clr.l	$ffff8a2e.w					; Inc X & Y dest
    move.l	#$ffff8240,$ffff8a32.w		; Adr dest
    move.w	VISw(pc),$ffff8a36.w	    ; Width
    move.w	#$203,$ffff8a3a.w			; op

	movem.l	d2-d7/a5,-(sp)

    movem.w VISregs(pc),d0-d7

    ext.l   d2
	move.l	VISlines(pc),a5
    lea     $ffff8a38.w,a6

VIStc_tunLoop:
	dbra.w	d6,VIStc_tunLoop
    
	move.w	VIStunX2(pc),d6
	lsl.l	d6,d7
   
VIStc_loop:

    move.w	d1,(a6)
	move.w	d3,4(a6)
    move.w  d7,$ffff8240.w
	move.b	(a5)+,$ffff8207.w
	move.b	(a5)+,$ffff8209.w
    lsl.w   d4,d7
    lsl.w   d4,d7

    move.w	d1,(a6)
	move.w	d3,4(a6)
    move.w  d7,$ffff8240.w
	move.b	(a5)+,$ffff8207.w
	move.b	(a5)+,$ffff8209.w
    lsl.w   d4,d7
    lsl.w   d4,d7

    move.w	d1,(a6)
	move.w	d3,4(a6)
    move.w  d7,$ffff8240.w
	move.b	(a5)+,$ffff8207.w
	move.b	(a5)+,$ffff8209.w
    lsl.w   d4,d7
    lsl.w   d4,d7

    move.w	d1,(a6)
	move.w	d3,4(a6)
    move.w  d7,$ffff8240.w
	move.b	(a5)+,$ffff8207.w
	move.b	(a5)+,$ffff8209.w
    lsl.l   d5,d7
    lsl.w   d5,d7

    add.l   d2,$ffff8a24.w

	dbra.w	d0,VIStc_loop

    movem.l	(sp)+,d2-d7/a5

    move.l  (sp)+,a6
    move.w  (sp)+,d1
	move.w	(sp)+,d0

    clr.l   $120.w      ; hack to synchronize with main thread

	rte

; -----------------------------------------
; void VISfade (void* _source, void* _table, u32 _dest, u32 _pitch, u32 _count);
; -----------------------------------------
VISfade:

    movem.l	d3-d7/a2-a3,-(sp)

    move.l  d0,a2
    move.l  d1,a3

    move.w  d2,-(sp)

VISfade_loop:

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    add.l   a3,a2
    add.l   a3,a0

    subq.w  #1,(sp)
    bne     VISfade_loop

    lea     2(sp),sp

    movem.l	(sp)+,d3-d7/a2-a3

    rts


; -----------------------------------------
; void VISfade3 (void* _source, void* _table, u32 _dest, u32 _pitch, u32 _count);
; -----------------------------------------
VISfade3:

    movem.l	d3-d7/a2-a3,-(sp)

    move.l  d0,a2
    move.l  d1,a3
	lea		48(a3),a3

    move.w  d2,-(sp)

VISfade3_loop:

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    movem.w (a0)+,d0-d7

    move.w  (a1,d0.w),(a2)+
    move.w  (a1,d1.w),(a2)+
    move.w  (a1,d2.w),(a2)+
    move.w  (a1,d3.w),(a2)+
    move.w  (a1,d4.w),(a2)+
    move.w  (a1,d5.w),(a2)+
    move.w  (a1,d6.w),(a2)+
    move.w  (a1,d7.w),(a2)+

    add.l   a3,a2
    add.l   a3,a0

    subq.w  #1,(sp)
    bne.s   VISfade3_loop

    lea     2(sp),sp

    movem.l	(sp)+,d3-d7/a2-a3

    rts

; -----------------------------------------
; u16 VISfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width);
; -----------------------------------------
VISfilsampl:

    move.w  d2,-(sp)
    move.w  d3,-(sp)
    move.w  d4,-(sp)
    move.l  a2,-(sp)
    move.l  a3,-(sp)

    subq.w  #1,d1
    move.l  d0,a2
    move.w  #$FC,d3

	moveq.l	#0,d0

VISfilsampl_loop:
    
    move.w  (a1)+,d2

    move.b  (a0),d4
    bge.s   VISfilsampl_pos

    neg.b   d4
    lsr.b   #1,d4
    and.w   d3,d4
    beq.s   VISfilsampl_zero
	add.w	d4,d0
    neg.w   d4
    jmp     VISfilsampl_jmp1+2(pc,d4.w)
    
offset: set 21*TC_WIDTH*2
    rept 20
offset: set offset-TC_WIDTH*2
    move.w  d2,offset(a2)
    endr
    move.w  d2,(a2)+
VISfilsampl_jmp1:
    
    lea     SAMPLOFFSET(a0),a0
    dbra.w  d1,VISfilsampl_loop

    bra.s   VISfilsampl_end

VISfilsampl_zero:
    lea     2(a2),a2
    lea     SAMPLOFFSET(a0),a0
    dbra.w  d1,VISfilsampl_loop

    bra.s   VISfilsampl_end

VISfilsampl_pos:

    lsr.b   #1,d4
    and.w   d3,d4
    beq.s   VISfilsampl_zero
	add.w	d4,d0
    neg.w   d4
    jmp     VISfilsampl_jmp2+2(pc,d4.w)
    
offset: set -21*TC_WIDTH*2
    rept 20
offset: set offset+TC_WIDTH*2
    move.w  d2,offset(a2)
    endr
    move.w  d2,(a2)+
VISfilsampl_jmp2:

    lea     SAMPLOFFSET(a0),a0
    dbra.w  d1,VISfilsampl_loop

VISfilsampl_end:

    move.l  (sp)+,a3
    move.l  (sp)+,a2
    move.w  (sp)+,d4
    move.w  (sp)+,d3
    move.w  (sp)+,d2

    rts


VISregs:

VISh:		dc.w	0		; d0
            dc.w	1		; d1
VISpitch:	dc.w	0		; d2
    		dc.w	$C000	; d3
VIStunX3:	dc.w	31		; d4
VIStunX4:	dc.w	13		; d5
VIStunX1:	dc.w	6       ; d6
            dc.w    0       ; d7

VIStunX2:	dc.w	2
VISw:       dc.w    0
VIScolor:	dc.w	0
VIStcBuf:	dc.l	0
VISlines:	dc.l	0
VIStrace:   dc.l    0

VISireg:	move.l	d0,a2
VISload:	move.w	$AA(a0),d0
VISstore:	move.w	(a1,d0.w),(a2)+