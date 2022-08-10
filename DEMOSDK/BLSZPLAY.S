; -------------------------------------------------------------------------------------------------------
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
; --------------------------------------------------------------------------------------------------------

	opt 	o+

managevolumeoffset:     set 0

;-------------------------------------------------------------------------------------
; set exportall to 1 when you want to export all routines separately
;-------------------------------------------------------------------------------------
exportall:  set 0

    IFNE    exportall
	
    output	D:\PROJECTS\DEMOS\OUTPUT\BLSPLAY.O

    xdef    ablzUpAllVoices,ablzUpSoundBuffers,ablzUp2SoundBuffers,ablzSetDMABuffer,ablzBlitzDecode
    
    ELSE

	output	D:\PROJECTS\DEMOS\OUTPUT\BLZPLAY.O

    xdef    aBLZupdate,aBLZ2update,aBLZupdAsync
    xdef    aBLZvbl,aBLZ2vbl
    xdef    aBLZplayer,aBLZbackground

    ENDC

    ; xref    SYSdbgBreak

BENCHMARK:               set 0

BLS_NBSAMPLES_PERFRAME:  set 1000
BLS_NBBYTES_PERFRAME:    set BLS_NBSAMPLES_PERFRAME*2
BLS_NBBYTES_OVERHEAD:    set 4
BLS_NBBYTES_CLEARFLAGS:  set 4
BLS_STEP_PERFRAME:       set (BLS_NBBYTES_PERFRAME+BLS_NBBYTES_OVERHEAD+BLS_NBBYTES_CLEARFLAGS)
BLS_NBPATTERNROWS:       set 64

BLSbs_STEP0:             set 0
BLSbs_STEP1:             set 4
BLSbs_STEP2:             set 8
BLSbs_START:             set 12

BLS_BASEKEY_FLAG:        set 1

BLS_BLITZ_PATTERN_DATA_MASK:  set $7FFFF

HW_BLITTER_HTONE:        set $FFFF8A00
HW_BLITTER_XINC_SOURCE:  set $FFFF8A20
HW_BLITTER_YINC_SOURCE:  set $FFFF8A22      
HW_BLITTER_ADDR_SOURCE:  set $FFFF8A24      

HW_BLITTER_ENDMASK1:     set $FFFF8A28  
HW_BLITTER_ENDMASK2:     set $FFFF8A2A 
HW_BLITTER_ENDMASK3:     set $FFFF8A2C  

HW_BLITTER_XINC_DEST:    set $FFFF8A2E 
HW_BLITTER_YINC_DEST:    set $FFFF8A30  
HW_BLITTER_ADDR_DEST:    set $FFFF8A32  

HW_BLITTER_XSIZE:        set $FFFF8A36  
HW_BLITTER_YSIZE:        set $FFFF8A38  

HW_BLITTER_HOP:          set $FFFF8A3A
HW_BLITTER_OP:           set $FFFF8A3B   
HW_BLITTER_CTRL1:        set $FFFF8A3C   
HW_BLITTER_CTRL2:        set $FFFF8A3D  

HW_BLITTER_CTRL2_FORCE_XTRA_SRC:    set $80
HW_BLITTER_CTRL2_NO_FINAL_SRC_READ: set $40

HW_BLITTER_HOP_BIT1:             set 0     
HW_BLITTER_HOP_SOURCE:           set 2
HW_BLITTER_HOP_SOURCE_AND_HTONE: set 3

HW_BLITTER_OP_BIT0:              set 0     
HW_BLITTER_OP_S:                 set 3

HW_BLITTER_CTRL1_BUSY:           set $80
HW_BLITTER_CTRL1_HOGMODE_BLIT:   set $40

HW_DMASOUND_STARTADR_H:		    set $FFFF8903
HW_DMASOUND_STARTADR_M:		    set $FFFF8905
HW_DMASOUND_STARTADR_L:		    set $FFFF8907

HW_DMASOUND_ENDADR_H:		    set $FFFF890F
HW_DMASOUND_ENDADR_M:		    set $FFFF8911
HW_DMASOUND_ENDADR_L:		    set $FFFF8913

HW_DMASOUND_COUNTER_H:          set $FFFF8909
HW_DMASOUND_COUNTER_M:          set $FFFF890B
HW_DMASOUND_COUNTER_L:          set $FFFF890D

HW_DMASOUND_CONTROL:            set $FFFF8901

HW_DMASOUND_CONTROL_PLAYONCE:   set	1
HW_DMASOUND_CONTROL_PLAYLOOP:   set	3

HW_MICROWIRE_MASK_SOUND:        set	$7FF
HW_MICROWIRE_VOLUME_LEFT:       set $540
HW_MICROWIRE_VOLUME_RIGHT:      set	$500
HW_MICROWIRE_DATA:              set $FFFF8922
HW_MICROWIRE_MASK:              set $FFFF8924

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

ArpeggioState_STOPPED:          set 0
ArpeggioState_STARTING:         set 4
ArpeggioState_RUNNING:          set 8
ArpeggioState_STOPPING:         set 12

BLSvoice_sizeof:                    set 32
BLSvoice_keys:                      set 0
BLSvoice_current:                   set BLSvoice_keys+12
BLSvoice_currentsource:             set BLSvoice_current+4
BLSvoice_mask:                      set BLSvoice_currentsource+4
BLSvoice_sampledelay:               set BLSvoice_mask+2           ; should be contiguous to retrigger and arpeggioState and aligned on 4 (optimization)
BLSvoice_volume:                    set BLSvoice_sampledelay+1    
BLSvoice_retrigger:                 set BLSvoice_volume+1
BLSvoice_arpeggioState:             set BLSvoice_retrigger+1
BLSvoice_mute_unused:               set BLSvoice_arpeggioState+1
BLSvoice_retriggercount_unused:     set BLSvoice_mute_unused+1
BLSvoice_arpeggioOffset:            set BLSvoice_retriggercount_unused+1
BLSvoice_volumeoffset:              set BLSvoice_arpeggioOffset+1

BLSplayer_sndtrack:                 set BLSvoice_sizeof*4
BLSplayer_buffer:                   set BLSplayer_sndtrack+4
BLSplayer_buffertoupdate:           set BLSplayer_buffer+4
BLSplayer_dmabufstart:              set BLSplayer_buffertoupdate+4
BLSplayer_dmabufend:                set BLSplayer_dmabufstart+4
BLSplayer_bufferstate:              set BLSplayer_dmabufend+4
BLSplayer_blizcurrent:	            set BLSplayer_bufferstate+2
BLSplayer_patternend:               set BLSplayer_blizcurrent+4
BLSplayer_volumeLeft:               set BLSplayer_patternend+4
BLSplayer_volumeRight:              set BLSplayer_volumeLeft+2
BLSplayer_volumeLeft2:              set BLSplayer_volumeRight+2
BLSplayer_volumeRight2              set BLSplayer_volumeLeft2+2
BLSplayer_trackindex:               set BLSplayer_volumeRight2+2
BLSplayer_tracklooped:              set BLSplayer_trackindex+1
BLSplayer_framenum:                 set BLSplayer_tracklooped+1
BLSplayer_maxframenum:              set BLSplayer_framenum+2

BLSsndtrack_samples:                set 0
BLSsndtrack_keys:                   set 4
BLSsndtrack_framesdata:             set 8
BLSsndtrack_patterndata:            set 12
BLSsndtrack_patternlength:          set 16
BLSsndtrack_track:                  set 20
BLSsndtrack_tracklen:               set 24

BLSprecomputedKey_sampleIndex:      set 0
BLSprecomputedKey_blitterTranspose: set 1
BLSprecomputedKey_freqmul:          set 2
BLSprecomputedKey_freqdiv:          set 4
BLSprecomputedKey_freqmulshift:     set 6
BLSprecomputedKey_freqdivshift:     set 7

BLSsample_sample:                   set 0
BLSsample_sampleLen:                set 4
BLSsample_sampleLoopStart:          set 8
BLSsample_sampleLoopLength:         set 12
BLSsample_flags:                    set 14    

BLS_SAMPLE_STORAGE_SHIFT_BIT:       set 2    
BLS_SAMPLE_STORAGE_INTERLACE_BIT:   set 3

BLIT_COLOR:                         set $700
NOBLIT_COLOR:                       set $70
UPDBUFFER_COLOR:                    set $7
BLITZDECODE_COLOR:                  set $47
RUNFX_COLOR:                        set $77 


    IFNE BENCHMARK

RASTERIZE MACRO
    move.w  #\1,$ffff8240.w
    ENDM

    ELSE

RASTERIZE MACRO
    ENDM

    ENDC


    IFEQ        exportall

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

; ------------------------------------------------------------------------
; VBL wrapper when using DMA sound in playonce mode
; ------------------------------------------------------------------------
aBLZ2vbl:
    move.b      #HW_DMASOUND_CONTROL_PLAYONCE,HW_DMASOUND_CONTROL.w

    move.l  a0,-(sp)
    lea     aBLZbackground(pc),a0
    move.w  (a0)+,$ffff8240.w
    move.l  (a0)+,a0
    bsr.s   aBLZ2update
    move.l  (sp)+,a0

    move.l  (a0)+,a1
    jmp     (a1)

; ------------------------------------------------------------------------
; void aBLZupdate2 (BLSplayer* _player)
; ------------------------------------------------------------------------
aBLZ2update:
    movem.l     a1-a6/d0-d7,-(sp)
    addq.w      #1,BLSplayer_framenum(a0)

    ENDC

; ------------------------------------------------------------------------
; void ablz2UpdateSoundBuffers (BLSplayer* _player)
; ------------------------------------------------------------------------
ablzUp2SoundBuffers:

    RASTERIZE   UPDBUFFER_COLOR

    IFNE        exportall
    movem.l     d1-d2/a1-a2,-(sp)
    ENDC

    ; microwire left
    move.w      BLSplayer_volumeLeft(a0),d2
    beq.s       .donotsetvolumeleft
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d2,HW_MICROWIRE_DATA.w
.donotsetvolumeleft:

    move.l      BLSplayer_buffer(a0),d1             ; d1 = buffer

    move.w      BLSplayer_bufferstate(a0),d2        ; switch(_player->bufferstate)    
    move.l      .switchBufferState(pc,d2.w),a2      
    jmp         (a2)

.switchBufferState:
    dc.l        .BLSbs_STEP0_CASE
    dc.l        .BLSbs_STEP1_CASE
    dc.l        .BLSbs_STEP2_CASE
    dc.l        .BLSbs_START_CASE

.BLSbs_START_CASE:
.BLSbs_STEP0_CASE:
    move.l      d1,BLSplayer_buffertoupdate(a0)     
    add.l       #BLS_STEP_PERFRAME+BLS_STEP_PERFRAME,d1
    move.l      d1,BLSplayer_dmabufstart(a0)

    move.w      #BLSbs_STEP1,BLSplayer_bufferstate(a0)
    bra.s       .endswitch
      
.BLSbs_STEP1_CASE:
    move.l      d1,d2
    add.l       #BLS_STEP_PERFRAME,d2
    move.l      d2,BLSplayer_buffertoupdate(a0)
    move.l      d1,BLSplayer_dmabufstart(a0)

    move.w      #BLSbs_STEP2,BLSplayer_bufferstate(a0)
    bra.s       .endswitch

.BLSbs_STEP2_CASE:
    move.l      d1,d2
    add.l       #BLS_STEP_PERFRAME+BLS_STEP_PERFRAME,d2
    move.l      d2,BLSplayer_buffertoupdate(a0)
    add.l       #BLS_STEP_PERFRAME,d1
    move.l      d1,BLSplayer_dmabufstart(a0)

    move.w      #BLSbs_STEP0,BLSplayer_bufferstate(a0)

.endswitch:
   
    add.l       #BLS_NBBYTES_PERFRAME,d1
    move.l      d1,BLSplayer_dmabufend(a0)

    ; microwire right
    move.w      BLSplayer_volumeRight(a0),d2
    beq.s       .donotsetvolumeright
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d2,HW_MICROWIRE_DATA.w
.donotsetvolumeright:

    move.l      BLSplayer_volumeLeft2(a0),BLSplayer_volumeLeft(a0)
    clr.l       BLSplayer_volumeLeft2(a0)

    IFNE        exportall
    movem.l     (sp)+,d1-d2/a1-a2
    rts
    ENDC

    bra         ablzBlitzDecode
; ------------------------------------------------------------------------

; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

; ------------------------------------------------------------------------
; void aBLZupdAsync(BLSplayer* _player)
; ------------------------------------------------------------------------
aBLZupdAsync:
    movem.l     a1-a6/d0-d7,-(sp)
    
    addq.w      #1,BLSplayer_framenum(a0)
    clr.l       BLSplayer_dmabufstart(a0)
    bra         ablzBlitzDecode


; ------------------------------------------------------------------------
; ------------------------------------------------------------------------

aBLZbackground: dc.w    0
aBLZplayer:     dc.l    0

; ------------------------------------------------------------------------
; VBL wrapper when using DMA sound in playloop mode
; ------------------------------------------------------------------------

    IFEQ        exportall

aBLZvbl:
    move.l  a0,-(sp)
    lea     aBLZbackground(pc),a0
    move.w  (a0)+,$ffff8240.w
    move.l  (a0)+,a0
    bsr.s   aBLZupdate
    move.l  (sp)+,a0

    move.l  (a0)+,a1
    jmp     (a1)

aBLZupdate:
    movem.l     a1-a6/d0-d7,-(sp)
    addq.w      #1,BLSplayer_framenum(a0)

    ENDC

; ------------------------------------------------------------------------
; void ablzUpdateSoundBuffers (BLSplayer* _player)
; ------------------------------------------------------------------------
ablzUpSoundBuffers:

    RASTERIZE   UPDBUFFER_COLOR

    IFNE        exportall
    movem.l     d0-d4/a1-a2,-(sp)
    ENDC

    move.l      BLSplayer_dmabufstart(a0),d3        ; d3 = dmabufstart
    move.l      BLSplayer_dmabufend(a0),d4          ; d4 = dmabufend

    move.l      BLSplayer_buffer(a0),d1             ; d1 = buffer

    move.w      BLSplayer_bufferstate(a0),d2        ; switch(_player->bufferstate)
    move.l      .switchBufferState(pc,d2.w),a2      

    lea         HW_DMASOUND_COUNTER_H-2.w,a1
    movep.l     (a1),d0
    and.l       #$FFFFFF,d0                         ; d0 = readcursor just before starting at start step

    jmp         (a2)

.switchBufferState:
    dc.l        .BLSbs_STEP0_CASE
    dc.l        .BLSbs_STEP1_CASE
    dc.l        .BLSbs_STEP2_CASE
    dc.l        .BLSbs_START

.BLSbs_STEP0_CASE:
    move.w      #BLSbs_STEP1,BLSplayer_bufferstate(a0)
    move.l      d1,BLSplayer_buffertoupdate(a0)     
    add.l       #BLS_STEP_PERFRAME+BLS_STEP_PERFRAME,d1
    bra.s       .endswitch
      
.BLSbs_STEP1_CASE:
    move.w      #BLSbs_STEP2,BLSplayer_bufferstate(a0)
    move.l      d1,d2
    add.l       #BLS_STEP_PERFRAME,d2
    move.l      d2,BLSplayer_buffertoupdate(a0)
    bra.s       .endswitch

.BLSbs_START:
    move.b      #HW_DMASOUND_CONTROL_PLAYLOOP,HW_DMASOUND_CONTROL.w   
    moveq.l     #-1,d0                              ; readcursor = -1

.BLSbs_STEP2_CASE:
    move.w      #BLSbs_STEP0,BLSplayer_bufferstate(a0)
    move.l      d1,d2
    add.l       #BLS_STEP_PERFRAME+BLS_STEP_PERFRAME,d2
    move.l      d2,BLSplayer_buffertoupdate(a0)
    add.l       #BLS_STEP_PERFRAME,d1

.endswitch:

    ; microwire left
    move.w      BLSplayer_volumeLeft(a0),d2
    beq.s       .donotsetvolumeleft
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d2,HW_MICROWIRE_DATA.w
.donotsetvolumeleft:

    ; manage dynamically buffer length
    move.l      d1,BLSplayer_dmabufstart(a0)
    
    add.l       #BLS_NBBYTES_PERFRAME,d1

    cmp.l       d0,d3
    bgt.s       .donotenlargeplaybuffer
    cmp.l       d0,d4
    blt.s       .donotenlargeplaybuffer
    addq.l      #BLS_NBBYTES_OVERHEAD,d1
.donotenlargeplaybuffer:

    move.l      d1,BLSplayer_dmabufend(a0)

    ; microwire right
    move.w      BLSplayer_volumeRight(a0),d2
    beq.s       .donotsetvolumeright
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d2,HW_MICROWIRE_DATA.w
.donotsetvolumeright:

    move.l      BLSplayer_volumeLeft2(a0),BLSplayer_volumeLeft(a0)
    clr.l       BLSplayer_volumeLeft2(a0)

    IFNE        exportall
    movem.l     (sp)+,d0-d4/a1-a2
    rts
    ENDC

; ------------------------------------------------------------------------
; void ablzBlitzDecode(BLSplayer* _player)
; ------------------------------------------------------------------------
ablzBlitzDecode:

	RASTERIZE   BLITZDECODE_COLOR
    
    IFNE        exportall
    movem.l     d0-d7/a1-a4/a6,-(sp)
    ENDC

	move.l		BLSplayer_blizcurrent(a0),a1		              ; u8*  blitzdata = _player->blizcurrent;            A1 = blitzdata

    ;-------------------------------------------------------------------------------
    ; YM decode
    ;-------------------------------------------------------------------------------

    lea         HW_YM_REGSELECT.w,a2

    move.b      (a1)+,d0                                          ; u8 ymvoicedata = *blitzdata++
    beq         .pcmdecode                                        ; if (ymvoicedata != 0)

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


    ;-------------------------------------------------------------------------------
    ; PCM decode
    ;-------------------------------------------------------------------------------
.pcmdecode:

    move.l      BLSplayer_sndtrack(a0),a6           ;                                                   A6 = sndtrack    
    move.l      BLSsndtrack_keys(a6),a2             ; BLSsoundTrack* sndtrack = _player->sndtrack->keys A2 = keys table    
	move.l		a0,a3								; BLSvoice* voice = _player->voices;                A3 = voice

	move.b		(a1)+,d0							; u8   voicedata = *blitzdata++;                    D0 = voicedata
    moveq.l     #3,d6                               ; u8   t                                            D6 = voiceloop counter 
    moveq.l     #3,d7                               ; preload 3 in D7 

.loop:

    add.b       d0,d0                               ; 
    bcc         .novoicedesc                        ;

    move.b      (a1)+,d2                            ; u8 voicedesc = *blitzdata++;                      D2 = voicedesc

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_KEYINDEX) 
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .nokeyindex
    moveq.l     #0,d3    

    move.l      d3,BLSvoice_current(a3)             ; voice->current = 0;
    move.l      d3,BLSvoice_currentsource(a3)       ; voice->currentsource = 0;
    move.l      d3,BLSvoice_sampledelay(a3)         ; voice->volume = 0; voice->arpeggioState = ArpeggioState_STOPPED; voice->retrigger = 0; voice->sampledelay = 0;

    move.b      (a1)+,d3
    beq.s       .nullkey
    subq.w      #1,d3
    lsl.w       d7,d3                               ; sizeof(BLSprecomputedKey) == 8
    add.l       a2,d3
.nullkey:
    move.l      d3,(a3)                             ; BLSvoice_keys(a3)         voice->keys[0] = &keys[*blitzdata++];
.nokeyindex:

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_ARPEGIOSTART)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .noarpeggiostart

    moveq.l     #0,d3    
    move.b      (a1)+,d3
    lsl.w       d7,d3                               ; sizeof(BLSprecomputedKey) == 8
    add.l       a2,d3
    move.l      d3,BLSvoice_keys+4(a3)              ; voice->keys[1] = &keys[*blitzdata++];                             

    moveq.l     #0,d3    
    move.b      (a1)+,d3
    lsl.w       d7,d3                               ; sizeof(BLSprecomputedKey) == 8
    add.l       a2,d3
    move.l      d3,BLSvoice_keys+8(a3)              ; voice->keys[2] = &keys[*blitzdata++];                             

    tst.b       BLSvoice_arpeggioState(a3)
    bne.s       .noarpeggiostart
    move.b      #ArpeggioState_STARTING,BLSvoice_arpeggioState(a3)
.noarpeggiostart:

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_ARPEGIOSTOP)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .noarpeggiostop
;    tst.b       d1                                  ; if (newnote == false)
;   bne.s       .noarpeggiostop
    tst.b       BLSvoice_arpeggioState(a3)          ; if (voice->arpeggioState != ArpeggioState_STOPPED)
    beq.s       .noarpeggiostop
    move.b      #ArpeggioState_STOPPING,BLSvoice_arpeggioState(a3) ; voice->arpeggioState = ArpeggioState_STOPPING;
.noarpeggiostop:

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_MASK)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2 
    bcc.s       .nodecodemask
    move.b      (a1),BLSvoice_mask(a3)              ; voice->mask  = *blitzdata++;
    move.b      (a1)+,BLSvoice_mask+1(a3)           ; voice->mask |= (u16)voice->mask << 8;
.nodecodemask:

    ;-------------------------------------------------------------------------------
    ; if  (voicedesc & BLSZ_DECODEMASK_RETRIGGERSAMPLE)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .noretriggersample
    clr.l       BLSvoice_current(a3)                ; voice->current = 0;
.noretriggersample:

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_VOLUME)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .novolume                           ; voice->volume = *blitzdata++;
    move.b      (a1)+,BLSvoice_volume(a3)
.novolume:

    ;-------------------------------------------------------------------------------
    ; if (voicedesc & BLSZ_DECODEMASK_SETSAMPLEOFFSET)
    ;-------------------------------------------------------------------------------
    add.b       d2,d2
    bcc.s       .nosetsampleoffset
    moveq.l     #0,d3   
    move.b      (a1)+,d3    
    swap        d3
    move.b      (a1)+,d3    
    lsl.w       #8,d3
    move.b      (a1)+,d3       
    move.l      d3,BLSvoice_current(a3)             

    ; u32 offset = *blitzdata++;
    ; offset <<= 8;
    ; offset |= *blitzdata++;
    ; offset <<= 8;
    ; offset |= *blitzdata++;
    ; voice->current = offset;

.nosetsampleoffset:

    ;-------------------------------------------------------------------------------
    ; Free code available for something else... if (voicedesc & BLSZ_DECODEMASK_CLIENTEVENT)
    ;-------------------------------------------------------------------------------
    ; add.b       d2,d2
    ; bcc.s       .noclientdata
    ; move.b      (a1)+,BLSvoice_clientevent(a3)      ; voice->clientEvent = *blitzdata++;
; .noclientdata:

.novoicedesc:
    
	btst.l		#4,d0
    seq.b		BLSvoice_sampledelay(a3)            ; enforce                                                    

    ;-------------------------------------------------------------------------------
    ; update arpeggio
    ;-------------------------------------------------------------------------------
    moveq.l #0,d1                                               ; d1 is 0 or 1 (newnote not used anymore) => optimize
    move.b  BLSvoice_arpeggioState(a3),d1                       
    move.l  .arpeggioswitch(pc,d1.w),a4
    jmp     (a4)                                                ; switch (voice->arpeggioState)

.arpeggioswitch:
    dc.l        .BLSarpeggio_STOPPED_CASE
    dc.l        .BLSarpeggio_STARTING_CASE
    dc.l        .BLSarpeggio_RUNNING_CASE
    dc.l        .BLSarpeggio_STOPPING_CASE

.BLSarpeggio_STOPPING_CASE:            
    move.b      #ArpeggioState_STOPPED,BLSvoice_arpeggioState(a3) ; voice->arpeggioState = ArpeggioState_STOPPED;
    move.b      #8,BLSvoice_arpeggioOffset(a3)                    ; voice->arpeggioIndex = 2 << 2;
                                                                  ; /* no break here */           
.BLSarpeggio_RUNNING_CASE:
    move.b      BLSvoice_arpeggioOffset(a3),d1                    ; voice->arpeggioOffset += 4;
    addq.b      #4,d1
    cmp.b       #12,d1                                            ; if (voice->arpeggioIndex == 3 << 2)
    bne.s      .noarpeggioloop
    moveq.l     #0,d1                                             ; voice->arpeggioIndex = 0;
.noarpeggioloop:
    move.b      d1,BLSvoice_arpeggioOffset(a3)
    move.l      BLSvoice_keys(a3,d1.w),a4                         ; A4 = voice->keys[voice->arpeggioIndex] - d1 has a small value before (0 -> 12) => do not need to clear

    move.b      BLSprecomputedKey_freqmulshift(a4),d3             ; freqmulshift = voice->keys[voice->arpeggioIndex]->freqmulshift;
    move.w      BLSprecomputedKey_freqmul(a4),d4                  ; freqmul      = voice->keys[voice->arpeggioIndex]->freqmul;
    move.w      BLSvoice_currentsource+2(a3),d5                   ; v = STDmulu((u16)voice->currentsource, freqmul);
    mulu.w      d5,d4  
    lsr.l       d3,d4                                             ; v >>= freqmulshift;
    move.w      d4,BLSvoice_current+2(a3)                         ; voice->current = v;
                  
    lea         BLSvoice_sizeof(a3),a3    
    dbf.w       d6,.loop
    bra.s       .endloop

.BLSarpeggio_STARTING_CASE:                                       ; case ArpeggioState_STARTING:
    move.b      #ArpeggioState_RUNNING,BLSvoice_arpeggioState(a3) ; voice->arpeggioState = ArpeggioState_RUNNING;
                                                                  ; /* no break here */
.BLSarpeggio_STOPPED_CASE:                                        ; case ArpeggioState_STOPPED:   voice->arpeggioIndex = 0;
    clr.b       BLSvoice_arpeggioOffset(a3)
    
    lea         BLSvoice_sizeof(a3),a3
    dbf.w       d6,.loop

    ; end voices loop
    ;-------------------------------------------------------------------------------
.endloop:   


    ;-------------------------------------------------------------------------------
    ; manage track and current pattern assignation
    ;-------------------------------------------------------------------------------
.managepatterns:
    move.l  a1,BLSplayer_blizcurrent(a0)                        ; _player->blizcurrent = blitzdata;
    cmp.l   BLSplayer_patternend(a0),a1                         ; if (_player->blizcurrent >= _player->patternend)
    blt.s   .nopatterninc

    move.b  BLSplayer_trackindex(a0),d1                         ; _player->trackindex++;
    addq.b  #1,d1
    cmp.b   BLSsndtrack_tracklen(a6),d1                         ; if (_player->trackindex >= _player->sndtrack->trackLen)
    blt.s   .noloopsong
    moveq.l #0,d1                                               ; _player->trackindex = 0;
    move.b  #1,BLSplayer_tracklooped(a0)
    move.w  BLSplayer_framenum(a0),BLSplayer_maxframenum(a0)
    clr.w   BLSplayer_framenum(a0)
.noloopsong:        
    move.b  d1,BLSplayer_trackindex(a0)

    move.l  BLSsndtrack_track(a6),a4
    move.b  (a4,d1.w),d1                                         ; patternindex = _player->sndtrack->track[_player->trackindex]

    add.w   d1,d1
    moveq.l #0,d2
    move.l  BLSsndtrack_patternlength(a6),a4
    move.w  (a4,d1.w),d2

    move.l  BLSsndtrack_patterndata(a6),a4
    add.w   d1,d1
    move.l  (a4,d1.w),d0                                         ; patterndata    = _player->sndtrack->patternsdata[ patternindex ]

    add.l   BLSsndtrack_framesdata(a6),d0
    move.l  d0,BLSplayer_blizcurrent(a0)                         ; _player->blizcurrent = _player->sndtrack->framesdata + patterndata;

    add.l   d2,d0
    move.l  d0,BLSplayer_patternend(a0)                          ; _player->patternend  = _player->blizcurrent + patterndatalen;
.nopatterninc:

    IFNE exportall
    movem.l (sp)+,d0-d7/a1-a4/a6
    rts
    ENDC

; ------------------------------------------------------------------------
; void ablzUpAllVoices (BLSplayer* _player)
; ------------------------------------------------------------------------
ablzUpAllVoices:

    RASTERIZE   NOBLIT_COLOR

    IFNE exportall
    movem.l d0-d7/a1-a6,-(sp)
    move.l  BLSplayer_sndtrack(a0),a6               ;       else prelaoded by blitzdecode function
    ENDC

    move.l  BLSplayer_buffertoupdate(a0),a1
    
    moveq.l #-1,d0   
    lea     HW_BLITTER_XINC_DEST.w,a5

    move.l  #$40004,(a5)                            ;       *HW_BLITTER_XINC_DEST   = 4;  *HW_BLITTER_YINC_DEST   = 4; /* multiplexing */    
                                                   
    move.l  d0,-(a5)                                ;       *HW_BLITTER_ENDMASK1  = 0xFFFF;  *HW_BLITTER_ENDMASK2  = 0xFFFF;
    move.w  d0,-(a5)                                ;       *HW_BLITTER_ENDMASK3  = 0xFFFF;
                                                                                                     
    moveq.l #1,d0
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[1], (u8*) backbuf    , true );
    bsr.s   ablzUpdateVoiceFrame                   
                                                   
    moveq.l #1,d0                                  
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[2], (u8*) backbuf + 2, true );
    addq.l  #2,a1
    bsr.s   ablzUpdateVoiceFrame

    move.l  #$FF00FF00,HW_BLITTER_ENDMASK1.w        ;       *HW_BLITTER_ENDMASK1  = PCENDIANSWAP16(0xFF00); *HW_BLITTER_ENDMASK2  = PCENDIANSWAP16(0xFF00);
    move.w  #$FF00,HW_BLITTER_ENDMASK3.w            ;       *HW_BLITTER_ENDMASK3  = PCENDIANSWAP16(0xFF00);

    moveq.l #0,d0
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[3], (u8*) backbuf + 2, false);
    bsr.s   ablzUpdateVoiceFrame

    moveq.l #0,d0
    lea     -3*BLSvoice_sizeof(a0),a0               ;       blsUpdateVoiceFrame (&_player->voices[0], (u8*) backbuf    , false);
    subq.l  #2,a1
    bsr.s   ablzUpdateVoiceFrame   
    
    move.l  BLS_NBBYTES_PERFRAME-4(a1),BLS_NBBYTES_PERFRAME(a1)

    IFNE exportall
    movem.l (sp)+,d0-d7/a1-a6
    rts
    ENDC

;---------------------------------------------------------------------------------------
; ablzSetDMABuffer(BLSplayer* _player)
;---------------------------------------------------------------------------------------
; a0 = _player
;---------------------------------------------------------------------------------------
ablzSetDMABuffer:

    IFNE exportall
    move.l d0,-(sp)
    move.l a1,-(sp)
    ENDC

    move.l      BLSplayer_dmabufstart(a0),d0
    beq.s       .end
    swap        d0
    move.b      d0,HW_DMASOUND_STARTADR_H.w
    swap        d0
    lea         HW_DMASOUND_STARTADR_M.w,a1
    movep.w     d0,(a1)

    move.l      BLSplayer_dmabufend(a0),d0
    swap        d0
    move.b      d0,HW_DMASOUND_ENDADR_H.w
    swap        d0
    lea         HW_DMASOUND_ENDADR_M.w,a1
    movep.w     d0,(a1)

.end:
    IFNE exportall
    move.l (sp)+,a1
    move.l (sp)+,d0
    ELSE
    movem.l (sp)+,d0-d7/a1-a6
    ENDC

    RASTERIZE   0

    rts

;---------------------------------------------------------------------------------------
; blsUpdateVoiceFrame(BLSsoundTrack* _sndtrack, BLSvoice* _voice, u8* _buffer, bool _firstpass)
;---------------------------------------------------------------------------------------
; a0 = _voice
; a1 = _buffer;
; d0 = _firstpass           temp        temp
;
; d1                                                    temp            
; d2                        transfer                    sampleLoopLength
; d3    temp                sampleLen-scrCurrentSampl     
; d4    volume
; d5    transpose
; d6    scrCurrentSampl
; d7    remain
; 
; a2    sample
; a3    pkey
; a4    sampleData
; a5    temp
; a6    sndtrack
;---------------------------------------------------------------------------------------
ablzUpdateVoiceFrame:

    move.l  #BLS_NBSAMPLES_PERFRAME/2,d7                ; d7 = remain

    move.l  a1,HW_BLITTER_ADDR_DEST.w                     

    ;--------------------------------------------------
    ; manage muted voice optimization
    ;--------------------------------------------------
    lea     BLS_NBBYTES_PERFRAME+BLS_NBBYTES_OVERHEAD(a1),a3  ; a3 : u8*  cleared2 = _buffer + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    lea     (a3,d0.w),a2                                      ; a2 : u8*  cleared  = cleared2 + _firstpass;
    
    tst.l   (a0)                             
    beq.s   .managecleared                              ; if ((_voice->keys[0] != NULL)
    tst.b   BLSvoice_sampledelay(a0)
    beq.s   .play                                       ; && (_voice->sampledelay == 0))

.managecleared:   
    tst.b   (a2)
    beq.s   .needtoclear                                ; if (*cleared) return;
    tst.b   BLSvoice_sampledelay(a0)
    beq.s   .return
    subq.b  #1,BLSvoice_sampledelay(a0)
.return:
    rts

.needtoclear:
    moveq.l #1,d1
    move.b  d1,(a2)                                     ; (*cleared2) = true;
    move.b  d1,(a3)                                     ; (*cleared)  = true;
    bra     .clear                                      ; goto clear;

.play:
    moveq.l #0,d3
    move.b  d3,(a2)                                        ; *cleared2  = false;
    move.b  d3,(a3)                                        ; *cleared   = false;    

    ;--------------------------------------------------
    ; need to play something
    ;-------------------------------------------------- 
    move.b  BLSvoice_arpeggioOffset(a0),d3              ; d3 already preloaded with 0
    move.l  BLSvoice_keys(a0,d3.w),a3                   ; a3 = pkey                 _voice->keys[_voice->arpeggioIndex]
    
    move.b  (a3),d3                                     ; BLSprecomputedKey_sampleIndex(a3)             d3 = sampleindex          u8 sampleIndex = key->sampleIndex;

    move.b  BLSprecomputedKey_blitterTranspose(a3),d5   ; d5 = transpose            s8   transpose = _voice->keys[_voice->arpeggioIndex]->blitterTranspose >> 1
    asr.b   #1,d5
    bcs.s   .isbasekey                                  ; if (BLS_IS_BASEKEY(key) == false)
    lsl.w   #3,d3                                       ; sizeof(BLSprecomputedKey) == 8
    move.l  BLSsndtrack_keys(a6),a2
    add.w   d3,a2                                       
    moveq.l #0,d3
    move.b  (a2),d3                                     ; BLSprecomputedKey_sampleIndex(a2) sampleIndex = _sndtrack->keys[sampleIndex].sampleIndex;
.isbasekey:

    lsl.w   #4,d3                                       ; sizeof(BLSsample) == 16
    move.l  (a6),a2                                     ; BLSsndtrack_samples(a6),a2
    add.w   d3,a2                                       ; a2 = sample               sample = &_sndtrack->samples[sampleIndex];
    
    move.l  BLSvoice_current(a0),d6                     ; d6 = scrCurrentSampl      u32  scrCurrentSampl = _voice->current
    
    move.l  (a2),a4                                     ; BLSsample_sample(a2)      a4 = sample->sample       s8*  sampleData       = sample->sample;

    ;--------------------------------------------------            
    ; manage volume 

    move.b  BLSvoice_volume(a0),d4                      ; d4 = volume

    IFNE managevolumeoffset
    btst.b  #BLS_SAMPLE_STORAGE_INTERLACE_BIT,BLSsample_flags+1(a2)             ;   if ((sample->flags & BLS_SAMPLE_STORAGE_INTERLACE) == 0)
    bne.s   .novolumefx   
    add.b   BLSvoice_volumeoffset(a0),d4                ;                           volume += _voice->volumeoffset;
    cmp.b   #8,d4                                                  
    ble.s   .novolumefx                                               
    moveq.l #8,d4                                       ;                           if (volume > 8) volume = 8
.novolumefx:
    ENDC

    ;--------------------------------------------------            
    ; manage bit shift (volume and pass)

    lsl.w   #3,d0                                       ; _firstpass 0 or 1 => 0 or 8

    btst.b  #BLS_SAMPLE_STORAGE_SHIFT_BIT,BLSsample_flags+1(a2) ;                   
    bne.s   .endvolume                                  ;                           if (storageshift)
       
    tst.b   d0                                          ;                           if (_firstpass)
    bne.s   .endvolume2
    move.b  d4,d0                                       ;                           *HW_BLITTER_CTRL2 = volume
    add.b   #8+HW_BLITTER_CTRL2_FORCE_XTRA_SRC+HW_BLITTER_CTRL2_NO_FINAL_SRC_READ,d0 ; *HW_BLITTER_CTRL2 = (8 | HW_BLITTER_CTRL2_FORCE_XTRA_SRC | HW_BLITTER_CTRL2_NO_FINAL_SRC_READ) + volume
    bra.s   .endvolume
.endvolume2:
    move.b  d4,d0                                       ;                           *HW_BLITTER_CTRL2 = volume
.endvolume:
    move.b  d0,HW_BLITTER_CTRL2.w

    ;--------------------------------------------------            
    ; manage logical operation (volume and bitmask)

    cmp.b   #8,d4
    bne.s   .volumenotnull                              ;  ---------------          if ( volume == 8 )
    move.w  #(HW_BLITTER_HOP_BIT1*256)+HW_BLITTER_OP_BIT0,HW_BLITTER_HOP.w ;        *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1; *HW_BLITTER_OP  = HW_BLITTER_OP_BIT0
    bra.s   .endsetlogicpass
.volumenotnull:

    move.w  BLSvoice_mask(a0),d4
    cmp.w   #$FFFF,d4               					;  ---------------			else if ( _voice->mask != 0xFFFF )
    beq.s   .nomask

    move.w  d4,d3
    swap    d4
    move.w  d3,d4
    lea     HW_BLITTER_HTONE.w,a5                       ;							STDmset (HW_BLITTER_HTONE, (mask << 16) | mask, 32);
    rept    8
    move.l  d4,(a5)+
    endr
    move.w  #(HW_BLITTER_HOP_SOURCE_AND_HTONE*256)+HW_BLITTER_OP_S,HW_BLITTER_HOP.w ; *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE_AND_HTONE; *HW_BLITTER_OP  = HW_BLITTER_OP_S
    bra.s   .endsetlogicpass

.nomask:                                                ;  ---------------
    move.w  #(HW_BLITTER_HOP_SOURCE*256)+HW_BLITTER_OP_S,HW_BLITTER_HOP.w ;         *HW_BLITTER_HOP = HW_BLITTER_HOP_SOURCE; *HW_BLITTER_OP  = HW_BLITTER_OP_S       

.endsetlogicpass:

    ;--------------
    lea     HW_BLITTER_XINC_SOURCE.w,a5

    move.l  BLSsample_sampleLen(a2),d3
    sub.l   d6,d3                                       ; d3 = sampleLen - scrCurrentSampl

    moveq.l  #$FFFFFF00|HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY,d0

    tst.b   d5                                          
    bge.s   .nomagnify                                  ;                           if (transpose < 0)

    ;-------------------------------------------------------------------------------------------------------
    ; transpose < 0
    ;-------------------------------------------------------------------------------------------------------

    neg.b   d5                                          ;                           transpose = -transpose;
                
    move.l  #2,(a5)                                     ;                           *HW_BLITTER_XINC_SOURCE = 0; *HW_BLITTER_YINC_SOURCE = 2

    move.l  d3,d2                                       ; d2 = transfer             transfer = (sampleLen - scrCurrentSampl) << transpose
    lsl.l   d5,d2
    cmp.l   d2,d7
    bge.s   .transferok                                 ;                           if (transfer > remain)
    move.l  d7,d2                                       ;                           transfer = remain
.transferok:

    move.l  d6,d4
    add.l	d4,d4
    add.l   a4,d4
    move.l  d4,HW_BLITTER_ADDR_SOURCE.w                 ;                           *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (scrCurrentSampl << 1))
                
    moveq.l #0,d4
    bset.l  d5,d4
    move.w  d4,HW_BLITTER_XSIZE.w                       ;                           *HW_BLITTER_XSIZE = 1 << transpose

    move.l  d2,d4
    lsr.l   d5,d4
    move.w  d4,HW_BLITTER_YSIZE.w                       ;                           *HW_BLITTER_YSIZE = (u16) (transfer >> transpose)
    
    RASTERIZE   BLIT_COLOR

    move.b  d0,HW_BLITTER_CTRL1.w ; *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR

    add.l   d4,d6                                       ;                            scrCurrentSampl += transfer >> transpose;
    sub.l   d2,d7                                       ;                            remain          -= transfer;

    bra     .finishtransfer

    ;-------------------------------------------------------------------------------------------------------
    ; transpose >= 0
    ;-------------------------------------------------------------------------------------------------------            
.nomagnify:

    moveq.l     #0,d4
    bset.l      d5,d4
    add.w       d4,d4
    move.w      d4,(a5)                                 ;                           *HW_BLITTER_XINC_SOURCE = 2 << transpose;
    
    move.l  d3,d2
    lsr.l   d5,d2                                       ;                           transfer = (sampleLen - scrCurrentSampl) >> transpose;
    beq     .finishtransfer                             ;                           if (transfer > 0) /* can be 0 when transpose > 0 */
    cmp.l   d2,d7                                       ;                           if (transfer > remain)
    bge.s   .transferIsLowerThanRemain
    move.l  d7,d2                                       ;                           transfer = remain;
.transferIsLowerThanRemain:

    move.l  d6,d4
    add.l   d4,d4
    add.l   a4,d4
    move.l  d4,HW_BLITTER_ADDR_SOURCE.w                 ;                           *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (scrCurrentSampl << 1))
    move.w  (a5),HW_BLITTER_YINC_SOURCE.w               ;                           *HW_BLITTER_YINC_SOURCE = *HW_BLITTER_XINC_SOURCE
    
    move.l  d2,d4
    swap    d4
    addq.l  #1,d4                                       ;                           *HW_BLITTER_XSIZE = (u16) transfer
    move.l  d4,HW_BLITTER_XSIZE.w                       ;                           *HW_BLITTER_YSIZE = (transfer >> 16) + 1
                    
    RASTERIZE   BLIT_COLOR
    
    move.b  d0,HW_BLITTER_CTRL1.w                       ;                           *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR

    move.l  d2,d4                                       ;
    lsl.l   d5,d4                                       ;                           
    add.l   d4,d6                                       ;                           scrCurrentSampl += transfer << transpose;
    sub.l   d2,d7                                       ;                           remain          -= transfer;
    ble.s   .finishtransfer                             ;                           if ((remain > 0)

    move.w  BLSsample_sampleLoopLength(a2),d2
    ble.s   .finishtransfer                             ;                           && (sampleLoopLength > 0))

    move.l  BLSsample_sampleLoopStart(a2),d6            ;                           scrCurrentSampl = sampleLoopStart;

    move.l  d2,d1
    lsr.l   d5,d1
    move.l  d7,d4                                       ; d1                        u16 translooplen = (u16)(sampleLoopLength >> transpose);
    divu.w  d1,d4                                       ;                           u32 div     = STDdivu(remain, translooplen)

    ;------------------------------------
    ; nbloops > 0

    tst.w   d4                                          ; d4 = nbloops              u16 nbloops = (u16) div
    beq.s   .noloop                                     ;                           if (nbloops > 0)

    move.l  d6,d3
    add.l   d3,d3
    add.l   a4,d3
    move.l  d3,HW_BLITTER_ADDR_SOURCE.w                 ;                           *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (sampleLoopStart << 1))
    
    move.w  d1,d3
    lsl.w   d5,d3
    add.w   d3,d3
    neg.w   d3
    add.w   (a5),d3
    move.w  d3,HW_BLITTER_YINC_SOURCE.w                 ;                           *HW_BLITTER_YINC_SOURCE = -(translooplen << (transpose + 1)) + *HW_BLITTER_XINC_SOURCE;

    move.w  d1,HW_BLITTER_XSIZE.w                       ;                           *HW_BLITTER_XSIZE       = sampleLoopLength >> transpose;
    move.w  d4,HW_BLITTER_YSIZE.w                       ;                           *HW_BLITTER_YSIZE       = nbloops;

    RASTERIZE   BLIT_COLOR

    move.b  d0,HW_BLITTER_CTRL1.w                       ;                           *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR
 
    swap    d4
    move.w  d4,d7                                       ;                           remain = div >> 16; 
                            
.noloop:

    ;------------------------------------
    ; remain > 0

    tst.w   d7
    ble.s   .finishtransfer                             ;                           if (remain > 0)
    
    move.l  d6,d1
    add.l   d1,d1
    add.l   a4,d1
    move.l  d1,HW_BLITTER_ADDR_SOURCE.w                 ;                           *HW_BLITTER_ADDR_SOURCE = (u32) (sampleData + (sampleLoopStart << 1));
    move.w  (a5),HW_BLITTER_YINC_SOURCE.w               ;                           *HW_BLITTER_YINC_SOURCE = *HW_BLITTER_XINC_SOURCE
    move.w  d7,HW_BLITTER_XSIZE.w
    move.w  #1,HW_BLITTER_YSIZE.w                       ;                           *HW_BLITTER_XSIZE       = (u16) remain; *HW_BLITTER_YSIZE       = 1;
                            
    RASTERIZE   BLIT_COLOR

    move.b  d0,HW_BLITTER_CTRL1.w                       ;                           *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR

    lsl.l   d5,d7
    add.l   d7,d6                                       ;                           scrCurrentSampl += remain << transpose
    moveq.l #0,d7                                       ;                           remain = 0
                                                       
    ;--------------                                    
                                                       
.finishtransfer:                                       
                                                       
    move.l  d6,BLSvoice_current(a0)                     ;                           _voice->current = scrCurrentSampl;
                                                       
    cmp.l   BLSsample_sampleLen(a2),d6                 
    blt.s   .donotstopsample                            ;                           if (sampleLen <= scrCurrentSampl)

    tst.w   BLSsample_sampleLoopLength(a2)
    beq.s   .stopsample
    move.l  BLSsample_sampleLoopStart(a2),BLSvoice_current(a0)
    bra.s   .donotstopsample

.stopsample:
    clr.l   (a0)                                        ; BLSvoice_keys(a0)             _voice->keys[0]       = NULL;
    clr.b   BLSvoice_arpeggioState(a0)                  ;                               _voice->arpeggioState = ArpeggioState_STOPPED;

.donotstopsample:

    ;-------------------------------------------------------------------------------------------------------
    ; clear end of buffer
    ;-------------------------------------------------------------------------------------------------------            
    tst.w   d7
    beq.s   .managerunningfx                        ;                               if (remain > 0)
    
.clear:
    clr.b   HW_BLITTER_CTRL2.w                      ;                               *HW_BLITTER_CTRL2 = 0;
    move.l  a1,HW_BLITTER_ADDR_SOURCE.w             ;                               *HW_BLITTER_ADDR_SOURCE = (u32)_buffer;
    move.w  #(HW_BLITTER_HOP_BIT1*256)+HW_BLITTER_OP_BIT0,HW_BLITTER_HOP.w ;        *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1; *HW_BLITTER_OP  = HW_BLITTER_OP_BIT0
    move.w  d7,HW_BLITTER_XSIZE.w                   ;                               *HW_BLITTER_XSIZE = (u16) remain;
    move.w  #1,HW_BLITTER_YSIZE.w                   ;                               *HW_BLITTER_YSIZE = 1;    

    RASTERIZE   BLIT_COLOR

    move.b  #HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY,HW_BLITTER_CTRL1.w ; *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR

    tst.b   BLSvoice_sampledelay(a0)
    beq.s   .stop
    subq.b  #1,BLSvoice_sampledelay(a0)
    rts

.stop:
    clr.l   (a0)                                    ; BLSvoice_keys(a0)             _voice->keys[0]    = NULL;
    clr.b   BLSvoice_arpeggioState(a0)              ;                               _voice->arpeggioState = ArpeggioState_STOPPED;

    rts

    ;------------------------------  
    ; manage arpeggio
    ;------------------------------  
.managerunningfx:

    tst.b   BLSvoice_arpeggioState(a0)
    beq.s   .norunningfx                            ;                               if (_voice->arpeggioState != ArpeggioState_STOPPED)

    move.w  BLSprecomputedKey_freqdiv(a3),d4        ;                                u16 freqdiv      = (u16) _voice->keys[_voice->arpeggioIndex]->freqdiv;
    move.b  BLSprecomputedKey_freqdivshift(a3),d1   ;                                u8  freqdivshift = (u16) _voice->keys[_voice->arpeggioIndex]->freqdivshift;

    move.w  BLSvoice_current+2(a0),d2               ;                                v = _voice->current;
    mulu.w  d2,d4                                   ;                                v = STDmulu((u16) v, freqdiv);
    lsr.l   d1,d4                                   ;                                v >>= freqdivshift;

    move.w  d4,BLSvoice_currentsource+2(a0)         ;                                _voice->currentsource = v;

.norunningfx:

    rts 
