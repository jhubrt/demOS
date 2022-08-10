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


#ifndef BPLAYER_H
#define BPLAYER_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\BLSSND.H"

void PlayerEntry    (void);
void PlayerTest     (u8 _mode);

void PlayerActivity (FSM* _fsm);
void PlayerExit	    (FSM* _fsm);
void PlayerBacktask (FSM* _fsm);

void PlayerSetParam (int argc, char** argv);


struct PlayerInterface_
{
    void (*init)        (MEMallocator* _allocator, MEMallocator* _allocatorTemp, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback);
    void*(*read)        (MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack);
    void (*playerInit)  (MEMallocator* _allocator, BLSplayer* _player, BLSsoundTrack* _sndtrack, BLZdmaMode _dmamode);
    void (*playerFree)  (MEMallocator* _allocator, BLSplayer* _player);
    void (*free)        (MEMallocator* _allocator, BLSsoundTrack* _sndtrack);
    void (*update)      (BLSplayer* _player);
    void (*updAsync)    (BLSplayer* _player);
    void (*gotoindex)   (BLSplayer* _player, u8 _trackindex);
    void (*testPlay)    (BLSplayer* _player, char* _filesamplename, char* _filetracename, u8 _mode);

    /* Advanced functions can play score mode / blitz mode and also capture blitz when BLS_ENABLE_OPTIMIZED_DUMP defined */
    /*u32             BLZasyncPlay     (BLSplayer* _player, u8 _trackindex, void* _buffer, u32 _buffersize);
    void            BLSgoto          (BLSplayer* _player, u8 _trackIndex);*/
};
typedef struct PlayerInterface_ PlayerInterface;


struct PlayerText_
{
    u8    state;
    u8    currentpos;
    u8    currentline;
    s16   animatecount;
};
typedef struct PlayerText_ PlayerText;


typedef void (*PlayerPlayRoutine)(BLSplayer* _player);

struct Player_
{
    BLSplayer           player;
    PlayerInterface     playerinterface;

    PlayerText          text;
    
    void* framebuffer;
    s8*   pcmcopy;
    u8    currentchannel;
    u8    currentmask;
    u8    testMode;
    char  filename[256];
    u16   rastermax;

    bool  play;
    bool  startdisplay;
    bool  ymemuldisplaysync;
    u8    leftvolume;
    u8    rightvolume;

    u8    panel;
    u8    lastpanel;
    bool  dmaplayonce;

	s32	  allocatedbytes;
};
typedef struct Player_ Player;

#endif
