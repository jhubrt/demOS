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

	output	D:\PROJECTS\DEMOS\OUTPUT\RASTERS.O
	
	opt 	o+

*-------------------------------------------------------------------------
* Rasters routines
*-------------------------------------------------------------------------

	* void RASsetColReg (u16 shortregaddress);
	* void* (function pointers) RAStop1,RASmid1,RASbot1,RASlowB
	* void* RASNextOpList

	xdef	RASsetColReg
	xdef	RASvbl1,RASvbl2,RAStop1,RASmid1,RASbot1,RASlowB
	xdef    RASvbl15,RASmid15,RASlowB15
	xdef	RASnextOpList
    xdef    RASvbldonothing

RASsetColReg:
	move.w	d0,RAStop1_automod+2			* set the color register address that will be affected
	move.w	d0,RASmid1_automod+2
	move.w	d0,RASbot1_automod+2
	move.w	d0,RASvbl2_automod+2
	rts


	* -----------------------------------------
	* OpList for RASvbl1
	* [.w]	background color
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASvbl1:
    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w    

    move.l  RASnextOpList(pc),a1
	move.w	(a1)+,$ffff8240.w
	move.w	(a1)+,$fffffa20.w
	move.l	(a1)+,$120.w
	move.l	a1,usp

	or.b	#1,$fffffa07.w
	move.b	#8,$fffffa1b.w

    move.l  (a0)+,a1
    jmp     (a1)

	* -----------------------------------------
	* OpList for VBL routine
	* [.w]	background color
	* [.w]	color change
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASvbl2:
    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w
    
    move.l  RASnextOpList(pc),a1

	move.w	(a1)+,$ffff8240.w
RASvbl2_automod:
	move.w	(a1)+,$ffff8242.w
	move.w	(a1)+,$fffffa20.w
	move.l	(a1)+,$120.w
	move.l	a1,usp

	or.b	#1,$fffffa07.w
	move.b	#8,$fffffa1b.w

    move.l  (a0)+,a1
    jmp     (a1)

	* -----------------------------------------
	* OpList for VBL routine
	* [.w]	15 colors
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASvbl15:
    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w
    
    move.l  RASnextOpList(pc),a1

	move.l	(a1)+,$ffff8242.w
	move.l	(a1)+,$ffff8246.w
	move.l	(a1)+,$ffff824A.w
	move.l	(a1)+,$ffff824E.w
	move.l	(a1)+,$ffff8252.w
	move.l	(a1)+,$ffff8256.w
	move.l	(a1)+,$ffff825A.w
	move.w	(a1)+,$ffff825E.w

	move.w	(a1)+,$fffffa20.w
	move.l	(a1)+,$120.w
	move.l	a1,usp

	or.b	#1,$fffffa07.w
	move.b	#8,$fffffa1b.w

    move.l  (a0)+,a1
    jmp     (a1)

	* -----------------------------------------
	* OpList for RAStop1
	* [.w]	background color
	* [.w]	color change
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------

RAStop1:
	move.l	a0,-(sp)
	move.l	usp,a0
	move.w	(a0)+,$ffff8240.w
RAStop1_automod:
	move.w	(a0)+,$ffff8242.w
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASmid1
	* [.w]	color change
	* if color value < 0 (means last call)
	*		[.w]	scan lines count to next inter call (timerb data register)
	*		[.l]	next raster routine
	* -----------------------------------------

RASmid1:
	move.l	a0,-(sp)
	move.l	usp,a0
RASmid1_automod:
	move.w	(a0)+,$ffff8242.w
	blt.s	RASmid1_gonext			* short branches are faster when condition is not verified
	move.l	a0,usp
	move.l	(sp)+,a0
	rte
RASmid1_gonext:
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASmid15
	* [.w]	15 colors
	* if color value < 0 (means last call)
	*		[.w]	scan lines count to next inter call (timerb data register)
	*		[.l]	next raster routine
	* -----------------------------------------
RASmid15:
	move.l	a0,-(sp)
	move.l	usp,a0

	move.l	(a0)+,$ffff8242.w
	move.l	(a0)+,$ffff8246.w
	move.l	(a0)+,$ffff824A.w
	move.l	(a0)+,$ffff824E.w
	move.l	(a0)+,$ffff8252.w
	move.l	(a0)+,$ffff8256.w
	move.l	(a0)+,$ffff825A.w
	move.w	(a0)+,$ffff825E.w

	blt.s	RASmid2_gonext			* short branches are faster when condition is not verified
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

RASmid2_gonext:
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte


	* -----------------------------------------
	* OpList for RASbot1
	* [.w]	background color
	* [.w]	color change
	* -----------------------------------------

RASbot1:
	move.l	a0,-(sp)
	move.l	usp,a0
	move.w	(a0)+,$ffff8240.w
RASbot1_automod:
	move.w	(a0)+,$ffff8242.w
	move.l	a0,usp
	clr.b	$fffffa1b.w
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASlowB
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASlowB:
	clr.b	$ffff820a.w					; 60hz
	move.l	a0,-(sp)
	move.l	usp,a0
	move.b	#2,$ffff820a.w		        ; 50hz
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASlowB15
	* [.w]	15 colors
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASlowB15:
	clr.b	$ffff820a.w					; 60hz
	move.l	a0,-(sp)
	move.l	usp,a0
	move.b	#2,$ffff820a.w		        ; 50hz
	
	move.l	(a0)+,$ffff8242.w
	move.l	(a0)+,$ffff8246.w
	move.l	(a0)+,$ffff824A.w
	move.l	(a0)+,$ffff824E.w
	move.l	(a0)+,$ffff8252.w
	move.l	(a0)+,$ffff8256.w
	move.l	(a0)+,$ffff825A.w
	move.w	(a0)+,$ffff825E.w

	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

RASvbldonothing:
	
    and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w

    move.l  (a0)+,a1
    jmp     (a1)

RASnextOpList:	dc.l	0

	END
