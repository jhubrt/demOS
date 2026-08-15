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
void PlayerActivity (FSM* _fsm);
void PlayerExit	    (FSM* _fsm);

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
};
typedef struct PlayerInterface_ PlayerInterface;


typedef void (*PlayerPlayRoutine)(BLSplayer* _player);

struct Player_
{
    BLSplayer           player;
    PlayerInterface     playerinterface;

    char  filename[256];

	s32	  allocatedbytes;
};
typedef struct Player_ Player;

#endif
