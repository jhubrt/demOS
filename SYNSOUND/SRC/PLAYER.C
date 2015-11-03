/*------------------------------------------------------------------------------  -----------------
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

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\SYNTH.H"

#include "SYNSOUND\SRC\SCREENS.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#endif

#include <time.h>

STRUCT(SynthSample)
{
    s8*     pcm;
    u16     length;
    u16     freq;
};

void SNDtest (void);


u16 value[8] = 
{
    0xFFFF,
    0xFEFE,
    0xFCFC,
    0xF8F8,
    0xF0F0,
    0xE0E0,
    0xC0C0,
    0x8080
};

/*		u32 value[7] = 
		{
			0xFFFFFFFF,
			0xEEEEEEEE,
			0xCCCCCCCC,
			0x88888888,
			0xCCCCCCCC,
			0xEEEEEEEE,
			0xFFFFFFFF,
		};*/

		/*u32 value[7] = 
		{
			0xFFFFFFFF,
			0x7F7F7F7F,
			0x3F3F3F3F,
			0x1F1F1F1F,
			0x3F3F3F3F,
			0x7F7F7F7F,
			0xFFFFFFFF,
		};*/

u16 mask[ARRAYSIZE(value)][16];
		
/*		for (t = 0 ; t < 7 ; t++)
		{
			if (t < 3)
			{
				STDmcpy (&mask[t][0], value, 16);
			}
			else
			{
				STDmset (&mask[t][0], -1, 16);
			}
		}*/

		/*SNDtestMask (testsound2.samples[0].data, testsound2.samples[0].length, mask);*/



void IntroEntry (FSM* _fsm)
{
    g_screens.intro = MEM_ALLOC_STRUCT( &sys.allocatorMem, Intro );
    DEFAULT_CONSTRUCT(g_screens.intro);

    g_screens.intro->framebuffer = RINGallocatorAlloc ( &sys.mem, 64000 );
    ASSERT (g_screens.intro->framebuffer != NULL);
    STDmset (g_screens.intro->framebuffer, 0, 64000);
    SYSwriteVideoBase ((u32)g_screens.intro->framebuffer);

    g_screens.intro->pcmcopy = (s8*) RINGallocatorAlloc ( &sys.mem, SND_FRAME_NBSAMPLES * 4);
    ASSERT (g_screens.intro->pcmcopy != NULL);

    STDmset (HW_COLOR_LUT+1, -1, 30);

    SNDtest ();

    {
        s16 t, i;

        for (t = 0 ; t < ARRAYSIZE(value) ; t++)
        {
            for (i = 0 ; i < 16 ; i++)
            {
                mask[t][i] = (u16) value[t];
            }
        }
    }

    /*STDmset (g_screens.intro->framebuffer, 0, 32000);
    TRACdrawScanlinesScale (g_screens.intro->framebuffer, 160, 2, 200);*/

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}

void IntroActivity	(FSM* _fsm)
{
    SNDsynUpdate(g_screens.intro->keyb[0], g_screens.intro->keyb[1]);

    if ( SYSkbHit )
    {
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;
        u8   scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);


        switch (scancode)
        {
        case HW_KEY_Q:
        case HW_KEY_W:
        case HW_KEY_E:
        case HW_KEY_R:
        case HW_KEY_T:
        case HW_KEY_Y:
        case HW_KEY_U:
        case HW_KEY_I:
        case HW_KEY_O:
        case HW_KEY_P:
        case HW_KEY_BRACKET_LEFT:
        case HW_KEY_BRACKET_RIGHT:

            if ( pressed )
            {
                g_screens.intro->keyb[g_screens.intro->channel] = scancode - HW_KEY_Q + 1;
            }
            else
            {
                if ( g_screens.intro->keyb[g_screens.intro->channel] == (scancode - HW_KEY_Q + 1 ))
                {
                    g_screens.intro->keyb[g_screens.intro->channel] = 0;
                }
            }
            break;
        }

        switch (sys.key)
        {
        case HW_KEY_NUMPAD_MINUS:
            if ( synth.voices[g_screens.intro->channel].frame.transpose > 0 )
            {
                synth.voices[g_screens.intro->channel].frame.transpose--;
            }
            break;

        case HW_KEY_NUMPAD_PLUS:
            if ( synth.voices[g_screens.intro->channel].frame.transpose < 4 )
            {
                synth.voices[g_screens.intro->channel].frame.transpose++;
            }
            break;

        case HW_KEY_SPACEBAR:
            g_screens.intro->channel ^= 1;
            break;

        case HW_KEY_LEFT:
            if ( synth.voices[g_screens.intro->channel].frame.volume > 0 )
            {
                synth.voices[g_screens.intro->channel].frame.volume--;
            }
            break;

        case HW_KEY_RIGHT:
            if ( synth.voices[g_screens.intro->channel].frame.volume < 7 )
            {
                synth.voices[g_screens.intro->channel].frame.volume++;
            }
            break;

        case HW_KEY_UP:
            g_screens.intro->mask[g_screens.intro->channel]--;

            if (g_screens.intro->mask[g_screens.intro->channel] == -2)                     
            {
                g_screens.intro->mask[g_screens.intro->channel] = ARRAYSIZE(value) - 1;
            }
            break;

        case HW_KEY_DOWN:
            g_screens.intro->mask[g_screens.intro->channel]++;

            if ( g_screens.intro->mask[g_screens.intro->channel] == ARRAYSIZE(value) )
            {
                g_screens.intro->mask[g_screens.intro->channel] = -1;
            }
            break;
        }
    }
}


void IntroBacktask (FSM* _fsm)
{
    static u8 flip = 0;
    u32 backframebuffer = (u32)g_screens.intro->framebuffer;
    
    backframebuffer += flip ? 32000 : 0;

    STDmset ((void*)backframebuffer, 0, 32000);

    SYSvsync;
    {
        u8 channel = g_screens.intro->channel;
        static s16 masknum = 0;

        s8* backbuf  = g_screens.intro->pcmcopy;
        s8* frontbuf = g_screens.intro->pcmcopy;
        s32 y = 0, k = 0;
/*        u32 cursor = EMULgetPlayOffset ();*/
/*        char temp [32]; */
        
        backbuf  -= channel;
        frontbuf -= channel;

        if (synth.backbuffer)
        {
            backbuf += SND_FRAME_NBSAMPLES * 2;
        }
        else
        {
            frontbuf += SND_FRAME_NBSAMPLES * 2;
        }

        STDmcpy (g_screens.intro->pcmcopy, synth.buffer, synth.dmabufferlen * 2);

        /*EMULgetSound (g_screens.intro->pcmcopy, 4000);

        {
            static clock_t last = 0;

            clock_t newclock = clock();
            if (newclock != last)
            {
                float diff = (float)CLOCKS_PER_SEC / (newclock - last);
                last = newclock;
        
                sprintf (temp, "%f", diff);

                WINsetColor  (w, 0, 0, 0);
                WINfilledRectangle (w, 0,0, 300, 16);
                WINsetColor  (w, 0, 255, 0);
                WINtext (w,0,0,temp);
            }
        }
        */

#       ifndef __TOS__
        /*{
            WINdow* w = EMULgetWindow ();
            WINgetMouse (w, NULL, &y, &k, NULL);

            if (k != 0)
            {
                masknum = (y / 32) % ARRAYSIZE(value);

                synth.voices[0].copyMask = mask[masknum];
            }
            else
            {
                synth.voices[0].copyMask = NULL;
            }
        }*/
#       endif

        /*
        sprintf (temp, "%u", cursor);
        WINsetColor  (w, 0, 0, 0);
        WINfilledRectangle (w, 0,0, 300, 16);
        WINsetColor  (w, 0, 255, 0);
        WINtext (w,0,0,temp);*/

        {
            u16 off = synth.voices[channel].sampledisplay;

            if (off > (SND_FRAME_NBSAMPLES - 640))
            {
                off = 0;
            }

            SNDsynSample_drawCurve (backbuf + off, 640, 2, (u8*)backframebuffer);
        }

        /*SNDsynSample_drawCurve ( g_screens.intro->pcmcopy, 640, 8, (u8*)backframebuffer);*/

        SNDsynSample_drawCurve ( frontbuf + synth.dmabufferlen - (160 * 2), 160, 2, (u8*) backframebuffer + 16040);
        SNDsynSample_drawCurve ( backbuf , 160, 2, (u8*) backframebuffer + 16080);

        SNDsynSample_drawXorPass (640, (u8*) backframebuffer + 16000);
        SNDsynSample_drawXorPass (640, (u8*) backframebuffer);

        SYSwriteVideoBase (backframebuffer);
    }
    flip ^= 1;
}

void IntroExit	(FSM* _fsm)
{
    MEM_FREE(&sys.allocatorMem, g_screens.intro);
    g_screens.intro = NULL; 

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
