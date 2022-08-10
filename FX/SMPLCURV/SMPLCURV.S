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

	output  D:\PROJECTS\DEMOS\OUTPUT\FX\SMPLCURV.O

	xdef	BSCrImportTable

BSCrImportTable:

	; Draw H -----------------------------

	dc.b	(SamCurveDrawHBegin_End-SamCurveDrawHBegin)/2,0
	dc.l	SamCurveDrawHBegin

	dc.b	(SamCurveDrawH_StartEnd-SamCurveDrawHStart)/2,0
	dc.l	SamCurveDrawHStart

	dc.b	(SamCurveDrawH_End-SamCurveDrawH)/2,0
	dc.l	SamCurveDrawH

	dc.b	(SamCurveDrawHEnd_End-SamCurveDrawHEnd)/2,0
	dc.l	SamCurveDrawHEnd
	
	; Draw YM H --------------------------

	dc.b	(SamCurveDrawHYMBegin_End-SamCurveDrawHYMBegin)/2,0
	dc.l	SamCurveDrawHYMBegin

	dc.b	(SamCurveDrawHYMStart_End-SamCurveDrawHYMStart)/2,0
	dc.l	SamCurveDrawHYMStart

	dc.b	(SamCurveDrawHYM_End-SamCurveDrawHYM)/2,0
	dc.l	SamCurveDrawHYM

	dc.b	(SamCurveDrawHYMEnd_End-SamCurveDrawHYMEnd)/2,0
	dc.l	SamCurveDrawHYMEnd



BSCrRegs:
	dc.w		1	; d2
	dc.w		4	; d3
	dc.w		8   ; d4
	dc.w       32	; d5
	dc.w	   64 	; d6	
	dc.w      128	; d7


; void SamCurveDrawHBegin (void* sample, void* display, u32 offsetytable)

SamCurveDrawHBegin:

	move.l		a4,-(sp)
	movem.w		d3-d7,-(sp)

	move.l		d0,a4

	movem.w		BSCrRegs,d2-d7

SamCurveDrawHBegin_End:


SamCurveDrawHStart:

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	bset.b		d3,$77(a1,d0.w)		; 20				=> set bit 4

	move.b	    $ABA(a0),d0			; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d4,$77(a1,d0.w)		; 20				d4 = 8 => set bit 3

	move.b	    $ABA(a0),d0			; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d3,$77(a1,d0.w)		; 20				d3 = 4 ; a1 = display address => set bit 2

	move.b	    $ABA(a0),d0			; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	bset.b		d2,$77(a1,d0.w)		; 20				=> set bit 1

	move.b	    $ABA(a0),d0			; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d2,$77(a1,d0.w)		; 20				d2 = 1 ; a1 = display address => set bit 0


SamCurveDrawH_StartEnd:


SamCurveDrawH:

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table
	or.b		d7,$77(a1,d0.w)		; 20				d7 = 128 => set bit 7

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d6,$77(a1,d0.w)		; 20				d6 = 64 => set bit 6

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table
	or.b		d5,$77(a1,d0.w)		; 20				d5 = 32 => set bit 5

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	bset.b		d3,$77(a1,d0.w)		; 20				=> set bit 4

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d4,$77(a1,d0.w)		; 20				d4 = 8 => set bit 3

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d3,$77(a1,d0.w)		; 20				d3 = 4 ; a1 = display address => set bit 2

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	bset.b		d2,$77(a1,d0.w)		; 20				=> set bit 1

	move.b	    $ABA(a0),d0		    ; 12	        	a0 = sample address - d0 = temp sample					
	ext.w		d0				    ; 4
	add.w	    d0,d0			    ; 4                 
	move.w	    (a4,d0.w),d0	    ; 16                a4 = y offset table			  
	or.b		d2,$77(a1,d0.w)		; 20				d2 = 1 ; a1 = display address => set bit 0

SamCurveDrawH_End:


SamCurveDrawHEnd:

	movem.w		(sp)+,d3-d7
	move.l		(sp)+,a4

	rts

SamCurveDrawHEnd_End:


; void SamCurveDrawHYMBegin (void* display1, void* display2, s16 _accstart, s16 _inc1, s16 _inc2)

SamCurveDrawHYMBegin:
	movem.w		d3-d7,-(sp)
	move.l		a2,-(sp)

	move.w      d2,a2
	movem.w		BSCrRegs,d2-d7

SamCurveDrawHYMBegin_End:

SamCurveDrawHYMStart:

	add.w		d1,d0
	bpl.s		*+8
	bset.b		d3,$77(a0)
	bra.s		*+6
	bset.b		d3,$77(a1)			; => set bit 4

	add.w		a2,d0
	bpl.s		*+8					
	or.b		d4,$77(a0)			
	bra.s		*+6
	or.b		d4,$77(a1)			; d4 = 8 => set bit 3

	add.w		d1,d0
	bpl.s		*+8					         
	or.b		d3,$77(a0)			    				
	bra.s		*+6
	or.b		d3,$77(a1)			; d3 = 4 ; a1 = display address => set bit 2
		
	add.w		a2,d0
	bpl.s		*+8					       
	bset.b		d2,$77(a0)			   				
	bra.s		*+6					
	bset.b		d2,$77(a1)			; => set bit 1

	add.w		d1,d0
	bpl.s		*+8					       
	or.b		d2,$77(a0)			
	bra.s		*+6					
	or.b		d2,$77(a1)			; d2 = 1 ; a1 = display address => set bit 0

SamCurveDrawHYMStart_End:


SamCurveDrawHYM:
	add.w		d1,d0
	bpl.s		*+8					       
	or.b		d7,$77(a0)		
	bra.s		*+6					
	or.b		d7,$77(a1)			; d7 = 128 => set bit 7

	add.w		a2,d0
	bpl.s		*+8					       
	or.b		d6,$77(a0)			
	bra.s		*+6					
	or.b		d6,$77(a1)			; d6 = 64 => set bit 6

	add.w		d1,d0
	bpl.s		*+8					       
	or.b		d5,$77(a0)
	bra.s		*+6					
	or.b		d5,$77(a1)			; d5 = 32 => set bit 5

	add.w		a2,d0
	bpl.s		*+8
	bset.b		d3,$77(a0)
	bra.s		*+6
	bset.b		d3,$77(a1)			; => set bit 4

	add.w		d1,d0
	bpl.s		*+8					
	or.b		d4,$77(a0)			
	bra.s		*+6
	or.b		d4,$77(a1)			; d4 = 8 => set bit 3

	add.w		a2,d0
	bpl.s		*+8					         
	or.b		d3,$77(a0)			    				
	bra.s		*+6
	or.b		d3,$77(a1)			; d3 = 4 ; a1 = display address => set bit 2
		
	add.w		d1,d0
	bpl.s		*+8					       
	bset.b		d2,$77(a0)			   				
	bra.s		*+6					
	bset.b		d2,$77(a1)			; => set bit 1

	add.w		a2,d0
	bpl.s		*+8					       
	or.b		d2,$77(a0)			
	bra.s		*+6					
	or.b		d2,$77(a1)			; d2 = 1 ; a1 = display address => set bit 0

SamCurveDrawHYM_End:


SamCurveDrawHYMEnd:
	move.l		(sp)+,a2
	movem.w		(sp)+,d3-d7
	rts	

SamCurveDrawHYMEnd_End:
