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

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\BLSSND.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#ifndef __TOS__
#include <conio.h>
#endif

struct PlayerInterface_
{
    void (*init)        (MEMallocator* _allocator, MEMallocator* _allocatorTemp, BLSsoundTrack* _sndtrack, BLSinitCallback _statCallback);
    void* (*read)        (MEMallocator* _allocator, MEMallocator* _allocatorTemp, void* _buffer, BLSsoundTrack** _sndtrack);
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

Player g_player;

extern u16 themuzik;


static void SetParam (int argc, char** argv)
{
    ASSERT(argc > 1);

    strcpy(g_player.filename , argv[1]);
}

static void DEMOSinitHW(void)
{
    static u16 vbl = 0x4E73;

    sys.OSneedRestore = true;

    sys.bakvideoMode = *HW_VIDEO_MODE;
    sys.bakvbl = *HW_VECTOR_VBL;
    sys.bakdma = *HW_VECTOR_DMA;

    sys.bakvideoAdr[0] = *HW_VIDEO_BASE_H;
    sys.bakvideoAdr[1] = *HW_VIDEO_BASE_M;
    sys.bakvideoAdr[2] = *HW_VIDEO_BASE_L;

    sys.bakmfpInterruptEnableA = *HW_MFP_INTERRUPT_ENABLE_A;
    sys.bakmfpInterruptMaskA = *HW_MFP_INTERRUPT_MASK_A;
    sys.bakmfpInterruptEnableB = *HW_MFP_INTERRUPT_ENABLE_B;
    sys.bakmfpInterruptMaskB = *HW_MFP_INTERRUPT_MASK_B;

    STDcpuSetSR(0x2700);

    *HW_MFP_INTERRUPT_ENABLE_A = 0;
    *HW_MFP_INTERRUPT_ENABLE_B = 0;

    *HW_MICROWIRE_MASK = 0x7FF;
    *HW_VECTOR_VBL = (u32)&vbl;

    sys.bakmfpVectorBase = *HW_MFP_VECTOR_BASE;

    sys.lastKey = sys.key = 0;
}

static void PlayerEntry(void)
{
    BLSsoundTrack* sndtrack;
    RINGallocatorFreeArea info;

    STDstop2300();
    *HW_VIDEO_SYNC = HW_VIDEO_SYNC_50HZ;

    DEFAULT_CONSTRUCT(&g_player.player);

    RINGallocatorFreeSize(&sys.mem, &info);
    g_player.allocatedbytes = info.size;

    {
#ifdef __TOS__
        void* buffer = &themuzik;

        ASSERT(BLS_FORMAT_BLITZ == themuzik);
#else
        void* buffer;

        buffer = STDloadfile(&sys.allocatorMem, g_player.filename, NULL);
#endif
        g_player.playerinterface.read = BLZread;
        g_player.playerinterface.init = BLSinit;
        g_player.playerinterface.playerInit = BLZplayerInit;
        g_player.playerinterface.update = BLZupdate;
        g_player.playerinterface.updAsync = BLZupdAsync;
        g_player.playerinterface.playerFree = BLZplayerFree;
        g_player.playerinterface.free = BLZfree;
        g_player.playerinterface.gotoindex = BLZgoto;
        g_player.playerinterface.testPlay = BLZtestPlay;

        g_player.playerinterface.read(&sys.allocatorMem, &sys.allocatorMem, buffer, &sndtrack);

#ifndef __TOS__
        MEM_FREE(&sys.allocatorMem, buffer);
#endif
    }

    g_player.playerinterface.init(&sys.allocatorMem, &sys.allocatorMem, sndtrack, (BLSinitCallback)NULL);

    RINGallocatorFreeSize(&sys.mem, &info);
    g_player.allocatedbytes -= info.size;

    g_player.playerinterface.playerInit(&sys.allocatorMem, &g_player.player, sndtrack, BLZ_DMAMODE_LOOP);
}

static void PlayerExit(void)
{
    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    g_player.playerinterface.free(&sys.allocatorMem, g_player.player.sndtrack);
    g_player.playerinterface.playerFree(&sys.allocatorMem, &g_player.player);

    ASSERT(RINGallocatorIsEmpty(&sys.mem));
}


static void DEMOSplayLoop(void)
{
    do
    {
        STDstop2300();

        #ifndef __TOS__
        EMULnewFrame();
        if (_kbhit())
            break;
        #endif
    
        /* no need to vsync here as main thread context is reset by idle thread switch */
        SYSkbAcquire;

        g_player.playerinterface.update(&(g_player.player));
   
        if (SYSkbHit)
        {
            SYSkbReset();
        }
    
        EMULrender();
    }
    while (g_player.player.tracklooped == false);   
} 


#define demOS_COREHEAPSIZE (64UL  * 1024UL)

int main(int argc, char** argv)
{
    STDmset (&g_player, 0, sizeof(g_player));

#ifndef __TOS__
    SetParam(argc, argv);
#endif

    sys.bakGemdos32 = SYSgemdosSetMode(NULL);

    {
        u32   size;
        u32   screenadr;
        u16   colors[16];
        u8    mode;


#       ifdef __TOS__
        {
            size_t maxsize = (u32) SYSmalloc(-1UL) - 12000UL;
            size = maxsize - demOS_COREHEAPSIZE;
        }
#       else
        size = 15 * 1024 * 1024;
#       endif

#       if blsLOGDMA
#           define demOS_LOGSIZE (256UL * 1024UL)
#           ifdef __TOS__
            tracLogger.logbase = (u8*) 0x3A0000UL;
            ASSERT(tracLogger.logbase >= (sys.coreHeapbase + demOS_COREHEAPSIZE + demOS_HEAPSIZE));
#           else
            tracLogger.logbase = (u8*) malloc(demOS_LOGSIZE);
#           endif

            tracLogger.logSize = demOS_LOGSIZE;
#       else
            tracLogger.logbase = 0;
            tracLogger.logSize = 0;
#       endif

        TRACinit ("_logs\\traclogpc.log");

        sys.membase = (u8*) malloc( EMULbufferSize(demOS_COREHEAPSIZE + size) );
        ASSERT(sys.membase != NULL);
        EMULinit (sys.membase, 660, 220, 0, g_player.filename);

        sys.coreHeapbase = EMULalignBuffer(sys.membase);
        sys.coreHeapsize = demOS_COREHEAPSIZE;
        sys.mainHeapbase = sys.coreHeapbase + demOS_COREHEAPSIZE;
        sys.mainHeapsize = size;

        SYSinit ();

        screenadr = SYSreadVideoBase();
        mode      = *HW_VIDEO_MODE;
        STDmcpy2 (colors, HW_COLOR_LUT, 32);

        PlayerEntry();

        {
            DEMOSinitHW ();

            HW_DISABLE_MOUSE(); /* deactivate mouse management on ACIA */

            DEMOSplayLoop();

            *HW_KEYBOARD_DATA = 0x8; /* activate mouse management on ACIA */
        }

        PlayerExit();

        SYSshutdown();

        *HW_VIDEO_MODE = mode;
        STDmcpy2 (HW_COLOR_LUT, colors, 32);
        SYSwriteVideoBase (screenadr);

        free (sys.membase);
    }

    SYSgemdosSetMode(sys.bakGemdos32);

    return 0;
}
