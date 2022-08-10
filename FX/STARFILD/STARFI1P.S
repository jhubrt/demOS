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

	output	D:\PROJECTS\DEMOS\OUTPUT\FX\STARFI1P.O

    xdef STARiniField1P,STARdraw1P,STARerase1P
    xdef STARiniField1P,STARdraw1P,STARerase1P


;--------------------------------------------------------------------------------------------------------
; void STARiniField1P (STARparam* _param)

; struct STARparam_

STAR_ZMAX:          set 127
STAR_INCMAX:        set 3
STAR_ZSHIFT:        set 7
STAR_PERSPEC:       set 32
STAR_PERSPEC_SHIFT: set 5
STAR_ACTIVATE_COPY: set 1
STAR_WIDTH:         set 336


presx:          set 0
presy:          set presx+4
rndx:           set presy+4
rndy:           set rndx+4

pixdrawoffset:  set rndy+4
starx:          set pixdrawoffset+4
stary:          set starx+4
starz:          set stary+4
erasebuffers:   set starz+4

pitch:          set erasebuffers+8
nbstars:        set pitch+2
pitchmul:       set nbstars+2

copyposx:       set pitchmul+4
copyposy:       set copyposx+4

xval:           set copyposy+4

inc:            set xval+4
height:         set inc+2

currentrndx:    set height+2
currentrndy:    set currentrndx+4

;--------------------------------------------------------------------------------------------------------

STARiniField1P:
    
    movem.l d0-d7/a2-a6,-(sp)

    move.l  a1,-(sp)

    move.l  xval(a0),a3                 ; a3 = xval
    move.w  #STAR_WIDTH,d6              ; d6 = width
    moveq.l #0,d3

.initx:                                 ; for (i = 0; i < STAR_WIDTH; i++)

    moveq.l #7,d4                       ; u8 bitshift = 7 - (i & 7);
    and.w   d3,d4
    neg.w   d4
    addq.w  #7,d4   
    move.b  d4,(a3)+

    moveq.l #$FFFFFFF0,d4               ; u8 offset   = (i & 0xFFF0) >> 1;
    and.w   d3,d4
    lsr.w   #1,d4

    btst.l  #3,d3                       ; if ((i & 15) >= 8)
    sne.b   d5                          ; offset++;
    sub.b   d5,d4
    move.b  d4,(a3)+                    ; _param->xval[i] = (bitshift << 8) | offset;

    addq.w  #1,d3
    cmp.w   d3,d6
    bne.s   .initx

    ; main init loop

    moveq.l #0,d3                      ; d3 = i

.loopz:                                ; for (i = 0 ; i < (STAR_ZMAX - 1) ; i++)    
   
    ; ------------------------

    move.w  d3,d4
    add.w   d4,d4

    move.l  presx(a0),a3                ; s16* prex = &(_param->prex[i]);
    move.l  presy(a0),a4                ; s16* prey = &(_param->prey[i]);

    add.w   d4,a3
    add.w   d4,a4
        
    ifne STAR_ACTIVATE_COPY          
    move.l  copyposx(a0),a5             ; copyposx = &(_param->copyposx[i]);
    move.l  copyposy(a0),a6             ; copyposy = &(_param->copyposy[i]);
    add.w   d4,a5
    add.w   d3,a6
    endc

    move.l  pitchmul(a0),a1             

    ; ------------------------
    move.w  height(a0),d7
    lsr.w   #1,d7

    move.w  d7,d6
    subq.w  #1,d6

    move.l  (sp),a2
    add.w   d7,a2
    addq.w  #1,a2

.loopy1:                               ; for (t = STAR_HEIGHT >> 1 ; t > 0; t--) 
    move.w  d7,d4
    sub.b   -(a2),d4                   ; s16 y = -(s16) _param->temp[t] + (STAR_HEIGHT >> 1);
    
    ifne    STAR_ACTIVATE_COPY
    move.b  d4,(a6)                    ; *copyposy = (u8)y;
    add.w   #1<<STAR_ZSHIFT,a6          ; copyposy += 1L << STAR_ZSHIFT;
    endc

    add.w   d4,d4                      
    move.w  (a1,d4.w),(a4)             ; *prey = _param->pitchmul[y];
    add.w   #2<<STAR_ZSHIFT,a4          ; prey += 1L << STAR_ZSHIFT;

    dbf    d6,.loopy1

    ; -------------

    subq.w  #1,a2

    move.w  d7,d6
    subq.w  #1,d6
       
.loopy2:                                ; for (t = 0 ; t < (STAR_HEIGHT >> 1) ; t++)
    move.w  d7,d4
    add.b   (a2)+,d4                    ; s16 y = (s16) _param->temp [t] + (STAR_HEIGHT >> 1);

    ifne    STAR_ACTIVATE_COPY
    move.b  d4,(a6)                     ; *copyposy = (u8)y;
    add.w   #1<<STAR_ZSHIFT,a6           ; copyposy += 1L << STAR_ZSHIFT;
    endc                               

    add.w   d4,d4
    move.w  (a1,d4.w),(a4)              ; *prey = _param->pitchmul[y];
    add.w   #2<<STAR_ZSHIFT,a4           ; prey += 1L << STAR_ZSHIFT;
                                       
    dbf    d6,.loopy2

    ;--------------------------------

    move.w  #STAR_WIDTH/2,d7

    move.l  (sp),a2
    add.w   d7,a2
    addq.w  #1,a2

    move.w  d7,d6
    subq.w  #1,d6

    move.l  xval(a0),a1
    moveq.l #0,d5

.loopx1:                                ; for (t = STAR_WIDTH >> 1 ; t > 0 ; t--)

    move.b  -(a2),d5                    ; s16 x = -(s16) _param->temp[t] + (STAR_WIDTH >> 1);
    move.w  d7,d4
    sub.w   d5,d4

    ifne    STAR_ACTIVATE_COPY
    move.w  d4,(a5)                     ; *copyposx = x;
    add.w   #2<<STAR_ZSHIFT,a5          ; copyposx += 1L << STAR_ZSHIFT;
    endc

    add.w   d4,d4
    move.w  (a1,d4.w),(a3)              ; *prex = _param->xval[x];
    add.w   #2<<STAR_ZSHIFT,a3          ; prex += 1L << STAR_ZSHIFT;

    dbf     d6,.loopx1

    ; ---------------
 
    subq.w  #1,a2

    move.w  d7,d6
    subq.w  #1,d6

.loopx2:                                ; for (t = 0 ; t < (STAR_WIDTH >> 1) ; t++)

    move.b  (a2)+,d5                    ; s16 x = (s16) _param->temp[t] + (STAR_WIDTH >> 1);
    move.w  d7,d4
    add.w   d5,d4

    ifne    STAR_ACTIVATE_COPY
    move.w  d4,(a5)                     ; *copyposx = x;
    add.w   #2<<STAR_ZSHIFT,a5          ; copyposx += 1L << STAR_ZSHIFT;
    endc

    add.w   d4,d4
    move.w  (a1,d4.w),(a3)              ; *prex = _param->xval[x];
    add.w   #2<<STAR_ZSHIFT,a3          ; prex += 1L << STAR_ZSHIFT;

    dbf     d6,.loopx2

    ;------------------

    add.l   #(STAR_WIDTH/2)+1,(sp)

    addq.w  #1,d3
    cmp.w   #STAR_ZMAX-STAR_INCMAX+1,d3
    bne     .loopz

    ; ---------------------------------------
    
    moveq.l #STAR_ZMAX-STAR_INCMAX+1,d3            ; d3 = i

    ; main init loop
    
.loopzend:                              ; for (i = (STAR_ZMAX - 1) ; i <= STAR_ZMAX ; i++)

    move.l  presx(a0),a3                ; s16* prex = &(_param->prex[i]);
    move.l  presy(a0),a4                ; s16* prey = &(_param->prey[i]);

    move.w  d3,d4
    add.w   d4,d4

    add.w   d4,a3
    add.w   d4,a4

    move.w  height(a0),d6
    subq.w  #1,d6

    moveq.l #0,d4
    move.w  #$8000,d5

.loophend:                              ; for (t = 0; t < STAR_HEIGHT; t++)
    move.w  d4,(a4)                     ; *prey = 0;
    add.w   #2<<STAR_ZSHIFT,a4          ; prey += 1L << STAR_ZSHIFT;
    dbf     d6,.loophend

    move.w  #STAR_WIDTH-1,d6

.loopxend:                              ; for (t = 0; t < STAR_WIDTH; t++)
    move.w  d5,(a3)                     ; *prex = 0x8000;
    add.w   #2<<STAR_ZSHIFT,a3           ; prex += 1L << STAR_ZSHIFT;
    dbf     d6,.loopxend

    ;--------------------------------

    addq.w  #1,d3
    cmp.w   #STAR_ZMAX+1,d3
    bne.s   .loopzend
    
    ;--------------------------------

    addq.l  #4,sp

    movem.l (sp)+,d0-d7/a2-a6
   
    rts


;--------------------------------------------------------------------------------------------------------
; void STARdraw1P (void* _framebuffer, u32* z, u16 _nbstars, u32 _erasebuffer, u32 _param)
;--------------------------------------------------------------------------------------------------------
; a0    _framebuffer
; a1    z
; d0    _nbstars
; d1    _erasebuffer
; d2    _param
;--------------------------------------------------------------------------------------------------------
STARdraw1P:

        movem.l a2-a6/d3-d7,-(sp)

        move.l  d1,a2                       ; a2   erasebuffer => d1 free
        move.l  d2,a3                       ; a3   param       => d2 free
       
        move.l  a3,-(sp)

        moveq.l #0,d1        
        move.w  inc(a3),d1
        move.l  currentrndx(a3),a5          ; a5    currentrndx
        move.l  currentrndy(a3),a6          ; a6    currentrndy
        move.l  rndx(a3),d4                 ; d4    rndx
        move.l  rndy(a3),d5                 ; d5    rndy

        ; a3 free
        ; a4 free
        
.loop:            
        
        move.l  (a1),a3                 ; u16* xt = (u16*) z[0];

        move.b  (a3)+,d2                ; u16 xpix     = *xt;
        blt.s   .reset                  ; if (xpix == 0x8000)

        add.l   d1,(a1)+                ; z[0] += 2;

        move.l  (a1),a4                 ; u16* yt = (u16*) z[1];
        add.l   d1,(a1)+                ; z[1] += 2;

                                        ; z += 2;
                                        ; u16 yoffset  = *yt;

        move.w  (a4),d3
        lea     (a0,d3.w),a4            ; u8* p = (u8*) _framebuffer;
                                        ; p += yoffset;
        moveq.l #0,d3
        move.b  (a3)+,d3                ; xpix >>= 8;
        add.w   d3,a4                   ; p += xpix & 0xFF;
        
        bset.b  d2,(a4)                 ; *p |= 1 << xpix;

        move.l  a4,(a2)+                ; *erasebuffer++ = p;

        dbf     d0,.loop
        bra.s   .end

.reset:

        ; z[0] = *_param->currentrndx++;
        ; if ( z[0] == 0 )
        ;     _param->currentrndx = _param->rndx
        ;     z[0] = *_param->currentrndx++;
        
        move.l  (a5)+,(a1)+
        tst.l   (a5)
        bne.s   .norndxloop
        move.l  d4,a5
.norndxloop:
        
        move.l  (a6)+,(a1)+
        tst.l   (a6)
        bne.s   .norndyloop
        move.l  d5,a6
.norndyloop:

        move.l  a0,(a2)+

        dbf     d0,.loop                    ; for (t = 0 ; t < _nbstars ; t++)

.end:    
        move.l  (sp)+,a3

        move.l  a5,currentrndx(a3)
        move.l  a6,currentrndy(a3)
    
        movem.l (sp)+,a2-a6/d3-d7
    
        rts

;--------------------------------------------------------------------------------------------------------
;   void STARerase (void* _framebuffer, u16 _nbstars / 6 - 1);
;--------------------------------------------------------------------------------------------------------
STARerase1P:

	movem.l	a1-a6/d1,-(sp)

    moveq.l #0,d1
	
.loop
    movem.l (a0)+,a1-a6
    move.b d1,(a1)
    move.b d1,(a2)
    move.b d1,(a3)
    move.b d1,(a4)
    move.b d1,(a5)
    move.b d1,(a6)

    dbra    d0,.loop

	movem.l	(sp)+,a1-a6/d1

    rts
