  
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

HW_VIDEO_MODE		                set $FFFF8260

HW_VIDEO_BASE_H		                set $FFFF8201
HW_VIDEO_BASE_M		                set $FFFF8203
HW_VIDEO_BASE_L		                set $FFFF820D

HW_VIDEO_COUNT_H                    set $FFFF8205
HW_VIDEO_COUNT_M	                set $FFFF8207
HW_VIDEO_COUNT_L	                set $FFFF8209
                                    
HW_VIDEO_SYNC   	                set $FFFF820A
                                    
HW_VIDEO_OFFSET				        set $FFFF820F
HW_VIDEO_PIXOFFSET			        set $FFFF8265
HW_VIDEO_PIXOFFSET_HIDDEN	        set $FFFF8264




    clr.l   -(sp)
    move.w  #$20,-(sp)
    trap    #1
    addq.l  #6,sp  

    move.l  #screen,d0
    clr.w   d0
    move.l  d0,base

    move.w  #0,-(sp)
    move.l  d0,-(sp)
    move.l  d0,-(sp)
    move.w  #5,-(sp)
    trap    #14
    lea     12(sp),sp

    move.l  #vbl,$70.w

	clr.b   HW_MFP_INTERRUPT_ENABLE_A.w
	clr.b   HW_MFP_INTERRUPT_ENABLE_B.w

    move.b  #2,HW_VIDEO_SYNC.w
    move.b  #1,HW_VIDEO_MODE.w
	move.b  #(256-160)/2,HW_VIDEO_OFFSET.w

    moveq.l #-1,d1

    move.w  #512/8*210,HW_BLITTER_XSIZE.w
    move.w  #0,HW_BLITTER_XINC_SOURCE.w
    move.w  #0,HW_BLITTER_YINC_SOURCE.w
    move.w  #0,HW_BLITTER_XINC_DEST.w
    move.w  #0,HW_BLITTER_YINC_DEST.w
    move.b  #0,HW_BLITTER_CTRL2.w
    move.w  #-1,HW_BLITTER_ENDMASK1.w
    move.w  #-1,HW_BLITTER_ENDMASK2.w
    move.w  #-1,HW_BLITTER_ENDMASK3.w
    move.w  #$303,HW_BLITTER_HOP.w
    move.l  #HW_VIDEO_COUNT_M,HW_BLITTER_ADDR_SOURCE.w
    move.l  #$ffff8240,HW_BLITTER_ADDR_DEST.w
    move.w  #$ff,HW_BLITTER_HTONE.w
    
loop:

    stop    #$2300

    move.w  #-1,$ffff8240.w

.wait:
    move.b  HW_VIDEO_COUNT_L.w,d0
    beq.s   .wait

    neg.b   d0
    add.b   #15,d0
    lsl.w   d0,d1

    move.w  #1,HW_BLITTER_YSIZE.w
    move.b  #(HW_BLITTER_CTRL1_HOGMODE_BLIT|HW_BLITTER_CTRL1_BUSY),HW_BLITTER_CTRL1.w

    move.w  #0,$ffff8240.w

    bra     loop

vbl: 
    rte

    ;-----------------------------------------------------------------------------
    section data
base:   
    dc.l   0 

    ds.b    65536
screen:
    ds.b    65536
