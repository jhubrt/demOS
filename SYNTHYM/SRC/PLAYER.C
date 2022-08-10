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
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYNTHYM.H"
#include "DEMOSDK\YMDISP.H"
#include "DEMOSDK\PC\EMUL.H"

#define PLAYER_C

#include "SYNTHYM\SRC\PLAYER.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#include <time.h>

#include <math.h>

Player g_player;

char* g_keyNames[] = {"C-", "C+", "D-", "D+", "E-", "F-", "F+", "G-", "G+", "A-", "A+", "B-"};



void SynthYMEntry (void)
{
    EMULtraceNewValue(0, 0, 0xFFF, 255, 255, 0);
    EMULtraceNewValue(1, -2048, 2048, 0, 255, 255);
    EMULtraceNewValue(2, -2048, 2048, 0, 255, 0);
    
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    g_player.framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (g_player.framebuffer != NULL);
    STDmset (g_player.framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)g_player.framebuffer);

    HW_COLOR_LUT[0] = 0;
    HW_COLOR_LUT[1] = -1;
    HW_COLOR_LUT[2] = PCENDIANSWAP16(0x70);
    HW_COLOR_LUT[3] = 0x700;

    g_player.ctrl.channels[0] = true;
    g_player.curvesync = true;

    /*    {
    int t;
    int periods[] = {175, 257, 131, 217, 109, 345};
    //        int periods[] = {367, 275, 293, 219, 245, 184};
    //        int periods[] = {387, 192, 327, 162, 261, 127};
    //        int periods[] = {410, 276, 327, 218, 274, 183};
    int minf = 5;

    for (t = 0 ; t < ARRAYSIZE(periods) ; t++)
    {
    float freq = 50000.0f / (float) periods[t];

    printf ("%f - %f - %f\n", (float) periods[t] / (float) periods[minf], freq , freq / (50000.0f / (float) periods[minf]));
    }

    for (t = 0 ; t <= 24 ; t++)
    {
    printf ("%03d - %f\n", t, powf(2.0f, (float)t/12.0f) );
    }
    }*/
}

#define PLAYER_KEYREPEAT_DELAY 35
#define PLAYER_KEYREPEAT_FREQ  1

void SynthYMActivity (void)
{
    u8   t;
    u8   scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);
    bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;
    bool repeat   = false;


    if ( SYSkbHit )
    {
        switch (scancode)
        {
        case HW_KEY_TAB:
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
        case HW_KEY_EQUAL:
        case HW_KEY_2:  
        case HW_KEY_3:  
        case HW_KEY_5:  
        case HW_KEY_6:  
        case HW_KEY_7:  
        case HW_KEY_9:  
        case HW_KEY_0:  
            {
                s8 key;

                switch (scancode)
                {
                case HW_KEY_TAB:           key = -1; break;
                case HW_KEY_Q:             key = 0;  break; 
                case HW_KEY_2:             key = 1;  break;
                case HW_KEY_W:             key = 2;  break;
                case HW_KEY_3:             key = 3;  break;
                case HW_KEY_E:             key = 4;  break; 
                case HW_KEY_R:             key = 5;  break;
                case HW_KEY_5:             key = 6;  break;
                case HW_KEY_T:             key = 7;  break;
                case HW_KEY_6:             key = 8;  break;
                case HW_KEY_Y:             key = 9;  break;   
                case HW_KEY_7:             key = 10; break;
                case HW_KEY_U:             key = 11; break;  
                case HW_KEY_I:             key = 12; break;  
                case HW_KEY_9:             key = 13; break;
                case HW_KEY_O:             key = 14; break;
                case HW_KEY_0:             key = 15; break;
                case HW_KEY_P:             key = 16; break;
                case HW_KEY_BRACKET_LEFT:  key = 17; break;
                case HW_KEY_EQUAL:         key = 18; break;
                case HW_KEY_BRACKET_RIGHT: key = 19; break;
                default: ASSERT(0);
                }

                if ( pressed )
                {
                    g_player.ctrl.keyb = key;

                    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
                    {
                        if (g_player.ctrl.channels[t])
                        {
                            g_player.ctrl.pressed[t] = 3;
                        }
                    }
                }
                else
                {
                    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
                    {
                        if (g_player.ctrl.channels[t])
                        {
                            if (key == g_player.ctrl.keyb)
                            {                            
                                g_player.ctrl.pressed[t] = 0;
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    if (pressed)
    {   
        if (g_player.keyboard[scancode] < PLAYER_KEYREPEAT_DELAY)
        {
            g_player.keyboard[scancode] += g_player.updated;
        }
    }
    else
    {
        g_player.keyboard[scancode] = 0;
    }

    if (g_player.keyboard[scancode] >= PLAYER_KEYREPEAT_DELAY)
    {       
        static u32 count = 0;

        count += g_player.updated;
        if (count > PLAYER_KEYREPEAT_FREQ)
        {
            count = 0;
            repeat = true;
        }
    }

    if (( SYSkbHit ) || repeat)
    {
        switch (sys.key)
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
            g_player.ctrl.transpose[g_player.ctrl.currentchannel] = (scancode - HW_KEY_F1) * 12;
            break;   

        case HW_KEY_BACKSPACE:
            g_player.curvesync = !g_player.curvesync;
            break;

        case HW_KEY_LEFT:
            if ( g_player.ctrl.transpose[g_player.ctrl.currentchannel] > 0 )
            {
                g_player.ctrl.transpose[g_player.ctrl.currentchannel]--;
            }
            break;

        case HW_KEY_RIGHT:
            if ( g_player.ctrl.transpose[g_player.ctrl.currentchannel] < (9 * 12) )
            {
                g_player.ctrl.transpose[g_player.ctrl.currentchannel]++;
            }
            break;

        case HW_KEY_A:
        case HW_KEY_S:					
        case HW_KEY_D:					
        case HW_KEY_F:					
        case HW_KEY_G:					
        case HW_KEY_H:					
        case HW_KEY_J:					
        case HW_KEY_K:					
        case HW_KEY_L:					
        case HW_KEY_SEMICOLON:			

        case HW_KEY_Z:
        case HW_KEY_X:
        case HW_KEY_C:
        case HW_KEY_V:
        case HW_KEY_B:
        case HW_KEY_N:
        case HW_KEY_M:
        case HW_KEY_COMMA:
        case HW_KEY_DOT:
        case HW_KEY_SLASH:
        {
            s8 pitchbend;
            
            
            if (sys.key >= HW_KEY_Z)
            {
                pitchbend = -(sys.key - HW_KEY_Z + 1);
            }
            else
            {
                pitchbend = sys.key - HW_KEY_A + 1;
            }

            g_player.player.commands[g_player.ctrl.currentchannel].pitchbendrange = pitchbend;
        }
        break;

        case HW_KEY_NUMPAD_1:
            if ( g_player.player.commands[g_player.ctrl.currentchannel].scorevolume > 0 )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].scorevolume--;
                g_player.player.commands[g_player.ctrl.currentchannel].scorevolumeset = true;
            }
            break;

        case HW_KEY_NUMPAD_3:
            if ( g_player.player.commands[g_player.ctrl.currentchannel].scorevolume < 15 )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].scorevolume++;
                g_player.player.commands[g_player.ctrl.currentchannel].scorevolumeset = true;
            }
            break;

        case HW_KEY_NUMPAD_4:
            if ( g_player.player.commands[g_player.ctrl.currentchannel].finetune > -128 )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].finetune--;
            }
            break;

        case HW_KEY_NUMPAD_6:
            if ( g_player.player.commands[g_player.ctrl.currentchannel].finetune < 127 )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].finetune++;
            }
            break;

        case HW_KEY_NUMPAD_7:
            g_player.player.commands[g_player.ctrl.currentchannel].portamientoticks--;
            g_player.player.commands[g_player.ctrl.currentchannel].pitchbendticks = g_player.player.commands[g_player.ctrl.currentchannel].portamientoticks;
            break;

        case HW_KEY_NUMPAD_9:
            g_player.player.commands[g_player.ctrl.currentchannel].portamientoticks++;
            g_player.player.commands[g_player.ctrl.currentchannel].pitchbendticks = g_player.player.commands[g_player.ctrl.currentchannel].portamientoticks;
            break;

        case HW_KEY_UP:
            if ( g_player.ctrl.currentchannel > 0 )
            {
                g_player.ctrl.currentchannel--;
            }
            break;

        case HW_KEY_DOWN:
            if ( g_player.ctrl.currentchannel < 2 )
            {
                g_player.ctrl.currentchannel++;
            }
            break;

        case HW_KEY_SPACEBAR:
            g_player.ctrl.channels[g_player.ctrl.currentchannel] ^= 1;
            break;

        case HW_KEY_NUMPAD_PLUS:
            if ( (g_player.player.commands[g_player.ctrl.currentchannel].soundindex + 1) < g_player.soundSet.nbSounds )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].soundindex++;
            }
            break;

        case HW_KEY_NUMPAD_MINUS:
            if ( g_player.player.commands[g_player.ctrl.currentchannel].soundindex > 0 )
            {
                g_player.player.commands[g_player.ctrl.currentchannel].soundindex--;
            }
            break;

#       ifndef __TOS__
        case HW_KEY_RETURN:
            {
                u16 t;
                bool result;

                SNDYMfreePlayer(&sys.allocatorStandard, &g_player.player);
                SNDYMfreeSounds(&sys.allocatorStandard, &g_player.soundSet);
                DEFAULT_CONSTRUCT(&g_player.player);
                result = SNDYMloadSounds (&sys.allocatorStandard, g_player.filename, &g_player.soundSet);
                if (result == false)
                {
                    printf ("%s", SNDYMgetError());
                }
                ASSERT(result);
                SNDYMinitPlayer (&sys.allocatorStandard, &g_player.player, &g_player.soundSet);

                for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
                {
                    if (g_player.player.commands[t].soundindex >= g_player.soundSet.nbSounds)
                    {
                        g_player.player.commands[t].soundindex = g_player.soundSet.nbSounds - 1;
                    }

                    g_player.ctrl.pressed[t] = 0;
                    g_player.ctrl.keyb       = 0;
                }
            }
            break;
#       endif
        }
    }

    {
        for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
        {
            g_player.player.commands[t].key          = g_player.ctrl.transpose [t] + g_player.ctrl.keyb + 1;
			g_player.player.commands[t].pressed      = g_player.ctrl.pressed   [t] != 0;
			g_player.player.commands[t].justpressed  = g_player.ctrl.pressed   [t] == 3;
        }

        {
#       ifdef __TOS__
            g_player.updated = true;
#       else
            static u8 buffer[1000 * 2];
            u8 playBuffer = EMULgetPlayOffset () >= (1000 * 2);
            {
                static u8 oldBuffer = 0;

                g_player.updated = (oldBuffer != playBuffer);
                oldBuffer = playBuffer;
            }

#       endif

            if (g_player.updated)
            {
                SNDYMupdate (&g_player.player);

#               ifndef __TOS__
                EMULplaysound (buffer, 1000 * 2, playBuffer != 0 ? 0 : 1000 * 2);
#               endif

                /* printf ("%d %d\n", g_player.keyb [g_player.channel], g_player.pressed[g_player.channel]); */

                for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
                {
                    g_player.ctrl.pressed[t] &= 1;
                    g_player.player.commands[t].pitchbendrange = 0;
                    g_player.player.commands[t].scorevolumeset = false;
                }
                
                g_player.framenum++;
            }
        }
    }
}

void SynthYMBacktask (void)
{
    u8* pinstr = (u8*) g_player.framebuffer;
    u8* p;
    u8  t;
    u32 backframebuffer;
    u8  ymregs[16];


#   if SNDYM_REGS_MIRRORING
    STDmcpy(ymregs, g_player.player.YMregs.regscopy, 16);
#   else
    aYMgetState(ymregs);
#   endif
    
    if (g_player.flip)
        pinstr += 32000;

    backframebuffer = (u32) pinstr;

    STDmset (pinstr, 0, 32000);
   
    p = pinstr;
   
    pinstr += 4;

    pinstr = SNDYMdrawSoundCurves(&g_player.player, g_player.ctrl.channels, pinstr);

    {
        char* keyname = g_keyNames[(g_player.ctrl.keyb + ARRAYSIZE(g_keyNames)) % ARRAYSIZE(g_keyNames)];
        char temp[4];

        temp[0] = keyname[0];
        temp[1] = keyname[1];
        temp[3] = 0;

        for (t = 0; t < SND_YM_NB_CHANNELS; t++)
        {
            if (t == g_player.ctrl.currentchannel)
            {
                SYSfastPrint(">", p, 160, 4, (u32)&SYSfont);
            }

            if (g_player.ctrl.channels[t])
            {
                static char text[3] = "  ";

                STDuxtoa(text, g_player.ctrl.transpose[t], 2);
                SYSfastPrint(text, p + 4, 160, 4, (u32)&SYSfont);

                temp[2] = (g_player.ctrl.transpose[t] / 12) + '1';

                if (g_player.ctrl.keyb < 0)
                {
                    temp[2]--;
                }
                else if (g_player.ctrl.keyb >= 12)
                {
                    temp[2]++;
                }

                SYSfastPrint(temp, p + 20 * 160 + 8, 160, 4, (u32)&SYSfont);
            }

            p += 160 * SND_YM_PLAYER_NBLINES_INSTR;
        }
    }

    SNDYMdrawYMstate (ymregs, pinstr);

    g_player.flip = g_player.flip == 0;

    SYSwriteVideoBase (backframebuffer);
    SYSvsync;
}
