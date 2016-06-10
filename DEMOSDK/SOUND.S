*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2016 J.Hubert
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

*--------------------------------------------------------------------------
* Defines ASM routines for system core functionnalities
* in particular implements the VBL interupts routine that manages:
* - soundtrack routine slicing
*--------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\SOUND.O
	
	opt 	o+

    xdef	SNDleftVolume,SNDrightVolume,SNDfademasterVolume,SNDdmaLoopCount,SNDmasterVolume
	xdef	SNDsourceTransfer,SNDdestTransfer,SNDendSourceTransfer,SNDlastDMAposition,SNDspeedfade
    xdef    SNDsoundtrackMonitor
    xdef    SNDplayNoise,SNDstopNoise
    

SND_TRANSFER_LEN:   set 16

*-------------------------------------------------------------------------
SNDsoundtrackMonitor:

	; ------------------------------------
	; things to do each time the DMA loops
	; ------------------------------------

	lea		$FFFF8907.w,a0
	movep.l	(a0),d0						; determine if DMA sound has looped during the frame
	and.l	#$FFFFFF,d0					; without using interruption (Timer A or screen)
    move.l  d0,a1

	cmp.l	SNDlastDMAposition(pc),d0	;
	slt.s	SNDhaslooped    			;
    bge.s   noloop

	addq.w	#1,SNDdmaLoopCount	        ;

    ; LEFT VOLUME

	moveq.l	#0,d0
	move.b	SNDleftVolume,d0
	blt.s	donotsetleft
	
    or.w	#$540,d0					; set volume left command
	or.b	#$80,SNDleftVolume

    move.w  #$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w				; send order to Microwire

donotsetleft:
noloop:									;

    move.l	a1,SNDlastDMAposition		; store current address for future compairison
	
	; ------------------------------------
	; now, things to do each frame
	; ------------------------------------

	; TRANSFER COMMAND

	move.l	SNDsourceTransfer(pc),d0
	cmp.l	SNDendSourceTransfer(pc),d0
	bge		notransfer

	movem.l	a1/d1-d7,-(sp)

	move.l	SNDdestTransfer(pc),a1
	move.l	d0,a0

var:	set 0
	rept SND_TRANSFER_LEN
	movem.l (a0)+,d0-d7
	movep.l	d0,var(a1) 
	movep.l	d1,var+8(a1) 
	movep.l	d2,var+16(a1) 
	movep.l	d3,var+24(a1) 
	movep.l	d4,var+32(a1) 
	movep.l	d5,var+40(a1) 
	movep.l	d6,var+48(a1) 
	movep.l	d7,var+56(a1) 
var:	set var+64
	endr

	lea		64*SND_TRANSFER_LEN(a1),a1
	move.l	a0,SNDsourceTransfer
	move.l	a1,SNDdestTransfer

	movem.l	(sp)+,a1/d1-d7

notransfer:

    tst.b   SNDhaslooped                ; avoid sending channel level orders and volume order in the same frame
    bne.s   managerightvolume          

	; MASTER VOLUME

	tst.b	SNDfademasterVolume		    ; master volume needs a fade ?
	beq.s	donotsetvolume
	blt.s	fadeout

	subq.b	#1,SNDfadecount
	bgt.s	donotsetvolume
	move.b	SNDspeedfade(pc),SNDfadecount

	moveq.l	#0,d0						; fade-in required
	move.b	SNDmasterVolume(pc),d0
	addq.b	#1,d0
	cmp.b	#41,d0
	bge.s	endsetvolume

	move.b	d0,SNDmasterVolume
	or.w	#$4C0,d0

    move.w	#$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w

	rts

fadeout:
	subq.b	#1,SNDfadecount
	bgt.s	donotsetvolume
	move.b	SNDspeedfade(pc),SNDfadecount

	moveq.l	#0,d0						; fade-out required
	move.b	SNDmasterVolume(pc),d0
	subq.b	#1,d0
	blt.s	endsetvolume

	move.b	d0,SNDmasterVolume
	or.w	#$4C0,d0

    move.w	#$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w

	rts

endsetvolume:
	clr.b	SNDfademasterVolume		    ; fade range limit reached

donotsetvolume:
    rts

   
managerightvolume:

	; RIGHT VOLUME

	moveq.l	#0,d0
	move.b	SNDrightVolume,d0
	blt.s	donotsetright

	or.w	#$500,d0					; set volume right command
	or.b	#$80,SNDrightVolume

waitmicrowire:                          ; wait in case we have sent an order for left channel
	cmp.w	#$7FF,$FFFF8924.w
	bne.s	waitmicrowire

    move.w  #$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w				; send order to Microwire

donotsetright:

	rts


SNDplayNoise:
    move.w           #$2700,sr

    move.b           #6,$ffff8800.w     ; select noise register
    move.b           d0,$ffff8802.w     ; data
    move.b           #8,$ffff8800.w     ; select level ch a register
    move.b           d1,$ffff8802.w     ; data
    move.b           #7,$ffff8800.w     ; select io and mixer register
    move.b           $ffff8802.w,d0
    and.b            #$C0,d0
    or.b             #$37,d0
    move.b           d0,$ffff8802.w
    
    move.w           #$2300,sr

    rts

SNDstopNoise:
    move.w           #$2700,sr

    move.b           #7,$ffff8800.w     ; select io and mixer register
    move.b           $ffff8802.w,d0
    and.b            #$C0,d0
    or.b             #$3F,d0
    move.b           d0,$ffff8802.w

    move.w           #$2300,sr

    rts



SNDleftVolume:			dc.b	-1
SNDrightVolume:		    dc.b	-1
SNDmasterVolume:		dc.b	40
SNDfademasterVolume:	dc.b	0
SNDspeedfade:			dc.b	25
SNDfadecount:			dc.b	0
SNDdmaLoopCount:		dc.w	0
SNDlastDMAposition:	    dc.l	0
SNDsourceTransfer:		dc.l	0
SNDdestTransfer:		dc.l	0
SNDendSourceTransfer:	dc.l	0
SNDhaslooped:           dc.b    0    

	END
