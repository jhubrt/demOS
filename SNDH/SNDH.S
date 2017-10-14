
	output	D:\PROJECTS\DEMOS\SNDTRACK.PRG
	
	opt 	m+

rasterization:  set 0

HW_DMASOUND_CONTROL:		set $FFFF8901

HW_DMASOUND_STARTADR_H:		set $FFFF8903
HW_DMASOUND_STARTADR_M:		set $FFFF8905
HW_DMASOUND_STARTADR_L:		set $FFFF8907

HW_DMASOUND_ENDADR_H:		set $FFFF890F
HW_DMASOUND_ENDADR_M:		set $FFFF8911
HW_DMASOUND_ENDADR_L:		set $FFFF8913

HW_DMASOUND_MODE:     		set $FFFF8920

HW_MICROWIRE_DATA:          set $FFFF8922
HW_MICROWIRE_MASK:          set $FFFF8924

HW_MICROWIRE_MASK_SOUND:    set $7FF
HW_MICROWIRE_VOLUME:        set $4C0
HW_MICROWIRE_VOLUME_LEFT:	set $540
HW_MICROWIRE_VOLUME_RIGHT:	set $500
HW_MICROWIRE_VOLUME_TREBLE:	set $480
HW_MICROWIRE_VOLUME_BASS:  	set $440

HW_DMASOUND_CONTROL_OFF:        set	0
HW_DMASOUND_CONTROL_PLAYONCE:	set 1
HW_DMASOUND_CONTROL_PLAYLOOP:	set 3

HW_DMASOUND_MODE_6258HZ:		set 0
HW_DMASOUND_MODE_12517HZ:	    set 1
HW_DMASOUND_MODE_25033HZ:	    set 2
HW_DMASOUND_MODE_50066HZ:	    set 3

HW_DMASOUND_MODE_MONO:          set $80
HW_DMASOUND_MODE_STEREO:        set	0

HW_MICROWIRE_MIXER_YM:          set	$401

SAMPLE_L:	set	89008			
SAMPLE_S:	set	44504

SND_TRANSFER_LEN:   set 16
*-------------------------------------------------------------------------

;-------------------------------------------------------------------------
; init
;-------------------------------------------------------------------------
    clr.l   -(sp)                                           ; supervisor
    move.w  #32,-(sp)
    trap    #1
    addq.l  #6,sp
    move.l  d0,stack

	clr.b	$484.w                                          ; keyboard system sound off

    move.w  #$2700,sr                                       ; install snd monitor in vbl

    move.l  $70.w,systemvbl+2
    move.l  #newvbl,$70.w

    move.w  #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK      ; set microwire volume to 0
	move.w  #HW_MICROWIRE_VOLUME,HW_MICROWIRE_DATA    
waitmicrowire1:
    cmp.w   #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK
    bne.s   waitmicrowire1

    move.w  #HW_MICROWIRE_MIXER_YM,HW_MICROWIRE_DATA        ; microwire : mix dma with ym outputs
waitmicrowire2:
    cmp.w   #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK
    bne.s   waitmicrowire2

    move.l  #DMABuffer+(SAMPLE_L-1)*2,d0                    ; set DMA buffer adress
    lea     HW_DMASOUND_ENDADR_H-2,a0
    movep.l d0,(a0)

    move.l  #DMABuffer,d0
    lea     HW_DMASOUND_STARTADR_H-2,a0
    movep.l d0,(a0)

	move.w  #HW_DMASOUND_MODE_25033HZ+HW_DMASOUND_MODE_STEREO,HW_DMASOUND_MODE   ; play 25khz / stereo
    move.b  #HW_DMASOUND_CONTROL_PLAYLOOP,HW_DMASOUND_CONTROL                    ; play

    move.l  #score,currentscore

    move.w  #$2300,sr
       
;-------------------------------------------------------------------------
; play
;-------------------------------------------------------------------------
loop:
	cmp.b	#185,$fffffc02.w    ; test key
    bne.s   loop

;-------------------------------------------------------------------------
; end
;-------------------------------------------------------------------------
    move.b  #HW_DMASOUND_CONTROL_OFF,HW_DMASOUND_CONTROL                    ; stop playing

    move.l  systemvbl+2,$70.w   ; restore system vbl

    move.l	stack(pc),-(sp)     ; restore user mode
	move.w	#$20,-(sp)	
	trap	#1
	addq.l	#6,sp

    clr.w    -(sp)              ; end program
    move.w   #$4c,-(sp) 
    trap	 #1			


stack:      dc.l    0

;----------------------------------------------------------------------------------
newvbl:
    movem.l d0-d7/a0-a6,-(sp)

    ifne    rasterization
    move.w  #2000,d0
.wait:
    dbra    d0,.wait
    move.w  #7,$ffff8240.w
    endc

    bsr.s   SNDsoundtrackMonitor

    lea     .return(pc),a1
    move.l  currentscore(pc),a0
    jmp (a0)                        ; play score routine (refresh regularly each frame or a few frames)

.return:
    move.l  a0,currentscore

    ifne    rasterization
    move.w  #-1,$ffff8240.w
    endc

    movem.l (sp)+,d0-d7/a0-a6

systemvbl:
    jmp     $12341234


SNDsoundtrackMonitor:

	; ------------------------------------
	; things to do each time the DMA loops
	; ------------------------------------

	lea		$FFFF8907.w,a0
	movep.l	(a0),d0						; determine if DMA sound has looped during the frame
	and.l	#$FFFFFF,d0					; without using interruption (Timer A or screen)
    move.l  d0,a1

	cmp.l	SNDlastDMAposition(pc),d0	;
	slt.s	SNDhaslooped    			;
    bge.s   noloop

	addq.w	#1,SNDdmaLoopCount	        ;

    ; LEFT VOLUME

	moveq.l	#0,d0
	move.b	SNDleftVolume(pc),d0
	blt.s	donotsetleft
	
    or.w	#$540,d0					; set volume left command
	or.b	#$80,SNDleftVolume

    move.w  #$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w				; send order to Microwire

donotsetleft:
noloop:									;

    move.l	a1,SNDlastDMAposition		; store current address for future compairison
	
	; ------------------------------------
	; now, things to do each frame
	; ------------------------------------

	; TRANSFER COMMAND

	move.l	SNDsourceTransfer(pc),d0
	cmp.l	SNDendSourceTransfer(pc),d0
	bge		notransfer

	movem.l	a1/d1-d7,-(sp)

	move.l	SNDdestTransfer(pc),a1
	move.l	d0,a0

var:	set 0
	rept SND_TRANSFER_LEN
	movem.l (a0)+,d0-d7
	movep.l	d0,var(a1) 
	movep.l	d1,var+8(a1) 
	movep.l	d2,var+16(a1) 
	movep.l	d3,var+24(a1) 
	movep.l	d4,var+32(a1) 
	movep.l	d5,var+40(a1) 
	movep.l	d6,var+48(a1) 
	movep.l	d7,var+56(a1) 
var:	set var+64
	endr

	lea		64*SND_TRANSFER_LEN(a1),a1
	move.l	a0,SNDsourceTransfer
	move.l	a1,SNDdestTransfer

	movem.l	(sp)+,a1/d1-d7

notransfer:

    tst.b   SNDhaslooped                ; avoid sending channel level orders and volume order in the same frame
    bne.s   managerightvolume          

	; MASTER VOLUME

	tst.b	SNDfademasterVolume		    ; master volume needs a fade ?
	beq.s	donotsetvolume
	blt.s	fadeout

	subq.b	#1,SNDfadecount
	bgt.s	donotsetvolume
	move.b	SNDspeedfade(pc),SNDfadecount

	moveq.l	#0,d0						; fade-in required
	move.b	SNDmasterVolume(pc),d0
	addq.b	#1,d0
	cmp.b	#41,d0
	bge.s	endsetvolume

	move.b	d0,SNDmasterVolume
	or.w	#$4C0,d0

    move.w	#$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w

	rts

fadeout:
	subq.b	#1,SNDfadecount
	bgt.s	donotsetvolume
	move.b	SNDspeedfade(pc),SNDfadecount

	moveq.l	#0,d0						; fade-out required
	move.b	SNDmasterVolume(pc),d0
	subq.b	#1,d0
	blt.s	endsetvolume

	move.b	d0,SNDmasterVolume
	or.w	#$4C0,d0

    move.w	#$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w

	rts

endsetvolume:
	clr.b	SNDfademasterVolume		    ; fade range limit reached

donotsetvolume:
    rts

   
managerightvolume:

	; RIGHT VOLUME

	moveq.l	#0,d0
	move.b	SNDrightVolume(pc),d0
	blt.s	donotsetright

	or.w	#$500,d0					; set volume right command
	or.b	#$80,SNDrightVolume

waitmicrowire:                          ; wait in case we have sent an order for left channel
	cmp.w	#$7FF,$FFFF8924.w
	bne.s	waitmicrowire

    move.w  #$7FF,$FFFF8924.w
	move.w	d0,$FFFF8922.w				; send order to Microwire

donotsetright:

	rts


SNDplayNoise:
    move.w           #$2700,sr

    move.b           #6,$ffff8800.w     ; select noise register
    move.b           d0,$ffff8802.w     ; data
    move.b           #8,$ffff8800.w     ; select level ch a register
    move.b           d1,$ffff8802.w     ; data
    move.b           #7,$ffff8800.w     ; select io and mixer register
    move.b           $ffff8802.w,d0
    and.b            #$C0,d0
    or.b             #$37,d0
    move.b           d0,$ffff8802.w
    
    move.w           #$2300,sr

    rts

SNDstopNoise:
    move.w           #$2700,sr

    move.b           #7,$ffff8800.w     ; select io and mixer register
    move.b           $ffff8802.w,d0
    and.b            #$C0,d0
    or.b             #$3F,d0
    move.b           d0,$ffff8802.w

    move.w           #$2300,sr

    rts



SNDleftVolume:			dc.b	-1
SNDrightVolume:		    dc.b	-1
SNDmasterVolume:		dc.b	40
SNDfademasterVolume:	dc.b	0
SNDspeedfade:			dc.b	25
SNDfadecount:			dc.b	0
SNDdmaLoopCount:		dc.w	0
SNDlastDMAposition:	    dc.l	0
SNDsourceTransfer:		dc.l	0
SNDdestTransfer:		dc.l	0
SNDendSourceTransfer:	dc.l	0
SNDhaslooped:           dc.b    0    


SNDchannelVolume:   dc.b     20,19,18,17,16,13,0

SND_LOADSAMPLE MACRO	
    move.l	#z_\1,sampleToPlay
    ENDM

SND_SETPANNING MACRO
    move.b	SNDchannelVolume+\1(pc),SNDleftVolume	 
    move.b	SNDchannelVolume+6-\1(pc),SNDrightVolume
    ENDM

SND_VOLUME_SET MACRO	
    move.b	#\1,SNDmasterVolume	
    move.w  #HW_MICROWIRE_MASK_SOUND,HW_MICROWIRE_MASK	
    move.w  #HW_MICROWIRE_VOLUME+\1,HW_MICROWIRE_DATA
    ENDM

SND_COPYCACHE_TO_LEFT MACRO
    move.l	#DMABuffer,SNDdestTransfer 	
    add.l	#\1*2,SNDdestTransfer     	
    move.l	sampleToPlay(pc),SNDsourceTransfer 	
    move.l	sampleToPlay(pc),SNDendSourceTransfer 	
    add.l	#\2,SNDendSourceTransfer
    ENDM

SND_COPYCACHE_TO_RIGHT MACRO
	move.l	#DMABuffer,SNDdestTransfer 	
    add.l	#(\1*2)+1,SNDdestTransfer 	
    move.l	sampleToPlay(pc),SNDsourceTransfer 	
    move.l	sampleToPlay(pc),SNDendSourceTransfer 	
    add.l	#\2,SNDendSourceTransfer
    ENDM

SND_WAIT_CACHECOPIED MACRO
    lea     *+4(pc),a0
    move.l  SNDsourceTransfer(pc),d0
	cmp.l	SNDendSourceTransfer(pc),d0
    bge.s	*+4
    jmp     (a1)
    ENDM
    	
SND_DMALOOP_RESET MACRO
    move.w SNDdmaLoopCount(pc),dmaLoopstart
    ENDM

SND_WAIT_DMALOOP_REACH MACRO
    lea     *+4(pc),a0
    move.w  SNDdmaLoopCount(pc),d0
    sub.w   dmaLoopstart(pc),d0
    cmp.w   #\1,d0
    bge.s   *+4
    jmp     (a1)
    ENDM

SND_VOLUME_FADEOUT MACRO
    move.b  #\1,SNDspeedfade
    move.b  #40,SNDmasterVolume
    move.b  #-1,SNDfademasterVolume
    ENDM

    ;----------------------------------------------------------------------------------------------------

score:

	SND_SETPANNING          0  
    SND_VOLUME_SET          0  
                                                      
    SND_LOADSAMPLE          4190
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_S
    SND_WAIT_CACHECOPIED
	SND_COPYCACHE_TO_LEFT   SAMPLE_S,SAMPLE_S
    SND_WAIT_CACHECOPIED

    SND_DMALOOP_RESET
    SND_WAIT_DMALOOP_REACH  1
    SND_VOLUME_SET          40

    SND_LOADSAMPLE          360222
	SND_COPYCACHE_TO_RIGHT  0,SAMPLE_S
    SND_WAIT_CACHECOPIED
	SND_COPYCACHE_TO_RIGHT  SAMPLE_S,SAMPLE_S
    SND_WAIT_CACHECOPIED

    SND_WAIT_DMALOOP_REACH  2

	SND_DMALOOP_RESET		
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  1

    SND_LOADSAMPLE          1606334
    SND_COPYCACHE_TO_LEFT   0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET   
    SND_SETPANNING          5
    SND_WAIT_DMALOOP_REACH  1
    SND_SETPANNING          4
    SND_WAIT_DMALOOP_REACH  2
	SND_SETPANNING          3
    SND_WAIT_DMALOOP_REACH  3
	SND_SETPANNING          2
    SND_WAIT_DMALOOP_REACH  4
	SND_SETPANNING          1
    SND_WAIT_DMALOOP_REACH  5
	SND_SETPANNING          0
    SND_WAIT_DMALOOP_REACH  6                                                        
    
	SND_LOADSAMPLE          1784382
    SND_COPYCACHE_TO_RIGHT  0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET	
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  1

    SND_LOADSAMPLE          3475534
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET									
    SND_WAIT_DMALOOP_REACH  1						
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  2
	SND_SETPANNING          5
    SND_WAIT_DMALOOP_REACH  3
	SND_SETPANNING          4				
    SND_WAIT_DMALOOP_REACH  4
	SND_SETPANNING          3				
    SND_WAIT_DMALOOP_REACH  5
	SND_SETPANNING          2				
    SND_WAIT_DMALOOP_REACH  6
	SND_SETPANNING          1				
    SND_WAIT_DMALOOP_REACH  7
	SND_SETPANNING          0				
    SND_WAIT_DMALOOP_REACH  8

	SND_LOADSAMPLE          3564542
	SND_COPYCACHE_TO_RIGHT  0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET	
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  1
                                                        
    SND_LOADSAMPLE          5967758
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_L
    SND_WAIT_CACHECOPIED
	SND_WAIT_DMALOOP_REACH  2
                                                        
	SND_DMALOOP_RESET									
    SND_WAIT_DMALOOP_REACH  1
	SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  2
	SND_SETPANNING          5
    SND_WAIT_DMALOOP_REACH  3
	SND_SETPANNING          4
    SND_WAIT_DMALOOP_REACH  4
	SND_SETPANNING          3
    SND_WAIT_DMALOOP_REACH  5
	SND_SETPANNING          2
    SND_WAIT_DMALOOP_REACH  6
	SND_SETPANNING          1
    SND_WAIT_DMALOOP_REACH  7
	SND_SETPANNING          0
    SND_WAIT_DMALOOP_REACH  8

    SND_LOADSAMPLE          6056766      
    SND_COPYCACHE_TO_RIGHT  0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET	
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  2
       
    ; Interlude 2
    SND_LOADSAMPLE          7391886
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET						
    SND_WAIT_DMALOOP_REACH  1
	SND_SETPANNING			6
    SND_WAIT_DMALOOP_REACH  2
	SND_SETPANNING			5				
    SND_WAIT_DMALOOP_REACH  3
	SND_SETPANNING			4				
    SND_WAIT_DMALOOP_REACH  4
	SND_SETPANNING			3				
    SND_WAIT_DMALOOP_REACH  5
	SND_SETPANNING			2				
    SND_WAIT_DMALOOP_REACH  6
	SND_SETPANNING			1				
    SND_WAIT_DMALOOP_REACH  7
    SND_SETPANNING			0				
    SND_WAIT_DMALOOP_REACH  8

    SND_LOADSAMPLE          7569902
	SND_COPYCACHE_TO_RIGHT  0,SAMPLE_L
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET	
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  1

    SND_LOADSAMPLE          6056766
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_L
    SND_WAIT_CACHECOPIED

    SND_WAIT_DMALOOP_REACH  8

    clr.w   d0
    move.w  #3,d1
    bsr SNDplayNoise

    SND_DMALOOP_RESET	
    SND_WAIT_DMALOOP_REACH  5

    SND_DMALOOP_RESET
    SND_SETPANNING          0
    SND_WAIT_DMALOOP_REACH  1
    
    bsr SNDstopNoise                                    

    SND_DMALOOP_RESET

    SND_LOADSAMPLE          12465342
	SND_COPYCACHE_TO_RIGHT  0,SAMPLE_L
    SND_WAIT_CACHECOPIED

    SND_WAIT_DMALOOP_REACH  3

	SND_DMALOOP_RESET									
    SND_WAIT_DMALOOP_REACH  1
	SND_SETPANNING			1				
    SND_WAIT_DMALOOP_REACH  2
	SND_SETPANNING			2				
    SND_WAIT_DMALOOP_REACH  3
	SND_SETPANNING			3				
    SND_WAIT_DMALOOP_REACH  4
	SND_SETPANNING			4				
    SND_WAIT_DMALOOP_REACH  5
	SND_SETPANNING			5				
    SND_WAIT_DMALOOP_REACH  6
	SND_SETPANNING			6
    SND_WAIT_DMALOOP_REACH  11

	SND_LOADSAMPLE          14245502
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_S
    SND_WAIT_CACHECOPIED
	SND_COPYCACHE_TO_LEFT   SAMPLE_S,SAMPLE_S
    SND_WAIT_CACHECOPIED

	SND_DMALOOP_RESET	
    SND_SETPANNING          0
    SND_WAIT_DMALOOP_REACH  1          
    SND_WAIT_DMALOOP_REACH  3
    
    SND_LOADSAMPLE          14646038
	SND_COPYCACHE_TO_RIGHT  0,SAMPLE_S
    SND_WAIT_CACHECOPIED
	SND_COPYCACHE_TO_RIGHT  SAMPLE_S,SAMPLE_S
    SND_WAIT_CACHECOPIED
	
    SND_DMALOOP_RESET	
    SND_SETPANNING          6
    SND_WAIT_DMALOOP_REACH  1

    SND_LOADSAMPLE          360222
	SND_COPYCACHE_TO_LEFT   0,SAMPLE_S
    SND_WAIT_CACHECOPIED
	SND_COPYCACHE_TO_LEFT   SAMPLE_S,SAMPLE_S
    SND_WAIT_CACHECOPIED

    SND_WAIT_DMALOOP_REACH  4

    SND_DMALOOP_RESET   
    SND_SETPANNING          0
    SND_WAIT_DMALOOP_REACH  3
	SND_VOLUME_FADEOUT      50
    SND_DMALOOP_RESET			
    SND_WAIT_DMALOOP_REACH  12

    lea *+0(pc),a0          ; end loop score
    jmp (a1)

    ;------------------------------------------------------------------------------	

    section data

currentscore:   dc.l    0
sampleToPlay:   dc.l    0
dmaLoopstart:   dc.w    0

z_4190: 	incbin	zik\4190.raw
z_360222: 	incbin	zik\360222.raw
z_1606334: 	incbin	zik\1606334.raw
z_1784382: 	incbin	zik\1784382.raw
z_3475534: 	incbin	zik\3475534.raw
z_3564542: 	incbin	zik\3564542.raw
z_5967758: 	incbin	zik\5967758.raw
z_6056766:  incbin	zik\6056766.raw
z_7391886:	incbin	zik\7391886.raw
z_7569902: 	incbin	zik\7569902.raw
z_12465342: incbin	zik\12465342.raw
z_14245502: incbin	zik\14245502.raw
z_14646038: incbin	zik\14646038.raw

        even

DMABuffer:  ds.w    SAMPLE_L
            ds.b    1024
