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

	output  D:\PROJECTS\DEMOS\OUTPUT\BLITZIK\ALLCURVE.O

	xdef	BACrImportTable

BACrImportTable:

	; Draw V -----------------------------

	dc.b	(SamCurveDrawVBegin_End-SamCurveDrawVBegin)/2,0
	dc.l	SamCurveDrawVBegin

	dc.b	(SamCurveDrawVDraw_End-SamCurveDrawVDraw)/2,0
	dc.l	SamCurveDrawVDraw	

	dc.b	(SamCurveDrawVEnd_End-SamCurveDrawVEnd)/2,0
	dc.l	SamCurveDrawVEnd

	; Draw V ym --------------------------

	dc.b	(SamCurveDrawVymBegin_End-SamCurveDrawVymBegin)/2,0
	dc.l	SamCurveDrawVymBegin

	dc.b	(SamCurveDrawVymDraw_End-SamCurveDrawVymDraw)/2,0
	dc.l	SamCurveDrawVymDraw	

	dc.b	(SamCurveDrawVymNoiseDraw_End-SamCurveDrawVymNoiseDraw)/2,0
	dc.l	SamCurveDrawVymNoiseDraw	

	dc.b	(SamCurveDrawVymEnd_End-SamCurveDrawVymEnd)/2,0
	dc.l	SamCurveDrawVymEnd

;----------------------------------------------

; void SamCurveDrawVBegin (void* sample, void* display, u32 pixeltable)


SamCurveDrawVBegin:

	move.l	a2,-(sp)
	move.w	d1,-(sp)
	move.w	d2,-(sp)
	
	move.w	#$1F8,d1

SamCurveDrawVBegin_End:


SamCurveDrawVDraw:

	move.b 		$ABA(a0),d2			; 12
	add.w		d2,d2				; 4
	and.w		d1,d2				; 4
	move.l		d0,a2				; 4
	add.w		d2,a2				; 8
	move.w		(a2)+,$ABC(a1)		; 16
	move.w		(a2)+,$ABC(a1)		; 16
	move.w		(a2)+,$ABC(a1)		; 16

SamCurveDrawVDraw_End:


SamCurveDrawVEnd:

	move.w	(sp)+,d2
	move.w	(sp)+,d1
	move.l	(sp)+,a2
	
	rts	

SamCurveDrawVEnd_End:

;----------------------------------------------

; void SamCurveDrawVymBegin (void* display, u16 inc, u32 curve1, u16 curve2)

; void SamCurveDrawVymNoiseBegin (void* display, void* noisyreg, u16 bittotest, u32 curve1, u16 curve2)

SamCurveDrawVymBegin:

	move.w	d3,-(sp)
	move.w	d4,-(sp)
	move.w	d5,-(sp)

	swap	d1
	move.w  d1,d3
	swap	d1

	sub.w	d4,d4
	move.w	d4,d5

SamCurveDrawVymBegin_End:


SamCurveDrawVymDraw:

	add.w		d0,d4				; 4
	bge.s		.positive			; 8 / 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12

.negative2:
	add.w		d0,d4				; 4
	bge.s		.positive2			; 8 / 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12

	bra.s		.end				; 12

.positive:
	move.w		d3,$ABC(a0)			; 12
	move.w		d1,$ABC(a0)			; 12
	move.w		d2,$ABC(a0)			; 12

	add.w		d0,d4				; 4
	blt.s		.negative2			; 8 / 12

.positive2:
	move.w		d3,$ABC(a0)			; 12
	move.w		d1,$ABC(a0)			; 12
	move.w		d2,$ABC(a0)			; 12

.end:
	
SamCurveDrawVymDraw_End:


SamCurveDrawVymNoiseDraw:

	btst.b		d0,(a1)				; 8
	bne.s		.positive			; 8 / 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12

.negative2:
	btst.b		d0,(a1)				; 8
	bne.s		.positive2			; 8 / 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12
	move.w		d5,$ABC(a0)			; 12

	bra.s		.end				; 12

.positive:
	move.w		d3,$ABC(a0)			; 12
	move.w		d1,$ABC(a0)			; 12
	move.w		d2,$ABC(a0)			; 12

	btst.b		d0,(a1)				; 8
	beq.s		.negative2			; 8 / 12

.positive2:
	move.w		d3,$ABC(a0)			; 12
	move.w		d1,$ABC(a0)			; 12
	move.w		d2,$ABC(a0)			; 12

.end:
	
SamCurveDrawVymNoiseDraw_End:




SamCurveDrawVymEnd:

	move.w	(sp)+,d5
	move.w	(sp)+,d4
	move.w	(sp)+,d3
	
	rts	

SamCurveDrawVymEnd_End:
