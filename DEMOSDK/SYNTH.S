*-------------------------------------------------------------------------
* Copyright J.Hubert 2015
*
* This file is part of demOS
*
* demOS is free software: you can redistribute it and/or modify it under the terms of 
* the GNU Lesser General Public License as published by the Free Software Foundation, 
* either version 3 of the License, or (at your option) any later version.
*
* demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with demOS.  
* If not, see <http://www.gnu.org/licenses/>.
*-------------------------------------------------------------------------

*--------------------------------------------------------------------------
* Defines ASM routines for system core functionnalities
* in particular implements the VBL interupts routine that manages:
* - soundtrack routine slicing
*--------------------------------------------------------------------------

	output	D:\PROJECTS\DEMOS\OUTPUT\SYNTH.O
	
	opt 	o+

    xdef    SNDsynMonitor
    xdef	SNDleftVolume,SNDrightVolume,SNDmasterVolume
    xdef    SNDplayNoise,SNDstopNoise,SNDsynTranspose
    
*-------------------------------------------------------------------------
SNDsynMonitor:
    rts

;-------------------------------------------------------------------------
; void SNDsynTranspose (s8* _sourcesample, s8* _dest, u16 _destlen, u32 _inc)
;
;   a0 = sourcesample
;   a1 = destsample
;   d0 = destlen
;   d1 = inc
;-------------------------------------------------------------------------
SNDsynTranspose:
    movem.l d3-d7/a2,-(sp)

    moveq.l #0,d4           ; d4 = acc
    subq.l  #1,d0           ; destlen - 1 (use dbra)

.loop:
    move.l  d4,d7           ; d7 = index
    swap    d7              ; index = acc >> 16

    moveq.l #0,d5
    moveq.l #0,d6
    lea     (a0,d7.w),a2
    move.b  (a2)+,d5        ; d5 = sample 0
    move.b  (a2),d6         ; d6 = sample 1
    add.b   #128,d5         ; go to signed pcm...
    add.b   #128,d6         ;
    
    mulu.w  d4,d6           ; STDmulu (sample1, error);
    moveq.l #-1,d7
    sub.w   d4,d7           
    mulu.w  d7,d5           ; STDmulu (sample0, 0xFFFF - error);

    add.l   d5,d6           ; (sm0 + sm1) >> 16;
    swap    d6
    sub.b   #128,d6         ; go signed pcm

    smi     (a1)+           ; if ( sample >= 0 )  *_dest++ = 0; else *_dest++ = 0xFF;
    move.b  d6,(a1)+        ; *_dest++ = sample;

    add.l   d1,d4           ; acc += sourceinc;
    
    dbra.w  d0,.loop

    movem.l (sp)+,d3-d7/a2

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

	END
