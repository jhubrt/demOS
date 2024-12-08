*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2024 J.Hubert
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

	output  D:\PROJECTS\DEMOS\OUTPUT\RELAPSE\INTERLUD.PRX

	opt		o+

*--------------------------------------------------------------------------------------------------------
cpu:		set		0
testmode:	set		0
*--------------------------------------------------------------------------------------------------------

modulebootstrap:
	lea		.exporttable(pc),a0
	rts

.exporttable:
	dc.l	init
	dc.l	hibordervbl
	dc.l	SNDblitzDecodeYM
gencode:
	dc.l	0
tun1:
	dc.w	0
tun2:
	dc.w	0
framebuffer:
	dc.l	0
colorsbuffer:
	dc.l	0
briksaddresses:
	dc.l	0
frame2ndwords:
	dc.l	0
plane3colors:
	dc.l	0
flexisource:
	dc.l	0
flexidest:
	dc.l	0


init:
	move.l	#hbl,$68.w
	rts

;-------------------------------------------------------------------------
; High border
;-------------------------------------------------------------------------

regpreload:
    dc.w	$C002	; d2
    dc.w	1		; d3
    dc.w	-2		; d4
    dc.w    -9		; d5
    dc.w	-17		; d6
	dc.w	-1		; d7

timerb1:
	clr.b	$fffffa1b.w			; Arret Timer B

	move.w	#$2100,sr			; /
	stop	#$2100				;  > Synchro HBL
	move.w	#$2700,sr			; /

	movem.l	d0-d7/a0-a6,-(sp)

	lea		colorsbuffer(pc),a6				; 8

	move.l	#$0002fff2,$ffff8a20.w			; source x & y increment		24
	move.l	(a6)+,$ffff8a24.w				; source address				24		colorsbuffer(pc)
	move.l	#-1,$ffff8a28.w					; endmask 1 & 2					24
	move.w	#-1,$ffff8a2c.w					; endmask 1 & 2					16
	ifeq testmode
	move.l	#$0002fff2,$ffff8a2e.w			; destination x & y increment	24
	endc
	ifne testmode
	move.l	#0,$ffff8a2e.w					; destination x & y increment	24
	endc
	move.l	#$ffff8250,$ffff8a32.w			; destination address			24
	move.w	#8,$ffff8a36.w					; x count						16
	move.w	#$0203,$ffff8a3a.w				; HOP & OP						16
	move.l  (a6)+,a2						;								12		briksaddresses(pc)
	move.l	(a6)+,a5						;								12		frame2ndwords(pc)
	move.l	(a6)+,d0						;								12		plane3colors(pc)

	lea		$ffff820a.w,a6		; 8

	rept	2
	nop
	endr

	; rept 	54					; \
	; nop						;  > Attend 82 nops
	; endr						; /

	move.w	d2,(a6)				; Passe en 60 Hz interne

	movem.w	regpreload(pc),d2-d7	; 12 + 4 + 4*6

	lea		$ffff8a26.w,a3		; 8
	lea		$ffff8a38.w,a4		; 8

	rept	2					; \
	nop							;  > Attend 15 nops
	endr						; /

	; rept	16					; \
	; nop						;  > Attend 15 nops
	; endr						; /

	move.b	d2,(a6)				; Passe en 50 Hz externe

.synclsl:
	move.b	$ffff8209.w,d1
	beq.s	.synclsl
	neg.b	d1
	add.b	#31,d1
	lsl.l	d1,d1

	move.w	#0,$ffff8240.w		; 16

	move.w	tun1(pc),d1
.wait:
	dbra	d1,.wait
	
	move.w	tun2(pc),d1
	lsl.w	d1,d1

	move.l	flexisource(pc),a0				; 16
	move.l	flexidest(pc),a1				; 16
	move.l	gencode(pc),a6					; 16
	move.b	framebuffer+3(pc),$ffff8209.w	; 20

	jsr		(a6)

	move.b	#2,$ffff820a.w
	move.b	#0,$ffff8260.w

	movem.l	(sp)+,d0-d7/a0-a6

	clr.l	$120.w

hbl:
	rte							; Retour s/p it

;-------------------------------------------------------------------------
hibordervbl:
		
	clr.b	$fffffa1b.w			; Arret Timer B
	move.b	#$64,$fffffa21.w	; $65 dans registre de donn‚es
	move.b	#4,$fffffa1b.w		; 200 pr‚-divisions
	bset.b	#0,$fffffa07.w		; Autorise it Timer B
	move.l	#timerb1,$120.w		; Revectorise Timer B

	CLR.B	$FFFF8260.W
	move.B	#2,$FFFF820a.W

	move.l  (a0)+,a1
	jmp		(a1)

;-------------------------------------------------------------------------

HW_YM_SEL_FREQCHA_L:            set 0
HW_YM_SEL_FREQCHA_H:            set 1
HW_YM_SEL_FREQCHB_L:            set 2
HW_YM_SEL_FREQCHB_H:            set 3
HW_YM_SEL_FREQCHC_L:            set 4
HW_YM_SEL_FREQCHC_H:            set 5
HW_YM_SEL_FREQNOISE:            set 6
HW_YM_SEL_IO_AND_MIXER:         set 7
HW_YM_SEL_LEVELCHA:             set 8
HW_YM_SEL_LEVELCHB:             set 9
HW_YM_SEL_LEVELCHC:             set 10
HW_YM_SEL_FREQENVELOPE_L:       set 11
HW_YM_SEL_FREQENVELOPE_H:       set 12
HW_YM_SEL_ENVELOPESHAPE:        set 13

HW_YM_REGSELECT:                set $FFFF8800
HW_YM_REGDATA:                  set $FFFF8802

SND_YM_NB_CHANNELS:             set 3


SNDblitzDecodeYM:

    movem.l d0-d2/a1-a2,-(sp)

    move.l  a0,a1

    ;-------------------------------------------------------------------------------
    ; YM decode
    ;-------------------------------------------------------------------------------

    lea         HW_YM_REGSELECT.w,a2

    move.b      (a1)+,d0                                          ; u8 ymvoicedata = *blitzdata++
    beq         .end                                              ; if (ymvoicedata != 0)

    move.w      #$2700,sr

    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nomixerdata                                      ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_IO_AND_MIXER*256,d1                    
    move.b      (a1)+,d1                                          ; u8 mixer = *blitzdata++
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, mixer)
.nomixerdata:


    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nofreqA                                          ; if (ymvoicedata & 0x80)
    moveq.l     #0,d1                                             ; = move.w #HW_YM_SEL_FREQCHA_L,d1                   
    move.b      (a1)+,d1                                          ; u8 freqL = *blitzdata++
    move.w      #HW_YM_SEL_FREQCHA_H*256,d2                       ; s8 freqH = *blitzdata++
    move.b      (a1)+,d2
    bge.s       .norestartA                                       ; if (freqH < 0)
    move.l      #HW_YM_SEL_FREQCHA_L*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (t << 1), 0)
    move.l      #HW_YM_SEL_FREQCHA_H*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (t << 1), 0)
.norestartA:
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHA_L, freqL);
    movep.w     d2,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHA_L, freqH);
.nofreqA:


    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nofreqB                                          ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_FREQCHB_L*256,d1                   
    move.b      (a1)+,d1                                          ; u8 freqL = *blitzdata++
    move.w      #HW_YM_SEL_FREQCHB_H*256,d2                       ; s8 freqH = *blitzdata++
    move.b      (a1)+,d2
    bge.s       .norestartB                                       ; if (freqH < 0)
    move.l      #HW_YM_SEL_FREQCHB_L*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_L + (t << 1), 0)
    move.l      #HW_YM_SEL_FREQCHB_H*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_H + (t << 1), 0)
.norestartB:
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_L, freqL);
    movep.w     d2,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_H, freqH);
.nofreqB:


    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nofreqC                                          ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_FREQCHC_L*256,d1                   
    move.b      (a1)+,d1                                          ; u8 freqL = *blitzdata++
    move.w      #HW_YM_SEL_FREQCHC_H*256,d2                       ; s8 freqH = *blitzdata++
    move.b      (a1)+,d2
    bge.s       .norestartC                                       ; if (freqH < 0)
    move.l      #HW_YM_SEL_FREQCHC_L*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_L + (t << 1), 0)
    move.l      #HW_YM_SEL_FREQCHC_H*256*256*256,(a2)             ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_H + (t << 1), 0)
.norestartC:
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_L, freqL);
    movep.w     d2,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQCHB_H, freqH);
.nofreqC:


    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nofreqenv                                        ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_FREQENVELOPE_L*256,d1                  
    move.b      (a1)+,d1                                          ; u8 freqL = *blitzdata++
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_L, freqL)
    move.w      #HW_YM_SEL_FREQENVELOPE_H*256,d1                  ; 
    move.b      (a1)+,d1                                          ; u8 freqH = *blitzdata++
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_H, freqH)
.nofreqenv:

    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nolevelA_noisefreq                               ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_LEVELCHA*256,d1                        
    move.b      (a1)+,d1                                          ; s8 levelA = *blitzdata++
    bge.s       .nonoisefreq                                      ; if (levelA < 0)
    move.w      #HW_YM_SEL_FREQNOISE*256,d2
    move.b      (a1)+,d2                                          ; u8 noisefreq = *blitzdata++;
    movep.w     d2,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_FREQNOISE, noisefreq)
.nonoisefreq:
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_LEVELCHA, levelA)
.nolevelA_noisefreq:

    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nolevelB_envshape                                ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_LEVELCHB*256,d1                        
    move.b      (a1)+,d1                                          ; s8 levelB = *blitzdata++
    bge.s       .noenvshape                                       ; if (levelB < 0)
    move.w      #HW_YM_SEL_ENVELOPESHAPE*256,d2
    move.b      (a1)+,d2                                          ; u8 envshape = *blitzdata++;
    movep.w     d2,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_ENVELOPESHAPE, envshape)
.noenvshape:
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_LEVELCHB, levelB)
.nolevelB_envshape:

    add.b       d0,d0                                             ; ymvoicedata <<= 1
    bcc.s       .nolevelC                                         ; if (ymvoicedata & 0x80)
    move.w      #HW_YM_SEL_LEVELCHC*256,d1                        
    move.b      (a1)+,d1                                          ; s8 levelC = *blitzdata++
    movep.w     d1,(a2)                                           ; BLS_YM_SET_REG(HW_YM_SEL_LEVELCHC, levelC)
.nolevelC:

.end:

    move.l  a1,a0

    movem.l (sp)+,d0-d2/a1-a2

    rts
