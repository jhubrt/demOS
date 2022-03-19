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


/*! @mainpage Main

    Here is the whole source code of 'rebirth' demo.
    It is implemented as:                                                           <br>
    - a low level reusable layer available in demOS folder                          <br>
    - specialized high level implementations                                        <br>
    
    So feel free to re-use, improve... demOS                                        <br>

    rebirth is implemented as a mix of C and ASM68k code. 
    It can be build with pure C without linking any standard library 
    (have a look at configuration is BASETYPES.H)

    3 project .PRJ files are provided:                                              <br>
    - debug target with .C metafile (for optimized build times) REBIRTHB.PRJ        <br>
    - final target with .C metafile (for optimized build times) REBIRTHO.PRJ        <br>
    - 'normal' target REBIRTH.PRJ (useful to identify link errors)                  <br>
      
    Trace system keymap:                                                            <br>

    [TAB]           shift trace plane display                                       <br>
    [BACKSPACE]     toggle video mode (2 planes / 4 planes)                         <br>
    [S]             toggle refresh rate (50hz / 60hz)                               <br>
    [V]             toggle verbose mode                                             <br>
    [F1]            display nb frames used by current effect                        <br>
    [F2]            display various hardware states (keyboard...)                   <br>
    [F3]            display allocators state                                        <br>
    [F4]            display loading system state                                    <br>
    [F5]            display soundtrack system state                                 <br>
    [F6]            display finite states machines                                  <br>
    [F8]            display build version number                                    <br>
    [F9]            global raster (background color change)                         <br>
    [F10]           set colors 1 to 15 in green                                     <br>
    
------------------------------------------------------------------------------  */

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "REBIRTH\REBIRTH1.H"
#include "REBIRTH\REBIRTH2.H"

static char* DEMOSbuildversion = "vC";

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
    TRACregisterDisplayService (SNDtrace,           16);   /* F5 */
    TRACregisterDisplayService (DEMOStrace,         32);   /* F6 */
    TRACregisterDisplayService (DEMOStraceversion, 128);   /* F8 */
}
#   endif


#define demOS_COREHEAPSIZE      (264UL * 1024UL)
#define demOS_HEAPSIZE          (700UL * 1024UL)
#define demOS_PRELOADSIZE       (1024UL * 1024UL)


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP();

    /*STD_unitTest();*/

    IGNORE_PARAM(argc);
    IGNORE_PARAM(argv);

#   ifdef DEMOS_LOAD_FROMHD
    SYSinitStdAllocator();
    LOADinitForHD(&RSC_REBIRTH1, RSC_REBIRTH1_NBENTRIES, RSC_REBIRTH1_NBMETADATA);
    LOADinitForHD(&RSC_REBIRTH2, RSC_REBIRTH2_NBENTRIES, RSC_REBIRTH2_NBMETADATA);
#   endif

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
#           define demOS_LOGSIZE 65536UL

#           ifdef __TOS__
            tracLogger.logbase = (u8*) 0x3A0000UL;
            ASSERT(tracLogger.logbase >= (sys.coreHeapbase + demOS_COREHEAPSIZE + demOS_HEAPSIZE));
#           else
            tracLogger.logbase = (u8*) malloc(demOS_LOGSIZE);
#           endif

            tracLogger.logSize = demOS_LOGSIZE;
#       endif

#       ifndef DEMOS_USES_BOOTSECTOR
        sys.bakGemdos32 = SYSgemdosSetMode(NULL);
#       endif

        TRACinit ("_logs\\traclogpc.log");

        ASSERT(sys.membase != NULL);

        SYScheckHWRequirements ();

        IGNORE_PARAM(base);

        /* STDmset (buffer, 0, size); */

        EMULinit (sys.membase, -1, -1, 0);

        FSMinit (&g_stateMachine	, states    , statesSize    , 0, DEBUG_STRING("main"));
        FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0, DEBUG_STRING("idle"));

        /* RingAllocator_unitTest(); */

        {
            u8* preloadbuffer = NULL;
            {
                SYSinitThreadParam  threadParam;

                SYSinit ();

                threadParam.idleThread          = DEMOSidleThread;
                threadParam.idleThreadStackSize = 1024;

                SYSinitHW ();

                SYSinitThreading ( &threadParam );

                SNDinit (&sys.coremem, 89008UL, 1);

#               ifndef DEMOS_LOAD_FROMHD
                LOADinit (&RSC_REBIRTH1, RSC_REBIRTH1_NBENTRIES, RSC_REBIRTH1_NBMETADATA);
                if (sys.has2Drives)
                {
                    LOADinitFAT (1, &RSC_REBIRTH2, RSC_REBIRTH2_NBENTRIES, RSC_REBIRTH2_NBMETADATA);
                }
#               endif

                SYSfastPrint(DEMOSbuildversion, (u8*)(SYSreadVideoBase()) + 160 * 192 + 152, 160, 4, (u32)&SYSfont);

#               ifdef DEMOS_DEBUG
                registerTraceServices();
#               endif

                /* BIT_unitTest(); */
            }

            if ( sys.has2Drives == false )
            {
#               if defined(DEMOS_OPTIMIZED) || defined(DEMOS_USES_BOOTSECTOR)
                preloadbuffer = sys.membase + demOS_COREHEAPSIZE + demOS_HEAPSIZE;
#               else
                preloadbuffer = (u8*) malloc( EMULbufferSize(demOS_PRELOADSIZE) );
#               endif
            }

            ScreensInit (preloadbuffer, demOS_PRELOADSIZE);		
            {
                volatile u16* color = HW_COLOR_LUT;

                do
                {
                    SYSswitchIdle();

                    /* no need to vsync here as main thread context is reset by idle thread switch */
                    SYSkbAcquire;

                    FSMupdate (&g_stateMachine);

                    snd.playerContext = playTrack ();

#                   if !defined(DEMOS_OPTIMIZED)
                    TRACdisplay((u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L)) + 1);

                    if ( SYSkbHit )
                    {
                        TRACmanage(sys.key);
                        SYSkbReset();					
                    }

                    EMULrender();
#                   endif
                }
                while( sys.key != (HW_KEY_SPACEBAR | HW_KEYBOARD_KEYRELEASE) );

#               if !defined(DEMOS_OPTIMIZED) && !defined(DEMOS_USES_BOOTSECTOR)	
                *color = -1;

                SNDshutdown (&sys.coremem);
                SYSshutdown ();

                SYSgemdosSetMode(sys.bakGemdos32);

                if (preloadbuffer != NULL)
                {
                    free(preloadbuffer);
                }
                free (sys.membase);
#			    else
                SYSreset ();
#               endif
            }
        }
    }

    return 0;
}
