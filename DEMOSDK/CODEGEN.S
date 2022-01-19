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

	output	D:\PROJECTS\DEMOS\OUTPUT\CODEGEN.O
	
	opt 	o+

	xdef	aCGENaddnops,aCGENpatchWords

aCGENaddnops:

    move.w  d1,-(sp)

    move.w  d0,d1               
    beq.s   .null

    move.l  d2,-(sp)
	move.l	#$4E714E71,d2

    lsr.w   #6,d1
	subq.w  #1,d1
    
    blt.s   .small    ; use fast fill loop (16 bytes / 16 bytes)

.loop:
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+
    move.l  d2,(a0)+

    dbf.w   d1,.loop

    and.w   #$3C,d0              ; if length modulo 16 = 0 => end
    beq.s   .end

.small:
	
	lsr.w   #1,d0
	neg.w   d0
	jmp		.end(pc,d0.w)

	rept 16
	move.w  d2,(a0)+
	endr

.end:
    move.l  (sp)+,d2

.null:

    move.w  (sp)+,d1
	
	rts
	
	

aCGENpatchWords: ; (void* _address, u16* _offsets, u16* => u32 _values, u16 _nbvalues)

	move.l	a2,-(sp)
	move.l	d0,a2

	add.w	d1,d1
	move.w  d1,d0
	add.w   d0,d0
	add.w	d1,d0
	neg.w	d0

	jmp		.end(pc,d0.w)

	rept	20
	move.w	(a1)+,d0
	move.w	(a2)+,(a0,d0.w)
	endr
.end

	move.l	(sp)+,a2
	
	rts

