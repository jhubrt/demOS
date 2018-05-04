/*-----------------------------------------------------------------------------------------------
 The MIT License (MIT)

 Copyright (c) 2015-2018 J.Hubert

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

/* ------------------------------------------------------------------------------  
  Keymap:                                                                       
  
  [ESC]            quit program                                                 
  [1-4]            select voice 1 to 4                                          
  [5-8]            mute voice 1 to 4                                            
  [SPACE]          mute selected voice                                          
  [Q-P/A-L]        apply bit decimation mask                                    
  [NUMPAD 4-9/1-2] step volume effect (only for sound with volume fx available) 
  [RETURN]         reset max raster bar                                         
------------------------------------------------------------------------------  */

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\SYNTH.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "BLSPLAY\SRC\SCREENS.H"
#include "BLSPLAY\SRC\BPLAYER.H"

#define FORCE_TESTMODE 0

/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\ARPEGGIO.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\BALANCE.BLS"*/ /* => microwire management currently de-actived */
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\CLIENT.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\DELAY.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\FINETUNE.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\JUMPS.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\LONGLOOP.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\MASK.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\PATRLOOP.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\SAMPLE.BLS"*/ /* => render differs from .MOD under milkytracker */
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\SPEED.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\UNITTEST\\VOLUME.BLS"*/

/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\LOADER.BLS"*/
/*#define bls_TEST_MODULE "BLSPLAY\\DATA\\QUICKIE.BLS"*/
#define bls_TEST_MODULE "BLSPLAY\\DATA\\_QUICKIE.BLS"


static void SetParam (int argc, char** argv)
{
    strcpy(g_player.filename, bls_TEST_MODULE);

    g_player.testMode = false;

    if (argc > 1)
    {
        int t;
        for (t = 2 ; t < argc ; t++)
        {
            if ( strcmp (argv[t], "-test") == 0 )
            {
                g_player.testMode = true;
            }
        }

        strcpy(g_player.filename , argv[1]);
    }
}

static void DEMOSidleThread(void)
{
#	ifdef __TOS__
    STDcpuSetSR(0x2300);
    while (true)
#	endif
    {
        FSMupdate (&g_stateMachineIdle);
    }
}


int main(int argc, char** argv)
{
    SetParam(argc, argv);

    sys.bakGemdos32 = SYSgemdosSetMode(NULL);

    {
        u32   coresize  = 64UL  * 1024UL;
        u32   size;
        u8*   corebuffer1;
        u8*   corebuffer;
        void* buffer;
        u32   screenadr;
        u16   colors[16];
        u8    mode;


#       ifdef __TOS__
        {
            size_t maxsize = (u32) SYSmalloc(-1UL) - 12000UL;

            printf ("free mem= %lu bytes\n", maxsize);
            size = maxsize - coresize;
        }
#       else
        size = 15 * 1024 * 1024;
#       endif

        SYSinitPrint ();

        corebuffer1 = (u8*) malloc( EMULbufferSize(coresize + size) );
        corebuffer  = (u8*) EMULalignBuffer (corebuffer1);
        ASSERT(corebuffer != NULL);
        EMULinit (corebuffer1);
        buffer      = corebuffer + coresize;

        {
            SYSinitParam sysparam;

            sysparam.adr	     = buffer;
            sysparam.size	     = size;
            sysparam.coreAdr     = corebuffer;
            sysparam.coreSize    = coresize;

            SYSinit ( &sysparam );
        }

        screenadr = SYSreadVideoBase();
        mode      = *HW_VIDEO_MODE;
        STDmcpy (colors, HW_COLOR_LUT, 32);

        PlayerEntry();

        g_player.testMode |= FORCE_TESTMODE;

        if (g_player.testMode)
        {
            PlayerTest();
        }
        else
        {
            SYSinitThreadParam threadParam;

            FSMinit (&g_stateMachine, statesPlay, statesPlaySize, 0);
            FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0);

            threadParam.idleThread = DEMOSidleThread;
            threadParam.idleThreadStackSize = 1024;

            SYSinitHW ();
            SYSinitThreading (&threadParam);

            SYScheckHWRequirements ();

            {
                do
                {
                    SYSswitchIdle();

                    /* no need to vsync here as main thread context is reset by idle thread switch */
                    SYSbeginFrameNum = SYSvblLcount;

                    SYSkbAcquire;

                    FSMupdate (&g_stateMachine);

                    if ( SYSkbHit )
                    {
                        SYSkbReset();
                    }

                    EMULrender();
                }
                while (g_player.play);

                SYSvsync;
            }
        }

        SYSshutdown();

        *HW_VIDEO_MODE = mode;
        STDmcpy (HW_COLOR_LUT, colors, 32);
        SYSwriteVideoBase (screenadr);

        free (corebuffer1);
    }

    SYSgemdosSetMode(sys.bakGemdos32);

    return 0;
}
