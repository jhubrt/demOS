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

	output	D:\PROJECTS\DEMOS\OUTPUT\RASTERS.O
	
	opt 	o+

*-------------------------------------------------------------------------
* Rasters routines
*-------------------------------------------------------------------------

	* void RASsetColReg (u16 shortregaddress);
	* void* (function pointers) RAStop1, RASmid1, RASbot1
	* void* RASNextOpList

	xdef	RASsetColReg
	xdef	RASvbl1,RASvbl2,RAStop1,RASmid1,RASbot1
	xdef	RASnextOpList

RASsetColReg:
	move.w	d0,RAStop1_automod+2			* set the color register address that will be affected
	move.w	d0,RASmid1_automod+2
	move.w	d0,RASbot1_automod+2
	move.w	d0,RASvbl2_automod+2
	rts


	* -----------------------------------------
	* OpList for RASvbl1
	* [.w]	background color
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASvbl1:
	move.w	(a0)+,$ffff8240.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.l	a0,usp
	rts

	* -----------------------------------------
	* OpList for VBL routine
	* [.w]	background color
	* [.w]	color change
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------
RASvbl2:
	move.w	(a0)+,$ffff8240.w
RASvbl2_automod:
	move.w	(a0)+,$ffff8242.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.l	a0,usp
	rts

	* -----------------------------------------
	* OpList for RAStop1
	* [.w]	background color
	* [.w]	color change
	* [.w]	scan lines count to next inter call (timerb data register)
	* [.l]	next raster routine
	* -----------------------------------------

RAStop1:
	move.l	a0,-(sp)
	move.l	usp,a0
	move.w	(a0)+,$ffff8240.w
RAStop1_automod:
	move.w	(a0)+,$ffff8242.w
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASmid1
	* [.w]	color change
	* if color value < 0 (means last call)
	*		[.w]	scan lines count to next inter call (timerb data register)
	*		[.l]	next raster routine
	* -----------------------------------------

RASmid1:
	move.l	a0,-(sp)
	move.l	usp,a0
RASmid1_automod:
	move.w	(a0)+,$ffff8242.w
	blt.s	RASmid1_gonext			* short branches are faster when condition is not verified
	move.l	a0,usp
	move.l	(sp)+,a0
	rte
RASmid1_gonext:
	clr.b	$fffffa1b.w
	move.w	(a0)+,$fffffa20.w
	move.l	(a0)+,$120.w
	move.b	#8,$fffffa1b.w
	move.l	a0,usp
	move.l	(sp)+,a0
	rte

	* -----------------------------------------
	* OpList for RASbot1
	* [.w]	background color
	* [.w]	color change
	* -----------------------------------------

RASbot1:
	move.l	a0,-(sp)
	move.l	usp,a0
	move.w	(a0)+,$ffff8240.w
RASbot1_automod:
	move.w	(a0)+,$ffff8242.w
	move.l	a0,usp
	clr.b	$fffffa1b.w
	move.l	(sp)+,a0
	rte

RASnextOpList:	dc.l	0
RASopList:		dc.l	0

	END
