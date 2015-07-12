/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of demOS

  demOS is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with demOS.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

/*! @brief @ref RAS @file */

/*! @defgroup  RAS
   
    RAS provides services to setup raster (synchronized color changes) interrupts frames    <br>

    Raster system provides default interrupt routines to change colors on the screen. 
    You can also provide your own routines. 
    Raster system uses a kind of display list where you can chain raster 
    interrupt routines and associated data. 
    The standard provided routines backup 68k registers but they use 'usp' hidden register 
    to save the list pointer.
*/

#ifndef RASTERS_H
#define RASTERS_H

#include "DEMOSDK\BASTYPES.H"


void RASsetColReg (u16 _shortregaddress);
void RASvbl1	  (void);
void RASvbl2	  (void);
void RAStop1	  (void);
void RASmid1	  (void);
void RASbot1	  (void);

#ifndef RASTERS_C
extern void* RASnextOpList;
#endif

/* raster OpList */

typedef void(*RASinterupt)(void);

/*! ----------------------------------------- @struct RASopVbl1
* OpList for VBL routine RASvbl1                                    <br>
* [.w]	background color                                            <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
STRUCT(RASopVbl1)
{
	u16			backgroundColor;
	u16			scanLinesTo1stInterupt;
	RASinterupt	nextRasterRoutine;
};

/*! ----------------------------------------- @struct RASopVbl2
* OpList for VBL routine RASvbl2                                    <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
STRUCT(RASopVbl2)
{
	u16			backgroundColor;
	u16			color;
	u16			scanLinesTo1stInterupt;
	RASinterupt	nextRasterRoutine;
};

/*! ----------------------------------------- @struct RASopTop1
* OpList for RAStop1                                                <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
STRUCT(RASopTop1)
{
	u16			backgroundColor;
	u16			color;
	u16			scanLinesToNextInterupt;
	RASinterupt	nextRasterRoutine;
};

/*! ----------------------------------------- @struct RASopMid1
* OpList for RASmid1                                                <br>
* [.w]	color change                                                <br>
* if color value < 0 (means last call)                              <br>
* > [.w] scan lines count to next inter call (timerb data register) <br>
* >	[.l] next raster routine                                        <br>
* -----------------------------------------*/
#define RASstopMask	0x8000

STRUCT(RASopMid1)
{
	u16			color;
	u16			scanLineToNextInterupt;
	RASinterupt	nextRasterRoutine;
};

/*! ----------------------------------------- @struct RASopBot1
* OpList for RASbot1                                                <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>    
* -----------------------------------------*/
STRUCT(RASopBot1)
{
	u16		backgroundColor;
	u16		color;
};

#endif
