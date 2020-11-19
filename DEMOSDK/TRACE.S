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

	output	D:\PROJECTS\DEMOS\OUTPUT\TRACE.O
	
	opt 	o+

    xdef    TRACmaxraster
    xdef    TRAC2bomb,TRAC3bomb,TRAC4bomb,TRAC5bomb,TRAC6bomb,TRAC7bomb,TRAC8bomb

    xref    SYSfastPrint


tracNBBOMBS:		set 72
tracREGSTORAGESIZE: set 74

;-----------------------------------------------------------------------------------------
; TRACbomb
;-----------------------------------------------------------------------------------------
TRAC2bomb:
    move.b  #'2',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC3bomb:
    move.b  #'3',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC4bomb:
    move.b  #'4',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC5bomb:
    move.b  #'5',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC6bomb:
    move.b  #'6',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC7bomb:
    move.b  #'7',buffer+tracNBBOMBS
    bra.s   TRACbomb

TRAC8bomb:
    move.b  #'8',buffer+tracNBBOMBS

TRACbomb:
    move.w  #$2700,sr           ; disable interupts

    movem.l d0-d7/a0-a7,buffer  ; store regs at fixed address => we can trash code because we wont exit from here

    move.l  usp,a0
    move.l  a0,buffer+64

	move.l	10(a7),buffer+68	; retrieve faulty instruction address

    lea     $ffff8201.w,a0      ; get the current screen adr
    moveq.l #0,d0
    movep.w (a0),d0
    lsl.l   #8,d0
    move.b  $ffff820d.w,d0
    move.l  d0,a1

    move.w  #$A00,$ffff8240.w   ; set colors
    move.l  #-1,$ffff8242.w

    clr.b   $ffff820f.w         ; video offset = 0
    clr.b   $ffff8265.w         ; pixel offset = 0
    move.b  #1,$ffff8260.w      ; video mode   = 2 planes

    move.w  #7999,d0            ; erase screen
    move.l  a1,a0
.cls:
    clr.l   (a0)+
    dbf.w   d0,.cls
    
    lea     buffer+tracREGSTORAGESIZE(pc),a0   ; string adr
    move.b  #'d',(a0)           
    move.b  #'0',1(a0)              
    move.b  #':',2(a0)          
    move.b  #' ',3(a0)          
    move.w  #160,d0             ; pitch
    moveq.l #4,d1               ; bitplane pitch
    lea     buffer(pc),a2       ; regs value adr
    moveq.l #17,d2              ; 16 registers + usp + faulty pc
.loop:
    move.l  (a2)+,d3            ; get current reg value
    
    lea     4(a0),a3            ; compose string

    moveq.l #7,d5               ; write current reg values
.loop2:
    rol.l   #4,d3
    moveq.l #$F,d4
    and.l   d3,d4
    cmp.b   #10,d4
    blt.s   .deci
    add.b   #'A'-'0'-10,d4
.deci:
    add.b   #'0',d4
    move.b  d4,(a3)+            ; output char
    dbf.w   d5,.loop2
    clr.b   (a3)+

    movem.l d0-d1/a0-a1,-(sp)
    bsr     SYSfastPrint
    movem.l (sp)+,d0-d1/a0-a1

    lea     160*8(a1),a1

    cmp.w   #10,d2
    bne.s   .donotinc
    move.w  #'a'*256+'0'-1,(a0)
.donotinc:
    addq.b  #1,1(a0)

	cmp.w	#1,d2
	bne.s	.notpc
    move.w  #'pc',(a0)
.notpc:

    dbf.w   d2,.loop

    lea     160*8(a1),a1
    move.l  a0,a3
    move.b  buffer+tracNBBOMBS,(a3)+
    move.b  #' ',(a3)+
    move.w  #'BO',(a3)+
    move.w  #'MB',(a3)+
    move.w  #'S'*256,(a3)+

    bsr     SYSfastPrint

.endlessloop:
    bra.s   .endlessloop

    rte
    
buffer:

;---------------------------------------
; u16 TRACmaxraster (u16 maxraster)
;---------------------------------------
TRACmaxraster:
    move.l  d1,-(sp)
    move.l  d2,-(sp)

	moveq.l	#0,d1
	move.b	$ffff8201.w,d1
	swap	d1
	move.b	$ffff8203.w,d1
	lsl.w	#8,d1
	move.b	$ffff820d.w,d1              ; video base address

	moveq.l	#0,d2
	move.b	$ffff8205.w,d2
	swap	d2
	move.b	$ffff8207.w,d2
	lsl.w	#8,d2
	move.b	$ffff8209.w,d2              ; current video address

    sub.l   d1,d2                       ; current - video

    cmp.w   d0,d2
    bhi.s   .getmax                     ; if current < max 

.loop:
	moveq.l	#0,d2
	move.b	$ffff8205.w,d2
	swap	d2
	move.b	$ffff8207.w,d2
	lsl.w	#8,d2
	move.b	$ffff8209.w,d2              ; current video address

    sub.l   d1,d2                       ; current - video
    cmp.w   d0,d2                       ;
    bls.s   .loop                       ; loop while current > max 

    move.l  (sp)+,d1
    move.l  (sp)+,d2

    rts

.getmax
    move.w  d2,d0

    move.l  (sp)+,d1
    move.l  (sp)+,d2

    rts

