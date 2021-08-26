*-------------------------------------------------------------------------------------------------------
* The MIT License (MIT)
*
* Copyright (c) 2015-2021 J.Hubert
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

;-------------------------------------------------------------------------
; Floppy load routines
;-------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\LOAD.O
	
	opt 	o+


NBMAXREQUESTS:   equ    3

LOAD_Request_processed:         equ 1
LOAD_Request_sector:            equ 2
LOAD_Request_reserved:          equ 4
LOAD_Request_side_drive:        equ 6
LOAD_Request_track:             equ 8
LOAD_Request_nbsectors:         equ 10
LOAD_Request_address:           equ 12
LOAD_Request_order:             equ 16

LOAD_Request_sizeof:            equ 18


loadfloppysideselect			equ $E00

;-------------------------------------------------------------------------
;-------------------------------------------------------------------------

	xdef LOADpush,LOADsecpertrack,LOADinprogress,LOADidle
    xdef LOADdeconnectAndStartRequest,LOADtoggleDriveStartRequest
    xdef LOADlastdrive

	; read only for debug only
	xdef LOADreq,LOADcurrentreq,LOADreqNb

    ; -----------------------------------------------------------

LOADpush:
    move.l      a2,-(sp)

    lea         LOADreq(pc),a2          ; lock less allocation of request

	rept		NBMAXREQUESTS            
	tas.b       (a2)
    beq.s       load_initReq
    lea         LOAD_Request_sizeof(a2),a2
    endr

    lea	        0.w,a0                  ; fail => no more free request
    move.l      (sp)+,a2

    rts

load_initReq:
    exg.l       a0,a2
    clr.b       LOAD_Request_processed(a0)
    move.l		a2,LOAD_Request_address(a0)
    move.w		d0,LOAD_Request_sector(a0)
    move.w		d1,LOAD_Request_track(a0)
    move.w		d2,LOAD_Request_nbsectors(a0)
    swap        d2
    move.w      d2,LOAD_Request_order(a0)

	move.w		#loadfloppysideselect,LOAD_Request_reserved(a0)

    move.w		#$400,LOAD_Request_side_drive(a0)     ; init side mask
	btst		#16,d1                  
    bne.s		load_drive
    move.w		#$500,LOAD_Request_side_drive(a0)     ;

load_drive:
    move.w      #$400,d2
	btst		#17,d1					              ; init drive mask
    beq.s		load_endinit
	eor.w		#$600,LOAD_Request_side_drive(a0)	  ; select drive 1
    move.w      #$200,d2

load_endinit:
    move.l      (sp)+,a2

	tas.b		LOADinprogress                        ; If FDC busy, returns the request handle (req will be processed later...)
    bne.s		.requestpending

    move.w      LOADlastdrive(pc),d1
    eor.w       d1,d2
    and.w       #$600,d2
    beq.s       LOADstartRequest

    move.l      a0,LOADdeconnectAndStartRequest

.requestpending:
    rts


LOADtoggleDriveStartRequest:
 
    btst.b      #2,LOADlastdrive
    bne.s       .drive0wasselected    

    move.w      LOADlasttrack(pc),LOADdrive1headpos         ; backup incremented track number from last request into drive1 head pos storage
    move.w      LOADdrive0headpos(pc),d0                    ; restore drive0 head pos
    bra.s       .drive1wasselected

.drive0wasselected:

    move.w      LOADlasttrack(pc),LOADdrive0headpos         ; same thing for opposite case
    move.w      LOADdrive1headpos(pc),d0

.drive1wasselected:

	move.w		#$82,$FFFF8606.w						    ; select FDC track register
	move.w		d0,$FFFF8604.w						        ; restore backuped value

LOADstartRequest: 
    move.l      a0,LOADcurrentreq

    move.l		LOAD_Request_reserved(a0),$FFFF8800.w		; select side + drive
	move.w		#$86,$FFFF8606.w			            	; select FDC data register 
    move.w		LOAD_Request_track(a0),$FFFF8604.w		    ; FDC data holds track number during seek command
    move.w		#$80,$FFFF8606.w				            ; select FDC register control 
    move.w		#$13,$FFFF8604.w				            ; seek command (enable spin up sequence, no verify, 3ms)

    move.l		#LOADreadSectorCommand,$11c.w 

    rts 
    
    ; -----------------------------------------------------------
  
LOADreadSectorCommand:
	move		#$2500,sr

    move.l      a0,-(sp)
    move.l      LOADcurrentreq(pc),a0

    move.l		LOAD_Request_reserved(a0),$FFFF8800.w	; select side + drive

	move.b		LOAD_Request_address+3(a0),$FFFF860D.w	; store address into DMA from low to high
    move.b		LOAD_Request_address+2(a0),$FFFF860B.w 
    move.b		LOAD_Request_address+1(a0),$FFFF8609.w

	move.w		#$90,$FFFF8606.w						; 
	move.w		#$190,$FFFF8606.w						; toggle dma read / write bit to reset DMA

	move.w		#$90,$FFFF8606.w						; select DMA count register
	move.w		#1,$FFFF8604.w							; sector count register

	move.w		#$84,$FFFF8606.w						; select sector register
	move.w		LOAD_Request_sector(a0),$FFFF8604.w		; store sector number
	move.w		#$80,$FFFF8606.w						; select FDC register control
	move.w		#$88,$FFFF8604.w						; read sector command (single sector, disable spin up sequence, no delay)

	subq.w		#1,LOAD_Request_nbsectors(a0)
	bne.s		loadroutine1_goon						; no more sector to load ?
	move.l		#LOADend,$11C.w							; load ended
	
    move.l      (sp)+,a0
	rte 

	  
loadroutine1_goon:
	move.w		d0,-(sp)	
    addq.w		#1,LOAD_Request_sector(a0)
    move.w		LOADsecpertrack(pc),d0
    cmp.w		LOAD_Request_sector(a0),d0
    bge.s		loadroutine1_nextsector 
    
	move.w		#1,LOAD_Request_sector(a0)

	eor.w		#$100,LOAD_Request_side_drive(a0)
	btst.b		#0,LOAD_Request_side_drive(a0)
	beq.s		loadroutine1_nextsector		        ; equal 0 means we just arrive on side 1 => go next sector
    
	move.l		#LOADseekTrackCommand,$11C.w		; else means we leave side 1 to side 0 => inc track
    addq.w		#1,LOAD_Request_track(a0)

loadroutine1_nextsector:
	add.l		#512,LOAD_Request_address(a0)
    move.w		(sp)+,d0

    move.l      (sp)+,a0
	rte

    ; -----------------------------------------------------------

LOADseekTrackCommand:
	move		#$2500,sr 

    move.l      a0,-(sp)
    move.l      LOADcurrentreq(pc),a0

    move.w		#$86,$FFFF8606.w				    ; select FDC data register 
    move.w		LOAD_Request_track(a0),$FFFF8604.w	; FDC data holds track number during seek command
    move.w		#$80,$FFFF8606.w				    ; select FDC register control 
    move.w		#$1B,$FFFF8604.w				    ; seek command (disable spin up sequence, no verify, 3ms)
    move.l		#LOADreadSectorCommand,$11C.w

    move.l      (sp)+,a0
    rte 


    ; --------------------------------------------------------------
    ; LOADend routine ends the current load request
    ; If another request has been queued, it restarts automatically
    ; the new load request
    ; --------------------------------------------------------------

LOADend:
	move		#$2500,sr

    move.l      a0,-(sp)
    move.w      d0,-(sp)
    move.w      d1,-(sp)
    move.l      d2,-(sp)

    move.l      LOADcurrentreq(pc),a0           ; declare current request as processed
    move.w      LOAD_Request_side_drive(a0),d0
    move.w      d0,LOADlastdrive
    move.w      LOAD_Request_track(a0),LOADlasttrack
    move.b      #1,LOAD_Request_processed(a0)

    lea         LOADreq(pc),a0                  ; look for waiting request  
    move.w      #-1,d1                          ; d1 = min order
    moveq.l     #0,d2                           ; d2 = request adr of min order

    ; loop the requests to find best choice into possible pending request

	rept		NBMAXREQUESTS
	tst.b       (a0)                            ; should be allocated
    beq.s       *+20
    
    tst.b       LOAD_Request_processed(a0)      ; not processed
    bne.s       *+14

    cmp.w       LOAD_Request_order(a0),d1
    blo.s       *+8

    move.l      a0,d2
    move.w      LOAD_Request_order(a0),d1
    
    lea         LOAD_Request_sizeof(a0),a0
	endr

    ; look at selected request

    tst.l       d2                              ; if none end
    beq.s       .switchoff

    move.l      d2,a0
    move.w      LOAD_Request_side_drive(a0),d1
    eor.w       d1,d0
    and.w       #$600,d0
    beq.s       .startRequestStraightOn

    move.l      d2,LOADdeconnectAndStartRequest
    bra.s       .return

.startRequestStraightOn:
    bsr         LOADstartRequest                ; start the request
    bra.s       .return

.switchoff:
    
	move.l		#LOADidle,$11c.w
	clr.b		LOADinprogress                  ; nothing to do : declare FDC as free

.return:
    move.l      (sp)+,d2
    move.w      (sp)+,d1
    move.w      (sp)+,d0
    move.l      (sp)+,a0

LOADidle:
	rte 


LOADinprogress:			        dc.w		0
LOADsecpertrack:		        dc.w		10	

LOADcurrentreq:		            dc.l		0
LOADdeconnectAndStartRequest:   dc.l        0

LOADdrive0headpos:              dc.w        0
LOADdrive1headpos:              dc.w        0
LOADlastdrive:                  dc.w        $400       ; default drive 0
LOADlasttrack:                  dc.w        0

LOADreqNb:                      dc.w    NBMAXREQUESTS

LOADreq:
    rept    NBMAXREQUESTS
    dc.l	0
    dc.l	0
    dc.l	0
    dc.l	0
	dc.w	0
    endr


	END
