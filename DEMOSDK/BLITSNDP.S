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

	output	D:\PROJECTS\DEMOS\OUTPUT\BLITSNDP.O
	
	opt 	o+


managevolumeoffset:     set 0

;-------------------------------------------------------------------------------------
; set exportall to 1 when you want to export all routines separately
;-------------------------------------------------------------------------------------
exportall:  set 0   

    IFNE    exportall
    xdef    ablsUpAllVoices,ablsUpSoundBuffers,ablsUpScore,ablsUpRunningEffects,ablsSetDMABuffer
    ELSE
    xdef    aBLSupdate,ablsUpdAsync
    ENDC


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

ArpeggioState_STOPPED:    set 0
ArpeggioState_STARTING:   set 4
ArpeggioState_RUNNING:    set 8
ArpeggioState_STOPPING:   set 12

BLSvoice_sizeof:          set 44
BLSvoice_samples:         set 0
BLSvoice_keys:            set 12
BLSvoice_current:         set 24
BLSvoice_currentsource:   set 28
BLSvoice_mask:            set 32
BLSvoice_mute:            set 34    ; should be contiguous to sampledelay and aligned on 2 (optimization) 
BLSvoice_sampledelay:     set 35
BLSvoice_volume:          set 36    ; should be contiguous to retrigger and arpeggioState and aligned on 2 (optimization)
BLSvoice_retrigger:       set 37
BLSvoice_arpeggioState:   set 38
BLSvoice_padding:         set 39
BLSvoice_retriggercount:  set 40
BLSvoice_arpeggioOffset:  set 41
BLSvoice_volumeoffset:    set 42

BLSplayer_sndtrack:       set BLSvoice_sizeof*4
BLSplayer_buffer:         set BLSplayer_sndtrack+4
BLSplayer_buffertoupdate: set BLSplayer_buffer+4
BLSplayer_dmabufstart:    set BLSplayer_buffertoupdate+4
BLSplayer_dmabufend:      set BLSplayer_dmabufstart+4
BLSplayer_bufferstate:    set BLSplayer_dmabufend+4
BLSplayer_speed:          set BLSplayer_bufferstate+2
BLSplayer_speedcount:     set BLSplayer_speed+1
BLSplayer_trackindex:     set BLSplayer_speedcount+1
BLSplayer_pattern:        set BLSplayer_trackindex+1
BLSplayer_row:            set BLSplayer_pattern+1
BLSplayer_loopstart:      set BLSplayer_row+1
BLSplayer_loopcount:      set BLSplayer_loopstart+1
BLSplayer_clientEvent:    set BLSplayer_loopcount+1
BLSplayer_volumeLeft:     set BLSplayer_clientEvent+1
BLSplayer_volumeRight:    set BLSplayer_volumeLeft+2
BLSplayer_volumeLeft2:    set BLSplayer_volumeRight+2
BLSplayer_volumeRight2    set BLSplayer_volumeLeft2+2

BLSsndtrack_samples:      set 4
BLSsndtrack_keys:         set 12
BLSsndtrack_patterns:     set 24
BLSsndtrack_track:        set 28
BLSsndtrack_tracklen:     set 38

BLSprecomputedKey_sampleIndex:      set 0
BLSprecomputedKey_blitterTranspose: set 1
BLSprecomputedKey_freqmul:          set 2
BLSprecomputedKey_freqdiv:          set 4
BLSprecomputedKey_freqmulshift:     set 6
BLSprecomputedKey_freqdivshift:     set 7

BLScell_fx:                         set 1
BLScell_value:                      set 2
BLScell_value2:                     set 3
BLScell_sizeof:                     set 4

BLSsample_sample:                   set 0
BLSsample_sampleLen:                set 4
BLSsample_sampleLoopStart:          set 8
BLSsample_sampleLoopLength:         set 12
BLSsample_flags:                    set 14    


BLS_SAMPLE_STORAGE_SHIFT_BIT:       set    2    
BLS_SAMPLE_STORAGE_INTERLACE_BIT:   set    3

BENCHMARK:                          set 0
BLIT_COLOR:                         set $700
NOBLIT_COLOR:                       set $70
UPDBUFFER_COLOR:                    set $7
UPDSCORE_COLOR:                     set $47
RUNFX_COLOR:                        set $77 


    IFNE BENCHMARK

RASTERIZE MACRO
    move.w  #\1,$ffff8240.w
    ENDM

    ELSE

RASTERIZE MACRO
    ENDM

    ENDC


; (BLSsoundTrack* sndtrack, u8 pkindex, u8 arpeggioOffset, BLSvoice* voice)
; d6 = pkindex
; d7 = arpeggioOffset ( = arpeggioIndex << 2 )

AssignBaseKeyToVoice MACRO
    lsl.w   #3,d6                            ; d6 = pkindex * sizeof(BLSprecomputedKey)
    move.l  BLSsndtrack_keys(a2),a5          
    add.w   d6,a5                            ; a5 = BLSprecomputedKey* key  = &sndtrack->keys[pkindex];
    move.l  a5,BLSvoice_keys(a4,d7.w)        ; voice->keys [arpeggioIndex] = key; d7 = arpeggioOffset 

    moveq.l #0,d6                            ; u8 sampleIndex = key->sampleIndex;
    move.b  (a5),d6                         

    btst.b  #0,BLSprecomputedKey_blitterTranspose(a5) ; BLS_IS_BASEKEY(key) == false
    bne.s   *+14                            ; .transposedkey

    lsl.w   #3,d6                           ; sizeof(BLSprecomputedKey) == 8
    move.l  BLSsndtrack_keys(a2),a6
    add.w   d6,a6
    moveq.l #0,d6
    move.b  (a6),d6                         ; sampleIndex = sndtrack->keys[sampleIndex].sampleIndex;

; .transposedkey

    lsl.w   #4,d6
    move.l  BLSsndtrack_samples(a2),a6
    add.w   d6,a6
    move.l  a6,(a4,d7.w)                    ; voice->samples[arpeggioIndex] = &sndtrack->samples[sampleIndex];

    ENDM


SCORE_END_LOOP MACRO
    lea         BLScell_sizeof(a3),a3                               ; for (i = 0 ; i < BLS_NBVOICES ; i++, cell++, voice++)
    lea         BLSvoice_sizeof(a4),a4
    dbf.w       d5,.voiceloop
    bra         .end
    ENDM


; ------------------------------------------------------------------------
; void ablsUpdate (BLSplayer* _player)
; ------------------------------------------------------------------------
    IFEQ        exportall
aBLSupdate:
    movem.l     a1-a6/d0-d7,-(sp)    
    ENDC

; ------------------------------------------------------------------------
; void ablsUpdateSoundBuffers (BLSplayer* _player)
; ------------------------------------------------------------------------
ablsUpSoundBuffers:

    RASTERIZE   UPDBUFFER_COLOR

    IFNE        exportall
    movem.l     d0-d4/a1-a2,-(sp)
    ENDC

    lea         HW_DMASOUND_COUNTER_H-2.w,a1
    movep.l     (a1),d0
    and.l       #$FFFFFF,d0                         ; d0 = readcursor

    move.w      BLSplayer_volumeLeft(a0),d1
    beq.s       .donotsetvolumeleft
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d1,HW_MICROWIRE_DATA.w
.donotsetvolumeleft:

    move.l      BLSplayer_dmabufstart(a0),d3        ; d3 = dmabufstart
    move.l      BLSplayer_dmabufend(a0),d4          ; d4 = dmabufend

    move.l      BLSplayer_buffer(a0),d1             ; d1 = buffer

    move.w      BLSplayer_bufferstate(a0),d2        ; switch(_player->bufferstate)
    move.l      .switchBufferState(pc,d2.w),a2      
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
    lea         HW_DMASOUND_STARTADR_M.w,a1
    swap        d1
    move.b      d1,HW_DMASOUND_STARTADR_H.w
    swap        d1
    movep.w     d1,(a1)

    lea         HW_DMASOUND_ENDADR_M.w,a1
    move.l      d1,d2
    add.l       #BLS_NBBYTES_PERFRAME+BLS_NBBYTES_OVERHEAD,d2
    swap        d2
    move.b      d2,HW_DMASOUND_ENDADR_H.w
    swap        d2
    movep.w     d2,(a1)

    move.b      #HW_DMASOUND_CONTROL_PLAYLOOP,HW_DMASOUND_CONTROL.w
    
    moveq.l     #-1,d0                              ; readcursor = -1

.BLSbs_STEP2_CASE:
    move.w      #BLSbs_STEP0,BLSplayer_bufferstate(a0)
    move.l      d1,d2
    add.l       #BLS_STEP_PERFRAME+BLS_STEP_PERFRAME,d2
    move.l      d2,BLSplayer_buffertoupdate(a0)
    add.l       #BLS_STEP_PERFRAME,d1

.endswitch:
    move.l      d1,BLSplayer_dmabufstart(a0)
    
    add.l       #BLS_NBBYTES_PERFRAME,d1

    cmp.l       d0,d3
    bgt.s       .donotenlargeplaybuffer
    cmp.l       d0,d4
    blt.s       .donotenlargeplaybuffer
    addq.l      #BLS_NBBYTES_OVERHEAD,d1
.donotenlargeplaybuffer:

    move.l      d1,BLSplayer_dmabufend(a0)

    move.w      BLSplayer_volumeRight(a0),d1
    beq.s       .donotsetvolumeright
    move.w      #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK.w
    move.w      d1,HW_MICROWIRE_DATA.w
.donotsetvolumeright:

    move.l      BLSplayer_volumeLeft2(a0),BLSplayer_volumeLeft(a0)
    clr.l       BLSplayer_volumeLeft2(a0)

    IFNE        exportall
    movem.l     (sp)+,d0-d4/a1-a2
    rts
    ELSE
    bra.s       ablsUpScore
    ENDC

; ------------------------------------------------------------------------
; void ablsUpdAsync(BLSplayer* _player)
; ------------------------------------------------------------------------
    IFEQ        exportall
ablsUpdAsync:
    movem.l     a1-a6/d0-d7,-(sp)
    clr.l       BLSplayer_dmabufstart(a0)
    ENDC

; ------------------------------------------------------------------------
; void ablsUpScore(BLSplayer* _player)
; ------------------------------------------------------------------------
ablsUpScore:
    
    RASTERIZE   UPDSCORE_COLOR

    subq.b      #1,BLSplayer_speedcount(a0)                 ; _player->speedcount--;
    bne         .nointerpret

.interpretscore:

    IFNE        exportall
    movem.l     a2-a6/d0-d7,-(sp)
    ENDC

    move.l      BLSplayer_sndtrack(a0),a2                   ; BLSsoundTrack* sndtrack = _player->sndtrack
    
    moveq.l     #0,d4                                       ; u8             delay        = 0;
 
    moveq.l     #0,d0   
    move.b      BLSplayer_trackindex(a0),d0
    move.l      BLSsndtrack_track(a2),a6
    moveq.l     #0,d1   
    move.b      (a6,d0.w),d1                                ; d1 : patternindex = sndtrack->track[_player->trackindex];

    moveq.l     #0,d2
    move.b      BLSplayer_row(a0),d2                        ; currentrow   = _player->row;
    move.w      d2,d6                                       ; d2
    move.l      BLSsndtrack_patterns(a2),a3                 ; sndtrack->patterns
    
    move.l      d1,d7
    lsl.w       #10-4,d7
    add.w       d2,d7
    lsl.w       #4,d7
    add.l       d7,a3                                       ; a3 = row = &(sndtrack->patterns[patternindex].rows[currentrow]);

;        u8             delay        = 0;

    addq.b      #1,d6                                       ; _player->row++;
    move.b      d6,BLSplayer_row(a0)
    cmp.b       #BLS_NBPATTERNROWS,d6
    blt.s       .rowok                                      ; if (_player->row >= BLS_NBPATTERNROWS)
    clr.w       BLSplayer_row(a0)                           ; _player->row = 0; _player->loopstart = 0;
    clr.b       BLSplayer_loopcount(a0)                     ; _player->loopcount = 0;

    addq.b      #1,d0                                       ; _player->trackindex++;
    cmp.b       BLSsndtrack_tracklen(a2),d0                 ; if (_player->trackindex >= sndtrack->trackLen)
    blt.s       .trackindexok                               ; 
    moveq.l     #0,d0                                       ; _player->trackindex = 0;
.trackindexok:                                              ;
    move.b      d0,BLSplayer_trackindex(a0)                 ;

.rowok:        

    ; interpret pattern 

    move.l      a0,a4                                       ; a4 = BLSvoice* voice = _player->voices;

                                                            ; a3 => BLScell*    cell  = row->cells;

    moveq.l     #3,d5                                       ; BLS_NBVOICES-1

.voiceloop:

    moveq.l     #0,d6
    move.b      (a3),d6
    beq.s       .nokey                                      ; if ( cell->precomputedKey != 0 )

    subq.b      #1,d6                                       ; cell->precomputedKey-1
    moveq.l     #0,d7                                       ; AssignBaseKeyToVoice(sndtrack, cell->precomputedKey-1, 0, voice);

    AssignBaseKeyToVoice

    clr.l       BLSvoice_current(a4)
    clr.l       BLSvoice_currentsource(a4)
    clr.l       BLSvoice_volume(a4)                         ; _voice->volume = 0; _voice->retrigger = 0; _voice->arpeggioState = 0;

.nokey:

    moveq.l     #0,d7    
    move.b      BLScell_fx(a3),d7
    move.l      .fxswitch(pc,d7.w),a5
    jmp         (a5)                                        ; switch (cell->fx)

.fxswitch:
    dc.l        .BLSfx_NONE_CASE
    dc.l        .BLSfx_VOLUME_CASE
    dc.l        .BLSfx_SPEED_CASE
    dc.l        .BLSfx_JUMP_CASE
    dc.l        .BLSfx_ARPEGGIO_CASE
    dc.l        .BLSfx_SETBALANCE_CASE
    dc.l        .BLSfx_PATTERN_BREAK_CASE
    dc.l        .BLSfx_BITMASK_CASE
    dc.l        .BLSfx_SETSAMPLEOFFSET_CASE
    dc.l        .BLSfx_RETRIGGER_SAMPLE_CASE
    dc.l        .BLSfx_CLIENT_EVENT_CASE
    dc.l        .BLSfx_VOICE_OFF_CASE
    dc.l        .BLSfx_LOOP_START_SET_CASE
    dc.l        .BLSfx_LOOP_CASE
    dc.l        .BLSfx_DELAY_PATTERN_CASE
    dc.l        .BLSfx_DELAY_SAMPLE_CASE


; ------------------------------------------------------------------------
.BLSfx_VOLUME_CASE:
    move.b      BLScell_value(a3),BLSvoice_volume(a4)            ; voice->volume = cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_SPEED_CASE:
    move.b      BLScell_value(a3),BLSplayer_speed(a0)            ; _player->speed = cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------                
.BLSfx_JUMP_CASE:
    move.b      BLScell_value(a3),BLSplayer_trackindex(a0)       ; _player->trackindex = cell->value;
    clr.b       BLSplayer_row(a0)
    SCORE_END_LOOP 

; ------------------------------------------------------------------------                
.BLSfx_PATTERN_BREAK_CASE:
    move.b      BLSplayer_trackindex(a0),d7 
    addq.b      #1,d7                                            ; _player->trackindex++;
    cmp.b       BLSsndtrack_tracklen(a2),d7                      ; if (_player->trackindex >= sndtrack->trackLen)
    blt.s       .trackindexok2                                   ; 
    moveq.l     #0,d7                                            ; _player->trackindex = 0;
.trackindexok2:                                                  ;
    move.b      d7,BLSplayer_trackindex(a0)                      ; _player->row = cell->value;
    move.b      BLScell_value(a3),BLSplayer_row(a0)
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_ARPEGGIO_CASE:

    tst.b       BLSvoice_arpeggioState(a4)                          ; switch (voice->arpeggioState) case ArpeggioState_STOPPED:
    bne.s       .arpeggiostarted
    move.b      #ArpeggioState_STARTING,BLSvoice_arpeggioState(a4)  ;   voice->arpeggioState = ArpeggioState_STARTING
.arpeggiostarted: 

    moveq.l     #0,d7
    move.b      BLScell_value(a3),d7
    lsl.w       #3,d7                                               ; sizeof(BLSprecomputedKey) == 8
    add.l       BLSsndtrack_keys(a2),d7                             ; BLSprecomputedKey* key1 = &sndtrack->keys[cell->value];
    cmp.l       BLSvoice_keys+4(a4),d7
    bne.s       .assignArpeggio
    
    moveq.l     #0,d7
    move.b      BLScell_value2(a3),d7
    lsl.w       #3,d7                                               ; sizeof(BLSprecomputedKey) == 8
    add.l       BLSsndtrack_keys(a2),d7
    cmp.l       BLSvoice_keys+8(a4),d7
    beq.s       .donotassignArpeggio
    
.assignArpeggio:                                                    ; if (( key1 != voice->keys[1] ) || ( key2 != voice->keys[2] ))

    move.b      #ArpeggioState_STARTING,BLSvoice_arpeggioState(a4)  ; voice->arpeggioState = ArpeggioState_STARTING;
    clr.l       BLSvoice_current(a4)
    clr.l       BLSvoice_currentsource(a4)                          ; voice->current = voice->currentsource = 0;

    moveq.l     #0,d6
    move.b      BLScell_value(a3),d6
    moveq.l     #4,d7
    AssignBaseKeyToVoice                                            ; AssignBaseKeyToVoice(sndtrack, cell->value , 1 << 2, voice);

    moveq.l     #0,d6
    move.b      BLScell_value2(a3),d6
    moveq.l     #8,d7
    AssignBaseKeyToVoice                                            ; AssignBaseKeyToVoice(sndtrack, cell->value , 2 << 2, voice);
                            
.donotassignArpeggio:
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_SETBALANCE_CASE:
    tst.b   d5                                              ; if ((i == 0) || (i == 3))
    beq.s   .leftvolume
    cmp.b   #3,d5
    beq.s   .leftvolume
    move.w  BLScell_value(a3),BLSplayer_volumeRight2(a0)    ; _player->volumeRight = *(u16*)&cell->value;
    SCORE_END_LOOP 

.leftvolume:
    move.w  BLScell_value(a3),BLSplayer_volumeLeft2(a0)     ; _player->volumeLeft = *(u16*)&cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_BITMASK_CASE:
    move.w  BLScell_value(a3),BLSvoice_mask(a4)             ; voice->mask = *(u16*)&cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_SETSAMPLEOFFSET_CASE:
    tst.l   (a4)                                            ; if ( voice->samples[0] != NULL )
    beq.s   .dontsetoffset
    move.w  BLScell_value(a3),d7                            ; u16 offset = *(u16*)&cell->value;
    move.l  BLSvoice_keys(a4),a6
    move.w  BLSprecomputedKey_freqmul(a6),d6
    mulu.w  d6,d7
    move.b  BLSprecomputedKey_freqmulshift(a6),d6
    lsr.l   d6,d7

    move.l  d7,BLSvoice_current(a4)                         ; voice->current = (u16)(STDmulu(voice->keys[0]->freqmul, offset) >> voice->keys[0]->freqmulshift);
.dontsetoffset:

    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_RETRIGGER_SAMPLE_CASE:
    move.b  BLScell_value(a3),d7                            ; voice->retrigger = voice->retriggercount = cell->value;
    move.b  d7,BLSvoice_retrigger(a4)                       ; voice->retrigger = voice->retriggercount = cell->value;
    move.b  d7,BLSvoice_retriggercount(a4)                      
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_CLIENT_EVENT_CASE:
    move.b  BLScell_value(a3),BLSplayer_clientEvent(a0)     ; _player->clientEvent = cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_VOICE_OFF_CASE:
    clr.l   (a4)                                            ; voice->samples[0] = NULL;
    clr.b   BLSvoice_arpeggioState(a4)                      ; voice->arpeggioState = ArpeggioState_STOPPED;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_LOOP_START_SET_CASE:
    move.b  d2,BLSplayer_loopstart(a0)                      ; _player->loopstart = currentrow;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_LOOP_CASE:
    
    tst.b   BLSplayer_loopcount(a0)
    blt.s   .endloopcase
    bne.s   .loopcase2
    
    move.b  BLScell_value(a3),BLSplayer_loopcount(a0)       ; if (_player->loopcount == 0)
    move.b  BLSplayer_loopstart(a0),BLSplayer_row(a0)       ; _player->loopcount = cell->value;
    bra.s   .endloopcase                                    ; _player->row = _player->loopstart;

.loopcase2:
                                                            ; else if (_player->loopcount > 0)
    subq.b  #1,BLSplayer_loopcount(a0)                      ; _player->loopcount--;
    ble.s   .endloopcase                                    ; if (_player->loopcount > 0)                       
    move.b  BLSplayer_loopstart(a0),BLSplayer_row(a0)       ; _player->row = _player->loopstart;

.endloopcase:
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_DELAY_PATTERN_CASE:
    moveq.l #0,d6       
    moveq.l #0,d4
    move.b  BLSplayer_speed(a0),d4
    move.b  BLScell_value(a3),d6
    mulu.w  d6,d4                                               ; delay = _player->speed * cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_DELAY_SAMPLE_CASE:
    move.b  BLScell_value(a3),BLSvoice_sampledelay(a4)          ; _voice->sampledelay = cell->value;
    SCORE_END_LOOP 

; ------------------------------------------------------------------------
.BLSfx_NONE_CASE:
    clr.b   BLSvoice_retrigger(a4)                              ; voice->retrigger = 0;
    tst.b   BLSvoice_arpeggioState(a4)                          ; if (voice->arpeggioState != ArpeggioState_STOPPED)
    beq.s   .donotchangearpeggiostate       
    move.b  #ArpeggioState_STOPPING,BLSvoice_arpeggioState(a4)  ; voice->arpeggioState = ArpeggioState_STOPPING;                    
.donotchangearpeggiostate:
    lea         BLScell_sizeof(a3),a3                           ; for (i = 0 ; i < BLS_NBVOICES ; i++, cell++, voice++)
    lea         BLSvoice_sizeof(a4),a4
    dbf.w       d5,.voiceloop                                   ; bra.s not needed here ...

; ------------------------------------------------------------------------
.end:
    
    add.b   BLSplayer_speed(a0),d4                              ; /* reset speed for next interpretation */ _player->speedcount = _player->speed + delay;
    move.b  d4,BLSplayer_speedcount(a0)   
    
    IFNE    exportall
    movem.l  (sp)+,a2-a6/d0-d7
    ENDC

.nointerpret:

    IFNE    exportall
    rts
    ENDC

; ------------------------------------------------------------------------
; void ablsUpRunningEffects (BLSplayer* _player)
; ------------------------------------------------------------------------
ablsUpRunningEffects:

    RASTERIZE   RUNFX_COLOR

    IFNE    exportall
    movem.l d0-d3/a2-a3,-(sp)
    ENDC

    moveq.l #3,d3                                               
    moveq.l #0,d0

    move.l  a0,a3                                               ; a0 = BLSvoice* voice = _player->voices;

.loop:

    tst.b   BLSvoice_retrigger(a3)
    ble.s   .noretrigger                                        ; if (voice->retrigger > 0)

    subq.b  #1,BLSvoice_retriggercount(a3)                      ; voice->retriggercount--;
    bne.s   .noretrigger                                        ; if (voice->retriggercount == 0)

    clr.l   BLSvoice_current(a3)                                ; voice->current = 0;
    move.b  BLSvoice_retrigger(a3),BLSvoice_retriggercount(a3)  ; voice->retriggercount = voice->retrigger;

.noretrigger:
 
    move.b  BLSvoice_arpeggioState(a3),d0
    move.l  .arpeggioswitch(pc,d0.w),a2
    jmp     (a2)                                                ; switch (voice->arpeggioState)

.arpeggioswitch:
    dc.l        .BLSarpeggio_STOPPED_CASE
    dc.l        .BLSarpeggio_STARTING_CASE
    dc.l        .BLSarpeggio_RUNNING_CASE
    dc.l        .BLSarpeggio_STOPPING_CASE

.BLSarpeggio_STOPPING_CASE:            
    move.b   #ArpeggioState_STOPPED,BLSvoice_arpeggioState(a3)  ; voice->arpeggioState = ArpeggioState_STOPPED;
    move.b   #8,BLSvoice_arpeggioOffset(a3)                     ; voice->arpeggioIndex = 2 << 2;
                                                                ; /* no break here */           
.BLSarpeggio_RUNNING_CASE:
    move.b   BLSvoice_arpeggioOffset(a3),d0                     ; voice->arpeggioOffset += 4;
    addq.b   #4,d0
    cmp.b    #12,d0                                             ; if (voice->arpeggioIndex == 3 << 2)
    bne.s   .noarpeggioloop
    moveq.l  #0,d0                                              ; voice->arpeggioIndex = 0;
.noarpeggioloop:
    move.b   d0,BLSvoice_arpeggioOffset(a3)
    move.l   BLSvoice_keys(a3,d0.w),a2                          

    move.b   BLSprecomputedKey_freqmulshift(a2),d0              ; freqmulshift = voice->keys[voice->arpeggioIndex]->freqmulshift;

    move.w   BLSprecomputedKey_freqmul(a2),d1                   ; freqmul      = voice->keys[voice->arpeggioIndex]->freqmul;
    move.w   BLSvoice_currentsource+2(a3),d2                    ; v = STDmulu((u16)voice->currentsource, freqmul);
    mulu.w   d2,d1
    lsr.l    d0,d1                                              ; v >>= freqmulshift;
    move.w   d1,BLSvoice_current+2(a3)                          ; voice->current = v;
                
    lea     BLSvoice_sizeof(a3),a3
    dbf.w   d3,.loop
    bra.s   .end

.BLSarpeggio_STARTING_CASE:                                     ; case ArpeggioState_STARTING:
    move.b   #ArpeggioState_RUNNING,BLSvoice_arpeggioState(a3)  ; voice->arpeggioState = ArpeggioState_RUNNING;
                                                                ; /* no break here */
.BLSarpeggio_STOPPED_CASE:                                      ; case ArpeggioState_STOPPED:   voice->arpeggioIndex = 0;
    clr.b   BLSvoice_arpeggioOffset(a3)
    lea     BLSvoice_sizeof(a3),a3
    dbf.w   d3,.loop  
.end:

    IFNE exportall
    movem.l (sp)+,d0-d3/a2-a3
    rts
    ENDC

; ------------------------------------------------------------------------
; void ablsUpAllVoices (BLSplayer* _player)
; ------------------------------------------------------------------------
ablsUpAllVoices:

    RASTERIZE   NOBLIT_COLOR

    IFNE exportall
    movem.l d3-d7/a2-a6,-(sp)
    ENDC

    move.l  BLSplayer_buffertoupdate(a0),a1

    moveq.l #-1,d0   
    lea     HW_BLITTER_XINC_DEST.w,a5

    move.l  #$40004,(a5)                            ;       *HW_BLITTER_XINC_DEST   = 4;  *HW_BLITTER_YINC_DEST   = 4; /* multiplexing */    
                                                   
    move.l  d0,-(a5)                                ;       *HW_BLITTER_ENDMASK1  = 0xFFFF;  *HW_BLITTER_ENDMASK2  = 0xFFFF;
    move.w  d0,-(a5)                                ;       *HW_BLITTER_ENDMASK3  = 0xFFFF;
                                                   
    lea     HW_BLITTER_XSIZE.w,a6                  
                                                   
    moveq.l #1,d0                                  
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[1], (u8*) backbuf    , true );
    bsr.s   ablsUpdateVoiceFrame                   
                                                   
    moveq.l #1,d0                                  
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[2], (u8*) backbuf + 2, true );
    lea     2(a1),a1
    bsr.s   ablsUpdateVoiceFrame

    move.l  #$FF00FF00,HW_BLITTER_ENDMASK1.w        ;       *HW_BLITTER_ENDMASK1  = PCENDIANSWAP16(0xFF00); *HW_BLITTER_ENDMASK2  = PCENDIANSWAP16(0xFF00);
    move.w  #$FF00,HW_BLITTER_ENDMASK3.w            ;       *HW_BLITTER_ENDMASK3  = PCENDIANSWAP16(0xFF00);

    moveq.l #0,d0
    lea     BLSvoice_sizeof(a0),a0                  ;       blsUpdateVoiceFrame (&_player->voices[3], (u8*) backbuf + 2, false);
    bsr.s   ablsUpdateVoiceFrame

    moveq.l #0,d0
    lea     -3*BLSvoice_sizeof(a0),a0               ;       blsUpdateVoiceFrame (&_player->voices[0], (u8*) backbuf    , false);
    lea     -2(a1),a1
    bsr.s   ablsUpdateVoiceFrame   
    
    move.l  BLS_NBBYTES_PERFRAME-4(a1),BLS_NBBYTES_PERFRAME(a1)

    IFNE exportall
    movem.l (sp)+,d3-d7/a2-a6
    rts
    ENDC

;---------------------------------------------------------------------------------------
; ablsSetDMABuffer(BLSplayer* _player)
;---------------------------------------------------------------------------------------
; a0 = _player
;---------------------------------------------------------------------------------------
ablsSetDMABuffer:

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
; blsUpdateVoiceFrame(BLSvoice* _voice, u8* _buffer, bool _firstpass)
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
; a6
;---------------------------------------------------------------------------------------
ablsUpdateVoiceFrame:

    move.l  #BLS_NBSAMPLES_PERFRAME/2,d7                ; d7 = remain

    move.l  a1,HW_BLITTER_ADDR_DEST.w                     

    ;--------------------------------------------------
    ; manage muted voice optimization
    ;--------------------------------------------------
    lea     BLS_NBBYTES_PERFRAME+BLS_NBBYTES_OVERHEAD(a1),a3    ; a3 : u8*  cleared2 = _buffer + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
    lea     (a3,d0.w),a2                                        ; a2 : u8*  cleared  = cleared2 + _firstpass;
    
    tst.l   (a0)                             
    beq.s   .managecleared                              ; if ((_voice->samples[0] != NULL)
    tst.w   BLSvoice_mute(a0)
    beq.s   .play                                       ; && (_voice->mute == false) && (_voice->sampledelay == 0))

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
    clr.b   (a2)                                        ; *cleared2  = false;
    clr.b   (a3)                                        ; *cleared   = false;    

    ;--------------------------------------------------
    ; need to play something
    ;-------------------------------------------------- 
    moveq.l #0,d3
    move.b  BLSvoice_arpeggioOffset(a0),d3              ;                           
    move.l  BLSvoice_keys(a0,d3.w),a3                   ; a3 = pkey                 _voice->keys[_voice->arpeggioIndex]
    move.l  (a0,d3.w),a2                                ; a2 = sample               BLSsample* sample = _voice->samples[_voice->arpeggioIndex]

    move.l  BLSvoice_current(a0),d6                     ; d6 = scrCurrentSampl      u32  scrCurrentSampl = _voice->current
    
    move.b  BLSprecomputedKey_blitterTranspose(a3),d5   ; d5 = transpose            s8   transpose = _voice->keys[_voice->arpeggioIndex]->blitterTranspose >> 1
    asr.b   #1,d5

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
    move.w  d4,(a6)                                     ;                           *HW_BLITTER_XSIZE = 1 << transpose

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
    move.l  d4,(a6)                                     ;                           *HW_BLITTER_YSIZE = (transfer >> 16) + 1
                    
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

    move.w  d1,(a6)                                     ;                           *HW_BLITTER_XSIZE       = sampleLoopLength >> transpose;
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
    move.w  d7,(a6)
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
    clr.l   (a0)                                    ; BLSvoice_samples(a0)          _voice->samples[0]    = NULL;
    clr.b   BLSvoice_arpeggioState(a0)              ;                               _voice->arpeggioState = ArpeggioState_STOPPED;

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
    move.w  d7,(a6)                                 ;                               *HW_BLITTER_XSIZE = (u16) remain;
    move.w  #1,HW_BLITTER_YSIZE.w                   ;                               *HW_BLITTER_YSIZE = 1;    

    RASTERIZE   BLIT_COLOR

    move.b  #HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY,HW_BLITTER_CTRL1.w ; *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY

    RASTERIZE   NOBLIT_COLOR

    tst.b   BLSvoice_sampledelay(a0)
    beq.s   .stop
    subq.b  #1,BLSvoice_sampledelay(a0)
    rts

.stop:
    clr.l   (a0)                                    ; BLSvoice_samples(a0)          _voice->samples[0]    = NULL;
    clr.b   BLSvoice_arpeggioState(a0)              ;                               _voice->arpeggioState = ArpeggioState_STOPPED;

    rts

    ;------------------------------  
    ; manage arpeggio
    ;------------------------------  
.managerunningfx:

    tst.b   BLSvoice_arpeggioState(a0)
    beq.s   .norunningfx                            ;                               if (_voice->arpeggioState != ArpeggioState_STOPPED)

    moveq.l #0,d4
    move.b  BLSvoice_arpeggioOffset(a0),d4
    move.l  BLSvoice_keys(a0,d4.w),a5

    move.w  BLSprecomputedKey_freqdiv(a5),d4        ;                                u16 freqdiv      = (u16) _voice->keys[_voice->arpeggioIndex]->freqdiv;
    move.b  BLSprecomputedKey_freqdivshift(a5),d1   ;                                u8  freqdivshift = (u16) _voice->keys[_voice->arpeggioIndex]->freqdivshift;

    move.w  BLSvoice_current+2(a0),d2               ;                                v = _voice->current;
    mulu.w  d2,d4                                   ;                                v = STDmulu((u16) v, freqdiv);
    lsr.l   d1,d4                                   ;                                v >>= freqdivshift;

    move.w  d4,BLSvoice_currentsource+2(a0)         ;                                _voice->currentsource = v;

.norunningfx:

    rts 
