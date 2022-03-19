/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

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
-------------------------------------------------------------------------------------------------*/


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

void RASsetColReg   (u16 _shortregaddress);
void RASvbl 	    (void);
void RASvbl1	    (void);
void RASvbl2	    (void);
void RASvbl15	    (void);
void RASvbl16	    (void);
void RAStop1	    (void);
void RASmid1	    (void);
void RASmid15	    (void);
void RASbot1	    (void);
void RASlowB        (void);
void RASlowB15      (void);
void RASvbldonothing(void);

#ifndef RASTERS_C
extern void* RASnextOpList;
#endif


#define RASstopMask	0x8000

/* raster OpList */

typedef void(*RASinterupt)(void);

/*! -----------------------------------------
* OpList for VBL routine RASvbl1                                    <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
struct RASopVbl_
{
    u16			scanLinesTo1stInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopVbl_ RASopVbl;

/*! -----------------------------------------
* OpList for VBL routine RASvbl1                                    <br>
* [.w]	background color                                            <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
struct RASopVbl1_
{
	u16			backgroundColor;
	u16			scanLinesTo1stInterupt;
	RASinterupt	nextRasterRoutine;
};
typedef struct RASopVbl1_ RASopVbl1;

/*! ----------------------------------------- @struct RASopVbl2
* OpList for VBL routine RASvbl2                                    <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
struct RASopVbl2_
{
	u16			backgroundColor;
	u16			color;
	u16			scanLinesTo1stInterupt;
	RASinterupt	nextRasterRoutine;
};
typedef struct RASopVbl2_ RASopVbl2;

/*! ----------------------------------------- @struct RASopVbl15
* OpList for VBL routine RASvbl15                                   <br>
* [.l]  15 colors                                                   <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
struct RASopVbl15_
{
    u16			colors[15];
    u16			scanLinesTo1stInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopVbl15_ RASopVbl15;

/*---------------------------------------------------------------------- 
  OpList for VBL routine RASvbl16                                  
  [.l]  16 colors                                                  
  [.w]	scan lines count to next inter call (timerb data register) 
  [.l]	next raster routine                                        
------------------------------------------------------------------------*/
struct RASopVbl16_
{
    u16			colors[16];
    u16			scanLinesTo1stInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopVbl16_ RASopVbl16;

/*! ----------------------------------------- @struct RASopTop1
* OpList for RAStop1                                                <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>
* [.w]	scan lines count to next inter call (timerb data register)  <br>
* [.l]	next raster routine                                         <br>
* -----------------------------------------*/
struct RASopTop1_
{
	u16			backgroundColor;
	u16			color;
	u16			scanLinesToNextInterupt;
	RASinterupt	nextRasterRoutine;
};
typedef struct RASopTop1_ RASopTop1;

/*! ----------------------------------------- @struct RASopMid1
* OpList for RASmid1                                                <br>
* [.w]	color change                                                <br>
* if color value < 0 (means last call)                              <br>
* > [.w] scan lines count to next inter call (timerb data register) <br>
* >	[.l] next raster routine                                        <br>
* -----------------------------------------*/
struct RASopMid1_
{
	u16			color;
	u16			scanLineToNextInterupt;
	RASinterupt	nextRasterRoutine;
};
typedef struct RASopMid1_ RASopMid1;

/*! ----------------------------------------- @struct RASopMid15
* OpList for RASmid15                                               <br>
* [.l]	15 colors                                                   <br>
* if color value < 0 (means last call)                              <br>
* > [.w] scan lines count to next inter call (timerb data register) <br>
* >	[.l] next raster routine                                        <br>
* -----------------------------------------*/
#define RASstopMask	0x8000

struct RASopMid15_
{
    u16			colors[15];
    u16			scanLineToNextInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopMid15_ RASopMid15;

struct RASopMid15Colors_
{
    u16			colors[15];
};
typedef struct RASopMid15Colors_ RASopMid15Colors;

/*! ----------------------------------------- @struct RASopBot1
* OpList for RASbot1                                                <br>
* [.w]	background color                                            <br>
* [.w]	color change                                                <br>    
* -----------------------------------------*/
struct RASopBot1_
{
	u16		backgroundColor;
	u16		color;
};
typedef struct RASopBot1_ RASopBot1;

/*! ----------------------------------------- @struct RASopLowB
* OpList for RASlowB                                         <br>
* [.w] scan lines count to next inter call (timerb data register) <br>
* [.l] next raster routine                                        <br>
* -----------------------------------------*/
struct RASopLowB_
{
    u16			scanLineToNextInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopLowB_ RASopLowB;

/*! ---------------------------------------- @struct RASopLowB15
* OpList for RASopLowB15                                       <br>
* [.w] 15 colors
* [.w] scan lines count to next inter call (timerb data register) <br>
* [.l] next raster routine                                        <br>
* -----------------------------------------*/
struct RASopLowB15_
{
    u16         colors[15];
    u16			scanLineToNextInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct RASopLowB15_ RASopLowB15;

#endif
