/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files (the "Software"), 
  to deal in the Software without restriction, including without limitation the rights to use, 
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies 
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
