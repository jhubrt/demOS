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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\LAYERZ.O

	xdef	LAYZcomputeVolumes,LAYZmid15,LAYZcount
	xdef	LAYZym,LAYZymnoise,LAZgenSprite


SPR_OPCODE_MOVE_DATA_X_A0   set     $317C
LAYERZ_PITCH                set     168
CGEN_OPCODE_RTS             set     $4E75

; void layerZcomputeVolumes (s8*p, u16* vol)


LAYZcomputeVolumes:

    movem.w d0-d5,-(sp)

    sub.w   d0,d0
    move.w  #249,d1
    sub.w   d2,d2
    sub.w   d3,d3
    sub.w   d4,d4
    sub.w   d5,d5

 .loop:                             ;   for (t = 0 ; t < 250 ; t++)

    move.b  (a0)+,d0                ; s = *p++;                     8
    bpl.s   *+4                     ;                               10
    neg.b   d0                      ;                               4
    add.w   d0,d2                   ; vol[0] += s > 0 ? s : -s;     4

    move.b  (a0)+,d0                ; s = *p++;   
    bpl.s   *+4
    neg.b   d0
    add.w   d0,d3                   ; vol[1] += s > 0 ? s : -s;

    move.b  (a0)+,d0                ; s = *p++;   
    bpl.s   *+4
    neg.b   d0
    add.w   d0,d5                   ; vol[3] += s > 0 ? s : -s;

    move.b  (a0)+,d0                ; s = *p++;   
    bpl.s   *+4
    neg.b   d0
    add.w   d0,d4                   ; vol[2] += s > 0 ? s : -s;
    
    addq.l  #4,a0                   ;                               8

    dbf.w   d1,.loop                ;                               12
                                    ; average = ((36 * 4) + 8 + 12) * 250 = 41000 => 80 scanlines
    move.w  d2,(a1)+
    move.w  d3,(a1)+
    move.w  d4,(a1)+
    move.w  d5,(a1)+

    movem.w (sp)+,d0-d5

    rts


LAYZmid15:
	move.l	a0,-(sp)
	move.l	a1,-(sp)
	
    move.l	usp,a0
    move.l  (a0)+,a1
	
    move.l	(a1)+,$ffff8242.w
	move.l	(a1)+,$ffff8246.w
	move.l	(a1)+,$ffff824A.w
	move.l	(a1)+,$ffff824E.w
	move.l	(a1)+,$ffff8252.w
	move.l	(a1)+,$ffff8256.w
	move.l	(a1)+,$ffff825A.w
	move.w	(a1)+,$ffff825E.w

	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp

    addq.w  #1,LAYZcount

	move.l	(sp)+,a1
	move.l	(sp)+,a0
	rte

LAYZcount: dc.w 0



LAYZym:

	move.w	d4,-(sp)
	move.w	d5,-(sp)

	sub.w	d4,d4
	move.w	d4,d5

	subq.w	#1,d2

.loop:

	add.w		d0,d4		
	bge.s		.positive	
	move.b		d5,(a0)		

.negative2:
	add.w		d0,d4		
	bge.s		.positive2	
	move.b		d5,(a0)		

	bra.s		.end		

.positive:
	move.b		d1,(a0)		
	
	add.w		d0,d4		
	blt.s		.negative2	

.positive2:
	move.b		d1,(a0)		

.end:
	
	lea			168(a0),a0

	dbf.w		d2,.loop

	move.w	(sp)+,d5
	move.w	(sp)+,d4
	
	rts	



LAYZymnoise:

	move.w	d4,-(sp)
	move.w	d5,-(sp)

	sub.w	d4,d4
	move.w	d4,d5

	subq.w	#1,d2

.loop:

	btst.b		d0,(a1)			
	bne.s		.positive		
	move.b		d5,(a0)			

.negative2:
	btst.b		d0,(a1)			
	bne.s		.positive2		
	move.b		d5,(a0)			

	bra.s		.end			

.positive:
	move.w		d1,(a0)			
	
	btst.b		d0,(a1)			
	beq.s		.negative2		

.positive2:
	move.w		d1,(a0)		

.end:

	lea			168(a0),a0

	dbf.w		d2,.loop	

	move.w	(sp)+,d5
	move.w	(sp)+,d4
	
	rts	


; static u8* LAZgenSprite(u16* data_, u16 nblines_, u8* dest_, u32 temp_)
    
LAZgenSprite:
        
    ; d0 = nblines
    ; a0 = source data
    ; a1 = sprite[]
    ; d1 = temp
            
    movem.l     d2-d7/a2-a4,-(sp)

    move.l      d1,a2
    move.l      a2,a3

    move.l      (a1),a4                            ; sprite[0] into a4
    subq.w      #1,d0
    moveq.l     #1,d2

    move.w      d0,d3
    moveq.l     #0,d7
.copydata:
    move.w      (a0)+,(a3)+
    move.w      (a0)+,(a3)+
    move.w      d7,(a3)+
    dbf.w       d3,.copydata



    move.w      #16,-(sp)
.loopx:
    move.l      a2,a3                               ; reload temp into a3
    moveq.l     #0,d4                               ; offset = 0
    
    move.w      d0,d3                               ;  nblines
   
    move.l      a4,(a1)+                            ; sprite[t] = d

.loopy:
    
    move.w      (a3)+,d5                
    beq.s       .w00                                ;  if (w0 != 0)                          
    move.w      #SPR_OPCODE_MOVE_DATA_X_A0,(a4)+    
    move.w      d5,(a4)+                            ; *d++ = PCENDIANSWAP16(w0);
    move.w      d4,(a4)+                            ; *d++ = offset;
.w00:
    addq.w      #8,d4                               ; offset += 8;

    move.w      (a3)+,d6
    beq.s       .w10                                ;  if (w1 != 0)                          
    move.w      #SPR_OPCODE_MOVE_DATA_X_A0,(a4)+    
    move.w      d6,(a4)+                            ; *d++ = PCENDIANSWAP16(w1);
    move.w      d4,(a4)+                            ; *d++ = offset;
.w10:
    addq.w      #8,d4                               ; offset += 8;

    move.w      (a3)+,d7
    beq.s       .w20                                ;  if (w2 != 0)                          
    move.w      #SPR_OPCODE_MOVE_DATA_X_A0,(a4)+    
    move.w      d7,(a4)+                            ; *d++ = PCENDIANSWAP16(w2);
    move.w      d4,(a4)+                            ; *d++ = offset;
.w20:
    add.w       #LAYERZ_PITCH-16,d4                 ; offset += PITCH-16;

    lsr.w       d2,d5
    roxr.w      d2,d6
    roxr.w      d2,d7

    subq.l      #6,a3
    move.w      d5,(a3)+
    move.w      d6,(a3)+
    move.w      d7,(a3)+

    dbf.w       d3,.loopy

    move.w      #CGEN_OPCODE_RTS,(a4)+

    subq.w      #1,(sp)
    bne.s       .loopx

    addq.l      #2,sp

    move.l      a4,a0

    movem.l     (sp)+,d2-d7/a2-a4

    rts



