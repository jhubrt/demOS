/*-------------------------------------------------------------------------------------------------
  Copyright J.Hubert 2015

  This file is part of relapse HD demo

  relapse HD demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  relapse HD demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with relapse HD 
  demo.
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

/*! @brief @ref MAIN @file */
/*! @defgroup MAIN
   
    Main program : init / shutdown / main loop
*/

/*! @mainpage Main

    Here is the whole source code of 'relapse HD' demo.
    It is implemented as:                                                           <br>
    - a low level reusable layer available in demOS folder                          <br>
    - specialized high level implementations                                        <br>
    
    So feel free to re-use, improve... demOS                                        <br>

    relapse HD is implemented as a mix of C and ASM68k code. 
    It can be build with pure C without linking any standard library 
    (have a look at configuration is BASETYPES.H)

    3 project .PRJ files are provided:                                              <br>
    - debug target with .C metafile (for optimized build times) RELAPSEB.PRJ        <br>
    - final target with .C metafile (for optimized build times) RELAPSEO.PRJ        <br>
    - 'normal' target RELAPSE.PRJ (useful to identify link errors)                  <br>
      
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

#include "SYNSOUND\SRC\SCREENS.H"

#include "REBIRTH\DISK1.H"
#include "REBIRTH\DISK2.H"

LOADdisk RSC_DISK1 = { NULL, RSC_DISK1_NBENTRIES, NULL, RSC_DISK1_NBMETADATA, 0, NULL
#	ifdef DEMOS_DEBUG
     ,"DISK1.ST", NULL
#   endif
};

LOADdisk RSC_DISK2 = { NULL, RSC_DISK2_NBENTRIES, NULL, RSC_DISK2_NBMETADATA, 1, NULL
#	ifdef DEMOS_DEBUG
     ,"DISK2.ST", NULL
#   endif
};


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


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP();

	u32 coresize    = 270UL * 1024UL;
	u32 size	    = 1024UL * 1024UL; /*768UL * 1024UL;*/
    u32 preloadsize = 1024UL * 1024UL;
 

	/*STD_unitTest();*/

	IGNORE_PARAM(argc);
	IGNORE_PARAM(argv);

	{
#       if defined(DEMOS_OPTIMIZED) || defined(DEMOS_USES_BOOTSECTOR)
        u8*   corebuffer    = base + 64;
#       else
		u8*   corebuffer1   = (u8*) malloc( EMULbufferSize(coresize + size) );
        u8*   corebuffer    = (u8*) EMULalignBuffer (corebuffer1);
#       endif
		void* buffer        = corebuffer + coresize;
        u8*   preloadbuffer = NULL;

		ASSERT(corebuffer != NULL);
        IGNORE_PARAM(base);

		/* STDmset (buffer, 0, size); */

		EMULinit (corebuffer1);

#       ifndef DEMOS_USES_BOOTSECTOR
		sys.bakGemdos32 = SYSgemdosSetMode(NULL);
#       endif
    
		FSMinit (&g_stateMachine	, states    , statesSize    , 0);
		FSMinit (&g_stateMachineIdle, statesIdle, statesIdleSize, 0);

		/* RingAllocator_unitTest(); */

		{
			SYSinitParam            sysparam;
            SNDsynPlayerInitParam   sndparam;

			sysparam.adr	     = buffer;
			sysparam.size	     = size;
			sysparam.coreAdr    = corebuffer;
			sysparam.coreSize   = coresize;
			sysparam.idleThread = DEMOSidleThread;
            sysparam.idleThreadStackSize = 1024;

			SYSinit ( &sysparam );
			SNDsynPlayerInit (&sys.coremem, &sndparam);
            LOADinit (&RSC_DISK1, RSC_DISK1_NBENTRIES, RSC_DISK1_NBMETADATA);
			TRACinit (&RSC_DISK1, RSC_DISK1_SYSTFONT_BIN);
            SYScheckHWRequirements ();

            SYSfastPrint(DEMOSbuildversion, (u8*)(SYSreadVideoBase()) + 160 * 192 + 152, 160, 4, (u32) sys.fontChars);

#           ifdef DEMOS_DEBUG
            registerTraceServices();
#           endif

			/* BIT_unitTest(); */
		}

        if ( sys.has2Drives == false )
        {
#           if defined(DEMOS_OPTIMIZED) || defined(DEMOS_USES_BOOTSECTOR)
            preloadbuffer = buffer1 + size;
#           else
            preloadbuffer = malloc( EMULbufferSize(preloadsize) );
#           endif
        }

		ScreensInit (preloadbuffer, preloadsize);		
		{
			u16* color = HW_COLOR_LUT;

			SYSvsync;

			do
			{
				SYSswitchIdle();

				/* no need to vsync here as main thread context is reset by idle thread switch */
    			SYSbeginFrameNum = SYSvblLcount;
                
                SYSkbAcquire;

				FSMupdate (&g_stateMachine);

#               if !defined(DEMOS_OPTIMIZED)
				TRACdisplay((u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L)) + 1);

				if ( SYSkbHit )
				{
					TRACmanage(sys.key);
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

            if (preloadbuffer != NULL)
            {
                free(preloadbuffer);
            }
			free (corebuffer1);
#			else
			SYSreset ();
#           endif
		}
	}

	return 0;
}
