;-----------------------------------------------------------------------------------------------
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
;-------------------------------------------------------------------------------------------------

       output	D:\PROJECTS\DEMOS\OUTPUT\SYNTHYMZ.O

       xdef SNDblitzDecodeYM


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
