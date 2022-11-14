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


/*-----------------------------------------------------------------------------------------------
    Main program : init / shutdown / main loop

    Here is the whole source code of 'BLITZWAV' demo.
    It is implemented as:                                                           
    - a low level reusable layer available in demOS folder                          
    - specialized high level implementations                                        
    
    So feel free to re-use, improve... demOS                                        

    BLITZWAV is implemented as a mix of C and ASM68k code. 
    It can be build with pure C without linking any standard library 
    (have a look at configuration is BASETYPES.H)

    3 project .PRJ files are provided:                                              
    - debug target with .C metafile (for optimized build times) BLITZWAB.PRJ
    - final target with .C metafile (for optimized build times) BLITZWAO.PRJ
    - final Hard Drive target with .C metafile (for optimized build times) BLITZWAH.PRJ
    - 'normal' target BLITZWAV.PRJ (useful to identify link errors)                           
-------------------------------------------------------------------------------------------------*/

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "BLITZIK\SRC\SCREENS.H"

#include "BLITZIK\BLITZWAV.H"

#ifdef DEMOS_UNITTEST
#include "DEMOSDK\UNITTEST.H"
#endif

static void DEMOSidleThread(void)
{
#	ifdef __TOS__
    STDcpuSetSR(0x2300);
	while (true)
#	endif
	{
		FSMupdate (&g_stateMachineIdle);
        SYSvsync;   /* be sure to give hand to main thread state before running new state */
	}
}

/* Linker options main thread stack size set to 1500 bytes (global settings) */
#if defined(DEMOS_OPTIMIZED)
#   define demOS_COREHEAPSIZE      (440UL * 1024UL)
#   define demOS_HEAPSIZE          (460UL * 1024UL)
#else
#   define demOS_COREHEAPSIZE      (700UL  * 1024UL)
#   define demOS_HEAPSIZE          (1000UL * 1024UL)
#endif


void BlitZmainLoop (void)
{
    do
    {
        SYSswitchIdle();
        /* no need to vsync here as main thread context is reset by idle thread switch */

#       ifdef DEMOS_DEBUG
        SYSbeginFrameNum = SYSvblLcount;
#       endif

        /* interpret soundtrack client event */
        if (g_screens.persistent.menu.playmode != BLZ_PLAY_INTERACTIVE)
        {
            if (g_screens.runningphase == BLZ_PHASE_FX)
            {
                if (g_screens.fxsequence.seq != NULL)
                {
                    u16 current;

                    if (g_screens.player.tracklooped)
                    {
                        TRAClog (TRAC_LOG_COMMANDS, "restartsequence", '\n');

                        g_screens.player.tracklooped = false;
                        g_screens.fxsequence.current = g_screens.fxsequence.seq;

                        if (g_screens.persistent.menu.playmode == BLZ_PLAY_AUTORUN)
                        {
                            BLZ_PUSH_COMMAND(BLZ_CMD_CODE_BACK);
                        }
                    }

                    current = PCENDIANSWAP16(*(u16*)g_screens.fxsequence.current);

                    if (g_screens.player.framenum >= current)
                    {
                        u8 nb, t;

                        g_screens.fxsequence.current += 2;
                        nb = *g_screens.fxsequence.current++;
                        TRAClogNumberS(TRAC_LOG_COMMANDS, "playframe", current, 4, '\n');

                        for (t = 0 ; t < nb ; t++)
                        {                    
                            u8 command = *g_screens.fxsequence.current++;
                            BLZ_PUSH_COMMAND(command);
                        }
                        
                        if ((nb & 1) == 0)
                            g_screens.fxsequence.current++;
                    }
                }
            }
        }

#       ifndef __TOS__
        if (EMULupdateRequested())
#       endif
        {             
            FSMupdate (&g_stateMachine);
        }

        SYSkbAcquire;
        if ( SYSkbHit )
        {
            BlitZmanageKey();
            sys.lastKey = sys.key;
            SYSemptyKb();
        }

        /*{
        static char trace[100] = "vbl:    ";
        STDuxtoa(&trace[5], SYSvblLcount, 3);
        TRAClogPush(trace);
        }*/

#       ifndef __TOS__
        EMULrender();
#       endif		     	
    }
    while (1);
}


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP() + 4;

    IGNORE_PARAM(argv);

    STDmset (&g_screens, 0, sizeof(g_screens));
    
    argc--;
    g_screens.compomode       = (argc & 1) != 0;
    g_screens.dmaplayoncemode = (argc & 2) != 0;

    if (g_screens.compomode)
    {
#       if defined(__TOS__) && defined(DEMOS_USES_BOOTSECTOR)
        SYSfastPrint ("Compo mode activated", (u8*)SYSreadVideoBase(), 160, 8, (u32)&SYSfont);
#       else
        printf ("Compo mode activated\n\n");
#       endif
    }
    else
    {
        static char debug [11] = "          ";
        STDuxtoa(debug, (u32)base, 8);
        if (g_screens.dmaplayoncemode)
            debug[9] = '<';

#       if defined(__TOS__) && defined(DEMOS_USES_BOOTSECTOR)
        SYSfastPrint (debug, (u8*)SYSreadVideoBase() + 160*190 + 120, 160, 8, (u32)&SYSfont);
#       else
        printf ("%s\n\n", debug);
#       endif
    }


#   ifdef DEMOS_LOAD_FROMHD
    SYSinitStdAllocator();
    LOADinitForHD(&RSC_BLITZWAV, RSC_BLITZWAV_NBENTRIES, RSC_BLITZWAV_NBMETADATA);
#   endif

    /*STD_unitTest();*/

	{
#       ifdef DEMOS_USES_BOOTSECTOR
        sys.membase = base + 8;
#       else
        sys.membase = (u8*) malloc( EMULbufferSize(demOS_COREHEAPSIZE + demOS_HEAPSIZE) );
        if (sys.membase == NULL)
        {
            printf("Not enought RAM\n");
            while(1);
        }
#       endif

        sys.coreHeapbase = EMULalignBuffer(sys.membase);
        sys.coreHeapsize = demOS_COREHEAPSIZE;
        sys.mainHeapbase = sys.coreHeapbase + demOS_COREHEAPSIZE;
        sys.mainHeapsize = demOS_HEAPSIZE;

#       ifdef DEMOS_DEBUG
#           define demOS_LOGSIZE (256UL * 1024UL)
#           ifdef __TOS__
            tracLogger.logbase = (u8*) 0x3A0000UL;
            ASSERT(tracLogger.logbase >= (sys.coreHeapbase + demOS_COREHEAPSIZE + demOS_HEAPSIZE));
#           else
            tracLogger.logbase = (u8*) malloc(demOS_LOGSIZE);
            ASSERT(tracLogger.logbase != NULL);
#           endif

            tracLogger.logSize = demOS_LOGSIZE;
#       endif

#       ifndef DEMOS_USES_BOOTSECTOR
		sys.bakGemdos32 = SYSgemdosSetMode(NULL);
#       endif

        TRACinit ("_logs\\traclogpc.log");

        ASSERT(sys.membase != NULL);

        SYScheckHWRequirements ();
        HW_DISABLE_MOUSE();

        IGNORE_PARAM(base);

		/* STDmset (buffer, 0, size); */

		EMULinit (sys.coreHeapbase, 1024, 313*2, 0, "Blitzwav");
        EMULsetVStretch (true);

		FSMinit (&g_stateMachine	, states    , statesSize    , 0, DEBUG_STRING("main"));
		FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0, DEBUG_STRING("idle"));

		/* RingAllocator_unitTest(); */

		SYSinit ();
        SYSinitHW ();

        {
            SYSinitThreadParam threadparam;

            threadparam.idleThread = DEMOSidleThread;
            threadparam.idleThreadStackSize = 1024;
            SYSinitThreading ( &threadparam );
        }
        
        /*PTRKinit ();*/
#       ifndef DEMOS_LOAD_FROMHD
        LOADinit (&RSC_BLITZWAV, RSC_BLITZWAV_NBENTRIES, RSC_BLITZWAV_NBMETADATA);

        /*#       if BLZ_DEVMODE()==0*/
        if (*OS_PHYTOP >= 0x200000UL)
        {
#           ifdef DEMOS_USES_BOOTSECTOR
            if (g_screens.compomode == false)
                g_screens.preload = sys.membase + demOS_COREHEAPSIZE + demOS_HEAPSIZE;
#           else
            g_screens.preload = (u8*)malloc(EMULbufferSize(RSC_BLITZWAV.mediapreloadsize));
#           endif 
        }
        /*#       endif  BLZ_DEVMODE() */
#       endif /* DEMOS_LOAD_FROMHD */


#       ifdef DEMOS_UNITTEST
/*      TESTrunUnitTests();*/
#       endif
   
        ScreensInit();
		           
        BlitZmainLoop();
	}

	return 0;
}
