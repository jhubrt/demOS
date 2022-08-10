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

	output	D:\PROJECTS\DEMOS\OUTPUT\BLSZIO.O
	
	opt 	o+

    xdef    ablsIIWsample,ablsIIBsample,ablsINWsample,ablsINBsample,ablsUndelta


;-------------------------------------------------------------------------------------------------
; void ablsUndelta(void* sourcesample_sample, u32 _len)
;-------------------------------------------------------------------------------------------------
ablsUndelta: 
	move.w  d1,-(sp)

	subq.l  #1,d0

.loop:
	move.b	(a0)+,d1
	add.b	d1,(a0)
	
	subq.l  #1,d0
	bne.s	.loop

	move.w  (sp)+,d1

	rts

;-------------------------------------------------------------------------------------------------
; void ablsIIWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 (divprecision << 16) | precisionmask, u32 freqdiv)
;-------------------------------------------------------------------------------------------------
ablsIIWsample:
    
    movem.l d3-d7/a2,-(sp)

    ; _sourceSample->sample => a0
    ; _dest                 => a1
       
    ; u32 samplelen         => d0
    ; u16 divprecision      => d1
    ; u16 precisionmask     => d4
    ; u32 freqdiv           => d2
    ; u32 acc               => d7

    move.w  d1,d4
    swap    d1
    
    moveq.l #0,d7           
            
.loop:
    move.l  d7,d6               ; d6 = index
    lsr.l   d1,d6               ; u32 index = acc  >> divprecision;

    move.l  a0,a2               
    add.l   d6,a2               ; a2 = s + index

    move.w  d7,d6           
    and.w   d4,d6               ; d6: interp2 = acc & precisionmask;

    move.w  d4,d5           
    sub.w   d6,d5               ; d5: interp1 = precisionmask - interp2;

    move.b  (a2)+,d3            ; sampl1 = *(s + index);
    ext.w   d3
    muls.w  d3,d5               ; sampl1 = STDmuls((s16)sampl1, interp1); 

    move.b  (a2)+,d3            ; sampl2 = *(s + (index + 1));
    ext.w   d3
    muls.w  d3,d6               ; sampl2 = STDmuls((s16)sampl2, interp2);

    add.l   d6,d5
    asr.l   d1,d5               ; sampl = (s8)((sampl1 + sampl2) >> divprecision);

    tst.b   d5                  ; *d++ = sampl < 0 ? 0xFF : 0;
    slt.b   (a1)+
    move.b  d5,(a1)+            ; *d++ = sampl;

    add.l   d2,d7               ; acc += freqdiv;

    subq.l  #1,d0               ; for (i = 0 ; i < sampleLen ; i++)
    bne.s   .loop

    movem.l (sp)+,d3-d7/a2

    rts

;-------------------------------------------------------------------------------------------------
; void ablsIIBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u32 (divprecision << 16) | precisionmask, u32 freqdiv)
;-------------------------------------------------------------------------------------------------
ablsIIBsample:
    movem.l d3-d7/a2,-(sp)

    ; _sourceSample->sample => a0
    ; _dest                 => a1
       
    ; u32 samplelen         => d0
    ; u16 divprecision      => d1
    ; u16 precisionmask     => d4
    ; u32 freqdiv           => d2
    ; u32 acc               => d7

    move.w  d1,d4
    swap    d1
    
    moveq.l #0,d7           
            
.loop:
    move.l  d7,d6               ; d6 = index
    lsr.l   d1,d6               ; u32 index = acc  >> divprecision;

    move.l  a0,a2               
    add.l   d6,a2               ; a2 = s + index

    move.w  d7,d6           
    and.w   d4,d6               ; d6: interp2 = acc & precisionmask;

    move.w  d4,d5           
    sub.w   d6,d5               ; d5: interp1 = precisionmask - interp2;

    move.b  (a2)+,d3            ; sampl1 = *(s + index);
    ext.w   d3
    muls.w  d3,d5               ; sampl1 = STDmuls((s16)sampl1, interp1); 

    move.b  (a2)+,d3            ; sampl2 = *(s + (index + 1));
    ext.w   d3
    muls.w  d3,d6               ; sampl2 = STDmuls((s16)sampl2, interp2);

    add.l   d6,d5
    asr.l   d1,d5               ; sampl = (s8)((sampl1 + sampl2) >> divprecision);

    addq.l  #1,a1
    move.b  d5,(a1)+            ; *d++ = sampl;

    add.l   d2,d7               ; acc += freqdiv;

    subq.l  #1,d0               ; for (i = 0 ; i < sampleLen ; i++)
    bne.s   .loop

    movem.l (sp)+,d3-d7/a2

    rts

;-------------------------------------------------------------------------------------------------
; void ablsINWsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv)
;-------------------------------------------------------------------------------------------------
ablsINWsample:
    movem.l d3-d7/a2,-(sp)    
    
    moveq.l #0,d7   

.loop:
    move.l  d7,d6               ; sampl = *(s + (acc  >> divprecision));
    lsr.l   d1,d6

    move.l  a0,a2
    add.l   d6,a2

    move.b  (a2)+,d3            ; sampl1 = *(s + index);
    slt.b   (a1)+               ; *d = sampl < 0 ? 0xFF : 0;
    move.b  d3,(a1)+            ; *d++ = sampl;

    add.l   d2,d7               ; acc += freqdiv;

    subq.l  #1,d0
    bne.s   .loop
    
    movem.l (sp)+,d3-d7/a2

    rts

;-------------------------------------------------------------------------------------------------
; void ablsINBsample(void* sourcesample_sample, void* _dest, u32 samplelen, u16 divprecision, u32 freqdiv)
;-------------------------------------------------------------------------------------------------
ablsINBsample:
    movem.l d3-d7/a2,-(sp)    
    
    moveq.l #0,d7  
    
 .loop:
    move.l  d7,d6               ; sampl = *(s + (acc  >> divprecision));
    lsr.l   d1,d6

    move.l  a0,a2
    add.l   d6,a2

    addq.l  #1,a1               ; d++
    move.b  (a2)+,(a1)+         ; sampl1 = *(s + index); *d++ = sampl;

    add.l   d2,d7               ; acc += freqdiv;

    subq.l  #1,d0
    bne.s   .loop
    
    movem.l (sp)+,d3-d7/a2

    rts

