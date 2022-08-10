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

	output  D:\PROJECTS\DEMOS\OUTPUT\FX\SNDSHADE.O

	opt	o+

    xdef SNSimportTable

TC_WIDTH:       set 48

SNSimportTable:
    dc.l    SNSgenerateCode
    dc.l    SNSinit
    dc.l    SNSfade
    dc.l    SNSfade3
    dc.l    SNSfilsampl
    dc.l    SNStc

SNSlines:	dc.l	0
SNStable:	dc.l	0
SNSempty:   dc.l    0

SNSregs:
SNSh:		dc.w	0		    ; d0
            dc.w	1		    ; d1
SNSpitch:	dc.w	TC_WIDTH	; d2
    		dc.w	$C000	    ; d3
SNStunX3:   dc.w    0           ; d4
SNSsampinc: dc.w    4           ; d5
SNSvoice2:  dc.w    0           ; d6
SNSmask:    dc.w    $FFFF       ; d7

SNSpushreg: move.l  a2,-(sp) 
SNSpopreg:  move.l  (sp)+,a2
SNSireg:	move.l	d0,a2
SNSload:	move.w	$AA(a0),d0
SNSstore:	move.w	(a1,d0.w),(a2)+

SNSblit:
            dc.w    2           ; source increment X        0           
SNSsrcIncY: dc.w    0           ; source increment Y        2
SNSsrcAdr:  dc.l    0           ; source address            4
            dc.w    -1          ; endmask1                  8
            dc.w    -1          ; endmask2                  10
            dc.w    -1          ; endmask3                  12
            dc.w    0           ; dest increment X          14
            dc.w    0           ; dest increment Y          16
SNSdestAdr: dc.l    $ffff8240   ; dest address              18
SNScountX:  dc.w    0           ; count X                   22  
            dc.w    1           ; count Y                   24
            dc.w    $303        ; HOP | OP                  26
                                ;                           28

;-----------------------------------------------
; u16* SNSgenerateCode ( u8* poff, u16* p, u16 _height )
;-----------------------------------------------
SNSgenerateCode:
    movem.l     d1-d7,-(sp)

    lsl.w       #4,d0                   ; height * 16 (then each loop make 3 steps => 3 * 16 == 48 == WIDTH)
    subq.w      #1,d0                   ; dbf need count - 1

    moveq.l     #0,d1
    moveq.l     #4,d4
    moveq.l     #6,d5

    move.w      SNSload(pc),d2
    move.l      SNSstore(pc),d3

    move.w      SNSpushreg(pc),(a1)+
    move.w      SNSireg(pc),(a1)+

.loop:
    
    move.l      (a0)+,d7                ; w
    
    move.w      d7,d6
    lsl.w       d5,d6                   ; 
    asr.w       #5,d6    
    neg.w       d6
    add.w       d1,d6
    move.w      d2,(a1)+                ; SNSload
    move.w      d6,(a1)+
    move.l      d3,(a1)+                ; SNSstore
    addq.w      #2,d1

    move.l      d7,d6
    lsr.l       d4,d6
    asr.w       d5,d6
    add.w       d6,d6
    neg.w       d6
    add.w       d1,d6
    move.w      d2,(a1)+                ; SNSload
    move.w      d6,(a1)+
    move.l      d3,(a1)+                ; SNSstore
    addq.w      #2,d1

    swap        d7
    asr.w       d5,d7
    add.w       d7,d7
    neg.w       d7
    add.w       d1,d7
    move.w      d2,(a1)+                ; SNSload
    move.w      d7,(a1)+
    move.l      d3,(a1)+                ; SNSstore
    addq.w      #2,d1

    dbf.w       d0,.loop

    move.w      SNSpopreg(pc),(a1)+
    move.w      #$4E75,(a1)+            ; rts opcode

    move.l      a1,a0

    movem.l     (sp)+,d1-d7

    rts


;-----------------------------------------------
; void SNSinit(void* _table, u16 _TCwidth, u16 _TCheight);
;-----------------------------------------------
SNSinit:
	move.w	d0,SNScountX				; Width
	add.w	d0,d0
	move.w	d0,SNSpitch
	move.w	#2,$ffff8a20.w				: Inc X source
	neg.w	d0
    addq.w  #2,d0
	move.w	d0,SNSsrcIncY				: Inc Y source
	move.w	#$203,$ffff8a3a.w			; op
    subq.w  #1,d1
	move.w	d1,SNSh                     ; 4 pixels hight
	move.l	a0,SNStable

	rts


; -----------------------------------------
; SNStc display interruption
; -----------------------------------------
SNStc:
	move.l	d0,-(sp)
    move.l  d1,-(sp)
   
    clr.b	$fffffa1b.w		; stop timer b 
   
    ; synchro lsl to be perfectly synchronized with display

    move.b  $ffff8209.w,d0
.lslSync:		
    move.b  d0,d1
	sub.b	$ffff8209.w,d1
	beq.s	.lslSync

    add.w	#31,d1
	lsl.w	d1,d0

    movem.l d2-d7/a2-a6,-(sp)

    ; re-param blitter 
    movem.l SNSblit(pc),d0-d6
    movem.l d0-d6,$ffff8a20.w

    movem.w SNSregs(pc),d0-d7
    move.w  d7,$fffff8a00.w
    moveq.l #0,d7

	move.l	SNSlines(pc),a5
    move.l  (a5),a5
    add.w   d6,a5
    move.l  SNStable(pc),a3
    lea     $ffff8a26.w,a2
    lea     $ffff8240.w,a4
    lea     $ffff8a38.w,a6    

    nop
  
.loop:
    move.w	d1,(a6)                 ; sizey = 1
	move.w	d3,4(a6)                ; run 
    move.w  d7,(a4)                 ; 8
    moveq.l #0,d6                   ; 4
    move.b  (a5),d6                 ; 8
    add.w   d5,a5
    add.w   d6,d6                   ; 4
    move.w  (a3,d6.w),d6            ; 16
    movep.w d6,$8207-$8240(a4)      ; 16
    nop
    nop
    nop
    nop
    nop
    nop
    lsl.w   d4,d7                   ; d4 = 10 => 28

    move.w	d1,(a6)                 ; sizey = 1
	move.w	d3,4(a6)                ; run 
    move.w  d7,(a4)                 ; 8
    moveq.l #0,d6                   ; 4
    move.b  (a5),d6                 ; 8
    add.w   d5,a5
    add.w   d6,d6                   ; 4
    move.w  (a3,d6.w),d6            ; 16
    movep.w d6,$8207-$8240(a4)      ; 16
    nop
    nop
    nop
    nop
    nop
    nop
    lsl.w   d4,d7                   ; d4 = 10 => 28

    move.w	d1,(a6)                 ; sizey = 1
	move.w	d3,4(a6)                ; run 
    move.w  d7,(a4)                 ; 8
    moveq.l #0,d6                   ; 4
    move.b  (a5),d6                 ; 8
    add.w   d5,a5
    add.w   d6,d6                   ; 4
    move.w  (a3,d6.w),d6            ; 16
    movep.w d6,$8207-$8240(a4)      ; 16
    nop
    nop
    nop
    nop
    nop
    nop
    lsl.w   d4,d7                   ; d4 = 10 => 28

    move.w	d1,(a6)                 ; sizey = 1
	move.w	d3,4(a6)                ; run 
    move.w  d7,(a4)                 ; 8
    moveq.l #0,d6                   ; 4
    move.b  (a5),d6                 ; 8
    add.w   d5,a5
    add.w   d6,d6                   ; 4
    move.w  (a3,d6.w),d6            ; 16
    movep.w d6,$8207-$8240(a4)      ; 16
    lsl.w   d4,d7                   ; d4 = 10 => 28

    add.w   d2,(a2)                 ; 20

	dbra.w	d0,.loop                ; 12

.end:

    move.b  SNSempty+1(pc),$ffff8205.w
    move.b  SNSempty+2(pc),$ffff8207.w
    move.b  SNSempty+3(pc),$ffff8209.w

    movem.l	(sp)+,d2-d7/a2-a6
    move.l  (sp)+,d1
    move.l  (sp)+,d0

    clr.l   $120.w      ; hack to synchronize with main thread

	rte

; -----------------------------------------
; void SNSfade (void* _source, void* _table, u32 _dest, u32 _pitch, u32 _count);
; -----------------------------------------
SNSfade:

    movem.l	d3-d7/a2-a3,-(sp)

    move.l  d0,a2
    move.l  d1,a3

    move.w  d2,-(sp)

.loop:

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
    bne     .loop

    lea     2(sp),sp

    movem.l	(sp)+,d3-d7/a2-a3

    rts


; -----------------------------------------
; void SNSfade3 (void* _source, void* _table, u32 _dest, u32 _pitch, u32 _count);
; -----------------------------------------
SNSfade3:

    movem.l	d3-d7/a2-a3,-(sp)

    move.l  d0,a2
    move.l  d1,a3
	lea		48(a3),a3

    move.w  d2,-(sp)

.loop:

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
    bne.s   .loop

    lea     2(sp),sp

    movem.l	(sp)+,d3-d7/a2-a3

    rts

; -----------------------------------------
; void SNSfilsampl (void* _sample, void* _startcolors, u32 _dest, u16 _width, s16 _sampleoffset);
; -----------------------------------------
SNSfilsampl:

    move.w  d3,-(sp)
    move.w  d4,-(sp)
    move.w  d5,-(sp)
    move.l  a2,-(sp)
    move.l  a3,-(sp)

    subq.w  #1,d1
    move.l  d0,a2
    move.w  #$FE,d3
    lea     .table(pc),a3

.loop:    
    move.w  (a1)+,d5            ; current color used to fill
    move.b  (a0),d4             ; current sample load
    and.w   d3,d4               ; 
    move.w  (a3,d4.w),d4
.jmpadr:
    jmp     .jmpadr(pc,d4.w) 

.neg:    
offset: set 16*TC_WIDTH*2
    rept 15
offset: set offset-TC_WIDTH*2
    move.w  d5,offset(a2)
    endr
    move.w  d5,(a2)+
    
    add.w   d2,a0
    dbra.w  d1,.loop
    bra.s   .end

.zero:
    lea     2(a2),a2
    add.w   d2,a0
    dbra.w  d1,.loop
    bra.s   .end

.pos:   
offset: set -17*TC_WIDTH*2
    rept 16
offset: set offset+TC_WIDTH*2
    move.w  d5,offset(a2)
    endr
    move.w  d5,(a2)+

    add.w   d2,a0
    dbra.w  d1,.loop

.end:

    move.l  (sp)+,a3
    move.l  (sp)+,a2
    move.w  (sp)+,d5
    move.w  (sp)+,d4
    move.w  (sp)+,d3

    rts


.table:

    dc.w   .zero-.jmpadr
    dc.w   .zero-.jmpadr

offset: set .pos-.jmpadr+16*4

    dc.w    offset
    dc.w    offset

offset: set offset-4

    rept    15
    dc.w    offset
    dc.w    offset
    dc.w    offset
    dc.w    offset

offset: set offset-4

    endr

offset: set .neg-.jmpadr

    rept    15
    dc.w    offset
    dc.w    offset
    dc.w    offset
    dc.w    offset

offset: set offset+4

    endr

    dc.w    offset
    dc.w    offset
    dc.w    offset
    dc.w   .zero-.jmpadr
