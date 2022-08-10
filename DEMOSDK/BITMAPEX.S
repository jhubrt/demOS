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

	output	D:\PROJECTS\DEMOS\OUTPUT\BITMAPEX.O
	
	opt 	o+

	xdef	BIT1pUncompress

; -----------------------------------------------------------------------
; void BIT1pUncompress (void* _opcode, void* _wdata, u32 _dest);
;
; a0    opcodes + byte data list 
; a1    word data list
; d0    display dest
; -----------------------------------------------------------------------

; this routine is placed before to have all jmp routines(pc,... at less than -/+ 128 bytes...
uncompress_xxx: 
                add.w       (a1)+,a2                ;                       12          ; routine 2    => d0 low bits contain 4

                ror.b       #1,d0                   ;                       8
                jmp         .draw-2(pc,d0.w)        ;                       16          ; minus 2 to compensate the 2 coming from opcode 4 >> 1

.end:           movem.l     (sp)+,a2-a5
                rts

.draw:          bra.s       .end                    ;   use maxrepeat = 64 (=> jump = 0) case as end code => avoid additional test
                nop

                rept        62
                move.w      (a1)+,(a2)              ;                       12
                addq.l      #8,a2                   ;                       8
                endr
                move.w      (a1)+,(a2)              ;   63                  12

                ;-------------------
                move.b    	(a0)+,d0                ;   ooooorrr            8           ; interlace loop to avoid double jump
                moveq.l     #6,d1                   ;                       4
                and.w       d0,d1                   ;   rr0                 4
                jmp         routines(pc,d1.w)       ;                       16


BIT1pUncompress:
                movem.l     a2-a5,-(sp)

                move.l      d0,a2

                lea         uncompress_x0(pc),a3
                lea         uncompress_xx(pc),a4
                lea         uncompress_xxx(pc),a5

                moveq.l     #$FFFFFFF8,d2
                moveq.l     #0,d0
                moveq.l     #0,d1

; loop ---------------------------------------------------
                move.b    	(a0)+,d0                ;   ooooorrr            8
                moveq.l     #6,d1                   ;                       4
                and.w       d0,d1                   ;   rr0                 4
                jmp         routines(pc,d1.w)       ;                       16
 
routines:       jmp         (a3)                    ; 8
                jmp         (a4)                    ; 8
                jmp         (a5)                    ; 8

;-------------
uncompress_0x:  and.w       d2,d0                   ;                       4           ; routine 3     => mask d0 low bits (contains 6)
                add.l       d0,a2                   ;                       8           
                move.b      (a0)+,1(a2)             ;                       12

                ;-------------------
                move.b    	(a0)+,d0                ;   ooooorrr            8           ; interlace loop to avoid double jump
                moveq.l     #6,d1                   ;                       4
                and.w       d0,d1                   ;   rr0                 4
                jmp         routines(pc,d1.w)       ;                       16

;-------------------------------------------
uncompress_x0:  add.l       d0,a2                   ;                       8           ; routine 0     => d0 low bits contain 0
                move.b      (a0)+,(a2)              ;                       12

                ;-------------------
                move.b    	(a0)+,d0                ;   ooooorrr            8           ; interlace loop to avoid double jump
                moveq.l     #6,d1                   ;                       4
                and.w       d0,d1                   ;   rr0                 4
                jmp         routines(pc,d1.w)       ;                       16
                
;-------------------------------------------
uncompress_xx:  add.l       d0,a2                   ;                       8           ; routine 1     => d0 low bits contain 2 (compensate with -(a2))
                move.w      (a1)+,-(a2)             ;                       12

                ;-------------------
                move.b    	(a0)+,d0                ;   ooooorrr            8           ; interlace loop to avoid double jump
                moveq.l     #6,d1                   ;                       4
                and.w       d0,d1                   ;   rr0                 4
                jmp         routines(pc,d1.w)       ;                       16

