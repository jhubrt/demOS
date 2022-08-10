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

#include "WIZPLAY\SRC\SCREENS.H"
#include "WIZPLAY\SRC\WIZPLAY.H"

#define PLAYERNAME "WIZPLAY"

static void SetParam (int argc, char** argv)
{
    if (argc == 1)
    {
        char   line [256];
        char*  myargv[10];
        int    myargc = 1;

        {
            char*  result;
            FILE* file = fopen(".\\" PLAYERNAME ".CFG", "r");

            ASSERT(file != NULL);
            result = fgets(line, (int)sizeof(line), file);
            ASSERT(result != NULL);
            fclose(file);
        }

        {
            char* p = strtok(line, "\t \r\n");
            while (p != NULL)
            {
                myargv[myargc++] = p;
                p = strtok(NULL, "\t \r\n");
                if (myargc >= ARRAYSIZE(myargv))
                    break;
            }
        }

        ASSERT(myargc > 0);

        SetParam (myargc, myargv);
    }
    else
    {
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

#define demOS_COREHEAPSIZE (64UL  * 1024UL)


int main(int argc, char** argv)
{
    SetParam(argc, argv);

    printf("Build on " __DATE__ " " __TIME__ "\n");

    sys.bakGemdos32 = SYSgemdosSetMode(NULL);

    {
        u32   size;
        u32   screenadr;
        u16   colors[16];
        u8    mode;


#       ifdef __TOS__
        {
            size_t maxsize = (u32) SYSmalloc(-1UL) - 12000UL;

            printf ("free mem= %lu bytes\n", maxsize);
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
            SYSinitThreadParam threadParam;

            FSMinit (&g_stateMachine    , statesPlay, statesPlaySize, 0, DEBUG_STRING("main"));
            FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0, DEBUG_STRING("idle"));
 
            threadParam.idleThread = DEMOSidleThread;
            threadParam.idleThreadStackSize = 1024;

            SYSinitHW ();
            SYSinitThreading (&threadParam);

            SYScheckHWRequirements ();

            HW_DISABLE_MOUSE(); /* deactivate mouse management on ACIA */

            {
                do
                {
                    SYSswitchIdle();

                    /* no need to vsync here as main thread context is reset by idle thread switch */
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

            *HW_KEYBOARD_DATA = 0x8; /* activate mouse management on ACIA */
        }

        SYSshutdown();

        *HW_VIDEO_MODE = mode;
        STDmcpy2 (HW_COLOR_LUT, colors, 32);
        SYSwriteVideoBase (screenadr);

        free (sys.membase);
    }

    SYSgemdosSetMode(sys.bakGemdos32);

    return 0;
}
