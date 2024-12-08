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
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"

#include "SYNSOUND\SRC\SYNTH.H"

#include "SYNSOUND\SRC\SCREENS.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#include <time.h>

struct SynthSample_
{
    s8*     pcm;
    u16     length;
    u16     freq;
};
typedef struct SynthSample_ SynthSample;

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



void SynthSoundEntry (FSM* _fsm)
{
    SyntheticSound* this = g_screens.proto = MEM_ALLOC_STRUCT( &sys.allocatorMem, SyntheticSound );
    DEFAULT_CONSTRUCT(this);

    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    this->framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (this->framebuffer != NULL);
    STDmset (this->framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)this->framebuffer);

    this->pcmcopy = (s8*) RINGallocatorAlloc ( &sys.mem, SND_FRAME_NBSAMPLES * 4);
    ASSERT (this->pcmcopy != NULL);

    STDmset (HW_COLOR_LUT+1, -1, 30);

    {
        s16 t, i;

        for (t = 0 ; t < ARRAYSIZE(value) ; t++)
        {
            for (i = 0 ; i < 16 ; i++)
            {
                mask[t][i] = (u16) value[t];
            }
        }

        for (t = 0; t < 16; t++)
        {
            u16 start = t;

            for (i = 0 ; i < 16 ; i++)
            {
                if (i <= t)
                    this->wavetable[t][i] = 0;
                else
                    this->wavetable[t][i] = 127 * (i - t + 1) / (16 - t);

                /* printf("[%d][%d] = %d\n", t, i, this->wavetable[t][i]); */
            }
        }
    }

    /*STDmset (this->framebuffer, 0, 32000);
    TRACdrawScanlinesScale (this->framebuffer, 160, 2, 200);*/

    SNDsynSquareInit();

    this->cyclicratio[0] = this->cyclicratio[1] = 32768UL;
    this->volume[0] = this->volume[1] = 127;

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}

void SynthSoundActivity	(FSM* _fsm)
{   
    SyntheticSound* this = g_screens.proto;
    u16 htone[16];
    u16 t, i = this->wtindex[this->channel];


    for (t = 0; t < 16; t++)
    {
        htone[t] = ((u32)this->wavetable[i][t] * (u32)(this->volume[this->channel] + 1)) >> 7;
    }

    SNDsynSquareUpdate(this->keyb, this->cyclicratio, this->porta, this->volume, htone);

    EMULwait(1);

    if ( SYSkbHit )
    {
        bool         pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;
        u8           scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);
        SNDsynVoice* voice    = &synth.voices[this->channel];

        switch (scancode)
        {
        case HW_KEY_F1: 
        case HW_KEY_F2: 
        case HW_KEY_F3: 
        case HW_KEY_F4: 
        case HW_KEY_F5: 
        case HW_KEY_F6: 
        case HW_KEY_F7: 
        case HW_KEY_F8: 
        case HW_KEY_F9: 
        case HW_KEY_F10:
            synth.keyboardtranspose = scancode - HW_KEY_F1;
            break;

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
                this->keyb[this->channel] = scancode - HW_KEY_Q + 1 + synth.keyboardtranspose * 12;
            }
            else
            {
                this->keyb[this->channel] = 0;
            }
            break;

        case HW_KEY_S:
        case HW_KEY_D:					
        case HW_KEY_F:					
        case HW_KEY_G:					
        case HW_KEY_H:					
        case HW_KEY_J:					
        case HW_KEY_K:					
        case HW_KEY_L:
        case HW_KEY_SEMICOLON:
            if (scancode == HW_KEY_S)
            {
                voice->frame.mask = NULL;
            }
            else
            {
                voice->frame.mask = &mask[scancode - HW_KEY_D][0];
            }
            break;

        case HW_KEY_Z:
        case HW_KEY_X:
        case HW_KEY_C:
        case HW_KEY_V:
        case HW_KEY_B:
        case HW_KEY_N:
        case HW_KEY_M:

            if (voice->frame.lastSound != NULL)
            {
                SNDsynSound* sound = voice->frame.lastSound;
                u8 keyindex = scancode - HW_KEY_Z;

                keyindex %= sound->nbsustainindexes; 
                sound->sampleindexes[ sound->sustainindex ] = sound->sustainindexes[keyindex];
            }
            break;

        case HW_KEY_SPACEBAR:
            this->channel ^= 1;
            break;

        case HW_KEY_NUMPAD_7:
            if (this->wtindex[this->channel] > 0)
                this->wtindex[this->channel]--;
            break;

        case HW_KEY_NUMPAD_9:
            if (this->wtindex[this->channel] < 16)
                this->wtindex[this->channel]++;
            break;
        }
    }

    switch (sys.key)
    {
    case HW_KEY_NUMPAD_1:
        if (this->porta[this->channel] > 0)
            this->porta[this->channel]--;
        break;

    case HW_KEY_NUMPAD_3:
        if (this->porta[this->channel] < 1024)
            this->porta[this->channel]++;
        break;

    case HW_KEY_LEFT:
        if (this->cyclicratio[this->channel] > 0)
            this->cyclicratio[this->channel] -= 256;
        /*printf("%u ", this->cyclicratio[this->channel]);*/
        break;

    case HW_KEY_RIGHT:
        if ( this->cyclicratio[this->channel] < 65536UL ) 
            this->cyclicratio[this->channel] += 256;
        /*printf("%u ", this->cyclicratio[this->channel]);*/
        break;

    case HW_KEY_UP:
        if (this->volume[this->channel] > 0)
            this->volume[this->channel]--;
        break;

    case HW_KEY_DOWN:
        if (this->volume[this->channel] < 127)
            this->volume[this->channel]++;       
        break;
    }
}


void SynthSoundBacktask (FSM* _fsm)
{
    SyntheticSound* this = g_screens.proto;
    static u8 flip = 0;
    u32 backframebuffer = (u32)this->framebuffer;
    
    backframebuffer += flip ? 32000 : 0;

    STDmset ((void*)backframebuffer, 0, 32000);

    SYSvsync;
    {
        u8 channel = this->channel;

        s8* backbuf  = synth.dmabuffers[synth.backbuffer];
/*        u32 cursor = EMULgetPlayOffset ();*/
/*        char temp [32]; */
        
        backbuf += channel;

        {
            u32 offset = synth.squarevoices[0].current * 2;

            if (offset > 280)
                offset = 0;

            STDmcpy (this->pcmcopy, backbuf + offset, synth.dmabufferlen * 2);
        }

        /*EMULgetSound (this->pcmcopy, 4000);

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
            static s16 masknum = 0;

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
			u16 nbsamples = SND_FRAME_NBSAMPLES;
		
			if (nbsamples > 640)
			{
				nbsamples = 640;
			}
			
            if (off > (SND_FRAME_NBSAMPLES - nbsamples))
            {
                off = 0;
            }

            /* SNDsynSample_drawCurve (backbuf + off, nbsamples, 2, (u8*)backframebuffer); */
        }

        SNDsynSample_drawCurve ( this->pcmcopy + 1, 500, 4, (u8*) backframebuffer + 16000);
 
        SNDsynSample_drawXorPass (500, (u8*) backframebuffer + 16000);

        SYSwriteVideoBase (backframebuffer);
    }
    flip ^= 1;
}

void SynthSoundExit (FSM* _fsm)
{
    MEM_FREE(&sys.allocatorMem, g_screens.proto);
    g_screens.proto = NULL; 

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
