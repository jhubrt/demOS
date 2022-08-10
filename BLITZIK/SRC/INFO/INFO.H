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

#ifndef INFO_H
#define INFO_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"

#include "FX\VECTOR\VECTOR.H"

void InfoEnter	   (FSM* _fsm);
void InfoBacktask  (FSM* _fsm);
void InfoExit	   (FSM* _fsm);
                      
void InfoActivity  (FSM* _fsm);

#define INFO_VOLUME_MAX 32


typedef void  (*FuncInfoDrawVolumes)    (u16* maxvolumes_, u8* image_, u32 volumesmasks_);
typedef void  (*FuncInfoComputeVolumes) (s8* dmabuffer_, u16* resultstable_);
typedef void  (*FuncInfoClearPlane)     (void* adr_, u16 nblines_, u16 value_);
typedef void  (*FuncInfoCopyPlane)      (void* src_, void* dst_, u32 dst2_, u16 nblines_);


struct InfoASMimport_
{
    FuncInfoDrawVolumes     drawvolumes;
    FuncInfoComputeVolumes  computevolumes;
    FuncInfoClearPlane      clearplane;
    FuncInfoCopyPlane       copyplane;
};
typedef struct InfoASMimport_ InfoASMimport;


struct Info_
{
    VECscene                scene;
    VECanimationCallback    animationCallback;
    PZanimationState        animationState;

    RASopVbl        rasvbl;
    RASopLowB       raslowB;

    SmplCurveYMcurveState ymstates[3];

    void*   code;
    InfoASMimport* asmimport;

    u16*	displist;
    u16*    displistp;
    u16     currentFrame;
    u16*    erase[2];
    u8*     erasebase[2];
        
    u16     volumemasks[INFO_VOLUME_MAX + 1][4];
    u16     maxvols[7];

    u8*     decompbuffer;
    u8* 	framebuffer;
    u8      flip;
    u8      bmpindex;
    bool    exit;
    
    u16     targetline;
    s16     displayedline;

    u16     printcursor;
    u16     targetcursor;
    s8      direction;

    u32     offset;
    u16     rollingline;

    char*   text;
    u16     textlines;

    u32     framebuffersize;
    u16     volumes[4];         /* delay compute volume to postpone at the end of the frame */

    u16     compocount;
    bool    compocountdonotreset;
};
typedef struct Info_ Info;

#endif
