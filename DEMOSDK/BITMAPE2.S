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

	output	D:\PROJECTS\DEMOS\OUTPUT\BITMAPE2.O
	
	opt 	o+

	xdef	BIT1pRLEUncompress
				
; -----------------------------------------------------------------------
; void BIT1pRLEUncompress (void* _wdata, void* _dest);
;
; a0    word data list
; a1    display dest
; -----------------------------------------------------------------------				
end:    
    rts

BIT1pRLEUncompress:
	move.w		(a0)+,d0
	beq.s		end
	bgt.s		.loop

	sub.w		d0,a1
	move.w		(a0)+,d0

.loop:
    jmp         .copy(pc,d0.w)

.copy:
    rept        4096
	move.w		(a0)+,(a1)
	addq.l		#8,a1
    endr
	
	move.w		(a0)+,d0
	beq.s		.end2
	
	sub.w		d0,a1
	move.w		(a0)+,d0
	
	bra 		.loop

.end2:	
	rts
