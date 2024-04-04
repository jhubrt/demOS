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

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"


static void DEMOSidleThread(void)
{
#	ifdef __TOS__
    STDcpuSetSR(0x2300);
	while (true)
#	endif
	{
	}
}


u8 g_mixer;

static void InitYM(void)
{    
    u8 mixer;

    /* Setup microwire */
    *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_MIXER_YM;     /* YM -12db does not work on default hardware unfortunately :( */
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

    *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

    *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_LEFT | 20;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

    *HW_MICROWIRE_MASK = HW_MICROWIRE_MASK_SOUND;    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME_RIGHT | 20;
    while (*HW_MICROWIRE_MASK != HW_MICROWIRE_MASK_SOUND);

    /* Setup YM */
    *HW_YM_REGSELECT = HW_YM_SEL_IO_AND_MIXER;
    g_mixer = HW_YM_GET_REG() & 0xC0;
    mixer = g_mixer | 0x37; /* mixer : all square off, noise on A only */

    HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, mixer);

    HW_YM_SET_REG(HW_YM_SEL_ENVELOPESHAPE, 10);  /* triangle */

    HW_YM_SET_REG(HW_YM_SEL_LEVELCHA, 16);
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHB, 16);
    HW_YM_SET_REG(HW_YM_SEL_LEVELCHC, 16);

    HW_YM_SET_REG(HW_YM_SEL_FREQNOISE, 4);      /* set noise freq */
} 


#define LOW_MIN  0x62
#define HIGH_MAX 0x10


static void PlayYM(void)
{
    static char flip = 0;
    static u16 freq  = LOW_MIN;

    void* adr = (void*) SYSreadVideoBase();
    static char temp[] = "      ";


    flip ^= 1;
    if (flip)
        HW_YM_SET_REG (HW_YM_SEL_ENVELOPESHAPE, 10); /* restart triangle every 2 vbl */

    HW_YM_SET_REG (HW_YM_SEL_FREQENVELOPE_H, freq >> 8); 
    HW_YM_SET_REG (HW_YM_SEL_FREQENVELOPE_L, freq & 0xFF); 

    switch (sys.key)
    {
    case HW_KEY_LEFT:
        if (freq < LOW_MIN)
            freq++;
        HW_YM_SET_REG(HW_YM_SEL_FREQNOISE, 10);      /* set noise freq */
        break;

    case HW_KEY_RIGHT:
        if (freq > HIGH_MAX)
            freq--;
        HW_YM_SET_REG(HW_YM_SEL_FREQNOISE, 4);      /* set noise freq */
        break;   
    }

    STDutoa(temp, freq, 6);   
    SYSdebugPrint(adr, 160, 2, 0, 0, temp);
}




#define demOS_COREHEAPSIZE      (64UL  * 1024UL)
#define demOS_HEAPSIZE          (512UL * 1024UL)


int main(int argc, char** argv)
{
    u8* base = (u8*) STDgetSP();

    IGNORE_PARAM(argc);
	IGNORE_PARAM(argv);

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

		sys.bakGemdos32 = SYSgemdosSetMode(NULL);

        ASSERT(sys.membase != NULL);
        IGNORE_PARAM(base);
            
        TRACinit ("_logs\\traclogpc.log");

        EMULinit (sys.membase, -1, -1, 0);
   
		{
            SYSinitThreadParam      threadParam;

			SYSinit ();

			threadParam.idleThread  = DEMOSidleThread;
            threadParam.idleThreadStackSize = 1024;

            SYSinitHW ();
            SYSinitThreading ( &threadParam ); 
		}
        
        InitYM();

		{       
            do
			{
				SYSswitchIdle();

                PlayYM();

				/* no need to vsync here as main thread context is reset by idle thread switch */               
                SYSkbAcquire;


                if ( SYSkbHit )
				{
                    SYSkbReset();
				}

				EMULrender();
			}
			while(sys.key != HW_KEY_ESC);

			SYSshutdown();

			SYSgemdosSetMode(sys.bakGemdos32);

            free (sys.membase);
		}
	}

	return 0;
}
