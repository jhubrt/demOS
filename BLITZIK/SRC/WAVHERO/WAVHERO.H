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

#ifndef WAVHERO_H
#define WAVHERO_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#include "FX\VECTOR\VECTOR.H"
#include "FX\STARFILD\STARFILD.H"

void WavHeroInitStatic (FSM* _fsm);
void WavHeroEnter	   (FSM* _fsm);
void WavHeroBacktask   (FSM* _fsm);
void WavHeroExit	   (FSM* _fsm);
                      
void WavHeroActivity   (FSM* _fsm);

#define WAVHERO_POLY_DRAWLIST_SIZE 32
#define WAVHERO_HEIGHT      248



enum WavHeroASMopcodes_
{
    WHrOp_Begin,
    WHrOp_StarErase,
    WHrOp_StarDrawBegin,
    WHrOp_StarDraw,
    WHrOp_StarDrawEnd,
    WHrOp_Raster,
    WHrOp_LowBorder,
    WHrOp_Volume,
    WHrOp_VolumeEnd,
    WHrOp_End,
    WHrOp_SIZE
};
typedef enum WavHeroASMopcodes_ WavHeroASMopcodes;


struct WavHeroASMimport_
{
    void*   colorstable;
    void*   samplebuffer;
    void*   currentstarerase;
    
    void*   rndx;
    void*   rndy;
    void*   starz;
    void*   framebuffer;
    u16     starinc;
    void*   currentrndx;
    void*   currentrndy;

    u16     volumes[4];

    CGENdesc opcodes[WHrOp_SIZE];
};
typedef struct WavHeroASMimport_ WavHeroASMimport;


struct WavHeroRAS_
{
    u16 starscolor;
    s16 inctable;
};
typedef struct WavHeroRAS_ WavHeroRAS;

struct WavHeroRaster_
{
    RASopVbl        vbl;
    WavHeroRAS      rasters[120];
};

typedef struct WavHeroRaster_ WavHeroRaster;


struct WavHeroStatic_
{
    u8  ymheight[4096];
    u8* starfielddiv;
};
typedef struct WavHeroStatic_ WavHeroStatic;


struct WavHero_
{
    u8* 	framebuffer[2];

    WavHeroRaster rasters[2];

    u8*     code;

    STARparam stars;
    u8      nbstarsdiv6minusone;
    u8      flip;
    u8      colormode;
    bool    feed; 

    u8*     keyh;
    u8*     keysnoteinfo;

    u16     rasheight[64];
    u16     rasindex[64];
    u16     rasstarcolors[64];
    u16     rasnb;
    u16     rasindexmax;
    u16     colorzero;
    u16     wavhero_noiseinc;

    u16     rascolors[256][8];
    u16     rascurrent;

    u16     keyscolors[97];

    s16     pitchmul[WAVHERO_HEIGHT];
    u16     dlist[256];
};

typedef struct WavHero_ WavHero;

#endif
