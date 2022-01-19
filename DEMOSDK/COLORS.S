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

	output	D:\PROJECTS\DEMOS\OUTPUT\COLORS.O
	
	opt 	o+

    xref    STDmcpy2,COL4b2ST,COLST24b

    xdef    COL16STST,COL164bST,COL164b4b

    
;--------------------------------------------------------------------------------------------------
; u16* COL164b4b (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)

COL164b4b:

    ; a0 = _startColors
    ; a1 = _endcolors
    ; d0 = _nbcolors
    ; d1 = _step
    ; d2 = _destColors

    ; switch (_step)

    tst.w   d1                  ; case 0:  
    beq     copystart

    cmp.w   #16,d1              ; case 16: 
    beq     copyend

    ; default:        
    ; ASSERT(_step < 16);

    movem.l d3-d7/a2,-(sp)

    move.l  d2,a2
    moveq.l #4,d2
    subq.w  #1,d0

.loop:                      ; for (n = 0; n < _nbColors; n++)

    move.w  (a0)+,d3        ; u16  sc = *scp++
    move.w  (a1)+,d4        ; u16  ec = *ecp++

    moveq.l #15,d5          ; s16 bs = COLST24b[sc & 0xF];
    moveq.l #15,d6          ; s16 be = COLST24b[ec & 0xF];

    and.w   d3,d5           ; d5 = sc & 0xF
    and.w   d4,d6           ; d6 = ec & 0xF
    sub.w   d5,d6           ; be - bs

    muls.w  d1,d6           ; ((be - bs) * _step)
    asr.w   d2,d6
    add.w   d5,d6           ; bs += ((be - bs) * _step) >> 4;

    moveq.l #0,d7
    move.b  d6,d7

    ;------------------------
    lsr.w   d2,d3           ; s16 gs = COLST24b[(sc >> 4) & 15];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6
    sub.w   d5,d6

    muls.w  d1,d6    
    asr.w   d2,d6
    add.w   d5,d6

    lsl.w   d2,d6
    or.b    d6,d7

    ;------------------------
    lsr.w   d2,d3           ; s16 rs =  COLST24b[sc1 & 0xF];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6
    sub.w   d5,d6

    muls.w  d1,d6           ; rs += ((re - rs) * _step) >> 4;
    asr.w   d2,d6
    add.w   d5,d6

    lsl.w   #8,d6
    or.w    d6,d7

    move.w  d7,(a2)+        ; *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];

    dbf.w   d0,.loop

    move.l  a2,a0

    movem.l (sp)+,d3-d7/a2

    rts
    


;--------------------------------------------------------------------------------------------------
; u16* COL164bST (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)

COL164bST:

    ; a0 = _startColors
    ; a1 = _endcolors
    ; d0 = _nbcolors
    ; d1 = _step
    ; d2 = _destColors

    ; default:        
    ; ASSERT(_step < 16);

    movem.l d3-d7/a2/a4,-(sp)

    move.l  d2,a2
    moveq.l #4,d2
    lea     COL4b2ST,a4
    subq.w  #1,d0

.loop:                      ; for (n = 0; n < _nbColors; n++)

    move.w  (a0)+,d3        ; u16  sc = *scp++
    move.w  (a1)+,d4        ; u16  ec = *ecp++

    moveq.l #15,d5          ; s16 bs = COLST24b[sc & 0xF];
    moveq.l #15,d6          ; s16 be = COLST24b[ec & 0xF];

    and.w   d3,d5           ; d5 = sc & 0xF
    and.w   d4,d6           ; d6 = ec & 0xF
    sub.w   d5,d6

    muls.w  d1,d6           ; ((be - bs) * _step)
    asr.w   d2,d6
    add.w   d5,d6           ; bs += ((be - bs) * _step) >> 4;

    moveq.l #0,d7
    move.b  (a4,d6.w),d7

    ;------------------------
    lsr.w   d2,d3           ; s16 gs = COLST24b[(sc >> 4) & 15];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6
    sub.w   d5,d6

    muls.w  d1,d6    
    asr.w   d2,d6
    add.w   d5,d6
    move.b  (a4,d6.w),d6

    lsl.w   d2,d6
    or.b    d6,d7

    ;------------------------
    lsr.w   d2,d3           ; s16 rs =  COLST24b[sc1 & 0xF];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6
    sub.w   d5,d6

    muls.w  d1,d6           ; rs += ((re - rs) * _step) >> 4;
    asr.w   d2,d6
    add.w   d5,d6
    move.b  (a4,d6.w),d6

    lsl.w   #8,d6
    or.w    d6,d7

    move.w  d7,(a2)+        ; *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];

    dbf.w   d0,.loop

    move.l  a2,a0

    movem.l (sp)+,d3-d7/a2/a4

    rts


;--------------------------------------------------------------------------------------------------
; u16* COL16STST (u16* _startColors, u16* _endcolors, u16 _nbColors, s16 _step, u16* _destColors)

COL16STST:

    ; a0 = _startColors
    ; a1 = _endcolors
    ; d0 = _nbcolors
    ; d1 = _step
    ; d2 = _destColors

    ; switch (_step)

    tst.w   d1                  ; case 0:  
    beq     copystart

    cmp.w   #16,d1              ; case 16: 
    beq     copyend

    ; default:        
    ; ASSERT(_step < 16);

    movem.l d3-d7/a2-a4,-(sp)

    move.l  d2,a2
    moveq.l #4,d2
    lea     COLST24b,a3
    lea     COL4b2ST,a4
    subq.w  #1,d0

.loop:                      ; for (n = 0; n < _nbColors; n++)

    move.w  (a0)+,d3        ; u16  sc = *scp++
    move.w  (a1)+,d4        ; u16  ec = *ecp++

    moveq.l #15,d5          ; s16 bs = COLST24b[sc & 0xF];
    moveq.l #15,d6          ; s16 be = COLST24b[ec & 0xF];

    and.w   d3,d5           ; d5 = sc & 0xF
    and.w   d4,d6           ; d6 = ec & 0xF

    move.b  (a3,d6.w),d6    ; COLST24b[ec & 0xF];
    move.b  (a3,d5.w),d5    ; COLST24b[sc & 0xF];
    sub.w   d5,d6

    muls.w  d1,d6           ; ((be - bs) * _step)
    asr.w   d2,d6
    add.w   d5,d6           ; bs += ((be - bs) * _step) >> 4;

    moveq.l #0,d7
    move.b  (a4,d6.w),d7

    ;------------------------
    lsr.w   d2,d3           ; s16 gs = COLST24b[(sc >> 4) & 15];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6

    move.b  (a3,d6.w),d6    ; gs += ((ge - gs) * _step) >> 4;
    move.b  (a3,d5.w),d5    ; COLST24b[(sc >> 4) & 0xF];
    sub.w   d5,d6

    muls.w  d1,d6    
    asr.w   d2,d6
    add.w   d5,d6
    move.b  (a4,d6.w),d6

    lsl.w   d2,d6
    or.b    d6,d7

    ;------------------------
    lsr.w   d2,d3           ; s16 rs =  COLST24b[sc1 & 0xF];
    lsr.w   d2,d4

    moveq.l #15,d5
    moveq.l #15,d6

    and.w   d3,d5
    and.w   d4,d6

    move.b  (a3,d6.w),d6
    move.b  (a3,d5.w),d5    ; COLST24b[sc >> 8];
    sub.w   d5,d6

    muls.w  d1,d6           ; rs += ((re - rs) * _step) >> 4;
    asr.w   d2,d6
    add.w   d5,d6
    move.b  (a4,d6.w),d6

    lsl.w   #8,d6
    or.w    d6,d7

    move.w  d7,(a2)+        ; *_destColors++ = (COL4b2ST[rs] << 8) | (COL4b2ST[gs] << 4) | COL4b2ST[bs];

    dbf.w   d0,.loop

    move.l  a2,a0

    movem.l (sp)+,d3-d7/a2-a4

    rts
    
    
copystart:

    ; if (_destColors != _startColors)
    ;   STDmcpy2 (_destColors, _startColors, _nbColors << 1);
    ; _destColors += _nbColors;

    cmp.l   a0,d2
    beq.s   donothing

    move.l  a0,a1
    move.l  d2,a0
    add.w   d0,d0
    neg.w   d0
    jmp     copy(pc,d0.w)

    rept    17
    move.w  (a1)+,(a0)+
    endr
copy:
    rts
    
donothing:
    move.l  d2,a0
    add.w   d0,d0
    add.w   d0,a0
    rts

copyend:

    ; if (_destColors != _endcolors)
    ;     STDmcpy2 (_destColors, _endcolors, _nbColors << 1); 
    ; _destColors += _nbColors;

    cmp.l   a1,d2
    beq.s   donothing

    move.l  d2,a0
    add.w   d0,d0
    neg.w   d0

    jmp     copy(pc,d0.w)

