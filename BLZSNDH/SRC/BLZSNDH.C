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

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYNTHYM.H"
#include "DEMOSDK\YMDISP.H"

#include "DEMOSDK\BLSSND.H"

#include "BLZSNDH\SRC\SCREENS.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#ifdef __TOS__
#   define bplayerUSEASM 1
#endif

void PlayerEntry (void)
{
    BLSsoundTrack* sndtrack;
    RINGallocatorFreeArea info;

    
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    DEFAULT_CONSTRUCT(&g_player.player);
    
    g_player.play = true;
    
    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes = info.size;

    {
        void* buffer;

        buffer = STDloadfile (&sys.allocatorMem, g_player.filename, NULL);

        g_player.playerinterface.read       = BLZread;
        g_player.playerinterface.init       = BLSinit;
        g_player.playerinterface.playerInit = BLZplayerInit;
        g_player.playerinterface.update     = BLZupdate;
        g_player.playerinterface.updAsync   = BLZupdAsync;
        g_player.playerinterface.playerFree = BLZplayerFree;
        g_player.playerinterface.free       = BLZfree;
        g_player.playerinterface.gotoindex  = BLZgoto;
        g_player.playerinterface.testPlay   = BLZtestPlay;

        g_player.playerinterface.read(&sys.allocatorMem, &sys.allocatorMem, buffer, &sndtrack);

        MEM_FREE(&sys.allocatorMem, buffer);
    }

    g_player.playerinterface.init (&sys.allocatorMem, &sys.allocatorMem, sndtrack, (BLSinitCallback)NULL);

    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes -= info.size;
    
    g_player.playerinterface.playerInit (&sys.allocatorMem, &g_player.player, sndtrack, BLZ_DMAMODE_LOOP);
}

void PlayerActivity	(FSM* _fsm)
{
    g_player.playerinterface.update (&(g_player.player));

    IGNORE_PARAM(_fsm);
}

void PlayerExit	(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    g_player.playerinterface.free(&sys.allocatorMem, g_player.player.sndtrack);
    g_player.playerinterface.playerFree(&sys.allocatorMem, &g_player.player);
   
    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
    
    g_player.play = false;

    FSMgotoNextState (&g_stateMachineIdle);
}
