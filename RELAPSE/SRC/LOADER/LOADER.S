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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\LOADER.PRX

	opt	o+


cpu:	set 0

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
    
    ; common
    dc.l	LOADERcomputeHtone
    dc.l	LOADERblitV
    dc.l	LOADERblitH
    dc.l    LOADERsin

LOADERcomputeHtone:
    movem.l      d2-d3,-(sp)

    moveq.w     #19,d3
.loop:
    rept        16
    add.w       d0,d1
    addx.w      d2,d2
    endr

    move.w      d2,(a0)+

    dbra        d3,.loop

    movem.l     (sp)+,d2-d3

    rts


LOADERblitV:

    movem.l  a2-a3,-(sp)

    lea $ffff8a00+20.w,a3

    rept 10
    move.w (a1)+,-(a3)
    endr

    lea $ffff8a28.w,a2

    move.l  #-1,(a2)+            ; endmask 1 & 2
    move.w  #-1,(a2)+            ; endmask 3
    move.w  #160,(a2)+           ; HW_BLITTER_XINC_DEST
    move.w  #-199*160+8,(a2)+    ; HW_BLITTER_YINC_DEST
    move.l  a0,(a2)+             ; HW_BLITTER_ADDR_DEST = framebuffer
    move.l  #200*65536+10,(a2)+  ; HW_BLITTER_XSIZE = 200
                                 ; HW_BLITTER_YSIZE = 10
    move.w  #$103,(a2)+          ; HW_BLITTER_OP = HW_BLITTER_OP_S,  HW_BLITTER_HOP = HW_BLITTER_HOP_HTONE

    move.w  #$8900,(a2)          ; HW_LINE = 9 ; HOG = 0 ; GO

.waitblit1:
	bset.b  #7,$FFFF8A3C.w       ; (re)start the BLiTTER
	nop                          ; BLiTTER will need a few cycles
	bne.s   .waitblit1           ; Loop if registers shows "busy"

    lea $ffff8a00+20.w,a3

    rept 10
    move.w  (a1)+,-(a3)
    endr

    move.w  #10,$ffff8a38.w
    move.w  #$8900,(a2)          ; HW_LINE = 9 ; HOG = 0 ; GO

.waitblit2:
	bset.b  #7,$FFFF8A3C.w       ; (re)start the BLiTTER
	nop                          ; BLiTTER will need a few cycles
	bne.s   .waitblit2           ; Loop if registers shows "busy"

    movem.l  (sp)+,a2-a3
                                 
    rts



    
LOADERblitH:

    ; d0 = space_
    ; d1 = y_

    movem.l     d2-d4/d7/a2-a6,-(sp)

    moveq.l     #0,d2
    move.w      d0,d2
    lsl.w       #5,d2
    move.l      d2,d4
    lsl.l       #2,d2
    add.l       d4,d2

    lea         32000(a0),a1    ; u32 endadr = (u32) framebuffer_ + 32000;

    ext.l       d1
    divu.w      d0,d1
    swap        d1
    ext.l       d1
    lsl.w       #5,d1
    move.l      d1,d4
    lsl.l       #2,d1
    add.l       d4,d1
    add.l       d1,a0           ; u32 adr    = (u32) framebuffer_ + (y_ % dispy) * 160;

    lea         $ffff8a28.w,a2   
    moveq.l     #-1,d7
    move.l      d7,(a2)+        ; *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    move.w      d7,(a2)+
    moveq.l     #8,d7
    move.w      d7,(a2)+        ; *HW_BLITTER_XINC_DEST = 8;
    move.w      d7,(a2)+        ; *HW_BLITTER_YINC_DEST = 8;
    addq.l      #4,a2
    move.w      #20,(a2)+       ; *HW_BLITTER_XSIZE = 20;
    addq.l      #2,a2
    move.w      #$F,(a2)+       ; *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1; *HW_BLITTER_OP = HW_BLITTER_OP_BIT1;

    lea         $ffff8a32.w,a3
    lea         $ffff8a38.w,a4
    lea         $ffff8a3c.w,a5

    clr.b       $ffff8a3d.w     ; *HW_BLITTER_CTRL2 = 0;

    move.w      #$C000,d3
    moveq.l     #1,d4

    ; a0 = adr
    ; a1 = endadr
    ; d2 = incadr
    ; d7 = acc

    cmpa.l      a0,a1
    ble.s       .end

.loop:

    move.l      a0,(a3)         ; *HW_BLITTER_ADDR_DEST = adr;
    move.w      d4,(a4)         ; *HW_BLITTER_YSIZE = 1;
    move.w      d3,(a5)         ; *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;

    add.l       d2,a0           ; adr += incadr;

    cmpa.l      a0,a1
    bgt.s       .loop           ; while (adr < endadr)

.end:

    movem.l     (sp)+,d2-d4/d7/a2-a6

    rts

LOADERsin:
    incbin  sin.bin