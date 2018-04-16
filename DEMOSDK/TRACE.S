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

    xdef    TRAmaxraster

;---------------------------------------
; u16 TRAmaxraster (u16 maxraster)
;---------------------------------------
TRAmaxraster:
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

