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

	output	D:\PROJECTS\DEMOS\OUTPUT\BPLAYER\BPLAYER.O
	
	opt 	o+

    xdef    adrawXorPass,adrawCurve


; --------------------------------------------------------
; void adrawXorPass (void* _screen)
; --------------------------------------------------------
adrawXorPass:

    movem.l d0-d1/a1-a2,-(sp)

    lea 160(a0),a1
    lea 68*160(a0),a2

    moveq.l #33,d1

.loop:  

offset:     set 0
    
    rept    40
    
    move.w  offset-160(a1),d0
    eor.w   d0,offset(a1)

    move.w  offset+160(a2),d0
    eor.w   d0,offset(a2)

offset:     set offset+4
    endr

    lea     160(a1),a1
    lea     -160(a2),a2

    dbf.w   d1,.loop

    movem.l (sp)+,d0-d1/a1-a2

    rts


; --------------------------------------------------------
; void adrawCurve (void* _sample, u16 _nbsamples, u16 _incx, void* _screen)
; --------------------------------------------------------
adrawCurve:

    movem.l d2-d4/a2,-(sp)

    lea 160*34(a1),a1
    lea table(pc),a2
    
    move.w  #$8000,d3

    lsr.w   #4,d0
    subq.w  #1,d0

    moveq.l #0,d4

loop:

offset:    set -33*160
    rept    65
    move.w  d4,offset(a1)
offset:    set offset+160
    endr

    rept    16
    move.b  (a0),d2
    ext.w   d2
    add.w   d2,d2
    move.w  (a2,d2.w),d2
    or.w    d3,(a1,d2.w)
    ror.w   #1,d3
    add.w   d1,a0
    endr

    addq.l  #4,a1

    dbf.w   d0,loop
    
    movem.l (sp)+,d2-d4/a2

    rts


offset:    set -32*160
    rept 32
    dc.w    offset
    dc.w    offset
    dc.w    offset
    dc.w    offset
offset:    set offset+160
    endr

table:

    rept 32
    dc.w    offset
    dc.w    offset
    dc.w    offset
    dc.w    offset
offset:    set offset+160
    endr
