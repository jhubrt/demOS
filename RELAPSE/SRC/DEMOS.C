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

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\TRACE.H"
#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "RELAPSE\SRC\SCREENS.H"

#include "RELAPSE\RELAPSE1.H"
#include "RELAPSE\RELAPSE2.H"


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


static void RelapseMainLoop (void)
{
    do
    {
        SYSswitchIdle();

        /* no need to vsync here as main thread context is reset by idle thread switch */                   
#       ifndef __TOS__
        if (EMULupdateRequested())
#       endif
        {             
            FSMupdate (&g_stateMachine);
        }             

        g_screens.justpressed = false;
        SYSkbAcquire;
        if ( SYSkbHit )
        {
            bool pressed = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;

            if (pressed)
            {
                u8 scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);

                g_screens.justpressed = g_screens.scancodepressed != scancode;
                g_screens.scancodepressed = scancode;

                switch (scancode)
                {
                case HW_KEY_ESC:
                    g_screens.next = true;
                    g_screens.gotomenu = true;
                    break;

                case HW_KEY_SPACEBAR:
                    g_screens.next = true;
                    break;

                case HW_KEY_NUMPAD_PLUS:
                    if (g_screens.bass < 12)
                    {
                        g_screens.bass++;
                        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS | g_screens.bass;
                    }
                    break;

                case HW_KEY_NUMPAD_MINUS:
                    if (g_screens.bass > 0)
                    {
                        g_screens.bass--;
                        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_BASS | g_screens.bass;
                    }
                    break;

                case HW_KEY_NUMPAD_DIVIDE:
                    if (g_screens.treble > 0)
                    {
                        g_screens.treble--;
                        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;
                    }
                    break;

                case HW_KEY_NUMPAD_MULTIPLY:
                    if (g_screens.treble < 12)
                    {
                        g_screens.treble++;
                        *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_TREBLE | g_screens.treble;
                    }
                    break;
                }
            }
            else
            {
                g_screens.scancodepressed = 0;
            }

            sys.lastKey = sys.key;
            SYSemptyKb();
        }

#       ifndef __TOS__
        EMULrender();
#       endif
    }
    while(1);
}


#define DEMOS_VERSION "2" /* should be one char */


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP();


	/*STD_unitTest();*/

    IGNORE_PARAM(argc);
    IGNORE_PARAM(argv);

#   ifdef DEMOS_LOAD_FROMHD
    SYSinitStdAllocator();
    LOADinitForHD(&RSC_RELAPSE1, RSC_RELAPSE1_NBENTRIES, RSC_RELAPSE1_NBMETADATA, RSC_RELAPSE1_SPLASH_SPLASH);
    LOADinitForHD(&RSC_RELAPSE2, RSC_RELAPSE2_NBENTRIES, RSC_RELAPSE2_NBMETADATA, 1);
#   endif

#   ifndef DEMOS_USES_BOOTSECTOR
    sys.bakGemdos32 = SYSgemdosSetMode(NULL);
#   endif

#   ifdef __TOS__
    {
        static char info[9] = "      " " " DEMOS_VERSION;
        STDuxtoa(info, (u32)base, 6);

        HW_COLOR_LUT[1] = 0xEEE;
        SYSfastPrint (info, (u8*)SYSreadVideoBase() + 160*190 + 120, 160, 8, (u32)&SYSfont);
    }
#   endif

    {
        u32 coreHeapSize = demOS_COREHEAPSIZE;
        u32 phystop = *(OS_PHYTOP);

#       if defined(DEMOS_DEBUG) && defined(__TOS__)
        if (phystop > 0x3A0000UL)
            phystop = 0x3A0000UL;
#       endif

#       ifdef DEMOS_LOAD_FROMHD
        sys.membase = (u8*) malloc( EMULbufferSize(demOS_COREHEAPSIZE + demOS_HEAPSIZE) );
        if (sys.membase == NULL)
        {
            printf("Not enought memory");
            while(1);
        }
#       else
        sys.membase    = base + 64;
        coreHeapSize = (phystop - (u32)sys.membase) - demOS_HEAPSIZE;
#       endif

        ASSERT(coreHeapSize >= demOS_COREHEAPSIZE);

        sys.coreHeapbase = EMULalignBuffer(sys.membase);
        sys.coreHeapsize = coreHeapSize;
        sys.mainHeapbase = sys.coreHeapbase + coreHeapSize;
        sys.mainHeapsize = demOS_HEAPSIZE;

#       ifdef DEMOS_DEBUG
#           define demOS_LOGSIZE (256UL * 1024UL)
#           ifdef __TOS__
            tracLogger.logbase = (u8*) 0x3A0000UL;
            ASSERT(tracLogger.logbase >= (sys.coreHeapbase + demOS_COREHEAPSIZE + demOS_HEAPSIZE));
#           else
            tracLogger.logbase = (u8*) malloc(demOS_LOGSIZE);
#           endif

            tracLogger.logSize = demOS_LOGSIZE;
#       endif

        ASSERT(sys.membase != NULL);

        TRACinit ("_logs\\traclogpc.log");

        TRAClogNumberS  (TRAC_LOG_ALL, "phytop:"      , *(OS_PHYTOP)    , 6, '\n');
        TRAClogNumberS  (TRAC_LOG_ALL, "membase:"     , (u32)sys.membase, 6, '\n');
        TRAClogNumber10S(TRAC_LOG_ALL, "coreHeapSize:", sys.coreHeapsize, 6, '\n');
        TRAClogNumber10S(TRAC_LOG_ALL, "mainHeapSize:", sys.mainHeapsize, 6, '\n');

        HW_DISABLE_MOUSE();
        SYScheckHWRequirements ();

        IGNORE_PARAM(base);

		EMULinit (sys.membase, 1024, 313*2, 0, "Relapse");
        EMULsetVStretch (true);

		FSMinit (&g_stateMachine	, states    , statesSize    , 0, DEBUG_STRING("main"));
		FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0, DEBUG_STRING("idle"));

		/* RingAllocator_unitTest(); */

		{
            SYSinitThreadParam threadparam;

			SYSinit ();

			threadparam.idleThread = DEMOSidleThread;
            threadparam.idleThreadStackSize = 2048;

            SYSinitHW ();
            SYSinitThreading ( & threadparam );
           
#           ifndef DEMOS_LOAD_FROMHD
            LOADinit (RSC_RELAPSE1_NBENTRIES, RSC_RELAPSE1_NBMETADATA, &RSC_RELAPSE1); 
#           endif

			/* BIT_unitTest(); */
		}

	    ScreensInit ();		

        WIZinit (&g_screens.playerInfo);

        /*SYSvsync;*/

        RelapseMainLoop();
	}

	return 0;
}
