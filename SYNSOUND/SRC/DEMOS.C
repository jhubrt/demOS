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

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "SYNSOUND\SRC\SCREENS.H"
#include "SYNSOUND\SRC\SYNTH.H"


static char* DEMOSbuildversion = "v0";

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

#ifdef DEMOS_DEBUG
static u16 DEMOStrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
    u16 y = _y;
        
    y += FSMtrace (&g_stateMachine    , _image, _pitch, _planePitch, y);
    y += FSMtrace (&g_stateMachineIdle, _image, _pitch, _planePitch, y);

    return y;
}

static u16 DEMOStraceversion (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
    SYSdebugPrint ( _image, _pitch, _planePitch, 30, _y, DEMOSbuildversion);
    
    return _y + 8;
}

static void registerTraceServices(void)
{
    TRACregisterDisplayService (SYStraceFPS,         1);   /* F1 */
	TRACregisterDisplayService (SYStraceHW,		     2);   /* F2 */
    TRACregisterDisplayService (SYStraceAllocators,  4);   /* F3 */
    TRACregisterDisplayService (LOADtrace,           8);   /* F4 */
    TRACregisterDisplayService (SNDsynPlayerTrace,  16);   /* F5 */
    TRACregisterDisplayService (DEMOStrace,         32);   /* F6 */
    TRACregisterDisplayService (DEMOStraceversion, 128);   /* F8 */
}
#   endif



#define demOS_COREHEAPSIZE      (64UL  * 1024UL)
#define demOS_HEAPSIZE          (512UL * 1024UL)


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP();

    IGNORE_PARAM(argc);
	IGNORE_PARAM(argv);

	{
#       if defined(DEMOS_OPTIMIZED) || defined(DEMOS_USES_BOOTSECTOR)
        sys.membase = base + 64;
#       else
		sys.membase = (u8*) malloc( EMULbufferSize(demOS_COREHEAPSIZE + demOS_HEAPSIZE) );
#       endif

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

#       ifndef DEMOS_USES_BOOTSECTOR
		sys.bakGemdos32 = SYSgemdosSetMode(NULL);
#       endif

        ASSERT(sys.membase != NULL);
        IGNORE_PARAM(base);
            
        TRACinit ("_logs\\traclogpc.log");

		/* STDmset (buffer, 0, size); */

        EMULinit (sys.membase, -1, -1, 0, "Synsound");
   
		FSMinit (&g_stateMachine	, states    , statesSize    , 0, DEBUG_STRING("main"));
		FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0, DEBUG_STRING("idle"));

        {
            SNDsynSoundSet* soundSet = SNDsynthLoad ("SYNSOUND\\SYNTH.INI");
            SNDtest (soundSet);
        }

		/* RingAllocator_unitTest(); */

		{
            SYSinitThreadParam      threadParam;
            SNDsynPlayerInitParam   sndparam;

			SYSinit ();

			threadParam.idleThread  = DEMOSidleThread;
            threadParam.idleThreadStackSize = 1024;

            SYSinitHW ();
            SYSinitThreading ( &threadParam ); 
			SNDsynPlayerInit (&sys.coremem, &sndparam);
            SYScheckHWRequirements ();

            SYSfastPrint(DEMOSbuildversion, (u8*)(SYSreadVideoBase()) + 160 * 192 + 152, 160, 4, (u32)&SYSfont);

#           ifdef DEMOS_DEBUG
            registerTraceServices();
#           endif

			/* BIT_unitTest(); */
		}
       
		ScreensInit ();		

		{
			u16* color = HW_COLOR_LUT;
        
            do
			{
				SYSswitchIdle();

				/* no need to vsync here as main thread context is reset by idle thread switch */               
                SYSkbAcquire;

				FSMupdate (&g_stateMachine);

#               if !defined(DEMOS_OPTIMIZED)
				TRACdisplay((u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L)) + 1);

				if ( SYSkbHit )
				{
                    if ( sys.key != HW_KEY_S )  /* do not allow 60hz  switch */  
                    {
					    TRACmanage(sys.key);
                    }
                    SYSkbReset();
				}

				EMULrender();
#               endif
			}
			while(1); /*  sys.key != (HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE) );*/

#           if !defined(DEMOS_OPTIMIZED) && !defined(DEMOS_USES_BOOTSECTOR)	
			*color = -1;

            SNDsynPlayerShutdown (&sys.coremem);
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
