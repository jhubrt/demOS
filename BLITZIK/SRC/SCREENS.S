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

	output	D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\SCREENS.O
	
	opt 	o+

    xdef    blitzvbl,blitz2vbl,blitzvpix,blitzvoffset,blitzvmode

    xref    aBLZupdate,aBLZ2update
    xref    aBLZbackground
    xref    aBLZvbl,aBLZ2vbl
    xref    SYSvblroutines

HW_DMASOUND_CONTROL:            set $FFFF8901

HW_DMASOUND_CONTROL_PLAYONCE:   set	1

; ------------------------------------------------------------------------
; vbl wrapper
; ------------------------------------------------------------------------
blitzvbl:
    move.l  a0,-(sp)
    
    lea     blitzvpix(pc),a0

    move.b  (a0)+,$ffff8265.w
    move.b  (a0)+,$ffff8264.w
    move.b  (a0)+,$ffff820f.w
    move.b  (a0)+,$ffff8260.w

    lea     aBLZbackground(pc),a0
    move.w  (a0)+,$ffff8240.w
    move.l  (a0)+,a0
    jsr     aBLZupdate

    move.l  #aBLZvbl,SYSvblroutines

    move.l  (sp)+,a0

    move.l  (a0)+,a1
    jmp     (a1)


blitz2vbl:
    move.b      #HW_DMASOUND_CONTROL_PLAYONCE,HW_DMASOUND_CONTROL.w

    move.l  a0,-(sp)
    
    lea     blitzvpix(pc),a0

    move.b  (a0)+,$ffff8265.w
    move.b  (a0)+,$ffff8264.w
    move.b  (a0)+,$ffff820f.w
    move.b  (a0)+,$ffff8260.w

    lea     aBLZbackground(pc),a0
    move.w  (a0)+,$ffff8240.w
    move.l  (a0)+,a0
    jsr     aBLZ2update

    move.l  #aBLZ2vbl,SYSvblroutines

    move.l  (sp)+,a0

    move.l  (a0)+,a1
    jmp     (a1)



blitzvpix:      dc.w    0
blitzvoffset:   dc.b    0
blitzvmode:     dc.b    0
