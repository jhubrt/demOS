/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2024 J.Hubert

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

#ifndef LOADER_H
#define LOADER_H

#include "DEMOSDK\FSM.H"

void LoaderEntry	(FSM* _fsm);
void LoaderActivity	(FSM* _fsm);
void LoaderExit		(FSM* _fsm);
void LoaderCheckDisk1Inserted (FSM* _fsm);
void LoaderCheckDisk2Inserted (FSM* _fsm);

typedef void(*LoaderComputeHTone)(void* dest_, u16 inc_, u16 acc_);
typedef void(*LoaderUpdateVGrid)(void* framebuffer_, u16* htone_);
typedef void(*LoaderUpdateHGrid)(void* framebuffer_, u16 inc_, u16 y_);


struct LoaderAsmImport_
{
    LoaderComputeHTone computeHTone;
    LoaderUpdateVGrid  loaderUpdateVGrid;
    LoaderUpdateHGrid  loaderUpdateHGrid;
    s16*               sin;
};
typedef struct LoaderAsmImport_ LoaderAsmImport;


struct Loader_
{
    u8* framebuffer;

    u8  lastprogress;
    u16 lastnbsectors;
    u32 lastdpakprogress;

    u16 x,angle;
    u16 size;
    u16 incsize;
    u16 plane;
    u16 fadestep;
    s16 fadeinc;
    bool gotomenudisplayed;
    bool countdownactive;
    s8  insertDiskMessage;
    s8  insertDiskMessageDisplayed;

    u16 htone[20];
    u16 gradient[16];
    u16 colorshistory[3];    
    u16 lut[16];
    u16 black[16];
    char text[16];

    void* prxbuffer;
    LoaderAsmImport* asmimport;

#   if RELAPSE_DEV() 
    u16 nbframes;
    u16 lastsec;
#   endif

    u16 temp[LOAD_SECTORSIZE / sizeof(u16)];
};
typedef struct Loader_ Loader;

#endif
