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

#ifndef SPACEWAV_H
#define SPACEWAV_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#include "FX\VECTOR\VECTOR.H"
#include "FX\STARFILD\STARFILD.H"

void SpaceWavEnter	  (FSM* _fsm);
void SpaceWavBacktask (FSM* _fsm);
void SpaceWavExit	  (FSM* _fsm);
                      
void SpaceWavActivity (FSM* _fsm);

#define SPACEWAV_POLY_DRAWLIST_SIZE 32
#define SPACEWAV_BUFFER_H    200


struct SpaceWavRaster_
{
    RASopVbl15       vbl;
    RASopMid15       rasbegin;
    RASopMid15Colors ras[30];
    RASopMid15       rasend;
};
typedef struct SpaceWavRaster_ SpaceWavRaster;

struct SpaceWavState_
{
    u16     front  ;
    u16     back   ;
    s16     lastkey;
    s16     ywav   ;
    s16     base   ;
    s16     levels ;
    u16     mask   ;
};
typedef struct SpaceWavState_ SpaceWavState;


struct SpaceWav_
{
    u8* 	framebuffer[2];

    SpaceWavRaster rasters;

    u8      flip;

    STARparam stars;
    u8      nbstarsdiv6minusone;

    u16     vlist[2][8][SPACEWAV_POLY_DRAWLIST_SIZE];
    bool    verase[2][8];
    u8*     keyh;
    u8*     keysnoteinfo;

    u16     backgroundcolor;
    u16     backgroundinc1;
    u16     backgroundinc2;

    SpaceWavState states[7];

    s16     pitchmul[SPACEWAV_BUFFER_H];
    u8      ymheight[4096];    
};

typedef struct SpaceWav_ SpaceWav;

#endif
