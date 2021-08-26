*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2021 J.Hubert
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

	output	D:\PROJECTS\DEMOS\OUTPUT\BITMAP.O
	
	opt 	o+

	xdef	BITpl2chunk

;-------------------------------------------------------------------------------------
; void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst)
;
; a0 = _src
; d0 = _h
; d1 = _nbchunks 
; d2 = _endlinepitch
; a1 = _dst
; if _endpitchline = 0, you can use a0 = a1 (same buffer)
;-------------------------------------------------------------------------------------
BITpl2chunk:

	movem.l	d3-d7,-(sp)
	move.w	d0,-(sp)

	subq.w	#1,d1

BIT_pl2chunk_loopy:

	move.w	d1,-(sp)

BIT_pl2chunk_loopx:

	moveq.l	#7,d3

BIT_pl2chunk_loopc:

	; pix1

	move.b	(a0)+,d0

	add.b	d0,d0
	addx.w	d4,d4

	add.b	d0,d0
	addx.w	d5,d5

	add.b	d0,d0
	addx.w	d6,d6

	add.b	d0,d0
	addx.w	d7,d7

	; pix2

	add.b	d0,d0
	addx.w	d4,d4

	add.b	d0,d0
	addx.w	d5,d5

	add.b	d0,d0
	addx.w	d6,d6

	add.b	d0,d0
	addx.w	d7,d7

	dbra	d3,BIT_pl2chunk_loopc

	move.w	d7,(a1)+
	move.w	d6,(a1)+
	move.w	d5,(a1)+
	move.w	d4,(a1)+

	dbra	d1,BIT_pl2chunk_loopx

	move.w	(sp)+,d1

	add.w	d2,a1

	subq.w	#1,(sp)
	bne.s	BIT_pl2chunk_loopy

	addq.l	#2,sp
	movem.l	(sp)+,d3-d7

	rts
