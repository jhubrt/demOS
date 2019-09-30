*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2018 J.Hubert
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

    ; exports
    xdef    SYSdetectEmu,SYSlockIdle,SYSlockMain,SYSreadVideoBase,SYSwriteVideoBase,SYSvidCountRead,SYSemptyKb

	xdef	SYSvblinterrupt,SYSvblcount,SYSvblLcount,SYSvblroutines,SYSvbldonothing,SYSvblrunRTSroutine,SYSvblend
	xdef	SYSsetIdlethread,SYSswitchIdle
    xdef    SYSgemdosSetMode,SYSmalloc,SYSreset
    xdef    SYSinitDbgBreak,SYSdbgBreak
	xdef    SYSfastPrint

    xdef    SYSwmovep,SYSlmovep

    ; imports
    xref    LOADdeconnectAndStartRequest,LOADtoggleDriveStartRequest
    xref    SYSfontdata,SYSfontchars


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
	movem.l	d0-d1/a1,.backup
	
	move.l	a0,-(sp)
	move.w	#32,-(sp)
	trap	#1
	addq.l	#6,sp
	move.l	d0,a0
	
	movem.l	.backup(pc),d0-d1/a1
	
	rts
	
.backup: 
	dc.l	0
	dc.l	0
	dc.l	0

*-------------------------------------------------------------------------
* void* SYSmalloc (size_t _size);
*-------------------------------------------------------------------------
SYSmalloc:
	move.l	d0,-(sp)
	move.w	#$48,-(sp)
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
* u32 SYSvidCountRead ();
*-------------------------------------------------------------------------
SYSvidCountRead:
	moveq.l	#0,d0
	move.b	$ffff8205.w,d0
	swap	d0
	move.b	$ffff8207.w,d0
	lsl.w	#8,d0
	move.b	$ffff8209.w,d0
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

SYSwmovep:
    movep.w (a0),d0
    rts

SYSlmovep:
    movep.l (a0),d0
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

;-----------------------------------------------------------------------------------------
; void SYSfastPrint (char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch) 
; a0 = string
; a1 = screendisplay
; d0 = screenpitch
; d1 = bitplanepitch
;-----------------------------------------------------------------------------------------
SYSfastPrint:

	movem.l	a2-a4/d4,-(sp)

    cmp.w   SYSfastPrintautomod+2(pc),d0
    beq.s   SYS_debugPrint_noautomod

    lea     SYSfastPrintautomod+2(pc),a4        ; check current automod state with current pitch
    lea     SYSfastPrintautomod2+2(pc),a3

    move.w  d0,d4
    move.w  d4,(a4)
    move.w  d4,(a3)
    add.w   d0,d4
    move.w  d4,4(a4)
    move.w  d4,4(a3)
    add.w   d0,d4
    move.w  d4,8(a4)
    move.w  d4,8(a3)
    add.w   d0,d4
    move.w  d4,12(a4)
    move.w  d4,12(a3)
    add.w   d0,d4
    move.w  d4,16(a4)
    move.w  d4,16(a3)
    add.w   d0,d4
    move.w  d4,20(a4)
    move.w  d4,20(a3)
    add.w   d0,d4
    move.w  d4,24(a4)
    move.w  d4,24(a3)

SYS_debugPrint_noautomod:

	move.l	#SYSfontdata,a3		; lea	SYSfontdata(pc),a3 => use absolute to avoid overflow at link
	move.l	#SYSfontchars,a2	; lea 	SYSfontchars(pc),a2

	subq.w  #1,d1

    move.w  a1,d4
    and.w   #1,d4
    bne.s   SYS_debugPrint_loop_part2

SYS_debugPrint_loop:

	moveq.l	#0,d4
    move.b  (a0)+,d4
	beq.s	SYS_debugPrint_end

	move.b	(a2,d4.w),d4
	beq.s	SYS_debugPrint_noprint

    add.w   d4,d4
	add.w   d4,d4
	lea		-4(a3,d4.w),a4		    ; a4 = char bitmap adr

	move.b	(a4)+,(a1)			
SYSfastPrintautomod:
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4),2(a1)			

SYS_debugPrint_noprint:
	
	addq.l	#1,a1

SYS_debugPrint_loop_part2:
    moveq.l #0,d4
    move.b  (a0)+,d4
    beq.s   SYS_debugPrint_end

	move.b	(a2,d4.w),d4
	beq.s	SYS_debugPrint_noprint2
	add.w	d4,d4
	add.w	d4,d4

	lea		-4(a3,d4.w),a4		    ; a4 = char bitmap adr

	move.b	(a4)+,(a1)			
SYSfastPrintautomod2:
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4),2(a1)			

SYS_debugPrint_noprint2:

	add.w	d1,a1                   ; add  bitplanepitch - 2

	bra.s	SYS_debugPrint_loop

SYS_debugPrint_end:

	movem.l	(sp)+,a2-a4/d4

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
* VBL sub routines
*-------------------------------------------------------------------------
SYSvbldonothing:
    move.l  (a0)+,a1
    jmp     (a1)

SYSvblrunRTSroutine:
    move.l  (a0)+,a1
    
    move.l  a0,-(sp)
    jsr     (a1)
    move.l  (sp)+,a0

    move.l  (a0)+,a1
    jmp     (a1)

*-------------------------------------------------------------------------
* Interupt routines
*-------------------------------------------------------------------------

SYSvblinterrupt:

	move.w	#$2700,sr					; blocks all interupts => will be restored by rte

	move.l	d0,-(sp)
    move.l  a0,-(sp)
    move.l  a1,-(sp)

    lea     SYSvblroutines(pc),a0
    move.l  (a0)+,a1
    jmp     (a1)

SYSvblend:

	* -------------------- floppy management -------------------------

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

    move.l  (sp)+,a1
    move.l  (sp)+,a0
	move.l	(sp)+,d0

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
	
	addq.l	#1,SYSvblcount

	rte


SYSvblroutines:
    dc.l    SYSvblend 
    dc.l    SYSvblend 
    dc.l    SYSvblend 
    dc.l    SYSvblend 
    dc.l    SYSvblend 
    dc.l    SYSvblend 

SYSemuDescriptor:
    dc.l    0
    dc.l    0
    dc.l    0

SYSvblcount:
	dc.w	0
SYSvblLcount:
	dc.w	0

mainthreadcontext:
	dc.l	0		; current sp backup

idlethreadcontext:
	dc.l	0		; current sp backup
	dc.l	0		; stack low
	dc.l	0		; stack high

	END
