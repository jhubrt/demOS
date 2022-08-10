;-----------------------------------------------------------------------------------------------
; The MIT License (MIT)
;
; Copyright (c) 2015-2022 J.Hubert
;
; Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
; and associated documentation files (the "Software"), 
; to deal in the Software without restriction, including without limitation the rights to use, 
; copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
; and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all copies 
; or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
; IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
; DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;------------------------------------------------------------------------------------------------

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\WAVHERO.O

    xdef WHimportTable,WHvbl
    xref RASnextOpList


WHdebugLowB:    set 0


WHimportTable:	 

    dc.l    0   ; colors table
    dc.l    0   ; sample
    dc.l    0   ; star erase
    dc.l    0   ; rndx
    dc.l    0   ; rdny
    dc.l    0   ; starz
    dc.l    0   ; framebuffer
    dc.w    0   ; starinc
currentrndx:    dc.l    0       ; static u16 incrndx = 0;
currentrndy:    dc.l    0       ; static u16 incrndy = 0;

pcmvolumes:
    dc.w    0   ; pcm volumes
    dc.w    0
    dc.w    0
    dc.w    0
   
	dc.b	(WHBegin_-WHBegin)/2,0
	dc.l	WHBegin

    dc.b    (WHstarErase_-WHstarErase)/2,20/2
    dc.l    WHstarErase

    dc.b    (WHstarDrawBegin_-WHstarDrawBegin)/2,100/2
    dc.l    WHstarDrawBegin

    dc.b    (WHstarDraw_-WHstarDraw)/2,156/2
    dc.l    WHstarDraw

    dc.b    (WHstarDrawEnd_-WHstarDrawEnd)/2,40/2
    dc.l    WHstarDrawEnd

	dc.b	(WHras_-WHras)/2,296/2
	dc.l	WHras

	dc.b	(WHlowB_-WHlowB)/2,56/2
	dc.l	WHlowB

    dc.b    (WHvol_-WHvol)/2,104/2
    dc.l    WHvol

    dc.b    (WHvolEnd_-WHvolEnd)/2,128/2
    dc.l    WHvolEnd

	dc.b	(WHEnd_-WHEnd)/2,0
	dc.l	WHEnd


WHBegin:      
        move.l  d6,-(sp)
        move.l  d7,-(sp)
        
        clr.b	$fffffa1b.w						; stop timer b 

        ;move.w  #$70,$ffff8240.w

        move.b  $ffff8209.w,d6
.lslSync:
        move.b  d6,d7
	    sub.b	$ffff8209.w,d7
	    beq.s	.lslSync

        add.b	#31,d7
	    lsl.w	d7,d6
               
        ;move.w  #$700,$ffff8240.w

        movem.l  a0-a6/d0-d5,-(sp)

        ; rasop : 
        ; - w   : starscolors
        ; - w   : colors table increment
       
        lea      WHimportTable,a6   ; a6    : import table

        move.l   (a6)+,a1           ; a1    : colors table
        move.l   (a6)+,a5           ; a5    : sample buffer
        move.l   (a6)+,a4           ; a4    : star erase

        move.l   usp,a0             ; a0    : rasops
        move.l   sp,usp
       
        moveq.l #0,d0               ; d0-d3 : sample total
        moveq.l #0,d1
        moveq.l #0,d2
        moveq.l #0,d3
        moveq.l #0,d4               ; d4    : 0       
        moveq.l #0,d7               ; d7    : 0
       
        lea     -16(a1),a1

	    move.l  d7,$120.w

WHBegin_:


; -------------------------------------------------------
; a0-a1         : persistent 
; a2-a3 / d5-d6 : transient

WHras:

    lea      $ffff8242.w,a2     ; 8 
    lea      $ffff8250.w,a3     ; 8
    
    move.w  (a0)+,d5            ; 8                 starcolor
    move.w  d5,(a3)+            ; 8

    rept 7
    move.w  (a1)+,d6            ; 8                 colorstable
    move.w  d6,(a2)+            ; 8                 ffff8242
    bne.s   *+4                 ; 12 / 8
    move.w  d5,d6               ; 0  / 4
    move.w  d6,(a3)+            ; 8                 ffff8252
    endr                        ; 36 * 7 = 252

    add.w   (a0)+,a1            ; 12                inc colorstable

WHras_:                         ; 252 + 32 + 12 = 296


WHlowB:
	clr.b	$ffff820a.w			; 60hz  16

    ifne WHdebugLowB

    move.w  #$70,$ffff8240.w   

	rept 2
	nop
	endr

    endc

    ifeq WHdebugLowB

	rept 6                      ;       24
	nop
	endr

    endc

	move.b	#2,$ffff820a.w		; 50hz  16

WHlowB_:


; -------------------------------------------------------
; d0-d3 / a5    persistent
; d4            transient but need 0 in word upper byte

WHvol:

    move.b  (a5)+,d4            ; 8
    bpl.s   *+4                 ; 12 / 8
    neg.b   d4                  ; 0  / 4
    add.w   d4,d0               ; 4
                                ; = 24
    move.b  (a5)+,d4
    bpl.s   *+4     
    neg.b   d4      
    add.w   d4,d1   

    move.b  (a5)+,d4
    bpl.s   *+4     
    neg.b   d4
    add.w   d4,d3

    move.b  (a5)+,d4
    bpl.s   *+4     
    neg.b   d4
    add.w   d4,d2              ; 96
    
    addq.l  #4,a5              ; 104

WHvol_:
    

WHvolEnd:

    lsr.w   #8,d0              ; 24
    lsr.w   #8,d1
    lsr.w   #8,d2
    lsr.w   #8,d3

    movem.w d0-d3,pcmvolumes   ; 32

WHvolEnd_:

; -------------------------------------------------------
; d7 / a4    persistent
; a2         transient

WHstarErase:
    move.l  (a4)+,a2            ; 12
    move.b  d7,(a2)             ; 8
WHstarErase_:


; -------------------------------------------------------
; a4-a7 / d0-d4/d7      persistent
; a2-a3 / d5            transient

WHstarDrawBegin:

        move.l  -4(a6),a4               ; 16    a4   erasebuffer
       
	    move.l  (a6)+,d0                ; 12    d0    s32*  rndx;
        move.l  (a6)+,d1                ; 12    d1    s32*  rndy;
        move.l  (a6)+,a7                ; 12    a7          starz
        move.l  (a6)+,d4                ; 12    d4    frame buffer

        moveq.l #0,d7                   ; 4
        move.w  (a6)+,d7                ; 8

        move.l  (a6)+,a5                ; 12    d2    incrndx
        move.l  (a6)+,a6                ; 12    d3    incrndy
            
WHstarDrawBegin_:


WHstarDraw:

        move.l  (a7),a3                 ; 12        u16* xt = (u16*) z[0];

        move.b  (a3)+,d5                ; 8         u16 xpix     = *xt;
        blt.s   .reset                  ; 8/12      if (xpix == 0x8000)

        add.l   d7,(a7)+                ; 20        z[0] += 2;

        move.l  (a7),a2                 ; 12        u16* yt = (u16*) z[1];
        add.l   d7,(a7)+                ; 20        z[1] += 2;
                                        ;           z += 2;

        move.w  (a2),d6                 ; 8         u16 yoffset  = *yt;
        move.l  d4,a2                   ; 4         u8* p = (u8*) _framebuffer;
        add.w   d6,a2                   ; 8         p += yoffset;

        moveq.l #0,d6                   ; 4
        move.b  (a3)+,d6                ; 8         xpix >>= 8;
        add.w   d6,a2                   ; 8         p += xpix & 0xFF;
        
        bset.b  d5,(a2)                 ; 12        *p |= 1 << xpix;

        move.l  a2,(a4)+                ; 12        *erasebuffer++ = p; <----------------------------------

        bra.s   WHstarDraw_             ; 12

                                        ; 156 (128 + 8 + 20)

.reset:
        ; z[0] = _param->rndx[incrndx++] + (u32) _param->prex;
        ; if ( _param->rndx[incrndx] < 0 )
        ;     incrndx = 0;
        
        move.l  (a5)+,(a7)+             ; 20
        tst.l   (a5)                    ; 12
        bne.s   .norndxloop             ; 12
        move.l  d0,a5
.norndxloop:
        
        move.l  (a6)+,(a7)+             ; 20
        tst.l   (a6)                    ; 12
        bne.s   .norndyloop             ; 12
        move.l  d1,a6
.norndyloop:

        move.l  d4,(a4)+                ; 12  < -------------------------------

        rept 6
        nop
        endr

WHstarDraw_:                            ; 100 ( + 12 + 20)


WHstarDrawEnd:
        move.l  a5,currentrndx      ; 20
        move.l  a6,currentrndy      ; 20
WHstarDrawEnd_:
 


WHEnd:

    clr.w   $ffff8240.w

    move.l   usp,sp
    movem.l  (sp)+,a0-a6/d0-d5
    
    move.l  (sp)+,d7
    move.l  (sp)+,d6

    rte

WHEnd_:


WHvbl:
    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w    

    move.l  RASnextOpList,a1
	move.w	(a1)+,$fffffa20.w
	move.l	(a1)+,$120.w

	or.b	#1,$fffffa07.w
	move.b	#8,$fffffa1b.w

    movem.l a2-a4/d5-d6,-(sp)
   
    move.w  (a1)+,d5            ; 8                 starcolor
    addq.w  #2,a1               ; 12                inc colorstable
	move.l	a1,usp

    lea      $ffff8242.w,a2         ; 8 
    lea      $ffff8250.w,a3         ; 8
    move.l   WHimportTable(pc),a4

    move.w  d5,(a3)+            ; 8

    rept 7
    move.w  (a4)+,d6            ; 8                 colorstable
    move.w  d6,(a2)+            ; 8                 ffff8242
    bne.s   *+4                 ; 12 / 8
    move.w  d5,d6               ; 0  / 4
    move.w  d6,(a3)+            ; 8                 ffff8252
    endr                        ; 36 * 7 = 252

    movem.l (sp)+,a2-a4/d5-d6

    move.l  (a0)+,a1
    jmp     (a1)
