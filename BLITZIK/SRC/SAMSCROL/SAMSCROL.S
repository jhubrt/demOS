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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\SAMSCROL.O

	xdef	BMscImportTable 


BMscRASTERIZE:  set 0

BMscPitch:		set 480
BMscFieldWidth:	set 240

;----------------------------------------
BMscImportTable:	 

BMscPlayfieldClearAddress:	dc.l	0
BMscFrontBuffer:			dc.l	0
BMscWaitLoop:				dc.w	0

BMscPreloadAddress:
	dc.l	0			; a0 = samplebase
	dc.l	0			; a1 = clearoffsets
	dc.l	0			; a2 = displaybase
	dc.l	0			; a3 = colorsource
	dc.l	0			; a4 = yoffsettable2
	dc.l	0			; a5 = xoffsettable
	dc.l	0			; a6 = yoffsettable

BMscPreloadData:
					    ; d0 = temp value
					    ; d1 = unused
	dc.w	0		    ; d2 = rolling pixel
					    ; d3 = unused
					    ; d4 = unused
	dc.w	$00FE	    ; d5 = 00FE 
	dc.w	1		    ; d6 = 1

	; -------------

	dc.b	(BMscBeginEnd-BMscBegin)/2,0
	dc.l	BMscBegin

	dc.b	(BMscClearEnd-BMscClear)/2,36/2
	dc.l	BMscClear

	dc.b	(BMscReloadRegEnd-BMscReloadReg)/2,108/2
	dc.l	BMscReloadReg

	dc.b	(BMscLeftBorderRemovalEnd-BMscLeftBorderRemoval)/2,24/2
	dc.l	BMscLeftBorderRemoval

	dc.b	(BMscSam1Sam2Sam1End-BMscSam1Sam2Sam1)/2,320/2
	dc.l	BMscSam1Sam2Sam1

	dc.b	(BMscSam2Sam1Sam2End-BMscSam2Sam1Sam2)/2,340/2
	dc.l	BMscSam2Sam1Sam2

	dc.b	(BMscRightBorderColoredEnd-BMscRightBorderColored)/2,88/2
	dc.l	BMscRightBorderColored

	dc.b	(BMscRightBorderRemovalEnd-BMscRightBorderRemoval)/2,24/2
	dc.l	BMscRightBorderRemoval

	dc.b	(BMscFillCurvesOffsetInitEnd-BMscFillCurvesOffsetInit)/2,8/2
	dc.l	BMscFillCurvesOffsetInit

	dc.b	(BMscFillCurvesOffsetEnd-BMscFillCurvesOffset)/2,8/2
	dc.l	BMscFillCurvesOffset

	dc.b	(BMscEndEnd-BMscEnd)/2,0
	dc.l	BMscEnd

	dc.b    (BMscClearCurve1End-BMscClearCurve1)/2,0
	dc.l	BMscClearCurve1

	dc.b    (BMscClearCurve2End-BMscClearCurve2)/2,0
	dc.l	BMscClearCurve2

	dc.b    (BMscClearCurve3End-BMscClearCurve3)/2,0
	dc.l	BMscClearCurve3

BMscsaveSP:	dc.l	0

;---------------------------------------------
; O V E R S C A N  /  D R A W   R O U T I N E
;---------------------------------------------

BMscBegin:

    ; synchro lsl to be perfectly synchronized with display
    move.w d6,-(sp)
    move.w d7,-(sp)

	move.w	#$2700,sr

    move.b  $ffff8209.w,d6

.lslSync:
    move.b  d6,d7
	sub.b	$ffff8209.w,d7
	beq.s	.lslSync

    clr.b	$fffffa1b.w						; stop timer b 

	add.w	#31,d7
	lsl.w	d7,d6

	move.w  $ffff8240.w,-(sp)

	ifeq BMscRASTERIZE
	clr.w	$ffff8240.w
	else
	move.w  #$500,$ffff8240.w
	endc

	movem.l a0-a6,-(sp)						    ; 
    movem.w d0/d2/d5,-(sp)						; 

	move.l  sp,BMscsaveSP

	move.w  BMscWaitLoop,d0
	move.w  d0,d2
	lsr.w	#3,d2
.wait:		
	nop
	dbf.w	d2,.wait

	and.w	#7,d0
	lsl.w	d0,d0

	move.l	BMscPlayfieldClearAddress,a0	; 20
	move.w	#2,d7							; 8

	lea		$ffff8205.w,a1					; 8
	move.l	BMscFrontBuffer,d0				; 20
	movep.l d0,(a1)							; 24

	moveq.l	#0,d0							; 4

	ifeq BMscRASTERIZE
	clr.w	$ffff8240.w
	else
	move.w  #$20,$ffff8240.w
	endc

BMscBeginEnd:


;------------------------------------------------------------------------------
; leftBorderRemoval < 360 cycles < rightBorderRemoval < 104 cycles
;------------------------------------------------------------------------------

BMscClear:
	move.l	d0,(a0)					; 12
	move.l	d0,-BMscFieldWidth(a0)	; 16
	lea		BMscPitch(a0),a0		; 8 
BMscClearEnd:


BMscReloadReg:
	movem.l	BMscPreloadAddress,a0-a6		; 20 + 7 * 8	= 76
	movem.w	BMscPreloadData,d2/d5-d6		; 20 + 3 * 4	= 32
BMscReloadRegEnd:


BMscLeftBorderRemoval:
	move.b	d7,$FFFF8260.w		; d7 = 2
	move.w	d7,$FFFF8260.w
BMscLeftBorderRemovalEnd:


BMscSam1Sam2Sam1:
	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a6,d0.w),d0	; 16                a6 = y offset table
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					a4 = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16				can save 4 cycles on one if delta adr between curve 1 and 2 <= 32k
	or.w		d2,$0ABA(sp)	; 16
								; = 104

	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a4,d0.w),d0	; 16                a4 = y offset table2
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					sp = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16		
	or.w		d2,$0ABA(sp)	; 16
								; = 104

	ror.w       d6,d2			; 8 ?		        d6 = 1
	add.w		(a5)+,a2		; 12
								; = 20

	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a6,d0.w),d0	; 16                a6 = y offset table
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					sp = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16		
	or.w		d2,$0ABA(sp)	; 16
								; = 104
								
								; 104 + 104 + 104 + 20 = 320 ( < 360)

	; dcb.w	7,$4e71
BMscSam1Sam2Sam1End:


BMscSam2Sam1Sam2:
	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a4,d0.w),d0	; 16                a4 = y offset table2
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					sp = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16				
	or.w		d2,$0ABA(sp)	; 16
								; = 104

	ror.w       d6,d2			; 8 ?		        d6 = 1
	add.w		(a5)+,a2		; 12
								; = 20

	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a6,d0.w),d0	; 16                a6 = y offset table
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					sp = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16				
	or.w		d2,$0ABA(sp)	; 16
								; = 104

	move.b	    $0BBA(a0),d0	; 12	        	a0 = sample address - d0 = temp sample					
	and.w	    d5,d0			; 4                 d5 = $FE - scale x0.5 on y axis while computing adr vertical offset
	move.w	    (a4,d0.w),d0	; 16                a4 = y offset table2
	move.w	    d0,(a1)+		; 8                 a1 = clear offsets		
	move.l	    a2,sp			; 4					a2 = display base										
	add.w	    d0,sp			; 8					sp = temp pixel display address
	or.w		d2,$0ABA(sp)	; 16		        d2 = rolling pixel
	or.w		d2,$0ABA(sp)	; 16				
	or.w		d2,$0ABA(sp)	; 16

	ror.w       d6,d2			; 8 ?		        d6 = 1
	add.w		(a5)+,a2		; 12
								
								; 104 + 20 + 104 + 104 + 20 = 340 ( < 360)

	; dcb.w	2,$4e71
BMscSam2Sam1Sam2End:


BMscRightBorderColored:										        
	move.w	(a3)+,$ffff8242.w	; 16
	move.l	(a3)+,$ffff8242.w	; 24
	move.l	(a3)+,$ffff8242.w	; 24
	move.l	(a3)+,$ffff8242.w	; 24
								; = 88 cycles

	; dcb.w	4,$4e71				; + 16 = 104 cycles
BMscRightBorderColoredEnd:


BMscRightBorderRemoval:
	move.w	d7,$FFFF820A.w							        
	move.b	d7,$FFFF820A.w					        
BMscRightBorderRemovalEnd:						
						

BMscFillCurvesOffsetInit:
	move.w  #-8,d0			    ; 8
BMscFillCurvesOffsetInitEnd:

BMscFillCurvesOffset:
	move.w  d0,(a1)+		    ; 8
BMscFillCurvesOffsetEnd:


BMscEnd:
	move.l	BMscsaveSP,sp
    movem.w (sp)+,d0/d2/d5
	movem.l (sp)+,a0-a6

	move.w	(sp)+,$ffff8240.w
    
	move.w	(sp)+,d7
	move.w	(sp)+,d6

	rte
BMscEndEnd:


;---------------------------------------------
; C L E A R   R O U T I N E
;---------------------------------------------

BMscClearCurve1:
	move.l	a2,-(sp)
	move.l	d0,a2

	add.w	d2,d2		; d2 * 12
	add.w	d2,d2
	move.w	d2,d0
	add.w	d2,d2
	add.w	d0,d2

	jmp		*+4(pc,d2.w)

BMscClearCurve1End:

BMscClearCurve2:		; 12 bytes
	move.w	(a0)+,d0
	move.w	d1,$7A(a1,d0.w)		; opcode xxxx 007a
	move.w	(a0)+,d0
	move.w	d1,$7A(a2,d0.w)		; opcode xxxx 007a
BMscClearCurve2End:

BMscClearCurve3:
	move.l	(sp)+,a2
	rts
BMscClearCurve3End:
