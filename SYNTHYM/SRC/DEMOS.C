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
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\SYNTHYM.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "SYNTHYM\SRC\PLAYER.H"

static void DEMOSidleThread(void)
{
#	ifdef __TOS__
    STDcpuSetSR(0x2300);
	while (true)
#	endif
	{
        SynthYMBacktask();
	}
}

#ifdef DEMOS_DEBUG

static void registerTraceServices(void)
{
    TRACregisterDisplayService (SYStraceFPS,         1);   /* F1 */
	TRACregisterDisplayService (SYStraceHW,		     2);   /* F2 */
    TRACregisterDisplayService (SYStraceAllocators,  4);   /* F3 */
    TRACregisterDisplayService (LOADtrace,           8);   /* F4 */
}
#   endif



#define demOS_COREHEAPSIZE      (64UL  * 1024UL)
#define demOS_HEAPSIZE          (128UL * 1024UL)

static void* stdAlloc(void* _alloc, u32 _size)
{
    IGNORE_PARAM(_alloc);
    return malloc(_size);
}

static void stdFree(void* _alloc, void* _adr)
{
    IGNORE_PARAM(_alloc);
    free(_adr);
}

void SYScheckHWRequirements (void) {}

#define SYNTHYM_DEFAULT_FILE "SYNTHYM.INI"


int main(int argc, char** argv)
{ 
    printf("Build on " __DATE__ " " __TIME__ "\n");

#   ifndef __TOS__

    printf ("Usage:\n\n"
        "TAB then octave A to $ (13 keys on line 0 & 1) : play note\n"
        "F1 -> F10    : change octave transpose\n"
        "LEFT/RIGHT   : +/- semi tone transpose\n"
        "UP/DOWN      : select channel\n"
        "SPACE        : activate / deactivate channel\n"
        "4/6 (numpad) : fine tune\n"
        "+/- (numpad) : select sound\n"
        "1/3 (numpad) : inc / dec score volume (0xF by default => the YM sound volume is applied as is, else it lowers the values defined into the sound)\n"
        "7/9 (numpad) : inc / dec portamiento (operate when != 0, value is number of vbls to slide)\n"
        "RETURN       : hot reloads the sound script\n"
        "BACKSPACE    : toggle YM PC emulator curve sync display (turn off if your PC is slow)\n"
        "² (PC only)  : change sound emulator (default is the most accurate)\n"
        "Q -> M / W -> ';' : apply pitch bend on x semitones, x depending on the key. It will not do anything if portaminento or pitchbend slide already runnning. Portamiento currently selected pace is used as pace for pitchbend. Q -> M for positive pitch / W -> ; for negative pitch.\n"
        "\n"
    );
#   endif

    IGNORE_PARAM(argc);
	IGNORE_PARAM(argv);

    sys.allocatorStandard.allocator = NULL;
    sys.allocatorStandard.alloc     = stdAlloc;
    sys.allocatorStandard.alloctemp = stdAlloc;
    sys.allocatorStandard.free      = stdFree;

    {
        bool result;

        DEFAULT_CONSTRUCT(&g_player);

        if (argc > 1)
        {
            strcpy (g_player.filename, argv[1]);
        }
        else
        {
            strcpy (g_player.filename, SYNTHYM_DEFAULT_FILE);
        }

        printf("Filename: %s\n", g_player.filename);

        result = SNDYMloadSounds (&sys.allocatorStandard, g_player.filename, &g_player.soundSet);
        if (result == false)
        {
            printf ("%s", SNDYMgetError());
        }
        ASSERT(result);
    }

    {
        sys.membase = (u8*) malloc( EMULbufferSize(demOS_COREHEAPSIZE + demOS_HEAPSIZE) );

        sys.coreHeapbase = EMULalignBuffer(sys.membase);
        sys.coreHeapsize = demOS_COREHEAPSIZE;
        sys.mainHeapbase = sys.coreHeapbase + demOS_COREHEAPSIZE;
        sys.mainHeapsize = demOS_HEAPSIZE;        

#       ifdef DEMOS_DEBUG
#           define demOS_LOGSIZE (256UL * 1024UL)
#           ifdef __TOS__
            tracLogger.logbase = (u8*) 0x3A0000UL;
#           else
            tracLogger.logbase = (u8*) malloc(demOS_LOGSIZE);
#           endif
            tracLogger.logSize = demOS_LOGSIZE;
#       endif

        ASSERT(sys.membase != NULL);

        sys.bakGemdos32 = SYSgemdosSetMode(NULL);
            
        TRACinit (NULL);

        EMULinit (sys.membase, 1024, 500, 0, g_player.filename);
   
		/* RingAllocator_unitTest(); */

		{
            SYSinitThreadParam      threadParam;

			SYSinit ();

			threadParam.idleThread  = DEMOSidleThread;
            threadParam.idleThreadStackSize = 1024;

            SYSinitHW ();
            SYSinitThreading ( &threadParam ); 
            SYScheckHWRequirements ();

            HW_DISABLE_MOUSE(); /* deactivate mouse management on ACIA */

#           ifdef DEMOS_DEBUG
            registerTraceServices();
#           endif

			/* BIT_unitTest(); */
		}
       
        SNDYMinitPlayer (&sys.allocatorStandard, &g_player.player, &g_player.soundSet);
        EMULcreateSoundBuffer (1000 * 2, true, 50000);

        SynthYMEntry();

		{
			u16* color = (u16*) HW_COLOR_LUT;


            do
			{
				SYSswitchIdle();

				/* no need to vsync here as main thread context is reset by idle thread switch */               
                SYSkbAcquire;

                SynthYMActivity();

				if ( SYSkbHit )
				{
                    SYSkbReset();
				}

#               ifndef __TOS__
                if (EMULupdateRequested())
                {
                    EMULdrawYMbuffer (10, 370, g_player.curvesync); 
                    EMULrender();
                }
#               endif
			}
			while(1); /*  sys.key != (HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE) );*/

#           if !defined(DEMOS_OPTIMIZED) && !defined(DEMOS_USES_BOOTSECTOR)	
			*color = -1;

			SYSshutdown();

			SYSgemdosSetMode(sys.bakGemdos32);

            free (sys.membase);
#			else
			SYSreset ();
#           endif
		}
	}

	return 0;
}
