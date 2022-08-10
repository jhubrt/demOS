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

#ifndef LAYERZ_H
#define LAYERZ_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\DATA\SYSTFNT.H"

#include "FX\COLPLANE\COLPLANE.H"
#include "FX\SMPLCURV\SMPLCURV.H"

#include "FX\VECTOR\VECTOR.H"

void LayerZEnter	 (FSM* _fsm);
void LayerZBacktask  (FSM* _fsm);
void LayerZExit	     (FSM* _fsm);
                      
void LayerZActivity  (FSM* _fsm);

void LayerZStaticInit (FSM* _fsm);


struct LayerZBmp_
{
    u8*  blitzbmp;
    u16  blitzbmpoffset;
};
typedef struct LayerZBmp_ LayerZBmp;


struct LayerZStatic_
{
    LayerZBmp bmps[RSC_BLITZWAV_METADATA_PICS_CYBER6_ARJX - RSC_BLITZWAV_METADATA_PICS_BLITZ_ARJX + 1];    
    u16* blitzsprite;
    u8* ymlinear;
};
typedef struct LayerZStatic_ LayerZStatic;


struct LayerZDisplay_
{
    u8      inc1;
    u8      inc2;
};
typedef struct LayerZDisplay_ LayerZDisplay;

/*! ----------------------------------------- @struct LAYZmid15
* OpList for LAYZmid15                                            <br>
* [.l] pointer on 15 colors                                       <br>
* [.w] scan lines count to next inter call (timerb data register) <br>
* [.l] next raster routine                                        <br>
* -----------------------------------------*/
struct LAYZopMid15_
{
    u16*		colors;
    u16			scanLineToNextInterupt;
    RASinterupt	nextRasterRoutine;
};
typedef struct LAYZopMid15_ LAYZopMid15;

struct LayerZRaster1_
{
    RASopVbl  vbl;
    RASopLowB lowb;
};
typedef struct LayerZRaster1_ LayerZRaster1;

struct LayerZRaster2_
{
    RASopVbl    vbl;
    LAYZopMid15 mid15[5];    /* 0: 37 (34+3) 1: 71 2: 105 3: 139 4: 173 */
    RASopLowB   lowb;
    LAYZopMid15 mid15_last;  /* 207 */
};
typedef struct LayerZRaster2_ LayerZRaster2;


enum LayerZop_
{
    LayerZop_OR,
    LayerZop_XOR,
    LayerZop_TOP,
    LayerZop_TRANPARENT,
    LayerZop_COUNT
};
typedef enum LayerZop_ LayerZop;

typedef void(*layerZActivitiesCall)(struct LayerZ_* this, u8* image_, s16 index_);


enum LayerZactivities_
{
    LayerZactivities_BMP_EXCLUSIVE_P3,
    LayerZactivities_BMP_EXCLUSIVE_P4,
    LayerZactivities_BMP_CONCURRENT_P3,
    LayerZactivities_BMP_CONCURRENT_P4,
    LayerZactivities_CURVE,
    LayerZactivities_SCORE,
    LayerZactivities_SCORECOLORS,
    LayerZactivities_COUNT
};
typedef enum LayerZactivities_ LayerZactivities;


struct LayerZ_
{
    u8* 	framebuffer[2];
    s16     offsets[256];
    s16     offsets2[256];
    u16     scorecolor[17];

    u16     layercolors[4];
    u8      layersop[4];
    u16     palette[2][8][16];
    u32     offsety;
    char    str[42];

    u8*     spritecode[16];

    layerZActivitiesCall activities[LayerZactivities_COUNT];
    u8                   bmpnum;
    u8                   decompresscount[2];
    u8                   colorscount[2];
    u8                   clearcount[2];
    u8                   bmpplane;
    u8                   scorematrixcolorscount;
    u8                   scorematrixcolorsindex;

    u16     backgroundcolor;
    u16     backgroundinc1;
    u16     backgroundinc2;

    LayerZRaster1       rasters1;
    LayerZRaster2       rasters2[2];

    SmplCurveYMcurveState ymstates[3];

    LayerZDisplay       samrequest;
    LayerZDisplay       samdisplay;

    u8      flip;
    u8      voice;
    u8      voice2;
    bool    inverttop;
    u8      xorfreeareah;

    u16     colorindex;
    s16     colorinc;
    u16     curvecolorcycling[5*16+1];
    u16     keyscolors[8];
    
    u16     multicolors[2][8];
    u8      multinbcolors;

    u8*     lastplayerframe;

    SYSFont font;
};
typedef struct LayerZ_ LayerZ;

#endif
