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

#ifndef BLITZLOADER_H
#define BLITZLOADER_H

#include "DEMOSDK\FSM.H"

void BlitZPreloadEnter	            (FSM* _fsm);
void BlitZPreloadExit	            (FSM* _fsm);

void BlitZLoadModuleEnter	        (FSM* _fsm);
void BlitZLoadModule                (FSM* _fsm);
void BlitZLoaderBacktask            (FSM* _fsm);
void BlitZLoadModuleExit		    (FSM* _fsm);

void BlitZLoaderActivity	        (FSM* _fsm);

void BlitZLoadModuleIdleActivity    (FSM* _fsm);
void BlitZLoadModuleIdleBacktask    (FSM* _fsm);
void BlitZLoadModuleIdleExit        (FSM* _fsm);


typedef bool (*BlitZLoaderUpdateProgressCallback)(struct BlitZLoader_* this);


struct BlitZLoader_
{
    u8*  framebuffer;
    u8*  codebuffer;

    u16  planeflip;
    u16  colorcount;

    u16  loadmul;
    u8*  depackstart;
    u16  depackmul;

    s16  blsinitprogress;
    u16  lastprogress;
    BlitZLoaderUpdateProgressCallback updateprogresscallback;

    struct LOADrequest_* loadRequest;
    u16 preloadresourcecount;
    u16 resourcesectorstoload;

    u16 preloadtotalsectors;
    u16 diskpreloadprogress[RSC_BLITZWAV_NBENTRIES - RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC];

    u16 charFunc[256];

    char text[2][256];
    u16  offset[2];
};
typedef struct BlitZLoader_ BlitZLoader;

#endif
