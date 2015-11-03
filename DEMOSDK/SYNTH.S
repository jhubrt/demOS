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
    xdef    SNDplayNoise,SNDstopNoise
    
*-------------------------------------------------------------------------
SNDsynMonitor:
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
