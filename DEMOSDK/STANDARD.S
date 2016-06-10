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

	output	D:\PROJECTS\DEMOS\OUTPUT\STANDARD.O
	
	opt 	o+

	xdef	STDmset,STDmcpy,STDmfrnd,STDifrnd,STDmulu,STDmuls,STDdivu,STDdivs
	xdef	STDcpuSetSR,STDgetUSP,STDsetUSP,STDgetSP


;-------------------------------------------------------------------------
; u16 STDmfrnd ();
;-------------------------------------------------------------------------
STDmfrnd:
		move.l	#$12345678,d0
        add.l	#$87654321,STDmfrnd+2		; initial seeds (<> 0)
        move.l	d0,STDmfrnd+8
        swap	d0
        rts  

;-------------------------------------------------------------------------
; u16 STDifrnd (); => same function but for for idle thread
;-------------------------------------------------------------------------
STDifrnd:
		move.l	#$12345678,d0
        add.l	#$87654321,STDifrnd+2		; initial seeds (<> 0)
        move.l	d0,STDifrnd+8
        swap	d0
        rts  

;-------------------------------------------------------------------------
; u32 STDmulu (u16 _x, u16 _y);
;-------------------------------------------------------------------------
STDmulu:
        mulu.w  d1,d0
        rts

;-------------------------------------------------------------------------
; s32 STDmuls (s16 _x, s16 _y);
;-------------------------------------------------------------------------
STDmuls:
        muls.w  d1,d0
        rts

;-------------------------------------------------------------------------
; u32 STDdivu (u32 _x, u16 _y);
;-------------------------------------------------------------------------
STDdivu:    
        divu.w  d1,d0
        rts

;-------------------------------------------------------------------------
; u32 STDdivs (s32 _x, s16 _y);
;-------------------------------------------------------------------------
STDdivs:    
        divs.w  d1,d0
        rts


;-------------------------------------------------------------------------
; void STDmset (void* _adr, u32 _value, u32 _length);
; do not use a lot of register like functions from stdlib 
; (better when using interuptions that do not restore all registers...)
; u32 value should be 4 bytes with the same value 
; (speed up the function not preparing the pattern systematically)
; else result is undetermined
;-------------------------------------------------------------------------

STDmset:

    tst.l   d1                  ; test if length is null => do nothing
    bne.s   STD_mset_exec
    rts

STD_mset_exec:

    move.l  d2,-(sp)

    move.w  a0,d2               ; is dest address odd ?
	btst.l	#0,d2
    beq.s   STD_mset_notodd

    move.b  d0,(a0)+            ; if so fill the byte
    subq.l  #1,d1               ; if length was 1 => end

    beq.s   STD_mset_end

STD_mset_notodd:

    move.l  d1,d2               ; fill the rest
    lsr.l   #4,d2               ; if length > 15
    
    beq.s   STD_mset_nolarge    ; use fast fill loop (16 bytes / 16 bytes)

STD_mset_loop:
    move.l  d0,(a0)+
    move.l  d0,(a0)+
    move.l  d0,(a0)+
    move.l  d0,(a0)+

    subq.l  #1,d2
    bne.s   STD_mset_loop

    and.w   #$F,d1              ; if length modulo 16 = 0 => end
    beq.s   STD_mset_end

STD_mset_nolarge:
    subq.w  #1,d1               

STD_mset_loop2:
    move.b  d0,(a0)+            ; finish the last bytes
    dbra.w  d1,STD_mset_loop2

STD_mset_end:

    move.l  (sp)+,d2

    rts


;-------------------------------------------------------------------------
; void STDmcpy (void* _dest, void* _source, u32 _length);
; do not use a lot of register like functions from stdlib 
; (better when using interuptions that do not restore all registers...)
;-------------------------------------------------------------------------
STDmcpy:

    tst.l   d0                  ; test if length is null => do nothing
    bne.s   STD_mcpy_exec
    rts

STD_mcpy_exec:

    move.l  d1,-(sp)

    move.w  a0,d1
    add.w   a1,d1
    and.w   #1,d1
    bne.s   STD_mcpy_exec_memshifted

    move.w  a0,d1               ; is dest address odd ?
    and.l   #1,d1

    beq.s   STD_mcpy_notodd

    move.b  (a1)+,(a0)+            ; if so fill the byte
    subq.l  #1,d0                  ; if length was 1 => end

    beq.s   STD_mcpy_end

STD_mcpy_notodd:

    move.l  d0,d1               ; fill the rest
    lsr.l   #4,d1               ; if length > 15
    
    beq.s   STD_mcpy_nolarge    ; use fast fill loop (16 bytes / 16 bytes)

STD_mcpy_loop:
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+
    move.l  (a1)+,(a0)+

    subq.l  #1,d1
    bne.s   STD_mcpy_loop

    and.w   #$F,d0              ; if length modulo 16 = 0 => end
    beq.s   STD_mcpy_end

STD_mcpy_nolarge:
    subq.w  #1,d0               

STD_mcpy_loop2:
    move.b  (a1)+,(a0)+         ; finish the last bytes
    dbra.w  d0,STD_mcpy_loop2

STD_mcpy_end:

    move.l  (sp)+,d1

    rts

STD_mcpy_exec_memshifted:       ; unaligned => slow method (ok in our context => if I need fast cpy, just align...)
    move.b  (a1)+,(a0)+
    subq.l  #1,d0
    bne.s   STD_mcpy_exec_memshifted
    
    move.l  (sp)+,d1

*-------------------------------------------------------------------------
* void STDcpuSetSR (u16 _status);
*-------------------------------------------------------------------------
STDcpuSetSR:
	move.w d0,sr
	rts

*-------------------------------------------------------------------------
* void* STDgetSP (void);
*-------------------------------------------------------------------------
STDgetSP:
	move.l	sp,a0
	rts

*-------------------------------------------------------------------------
* void* STDgetUSP (void);
*-------------------------------------------------------------------------
STDgetUSP:
	move.l	usp,a0
	rts

*-------------------------------------------------------------------------
* void STDsetUSP (void*);
*-------------------------------------------------------------------------
STDsetUSP:
	move.l	a0,usp
	rts
