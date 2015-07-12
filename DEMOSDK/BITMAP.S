*-------------------------------------------------------------------------
* Copyright J.Hubert 2015
*
* This file is part of demOS
*
* demOS is free software: you can redistribute it and/or modify it under the terms of 
* the GNU Lesser General Public License as published by the Free Software Foundation, 
* either version 3 of the License, or (at your option) any later version.
*
* demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with demOS.  
* If not, see <http://www.gnu.org/licenses/>.
*-------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\BITMAP.O
	
	opt 	o+

	xdef	BITpl2chunk;

;-------------------------------------------------------------------------------------
; void BITpl2chunk(void* _src, u16 _h, u16 _nbchunks, u16 _endlinepitch, void* _dst)
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
