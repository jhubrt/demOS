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

	output	D:\PROJECTS\DEMOS\OUTPUT\TRACE.O
	
	opt 	o+

	xdef SYSfontbitmap,SYSfontchars,SYSfastPrint

;-----------------------------------------------------------------------------------------
; void SYSfastPrint (char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch) 
; a0 = string
; a1 = screendisplay
; d0 = screenpitch
; d1 = bitplanepitch
;-----------------------------------------------------------------------------------------
SYSfastPrint:

	movem.l	a2-a4/d4,-(sp)

    cmp.w   SYSfastPrintautomod+2(pc),d0
    beq.s   SYS_debugPrint_noautomod

    lea     SYSfastPrintautomod+2(pc),a4        ; check current automod state with current pitch
    lea     SYSfastPrintautomod2+2(pc),a3

    move.w  d0,d4
    move.w  d4,(a4)
    move.w  d4,(a3)
    add.w   d0,d4
    move.w  d4,4(a4)
    move.w  d4,4(a3)
    add.w   d0,d4
    move.w  d4,8(a4)
    move.w  d4,8(a3)
    add.w   d0,d4
    move.w  d4,12(a4)
    move.w  d4,12(a3)
    add.w   d0,d4
    move.w  d4,16(a4)
    move.w  d4,16(a3)
    add.w   d0,d4
    move.w  d4,20(a4)
    move.w  d4,20(a3)
    add.w   d0,d4
    move.w  d4,24(a4)
    move.w  d4,24(a3)

SYS_debugPrint_noautomod:

	move.l	SYSfontbitmap(pc),a3
	lea 	SYSfontchars(pc),a2

	subq.w  #1,d1

    move.w  a1,d4
    and.w   #1,d4
    bne.s   SYS_debugPrint_loop_part2

SYS_debugPrint_loop:



	moveq.l	#0,d4
    move.b  (a0)+,d4
	beq.s	SYS_debugPrint_end

	add.w	d4,d4
	move.w	(a2,d4.w),d4
	blt.s	SYS_debugPrint_noprint

	lea		(a3,d4.w),a4		    ; a4 = char bitmap adr

	move.b	(a4)+,(a1)			
SYSfastPrintautomod:
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4),2(a1)			

SYS_debugPrint_noprint:
	
	addq.l	#1,a1




SYS_debugPrint_loop_part2:
    moveq.l #0,d4
    move.b  (a0)+,d4
    beq.s   SYS_debugPrint_end

	add.w	d4,d4
	move.w	(a2,d4.w),d4
	blt.s	SYS_debugPrint_noprint2

	lea		(a3,d4.w),a4		    ; a4 = char bitmap adr

	move.b	(a4)+,(a1)			
SYSfastPrintautomod2:
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4)+,2(a1)			
	move.b	(a4),2(a1)			

SYS_debugPrint_noprint2:

	add.w	d1,a1                   ; add  bitplanepitch - 2

	bra.s	SYS_debugPrint_loop

SYS_debugPrint_end:

	movem.l	(sp)+,a2-a4/d4

	rts


SYSfontbitmap:	dc.l	0
SYSfontchars:   ds.w    256,0
