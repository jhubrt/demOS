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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\INFO.PRX

	xdef	INFOimport


INFOImport:
	lea		INFOimportTable(pc),a0
	rts


INFO_DISPLAY_H                 set 245
VEC_PITCH                      set 168
INFO_FRAMEBUFFER_SIZE          set 168*9*(32+32+2)

HW_BLITTER_HTONE               set $FFFF8A00
HW_BLITTER_XINC_SOURCE         set $FFFF8A20
HW_BLITTER_YINC_SOURCE         set $FFFF8A22
HW_BLITTER_ADDR_SOURCE         set $FFFF8A24
HW_BLITTER_ENDMASK1            set $FFFF8A28
HW_BLITTER_ENDMASK2            set $FFFF8A2A
HW_BLITTER_ENDMASK3            set $FFFF8A2C
HW_BLITTER_XINC_DEST           set $FFFF8A2E
HW_BLITTER_YINC_DEST           set $FFFF8A30
HW_BLITTER_ADDR_DEST           set $FFFF8A32
HW_BLITTER_XSIZE               set $FFFF8A36
HW_BLITTER_YSIZE               set $FFFF8A38
HW_BLITTER_HOP                 set $FFFF8A3A 
HW_BLITTER_OP                  set $FFFF8A3B 
HW_BLITTER_CTRL1               set $FFFF8A3C 
HW_BLITTER_CTRL2               set $FFFF8A3D 


INFOimportTable:
    dc.l    INFOdrawVolumes
    dc.l    INFOcomputeVolumes
    dc.l    INFOclearPlane
    dc.l    INFOcopyPlane


INFOdrawVolumes:

    ; a0: this->maxvols
    ; a1: image
    ; d0: this->volumesmasks

        movem.l a2-a6/d1-d7,-(sp)
       
        lea     HW_BLITTER_ENDMASK1.w,a2
        moveq.l #-1,d7                          ; HW_BLITTER_ENDMASK1 = HW_BLITTER_ENDMASK2 = HW_BLITTER_ENDMASK1 = -1;
        move.l  d7,(a2)+
        move.w  d7,(a2)+
        move.l  #8*65536+VEC_PITCH,(a2)+        ; *HW_BLITTER_XINC_DEST = 8; *HW_BLITTER_YINC_DEST = VEC_PITCH;
        move.w  #1,HW_BLITTER_XSIZE.w           ; *HW_BLITTER_XSIZE     = 1;
        move.w  #$103,HW_BLITTER_HOP.w          ; *HW_BLITTER_HOP       = HW_BLITTER_HOP_HTONE; *HW_BLITTER_OP        = HW_BLITTER_OP_S;
       
        moveq.l #6,d4                           ; 7 channels
        move.l  #HW_BLITTER_HTONE,d6
        lea     HW_BLITTER_YSIZE.w,a2
        lea     HW_BLITTER_CTRL1.w,a5
        lea     HW_BLITTER_ADDR_DEST.w,a6
        move.w  #INFO_DISPLAY_H,d2
        move.w  #$C000,d3
        moveq.l #3,d7

.loop:                                      ; for (t = 0; t < ARRAYSIZE(this->maxvols) ; t++)
        move.w (a0)+,d5                     ; u16* p = this->volumemasks[this->maxvols[t]];           
                                            ; u16 i;
                                            ; u16 mask;
    
        lsl.w   d7,d5
        move.l  d0,a3
        add.w   d5,a3

        rept    3                           ; 3 columns for each channel
        
        move.w  (a3)+,d1                    ; mask = *p++;
        move.w  d1,d5
        swap    d1
        move.w  d5,d1

        move.l  d6,a4
                                            ; for(i = 0 ; i < 16 ; i++)
        move.l  d1,(a4)+                    ; HW_BLITTER_HTONE[i] = mask; 
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  
        move.l  d1,(a4)+                    ;  

        move.l  a1,(a6)                     ; *HW_BLITTER_ADDR_DEST = (u32)image;
        addq.l  #8,a1                       ; image += 8;

        move.w  d2,(a2)                     ; *HW_BLITTER_YSIZE = INFO_DISPLAY_H
        move.w  d3,(a5)                     ;  HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;        /* run */

        endr

        dbf.w d4,.loop

        movem.l (sp)+,a2-a6/d1-d7

        rts


INFOcomputeVolumes: 

        ; a0 : dmabuffer
        ; a1 : results table

        movem.l d0/d3-d7,-(sp)
       
        moveq.l #0,d0
        moveq.l #124,d3

        move.w #256,d4
        move.w #256,d5
        move.w #256,d6
        move.w #256,d7

.loop:

        ; channel 1
        move.b  (a0)+,d0
    	bpl.s	.positive1
        neg.b   d0
.positive1:
        add.w   d0,d4

        ; channel 2
        move.b  (a0)+,d0
    	bpl.s	.positive2
        neg.b   d0
.positive2:
        add.w   d0,d5

        ; channel 3
        move.b  (a0)+,d0
    	bpl.s	.positive3
        neg.b   d0
.positive3:
        add.w   d0,d6

        ; channel 4
        move.b  (a0)+,d0
    	bpl.s	.positive4
        neg.b   d0
.positive4:
        add.w   d0,d7

        lea     12(a0),a0

        dbf.w   d3,.loop

        moveq.l #9,d3

        lsr.w   d3,d4
        lsr.w   d3,d5
        lsr.w   d3,d6
        lsr.w   d3,d7

        move.w  d4,(a1)+
        move.w  d5,(a1)+
        move.w  d6,(a1)+
        move.w  d7,(a1)+

        movem.l (sp)+,d0/d3-d7

        rts



INFOclearPlane:        

        subq.w  #1,d0

.loop:
        move.w  d1,(a0)
        move.w  d1,8(a0)
        move.w  d1,16(a0)
        move.w  d1,24(a0)
        move.w  d1,32(a0)
        move.w  d1,40(a0)
        move.w  d1,48(a0)
        move.w  d1,56(a0)
        move.w  d1,64(a0)
        move.w  d1,72(a0)
        move.w  d1,80(a0)
        move.w  d1,88(a0)
        move.w  d1,96(a0)
        move.w  d1,104(a0)
        move.w  d1,112(a0)
        move.w  d1,120(a0)
        move.w  d1,128(a0)
        move.w  d1,136(a0)
        move.w  d1,144(a0)
        move.w  d1,152(a0)
        move.w  d1,160(a0)

        lea     VEC_PITCH(a0),a0

        dbf.w   d0,.loop

        rts


INFOcopyPlane:        
    
        movem.l  a2-a4,-(sp)
        
        move.l  d0,a2
        
        move.l  a1,a3
        move.l  a2,a4

        add.l   #INFO_FRAMEBUFFER_SIZE,a3
        add.l   #INFO_FRAMEBUFFER_SIZE,a4

        subq.w  #1,d1

.loop:
    
        move.w  (a0),d0
        move.w  d0,(a1)
        move.w  d0,(a2)
        move.w  d0,(a3)
        move.w  d0,(a4)

offset  set 0
        rept 20
offset  set offset+8 
        move.w  offset(a0),d0 
        move.w  d0,offset(a1)
        move.w  d0,offset(a2)
        move.w  d0,offset(a3)
        move.w  d0,offset(a4)
        endr

        lea     VEC_PITCH(a0),a0
        lea     VEC_PITCH(a1),a1
        lea     VEC_PITCH(a2),a2
        lea     VEC_PITCH(a3),a3
        lea     VEC_PITCH(a4),a4

        dbf.w   d1,.loop

        movem.l  (sp)+,a2-a4

        rts
