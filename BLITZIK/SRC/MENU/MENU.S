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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\MENU.PRX

	opt	o+

	xdef	BMpImport


BMpImport:
	lea		BMpImportTable(pc),a0
	rts


BMpImportTable:

BMpColor1:		dc.w	0
BMpColor2:		dc.w	0

BMpWaitLoop:	dc.w	0
BMpWaitLoop2:	dc.w	0

BMpPcIcons:		dc.b	0
BMpPxIcons:		dc.b	0

BMpPcTips:		dc.b	0
BMpPxTips:		dc.b	0

BMpEmpty:		dc.l	0
BMpSc1: 		dc.l 	0
BMpSc2: 		dc.l 	0
BMpSc2Shift:	dc.l	0
BMpASIcons:		dc.l	0
BMpASTips:		dc.l	0

	; a0 : color
	; a1 : pp
	; a2 : pp2
	; a3 : SN
	; a4 : sn2
	; a5 : ffff8203
	; a6 : ffff8a3c
	
BMpRegs:
	dc.l	0			; BMpRasters
	dc.l	0			; p1plasma
	dc.l	0			; p2plasma
	dc.l	0			; sn
	dc.l	0			; sn2
	dc.l	$ffff8203
	dc.l	$ffff8a3c

	; functions table
	dc.l	BMpMenuCopyIcons
	dc.l	BMpMenuXor
	dc.l	BMpMenuClear

BMplGenTable:
	;-----------------------------------------------------
	;	len / 2, cycles / 2
	;	routine address
	;-----------------------------------------------------
	
	dc.b	(BMplBeginEnd-BMplBegin)/2,0
	dc.l	BMplBegin

	dc.b	(BMplEndEnd-BMplEnd)/2,0
	dc.l	BMplEnd

	dc.b	(BMplBlitEnd-BMplBlit)/2,228
	dc.l	BMplBlit

	dc.b	(BMplBlit2End-BMplBlit2)/2,228
	dc.l	BMplBlit2

	dc.b	(BMpC2000end-BMpC2000)/2,6
	dc.l	BMpC2000

	dc.b	(BMpC1555end-BMpC1555)/2,8
	dc.l	BMpC1555

	dc.b	(BMpC1666end-BMpC1666)/2,8
	dc.l	BMpC1666

	dc.b	(BMpC1FFFend-BMpC1FFF)/2,8
	dc.l	BMpC1FFF	

	dc.b 	(BMpC3777end-BMpC3777)/2,8
	dc.l	BMpC3777

	dc.b 	(BMpC3Rend-BMpC3R)/2,8
	dc.l    BMpC3R

	dc.b	(BMpPc160end-BMpPc160)/2,6
	dc.l 	BMpPc160

	dc.b  	(BMpSPcIconsend-BMpSPcIcons)/2,10
	dc.l	BMpSPcIcons

	dc.b	(BMpSPcTipsend-BMpSPcTips)/2,10
	dc.l	BMpSPcTips

	dc.b 	(BMpSPx0end-BMpSPx0)/2,6
	dc.l	BMpSPx0

	dc.b 	(BMpSPxIconsend-BMpSPxIcons)/2,10
	dc.l 	BMpSPxIcons

	dc.b 	(BMpSPxTipsend-BMpSPxTips)/2,10
	dc.l 	BMpSPxTips

	dc.b	(BMpSAEmptyend-BMpSAEmpty)/2,12
	dc.l    BMpSAEmpty

	dc.b 	(BMpSASc1end-BMpSASc1)/2,12
	dc.l 	BMpSASc1

	dc.b	(BMpSASc2end-BMpSASc2)/2,12
	dc.l 	BMpSASc2

	dc.b 	(BMpSAS2send-BMpSAS2s)/2,12
	dc.l    BMpSAS2s

	dc.b 	(BMpSAIconsScrolend-BMpSAIconsScrol)/2,12
	dc.l 	BMpSAIconsScrol

	dc.b 	(BMpSATipsScrolend-BMpSATipsScrol)/2,12	
	dc.l 	BMpSATipsScrol		

BMpBackSP:
	dc.l	0

;-----------------------------------------
; No relative pc code in generated code...
;-----------------------------------------

;-----------------------------------------
; BMplasma display interruption
;-----------------------------------------
BMplBegin:
	move.w	#0,$ffff8240.w			; autogenerate to save some cycles here to improve stabilization
	
	; move.w	BMpColor1,$ffff8240.w			; absolute adressing because of generated code

    ; synchro lsl to be perfectly synchronized with display
	move.l	a0,usp		; this trick instead of saving d6.w and d7.w on stack to save 8 cycles
	move.l	d7,a0
	move.w	d6,d7
	swap	d7
	
    move.b  $ffff8209.w,d6
.lslSync:		
    move.b  d6,d7
	sub.b	$ffff8209.w,d7
	beq.s	.lslSync

    clr.b	$fffffa1b.w		; stop timer b 

	add.w	#31,d7
	lsl.w	d7,d6

	swap	d7
	move.w	d7,d6
	move.l	a0,d7
	move.l	usp,a0

	move.w	d6,-(sp)
	move.w	d7,-(sp)

    ;move.w  #$700,$ffff8240.w

    movem.l d0-d5/a0-a6,-(sp)

    move.w  #$2700,sr		; cut all interrupts
	move.l	sp,BMpBackSP
	lea		BMpImportTable,sp	; generated code => no pc relative

	;move.w  #$7,$ffff8240.w

	move.w	BMpWaitLoop-BMpImportTable(sp),d0
.waitloop:	dbf.w d0,.waitloop
    
	;move.w  #$70,$ffff8240.w

    ; Re-param the blitter 
    movem.l BMpblitregs-BMpImportTable(sp),d0-d5/a0
    movem.l d0-d5/a0,$ffff8a20.w

	; Loads / preloads registers
	movem.l	BMpEmpty-BMpImportTable(sp),d0-d5/a0-a6

	move.w	BMpColor2-BMpImportTable(sp),d6
	move.w	#$C000,d7	; d7 can be used to start blitter, 0 color, 0 for bytes...

    move.w	#$555,$ffff8242.w
    move.w	d7,$ffff8244.w
	move.w	(a0)+,$ffff8246.w ; to save time on 1st line...

	move.w	BMpColor2-BMpImportTable(sp),$ffff8240.w

BMplBeginEnd:

;-----------------------------------------
; Set colors
;-----------------------------------------
BMpC2000:
	move.w	d7,$ffff8244.w
BMpC2000end:

;-----------------------------------------
BMpC1555:
	move.w	#$555,$ffff8242.w
BMpC1555end:

;-----------------------------------------
BMpC1666:
	move.w	#$666,$ffff8242.w
BMpC1666end:

;-----------------------------------------
BMpC1FFF:
	move.w	#$FFF,$ffff8242.w
BMpC1FFFend:

;-----------------------------------------
BMpC3777:
	move.w	#$777,$ffff8246.w
BMpC3777end:

;-----------------------------------------
BMpC3R:
	move.w	(a0)+,$ffff8246.w
BMpC3Rend:

;-----------------------------------------
; Set line width
;-----------------------------------------
BMpPc160:
	move.b	d7,$ffff820f.w
BMpPc160end:

;-----------------------------------------
BMpSPcIcons:
	move.b	BMpPcIcons-BMpImportTable(sp),$ffff820f.w
BMpSPcIconsend:

;-----------------------------------------
BMpSPcTips:
	move.b	BMpPcTips-BMpImportTable(sp),$ffff820f.w
BMpSPcTipsend:

;-----------------------------------------
; Set pixel offsets
;-----------------------------------------
BMpSPx0:
	move.b	d7,$ffff8265.w
BMpSPx0end:

;-----------------------------------------
BMpSPxIcons:
	move.b	BMpPxIcons-BMpImportTable(sp),$ffff8265.w
BMpSPxIconsend:

;-----------------------------------------
BMpSPxTips:
	move.b	BMpPxTips-BMpImportTable(sp),$ffff8265.w
BMpSPxTipsend:

;-----------------------------------------
; Set adresses
;-----------------------------------------
BMpSAEmpty:
	movep.l d0,(a5)
BMpSAEmptyend:

;-----------------------------------------
BMpSASc1:
	movep.l d1,(a5)
BMpSASc1end:

;-----------------------------------------
BMpSASc2:
	movep.l d2,(a5)
BMpSASc2end:

;-----------------------------------------
BMpSAS2s:
	movep.l d3,(a5)
BMpSAS2send:

;-----------------------------------------
BMpSAIconsScrol:
	movep.l d4,(a5)
BMpSAIconsScrolend:

;-----------------------------------------
BMpSATipsScrol:
	movep.l d5,(a5)					; 24
BMpSATipsScrolend:

;-----------------------------------------

;-----------------------------------------
BMplBlit:

    move.w	#1,$ffff8a38.w				; vertical size => 1		12
	move.l	a1,$ffff8a24.w				; address source			16
	add.w	2(a3),a1					;							16
    move.w	d7,(a6)						; run blitter				8 
										;							384 ?
    move.w  d6,$ffff8240.w				; color          			12

BMplBlitEnd:


;-----------------------------------------
BMplBlit2:

    move.w	#1,$ffff8a38.w				; vertical size => 1		12
	move.l	a2,$ffff8a24.w				; address source			16
	add.w	2(a4),a2					;							16
    move.w	d7,(a6)						; run blitter				8 
										;							384 ?
    move.w  d6,$ffff8240.w				; color2					12

BMplBlit2End:


;-----------------------------------------
BMplEnd:

	move.b	BMpEmpty+2-BMpImportTable(sp),$ffff8203.w	; restore base that has been crunched by the movep.l
	move.l  BMpBackSP,sp
	
	move.w	BMpColor1,$ffff8240.w	; absolute adressing because of generated code...
	move.w	BMpWaitLoop2,d6

    movem.l	(sp)+,d0-d5/a0-a6
    move.w	(sp)+,d7

.waitloop:
	dbf.w	d6,.waitloop

    move.w	(sp)+,d6

	clr.w	$ffff8240.w

    ; clr.l   $120.w      ; hack to synchronize with main thread
	; move.w	#$70,$ffff8240.w

	rte

BMplEndEnd:


;---------------------------------------------------------------------------
; void* blitzMenuCopyIcons(void* _src, void* _dst, u16 _pitchdst)
;---------------------------------------------------------------------------

BMpMenuCopyIcons:

	move.w	d1,-(sp)
	move.w	d2,-(sp)

	move.w	(a0)+,d1		; u16 nbwords = *s++;
    move.w  (a0)+,d2		; u16 h = *s++;

	sub.w	d1,d0			; _pitchdst >>= 1; _pitchdst -= nbwords << 1;
	sub.w	d1,d0
	sub.w	d1,d0
	sub.w	d1,d0

	subq.w  #1,d2			; dbf loopy...
   
	neg.w	d1
	add.w	#80,d1
	add.w	d1,d1

.loopy:
	jmp		.loopx(pc,d1.w)
	
.loopx:
	rept	80
	move.l  (a0)+,(a1)+
	endr

	add.w	d0,a1
	dbf.w	d2,.loopy		; d += _pitchdst;

	move.w	(sp)+,d2
	move.w	(sp)+,d1

	rts


;---------------------------------------------------------------------------
; void blitzMenuXor (void* _adr, u16 _nbwords, u16 _h, u16 _pitch)
;---------------------------------------------------------------------------

BMpMenuXor:

	move.w	d3,-(sp)
	move.w	d4,-(sp)

	sub.w	d0,d2			; _pitchdst >>= 1; _pitchdst -= nbwords << 1;
	sub.w	d0,d2
	sub.w	d0,d2
	sub.w	d0,d2

	subq.w  #1,d0			; dbf loopx...
	subq.w  #1,d1			; dbf loopy...
   
.loopy:
	move.w	d0,d3
	
.loopx:
	move.w  2(a0),d4
	eor.w   d4,(a0)
	addq.l  #4,a0
	dbf.w	d3,.loopx		

	add.w	d2,a0			; d += _pitchdst;
	dbf.w	d1,.loopy		

	move.w	(sp)+,d4
	move.w	(sp)+,d3

	rts


;---------------------------------------------------------------------------
; void blitzMenuClear(void* _adr, u16 _nbwords, u16 _h, u16 _pitch)
;---------------------------------------------------------------------------

BMpMenuClear:

	move.w	d3,-(sp)

	sub.w	d0,d2			; _pitchdst >>= 1; _pitchdst -= nbwords << 1;
	sub.w	d0,d2
	sub.w	d0,d2
	sub.w	d0,d2

	moveq.l #0,d3

	subq.w  #1,d1			; dbf loopy...
   
	neg.w	d0
	add.w	#80,d0
	add.w	d0,d0
	add.w	d0,d0

.loopy:
	jmp		.loopx(pc,d0.w)
	
.loopx:
	rept	80
	move.w  d3,(a0)
	addq.l  #4,a0
	endr

	add.w	d2,a0
	dbf.w	d1,.loopy		; d += _pitchdst;

	move.w	(sp)+,d3

	rts



BMpblitregs:
    dc.w    2           ; source increment X        0           
    dc.w    0           ; source increment Y        2
    dc.l    0           ; source address            4
    dc.w    -1          ; endmask1                  8
    dc.w    -1          ; endmask2                  10
    dc.w    -1          ; endmask3                  12
    dc.w    0           ; dest increment X          14
    dc.w    0           ; dest increment Y          16
    dc.l    $ffff8240   ; dest address              18
	dc.w    48          ; count X                   22  
    dc.w    1           ; count Y                   24
    dc.w    $203        ; HOP | OP                  26

