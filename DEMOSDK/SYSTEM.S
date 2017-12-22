*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2017 J.Hubert
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

*--------------------------------------------------------------------------
* Defines ASM routines for system core functionnalities
* in particular implements the VBL interupts routine that manages:
* - bi-threading: returns from idle thread to main thread if needed
* - call raster system routine if defined
*--------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\SYSTEM.O
	
	opt 	o+

    xdef    SYSsoundtrackUpdate
    xdef    SYSdetectEmu,SYSlockIdle,SYSlockMain,SYSreadVideoBase,SYSwriteVideoBase,SYSemptyKb

	xdef	SYSvblinterrupt,SYSvblcount,SYSvblLcount,SYSOSVBL
	xdef	SYSsetIdlethread,SYSswitchIdle
    xdef    SYSgemdosSetMode,SYSreset
    xdef    SYSinitDbgBreak,SYSdbgBreak

	xref	RASnextOpList
    
    xref    LOADdeconnectAndStartRequest,LOADtoggleDriveStartRequest


*-------------------------------------------------------------------------
* void SYSinitDbgBreak (void);
*-------------------------------------------------------------------------
SYSinitDbgBreak:
    move.l  #.return,$0A8.w
    rts
.return
    rte
    
*-------------------------------------------------------------------------
* void SYSdbgBreak (void);
*-------------------------------------------------------------------------
SYSdbgBreak:
    trap    #10
    rts

*-------------------------------------------------------------------------
* void* SYSgemdosSetMode (void* _adr);
*-------------------------------------------------------------------------
SYSgemdosSetMode:
	move.l	a0,-(sp)
	move.w	#32,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,a0
	rts

*-------------------------------------------------------------------------
* void SYSreset (void);
*-------------------------------------------------------------------------
SYSreset:
	move.l	4.w,a0
	jmp		(a0)


*-------------------------------------------------------------------------
* u32 SYSreadVideoBase ();
*-------------------------------------------------------------------------
SYSreadVideoBase:	
	moveq.l	#0,d0
	move.b	$ffff8201.w,d0
	swap	d0
	move.b	$ffff8203.w,d0
	lsl.w	#8,d0
	move.b	$ffff820d.w,d0
	rts

*-------------------------------------------------------------------------
* void SYSwriteVideoBase (u32 _val);
*-------------------------------------------------------------------------
SYSwriteVideoBase:	
	swap	d0
	move.b	d0,$ffff8201.w
	swap	d0
	ror.w	#8,d0
	move.b	d0,$ffff8203.w
	lsr.w	#8,d0
	move.b	d0,$ffff820d.w
	rts

*-------------------------------------------------------------------------
* void SYSemptyKb (void);
*-------------------------------------------------------------------------
SYSemptyKb: 				
	btst	#5,$fffffc00.w
	beq.s	.noerror
	tst.b	$fffffc02.w
	bra.s	SYSemptyKb
.noerror:
	btst	#0,$fffffc00.w
	beq.s	.vidbuff                          
	tst.b	$fffffc02.w
	bra.s	SYSemptyKb
.vidbuff:
	move.w	#0,$fffffc02.w
	rts

*-------------------------------------------------------------------------
* u32* SYSdetectEmu (void)
*-------------------------------------------------------------------------
SYSdetectEmu:
        movem.l d0-d7/a1-a6,-(sp)

        move.l  #'Emu?',d6
        move.l  d6,d7
        move    #$25,-(sp)
        trap    #14
        addq.l  #2,sp
        
        lea     SYSemuDescriptor(pc),a1

        move.l  d6,(a1)
        move.l  d7,4(a1)
        move.l  a0,8(a1)

        move.l  a1,a0

        movem.l (sp)+,d0-d7/a1-a6

        rts

*-------------------------------------------------------------------------
* bool SYSlockMain (SYSlock* _lock);
*-------------------------------------------------------------------------
SYSlockMain:
    tas     (a0)
    bne.s   SYSlockMain_locked
    moveq.l #1,d0
    rts

SYSlockMain_locked:
    moveq.l #0,d0
    rts

*-------------------------------------------------------------------------
* void SYSlockIdle (SYSlock* _lock);
*-------------------------------------------------------------------------
SYSlockIdle:
    tas     (a0)
    bne.s   SYSlockMain
    rts

*-------------------------------------------------------------------------
* Idle thread management
*-------------------------------------------------------------------------

* ---- bool setidlethread(void* _stacklow, void*_stackhigh) --------------

SYSsetIdlethread:
	move.l	a0,idlethreadcontext+4
	move.l	a1,idlethreadcontext+8

	move.l	(sp),-(a1)					; store the return address on the thread stack
	move.w	sr,-(a1)						; store the sr state on the thread stack
	moveq.l	#1,d0						; set the future return parameter
	movem.l	d0-d7/a0-a6,-(a1)			; store the current registers state on the thread stack
	move.l	a1,idlethreadcontext		; save the thread stack pointer

	moveq.l	#0,d0
	rts

* ---- void SYSswitchIdle() ---------------------------------------------

SYSswitchIdle:
	move.w	#$2700,sr					; blocks all interrupts => SR will be restored by rte
	move.w	#$2300,-(sp)				; stack SR manually => we will use a rte to return (call adr has been stored by jsr)
	;move.w	sr,-(sp)					; stack SR manually => we will use a rte to return (call adr has been stored by jsr)

	movem.l d0-d7/a0-a6,-(sp)			; store main thread context
	move.l	sp,mainthreadcontext		; save main thread SP

	move.l	idlethreadcontext(pc),sp	; restore idle thread SP and context
	movem.l (sp)+,d0-d7/a0-a6

	rte									; trick: use rte instead of rts while we were called by bsr

*-------------------------------------------------------------------------
* Interupt routines
*-------------------------------------------------------------------------

SYSvblinterrupt:
	move.w	#$2700,sr					; blocks all interupts => will be restored by rte

	and.b	#$FE,$fffffa07.w			; disable timer B
	clr.b	$fffffa1b.w

	* -------------------- idle thread management ------------------------

	cmp.l	idlethreadcontext+4(pc),sp	; look if we are into idlethread by comparing the sp to the allocated stack space
	blt.s	donotswitchthread			; if we are in main do nothing

	cmp.l	idlethreadcontext+8(pc),sp
	bge.s	donotswitchthread

	* need to switch

	movem.l d0-d7/a0-a6,-(sp)			; store idle thread context and stack pointer
	move.l	sp,idlethreadcontext

	move.l	mainthreadcontext(pc),sp	; restore main thread context and stack pointer
	movem.l	(sp)+,d0-d7/a0-a6

donotswitchthread:

	move.l	d0,-(sp)
	move.l	a0,-(sp)
    move.l	a1,-(sp)
	
	* -------------------- rasters start management ----------------------

	move.l	RASnextOpList(pc),a0
	move.l	a0,d0
	beq.s	SYSvblinterrupt_noraster

	move.l	(a0)+,a1
	jsr		(a1)

	or.b	#1,$fffffa07.w
	move.b	#8,$fffffa1b.w

SYSvblinterrupt_noraster:

	* -------------------- soundtrack management -------------------------

    move.l  SYSsoundtrackUpdate(pc),d0
    beq.s   SYSvblinterrupt_nosoundtrack
    move.l  d0,a0
	jsr		(a0)

SYSvblinterrupt_nosoundtrack:

    move.l  LOADdeconnectAndStartRequest(pc),a0
    move.l  a0,d0
    beq.s   SYSvblinterrupt_nodrivedeselect

	move.w	#$80,$FFFF8606.w			; select FDC register control
    move.w  $FFFF8604.w,d0              ; get FDC status register
    btst    #7,d0
    bne.s   SYSvblinterrupt_nodrivedeselect

    clr.l   LOADdeconnectAndStartRequest
    bsr     LOADtoggleDriveStartRequest

SYSvblinterrupt_nodrivedeselect:

	move.l	(sp)+,a1
	move.l	(sp)+,a0
	move.l	(sp)+,d0
	
	addq.l	#1,SYSvblcount

	tst.w	SYSOSVBL+2					; debug feature: call TOS vbl routine when running into support debugger mode
	beq.s	endvbl

	move.l	SYSOSVBL(pc),automodjmp+2	
	nop
automodjmp:
	jmp		SYSOSVBL

endvbl:
	rte


SYSsoundtrackUpdate:
    dc.l    0

SYSemuDescriptor:
    dc.l    0
    dc.l    0
    dc.l    0

SYSvblcount:
	dc.w	0
SYSvblLcount:
	dc.w	0

SYSOSVBL:
	dc.l	0

mainthreadcontext:
	dc.l	0		; current sp backup

idlethreadcontext:
	dc.l	0		; current sp backup
	dc.l	0		; stack low
	dc.l	0		; stack high

	END
