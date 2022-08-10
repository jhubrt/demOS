; -------------------------------------------------------------------------------------------------------
;  The MIT License (MIT)
; 
;  Copyright (c) 2015-2022 J.Hubert
; 
;  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
;  and associated documentation files (the "Software"), 
;  to deal in the Software without restriction, including without limitation the rights to use, 
;  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
;  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
; 
;  The above copyright notice and this permission notice shall be included in all copies 
;  or substantial portions of the Software.
; 
;  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
;  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
;  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
;  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
;  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
;  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; --------------------------------------------------------------------------------------------------------


	output	D:\PROJECTS\DEMOS\OUTPUT\YMDISP.O
	
	opt 	o+

    xdef    aYMgetState


HW_YM_REGSELECT:	set $FFFF8800
HW_YM_REGDATA:		set $FFFF8802

; --------------------------------------------------------
; void aYMgetState (u8 _regs[14])
; --------------------------------------------------------
aYMgetState:

	move.w	#$2700,sr

i:	set 0
	
	rept 14
	move.b	#i,HW_YM_REGSELECT.w
	move.b	HW_YM_REGSELECT.w,i(a0)
i:	set i+1
	endr

	move.w	#$2300,sr

	rts
