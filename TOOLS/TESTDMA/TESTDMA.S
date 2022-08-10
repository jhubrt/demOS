HW_DMASOUND_CONTROL			    set $FFFF8901
HW_DMASOUND_STARTADR_H		    set $FFFF8903
HW_DMASOUND_STARTADR_M		    set $FFFF8905
HW_DMASOUND_STARTADR_L		    set $FFFF8907
HW_DMASOUND_ENDADR_H		    set $FFFF890F
HW_DMASOUND_ENDADR_M		    set $FFFF8911 
HW_DMASOUND_ENDADR_L		    set $FFFF8913 
HW_DMASOUND_COUNTER_H		    set $FFFF8909  	
HW_DMASOUND_COUNTER_M		    set $FFFF890B  	
HW_DMASOUND_COUNTER_L		    set $FFFF890D  	
HW_DMASOUND_MODE     		    set $FFFF8920

HW_DMASOUND_CONTROL_OFF			set 0
HW_DMASOUND_CONTROL_PLAYONCE	set 1
HW_DMASOUND_CONTROL_PLAYLOOP	set 3

HW_DMASOUND_MODE_6258HZ		    set 0
HW_DMASOUND_MODE_12517HZ	    set 1
HW_DMASOUND_MODE_25033HZ	    set 2
HW_DMASOUND_MODE_50066HZ	    set 3

HW_DMASOUND_MODE_MONO		    set $80
HW_DMASOUND_MODE_STEREO		    set 0
    
HW_MFP_INTERRUPT_ENABLE_A		set $FFFFFA07
HW_MFP_INTERRUPT_ENABLE_B		set $FFFFFA09

HW_BLITTER_HOP_BIT1                 set 0     
HW_BLITTER_HOP_HTONE                set 1
HW_BLITTER_HOP_SOURCE               set 2
HW_BLITTER_HOP_SOURCE_AND_HTONE     set 3
HW_BLITTER_OP_BIT0                  set 0     
HW_BLITTER_OP_S_AND_D               set 1
HW_BLITTER_OP_S_AND_NOT_D           set 2
HW_BLITTER_OP_S                     set 3
HW_BLITTER_OP_NOT_S_AND_D           set 4
HW_BLITTER_OP_D                     set 5
HW_BLITTER_OP_S_XOR_D               set 6
HW_BLITTER_OP_S_OR_D                set 7
HW_BLITTER_OP_NOT_S_AND_NOT_D       set 8
HW_BLITTER_OP_NOT_S_XOR_D           set 9
HW_BLITTER_OP_NOT_D                 set 10
HW_BLITTER_OP_S_OR_NOT_D            set 11
HW_BLITTER_OP_NOT_S                 set 12
HW_BLITTER_OP_NOT_S_OR_D            set 13
HW_BLITTER_OP_NOT_S_OR_NOT_D        set 14  
HW_BLITTER_OP_BIT1                  set 15
HW_BLITTER_CTRL1_BUSY               set $80
HW_BLITTER_CTRL1_HOGMODE_BLIT       set $40
HW_BLITTER_CTRL1_SMUDGE             set $20
HW_BLITTER_CTRL2_FORCE_XTRA_SRC     set $80
HW_BLITTER_CTRL2_NO_FINAL_SRC_READ  set $40

HW_BLITTER_HTONE                    set $FFFF8A00
HW_BLITTER_XINC_SOURCE              set $FFFF8A20
HW_BLITTER_YINC_SOURCE              set $FFFF8A22 
HW_BLITTER_ADDR_SOURCE              set $FFFF8A24 
HW_BLITTER_ENDMASK1                 set $FFFF8A28
HW_BLITTER_ENDMASK2                 set $FFFF8A2A
HW_BLITTER_ENDMASK3                 set $FFFF8A2C
HW_BLITTER_XINC_DEST                set $FFFF8A2E
HW_BLITTER_YINC_DEST                set $FFFF8A30
HW_BLITTER_ADDR_DEST                set $FFFF8A32
HW_BLITTER_XSIZE                    set $FFFF8A36
HW_BLITTER_YSIZE                    set $FFFF8A38
HW_BLITTER_HOP                      set $FFFF8A3A
HW_BLITTER_OP                       set $FFFF8A3B
HW_BLITTER_CTRL1                    set $FFFF8A3C
HW_BLITTER_CTRL2                    set $FFFF8A3D



    clr.l   -(sp)
    move.w  #$20,-(sp)
    trap    #1
    addq.l  #6,sp  

    move.w  #0,-(sp)
    move.l  #screen,-(sp)
    move.l  #screen,-(sp)
    move.w  #5,-(sp)
    trap    #14
    lea     12(sp),sp

    move.l  #sound_buffer,d0
    swap    d0
    move.b  d0,HW_DMASOUND_STARTADR_H.w
    swap    d0
    ror.w   #8,d0
    move.b  d0,HW_DMASOUND_STARTADR_M.w
    ror.w   #8,d0
    move.b  d0,HW_DMASOUND_STARTADR_L.w

    move.l  #sound_buffer+2000-2,d0
    swap    d0
    move.b  d0,HW_DMASOUND_ENDADR_H.w
    swap    d0
    ror.w   #8,d0
    move.b  d0,HW_DMASOUND_ENDADR_M.w
    ror.w   #8,d0
    move.b  d0,HW_DMASOUND_ENDADR_L.w

    move.b  #(HW_DMASOUND_MODE_50066HZ|HW_DMASOUND_MODE_STEREO),HW_DMASOUND_MODE.w
    move.b  #HW_DMASOUND_CONTROL_PLAYLOOP,HW_DMASOUND_CONTROL.w

    move.l  #vbl,$70.w

	clr.b   HW_MFP_INTERRUPT_ENABLE_A.w
	clr.b   HW_MFP_INTERRUPT_ENABLE_B.w
	
    moveq.l #-1,d1

    move.w  #160/8,HW_BLITTER_XSIZE.w
    move.w  #8,HW_BLITTER_XINC_SOURCE.w
    move.w  #8,HW_BLITTER_YINC_SOURCE.w
    move.w  #8,HW_BLITTER_XINC_DEST.w
    move.w  #8,HW_BLITTER_YINC_DEST.w
    move.b  #0,HW_BLITTER_CTRL2.w
    move.w  #-1,HW_BLITTER_ENDMASK1.w
    move.w  #-1,HW_BLITTER_ENDMASK2.w
    move.w  #-1,HW_BLITTER_ENDMASK3.w
    
loop:

    ; clear
    move.b  #HW_BLITTER_HOP_BIT1,HW_BLITTER_HOP.w
    move.b  #HW_BLITTER_OP_BIT0,HW_BLITTER_OP.w
    move.l  #screen,HW_BLITTER_ADDR_DEST.w
    move.w  #200,HW_BLITTER_YSIZE.w
    move.b  #(HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY),HW_BLITTER_CTRL1.w

    lea     screen(pc),a0
offset  set 0
    rept    20
    move.w  d1,offset(a0)
offset  set offset+8
    endr

    ; draw    
    move.b  #HW_BLITTER_HOP_SOURCE,HW_BLITTER_HOP.w
    move.b  #HW_BLITTER_OP_S_XOR_D,HW_BLITTER_OP.w
    move.l  #screen,HW_BLITTER_ADDR_SOURCE.w
    move.l  #screen+160,HW_BLITTER_ADDR_DEST.w
    move.w  #199,HW_BLITTER_YSIZE.w
    move.b  #(HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY),HW_BLITTER_CTRL1.w

    bra     loop

vbl: 
    rte

    ;-----------------------------------------------------------------------------
    section data

sound_buffer: 
    ds.b  2000

screen:
    ds.b  32000
