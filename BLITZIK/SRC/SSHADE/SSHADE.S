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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\SSHADE.O

	opt	o+

    xdef SShadeSet,SShadeColorTableInit
    xref COLST24b,COL4b2ST


VIS_NBSTARTCOLORS   set 64


; static u16* SShadeSet(u16* p, u16* p2, u16 len, s16 inc, u16 mask)

SShadeSet:

    move.w  d3,-(sp)

    move.w  #$AAAA,d3

    add.w   d0,d0
    add.w   d0,d0
    add.w   d0,d0
    neg.w   d0

    jmp .end(pc,d0.w)

    rept    15
    move.w  d3,(a0)
    move.w  d3,(a1)
    add.w   d1,a0
    add.w   d1,a1
    endr
.end:

    sub.w   d1,a0
    sub.w   d1,a1
    
    and.w   d2,(a0)
    and.w   d2,(a1)

    move.w  (sp)+,d3

    rts


; void SShadeColorTableInit(void* g_screens.sshadeStatic.colors, void* startcolors, u32 this->opAdd, u32 count)

SShadeColorTableInit:

    movem.l     d2-d7/a0-a5,-(sp)

    move.l      d0,a2                             ; a2 = opAdd
    move.l      d1,a4                             ; a4 = count[]
    lea         COLST24b,a5
    lea         COL4b2ST,a3

    move.w      #VIS_NBSTARTCOLORS,-(sp)
    move.w      #$FFE,d2    

.loopcolors:                                      ; for (i = 0; i < VIS_NBSTARTCOLORS; i++)                                                  

    move.w      (a0)+,d3                          ; s16 lastColor = g_screens.sshadeStatic.colors[i];
    move.w      d3,d7                             ; d3 = lastColor, duplicate into d7

    moveq.l     #15,d4
    moveq.l     #15,d5
    moveq.l     #15,d6

    and.w       d7,d6                             ; d6 = cb
    lsr.w       #4,d7
    and.w       d7,d5                             ; d5 = cg  
    lsr.w       #4,d7
    and.w       d7,d4                             ; d4 = cr

    moveq.l     #0,d7
    move.b      (a5,d6.w),d7                      ; u16 cb = COLST24b[lastColor & 0xF];
    lsl.w       #8,d7
    or.b        (a5,d4.w),d7                      ; u16 cr = COLST24b[(lastColor >> 8) & 0xF];
    move.b      (a5,d5.w),d5                      ; u16 cg = COLST24b[(lastColor >> 4) & 0xF];

    move.w      d7,d0                             ; backup cr,cg,cb into d0,d1
    move.w      d5,d1
                                                  
    move.w      (a1)+,d7                          ; branch start color on gradient 1st color 
    move.w      d3,(a2,d7.w)                      ; *(u16*)(this->opAdd + (s16)this->startcolors[i]) = lastColor;

    moveq.l     #15,d7

.loopgradient:                                    ; for (t = 16; t > 0;) t--;

    move.w      d0,d4                             ; d4 = cb__cr
    move.w      d1,d5                             ; d5 = ____cg

    mulu.w      d7,d4                             ; u16 dr = (cr * t) >> 4;
    lsr.w       #4,d4                             ; u16 db = (cb * t) >> 4;

    mulu.w      d7,d5                             ; u16 dg = (cg * t) >> 4;
    lsr.w       #4,d5                             ; ____cg in 4b
    move.b      (a3,d5.w),d5                      ; ____cg in STe
    lsl.w       #4,d5                             ; __cg__ in STe

    moveq.l     #15,d6                            ;
    and.w       d4,d6                             ; d6 = cr 
    move.b      (a3,d6.w),d6                      ; cr from 4b to STe
          
    lsl.w       #8,d6                             ; d6 = cr____ in STe
    lsr.w       #8,d4                             ; d4 = ____cb in 4b 
    move.b      (a3,d4.w),d6                      ; d6 = cr__cb in STe

    or.b        d5,d6                             ; d6 = crcgcb         u16 nextColor = ((u16)COL4b2ST[dr] << 8) | ((u16)COL4b2ST[dg] << 4) | (u16)COL4b2ST[db];

    and.w       d2,d6                             ; nextColor &= 0xFFE;
    beq.s       .zerocolor                        ; if (nextColor != 0)

    move.w      d6,d5                            ; u16 c = count[nextColor];
    or.w        (a4,d6.w),d6                     ; count[nextColor]++;
    add.w       #4096,(a4,d5.w)                  ; nextColor |= c << 12;
.zerocolor:
    
    move.w      d6,(a2,d3.w)                     ; *(u16*)(this->opAdd + lastColor) = nextColor;
    move.w      d6,d3                            ; lastColor = nextColor;

    dbf.w       d7,.loopgradient

    clr.w      (a2,d3.w)                         ; *(u16*)(this->opAdd + lastColor) = 0;
    
    subq.w     #1,(sp)
    bne.s      .loopcolors 

    clr.w      (a2)                              ; this->opAdd[0] = 0;     /* block at 0 */

    addq.l     #2,sp

    movem.l    (sp)+,d2-d7/a0-a5

    rts
