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
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"

#include "DEMOSDK\BLITSND.H"

#include "BLSPLAY\SRC\SCREENS.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#include <time.h>

#define BLSPLAY_TITLE "BLSplay v1.6.1"

#ifdef __TOS__
#   define bplayerUSEASM 1
#endif

#if bplayerUSEASM
#   define drawXorPass  adrawXorPass
#   define drawCurve    adrawCurve
void adrawXorPass (void* _screen);
void drawCurve (void* _sample, u16 _nbsamples, u16 _incx, void* _screen);
#endif

u16 g_maskValues[] = 
{
    0xFFFF,
    0xFEFE,
    0xFCFC,
    0xF8F8,
    0xF0F0,
    0xE0E0,
    0xC0C0,
    0x8080,
    
    0xEEEE,
    0xCCCC,
    0x8888,
    
    0x7F7F,
    0x3F3F,
    0x1F1F,
};


ENUM(TextdisplayState)
{
    TDS_WRITELINE,
    TDS_WAITLINE
};

#ifdef __TOS__
#   define TDS_WAITLINECOUNT   50
#else
#   define TDS_WAITLINECOUNT   100
#endif

char* g_textdisplayContent[] =
{
    "BLITsnd by Metal Ages from CYBERNETICS [soundtrack routine using blitter]",
    "BLSconvert converts MOD format into BLS format [with constraints...]",
    "BLSplay replays BLS files [routine uses up to 16% of VBL on 8mhz STe]",
    "Thanks fly to NPomarede for his great advices about DMA sound",
    "Greetings go to all friends and kind people from the scene" 
};


static void playerInit (u16 _i, u16 _nb)
{
    u8* framebuffer = (u8*)SYSreadVideoBase();
    char temp[] = "   /   ";


    STDuxtoa(temp, _i, 3);
    STDuxtoa(&temp[4], _nb, 3);
    SYSdebugPrint(framebuffer, 160, 2, 0, 16, temp);
}

#define PCMCOPYSIZE 16*160

STATIC_ASSERT(PCMCOPYSIZE >= (BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD));

void PlayerEntry (void)
{
    BLSsoundTrack* sndtrack;
    RINGallocatorFreeArea info;
    s32 deltasize;
    
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    DEFAULT_CONSTRUCT(&g_player.player);
    
    g_player.play = true;
    
    g_player.currentchannel = 0;
    g_player.currentmask = 0;

    g_player.leftvolume  = 20;
    g_player.rightvolume = 20;

    RINGallocatorFreeSize(&sys.mem, &info);
    deltasize = info.size;

    sndtrack = BLSload (&sys.allocatorMem, g_player.filename);
    BLSinit (&sys.allocatorMem, sndtrack, playerInit);

    RINGallocatorFreeSize(&sys.mem, &info);
    deltasize -= info.size;
    
    g_player.framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (g_player.framebuffer != NULL);
    STDmset (g_player.framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)g_player.framebuffer);

    g_player.pcmcopy = (s8*) RINGallocatorAlloc ( &sys.mem, PCMCOPYSIZE);
    ASSERT (g_player.pcmcopy != NULL);
    STDmset(g_player.pcmcopy, 0, PCMCOPYSIZE);

    {     
        char temp[] = "alloc=0x      ";
        u8* framebuffer = (u8*)g_player.framebuffer;
        u16 t = 0;

        SYSdebugPrint(framebuffer        , 160, 2, 66, 0, BLSPLAY_TITLE);
        SYSdebugPrint(framebuffer + 32000, 160, 2, 66, 0, BLSPLAY_TITLE);

        STDuxtoa(&temp[8], deltasize, 6);
        SYSdebugPrint(framebuffer        , 160, 2, 66, 16, temp);
        SYSdebugPrint(framebuffer + 32000, 160, 2, 66, 16, temp);

#       ifdef __TOS__
        SYSdebugPrint(framebuffer        , 160, 2, 66, 32, "DMAsync");
        SYSdebugPrint(framebuffer + 32000, 160, 2, 66, 32, "DMAsync");

        temp[2] = 0;

        for (t = 0 ; t < 14 ; t++, framebuffer += 5*160)
        {       
            if (t & 1)
            {
                STDmset(framebuffer + 16    , 0x55555555UL, 112);
                STDmset(framebuffer + 32016 , 0x55555555UL, 112);
            }
            else
            {
                STDmset(framebuffer + 8    , 0xFFFFFFFFUL, 120);
                STDmset(framebuffer + 32008, 0xFFFFFFFFUL, 120);

                STDuxtoa(temp, t*5, 2);
                SYSfastPrint (temp, framebuffer, 160, 4);
                SYSfastPrint (temp, framebuffer + 32000, 160, 4);
            }
        }
#       else
        IGNORE_PARAM(t);
        printf ("bytes allocated= %d\n", deltasize);
#       endif
    }

    BLSplayerInit  (&sys.allocatorMem, &(g_player.player), sndtrack, !g_player.testMode);
}

#if bplayerUSEASM==0
static void drawCurve (void* _sample, u16 _nbsamples, u16 _incx, void* _screen)
{
    u16* disp = ((u16*) _screen) + (34*80);
    s8* sample = (s8*)_sample;
    u16 i;


    for (i = 0 ; i < _nbsamples ; i += 16)
    {
        s16 t;      
        u16 p1 = 0x8000;

        for (t = -34*80 ; t < 34*80 ; t += 80)
        {
            disp[t] = 0;
        }

        do
        {
            s16 s = *sample;
            sample += _incx;
            s &= 0xFFFC;
            s = (s << 2) + (s << 4);

            disp[s] |= PCENDIANSWAP16(p1);

            p1 >>= 1;
        }
        while (p1 != 0);

        disp += 2;
    }
}

static void drawXorPass (void* _screen)
{
    u16* line = (u16*) _screen;
    u16* disp;
    u16 i,t;
    u16  nbwords = 40;

    u16* l = line + 80; 

    for (i = 0 ; i < 34 ; i++)
    {
        disp = l;

        for (t = 0 ; t < nbwords ; t += 4)
        {
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
            *disp ^= disp[-80]; disp += 2;
        }

        l += 80;
    }

    l = line + (68 * 80);

    for (i = 0 ; i < 34 ; i++)
    {
        disp = l;

        for (t = 0 ; t < nbwords ; t += 4)
        {
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;
            *disp ^= disp[80]; disp += 2;        
        }

        l -= 80;
    }
}
#endif


void PlayerActivity	(FSM* _fsm)
{
    if (g_player.startdisplay)
    {
        *HW_COLOR_LUT = 0x70;
        BLSupdate (&(g_player.player));
    }
    else
    {
        *HW_COLOR_LUT = 0;
#       ifdef __TOS__
        {
            u8 count = *HW_VIDEO_COUNT_L;
            while (*HW_VIDEO_COUNT_L == count);
        }
        *HW_COLOR_LUT = 0x70;
#       endif

        BLSupdate (&(g_player.player));

        *HW_COLOR_LUT = 0x34;
        g_player.rastermax = TRACmaxraster(g_player.rastermax);
    }

    *HW_COLOR_LUT = g_player.player.clientEvent;

#   ifdef __TOS__
    {
        u16  mask = (g_player.player.dmabufend - g_player.player.dmabufstart) > BLS_NBBYTES_PERFRAME ? 0xFFFF : 0;
        u8* framebuffer = (u8*)g_player.framebuffer;

        *(u16*)(framebuffer +         (74 * 2 + 160 * 32))           = mask;
        *(u16*)(framebuffer + 32000 + (74 * 2 + 160 * 32))           = mask;

        mask = ~mask;

        *(u16*)(framebuffer +         (74 * 2 + 160 * 32 + 160 * 7)) = mask;
        *(u16*)(framebuffer + 32000 + (74 * 2 + 160 * 32 + 160 * 7)) = mask;
    }
#   else
    if ( WINisClosed(EMULgetWindow()) )
    {
        FSMgotoNextState(&g_stateMachineIdle);
    }
#   endif

    if (g_player.player.volumeLeft != 0)
    {
        g_player.leftvolume = g_player.player.volumeLeft & 0x1F;
    }

    if (g_player.player.volumeRight != 0)
    {
        g_player.rightvolume = g_player.player.volumeRight & 0x1F;
    }

    if ( SYSkbHit )
    {
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;

        if (pressed)
        {
            u8 scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);

            switch (scancode)
            {
            case HW_KEY_UP:
                BLSgoto(&(g_player.player), g_player.player.trackindex);
                break;

            case HW_KEY_LEFT:
                {
                    s16 index = (s16) g_player.player.trackindex - 1;
                    if (index < 0)
                    {
                        index = (s16) g_player.player.sndtrack->trackLen - 1;
                    }

                    BLSgoto(&(g_player.player), (u8) index);
                }
                break;

            case HW_KEY_RIGHT:
                {
                    u8 index = g_player.player.trackindex + 1;
                    if (index < g_player.player.sndtrack->trackLen)
                    {
                        BLSgoto(&(g_player.player), index);
                    }
                }
                break;

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
                {
                    u8 index = scancode - HW_KEY_F1;
                    if (index < g_player.player.sndtrack->trackLen)
                    {
                        BLSgoto(&(g_player.player), index);
                    }
                }
                break;

            case HW_KEY_1:
            case HW_KEY_2:
            case HW_KEY_3:
            case HW_KEY_4:
                g_player.currentchannel = scancode - HW_KEY_1;
                break;

            case HW_KEY_5:
            case HW_KEY_6:
            case HW_KEY_8:
            case HW_KEY_7:
                g_player.player.voices[scancode - HW_KEY_5].mute ^= 1;
                break;

            case HW_KEY_NUMPAD_7:
            case HW_KEY_NUMPAD_8:
            case HW_KEY_NUMPAD_9:
            case HW_KEY_NUMPAD_4:
            case HW_KEY_NUMPAD_5:
            case HW_KEY_NUMPAD_6:
            case HW_KEY_NUMPAD_1:
            case HW_KEY_NUMPAD_2:
                g_player.player.voices[g_player.currentchannel].volumeoffset = scancode - HW_KEY_NUMPAD_7;
                break;

            case HW_KEY_SPACEBAR:
                g_player.player.voices[g_player.currentchannel].mute ^= 1;
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
                {
                    u16 index = scancode - HW_KEY_Q;
                    
                    if (index < ARRAYSIZE(g_maskValues))
                    {
                        g_player.player.voices[g_player.currentchannel].mask = g_maskValues[index];
                    }
                }
                break;

            case HW_KEY_A:
            case HW_KEY_S:
            case HW_KEY_D:
            case HW_KEY_F:
            case HW_KEY_G:
            case HW_KEY_H:
                {
                    u16 index = scancode - HW_KEY_A + 10;
                    
                    if (index < ARRAYSIZE(g_maskValues))
                    {
                        g_player.player.voices[g_player.currentchannel].mask = g_maskValues[index];
                    }
                }
                break;

            case HW_KEY_RETURN:
                g_player.rastermax = 0;
                break;

            case HW_KEY_ESC:
                FSMgotoNextState(&g_stateMachineIdle);
                break;

            case HW_KEY_BACKSPACE:
                g_player.startdisplay = !g_player.startdisplay;
                break;

            case HW_KEY_NUMPAD_0:
                {
                    u16 t;

                    for (t = 0; t < BLS_NBVOICES ; t++)
                    {
                        g_player.player.voices[t].mask = 0xFFFF;
                    }
                    g_player.player.volumeLeft2   = 20 | HW_MICROWIRE_VOLUME_LEFT;
                    g_player.player.volumeRight2  = 20 | HW_MICROWIRE_VOLUME_RIGHT;
                    break;
                }
            }
        }
    }

    IGNORE_PARAM(_fsm);
}

void PlayerTest	(void)
{
    u16 t, last = 0;
    u16 len = strlen(g_player.filename);
    char* p;
    char filesample[128] = "_logs\\";
    char filetrace [128] = "";
    char fileheap  [128] = "";


#   ifdef __TOS__
#       if blsUSEASM
            strcat(filesample, "ASM\\");
#       else
            strcat(filesample, "C\\");
#       endif
#   else
        strcat(filesample, "PC\\");
#   endif

    for (t = len ; t > 0 ; t--)
    {
        if (g_player.filename[t] == '\\')
        {
            last = t+1;
            break;
        }
    }

    strcat (filesample, &g_player.filename[last]);
    p = strchr(filesample, '.');
    if (p != NULL)
        *p = 0;
	
    strcpy (filetrace, filesample);
    strcpy (fileheap , filesample);

	strcat (filesample, ".RAW");
    strcat (filetrace , ".TXT");
    strcat (fileheap  , ".HEP");

    *HW_COLOR_LUT = 0;

    BLSdumpHeap (g_player.player.sndtrack, fileheap);
	BLStestPlay (&(g_player.player), g_player.player.sndtrack->trackLen - 1, filesample, filetrace);
}


void TextDisplayUpdate (PlayerText* _text)
{
    u8* frame1 = (u8*) g_player.framebuffer + 2;
    u8* frame2;

#   ifdef __TOS__
    frame1 += 72*160;
#   else
    frame1 += 191*160;
#   endif

    frame2 = frame1 + 32000;

    switch (_text->state)
    {
    case TDS_WRITELINE:
        
        _text->animatecount = 0;
        _text->currentchar = g_textdisplayContent[_text->currentline][_text->currentpos];
        
        if (_text->currentchar != 0)
        {
            char temp[2] = "";

            *temp = _text->currentchar;

            SYSdebugPrint(frame1, 160, 2, _text->currentpos, 0, temp);
            SYSdebugPrint(frame2, 160, 2, _text->currentpos, 0, temp);
            _text->currentpos++;
        }
        else
        {
            _text->currentline++;
            _text->currentpos = 0;

            if (_text->currentline >= ARRAYSIZE(g_textdisplayContent))
            {
                _text->currentline = 0;
            }

            _text->state = TDS_WAITLINE;
        }
        break;

    case TDS_WAITLINE:

        _text->animatecount++;

        if (_text->animatecount >= TDS_WAITLINECOUNT)
        {
            STDmset (frame1, 0, 8*160);
            STDmset (frame2, 0, 8*160);
            _text->state = TDS_WRITELINE;
        }
        break;
    }
}


void PlayerBacktask (FSM* _fsm)
{
    static u8 flip = 0;
    u8* backframebuffer = (u8*)g_player.framebuffer;
    

#   ifndef __TOS__
    static u32 framenum = 0;
    if (framenum == g_player.player.framenum)
    {
        return;
    }
    framenum = g_player.player.framenum;
#   endif

    IGNORE_PARAM(_fsm);

    backframebuffer += flip ? 32000 : 0;

    SYSvsync;
    {
        char cleared[4];

        if (g_player.player.buffertoupdate != NULL)
        {
            s8* bufclearflags = g_player.player.buffertoupdate + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
            STDmcpy (g_player.pcmcopy, g_player.player.buffertoupdate, BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD);
            
            cleared[0] = bufclearflags[0];
            cleared[1] = bufclearflags[1];
            cleared[2] = bufclearflags[3];
            cleared[3] = bufclearflags[2];
        }

        {
            u8* line = (u8*) backframebuffer + ((200 - 140) * 160);

#           ifdef __TOS__
            line += 70*160;
#           endif

            drawCurve ( g_player.pcmcopy   , 160, 12, (u8*) line);
            drawCurve ( g_player.pcmcopy+1 , 160, 12, (u8*) line + 40);
            drawCurve ( g_player.pcmcopy+3 , 160, 12, (u8*) line + 80);
            drawCurve ( g_player.pcmcopy+2 , 160, 12, (u8*) line + 120);
            drawXorPass (line);

#           ifndef __TOS__
            line += 160*70;

            drawCurve   (g_player.pcmcopy, 640, 1, line);
            drawXorPass (line);
#           endif
        }

        {
            u8* line = (u8*) backframebuffer + (50 * 160);
            BLSplayer* player = &g_player.player;
            BLSvoice*  voice;
            BLSsoundTrack* sndtrack = g_player.player.sndtrack;
            static char text [] = "trk=  /   pat=   row=   L=   R=   ";
            static char text2[] = " s=   v=  t=  vo=   ";
            u16 t;

            
#           ifdef __TOS__
            line += 50*160;
#           endif

            STDmset(line, 0, 20*160);

            STDuxtoa(&text[4] , player->trackindex, 2);
            STDuxtoa(&text[7] , player->sndtrack->trackLen, 2);
            STDuxtoa(&text[14], player->sndtrack->track[player->trackindex], 2);
            STDuxtoa(&text[21], player->row, 2);
            STDuxtoa(&text[26], g_player.leftvolume, 2);
            STDuxtoa(&text[31], g_player.rightvolume, 2);

            SYSdebugPrint(line, 160, 2, 0, 0, text);

            line += 10 * 160;

            for (t = 0 ; t < 4 ; t++, line += 40, voice++)
            {
                u16* d = (u16*)(line - 20*160);
                
                voice = &player->voices[t];

                text2[0] = t == g_player.currentchannel ? '>' : ' ';
                
                if (voice->samples[0] != NULL)
                {
                    STDuxtoa(&text2[3], voice->samples[0] - sndtrack->samples, 2);
                }
                else
                {
                    text2[3] = ' ';
                    text2[4] = '-';
                }

                if (cleared[t])
                {
                    text2[8] = '*';
                }
                else
                {
                    STDuxtoa(&text2[8], voice->volume, 1);
                }

                if (voice->keys[0] != NULL)
                {
                    STDuxtoa(&text2[12], voice->keys[0]->blitterTranspose, 2);
                }
                else
                {
                    text2[12] = '-';
                    text2[13] = ' ';
                }

                STDuxtoa(&text2[17], voice->volumeoffset  , 1);
                text2[19] = voice->mute ? '*' : ' ';

                SYSdebugPrint(line, 160, 2, 0, 0, text2);

                {
                    u32 m  = voice->mask;
                    if (m != 0xFFFFFFFFUL)
                    {
                        *d = (u16) m;
                    }
                    else
                    {
                        *d = 0;
                    }
                }
            }           
        }

        TextDisplayUpdate (&g_player.text);

        SYSvsync;
        SYSwriteVideoBase ((u32)backframebuffer);
    }
    flip ^= 1;
}

void PlayerExit	(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    BLSfree(&sys.allocatorMem, g_player.player.sndtrack);
    BLSplayerFree(&sys.allocatorMem, &g_player.player);

    RINGallocatorFree(&sys.mem, g_player.framebuffer);
    RINGallocatorFree(&sys.mem, g_player.pcmcopy);

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
    
    g_player.play = false;

    FSMgotoNextState (&g_stateMachineIdle);
}
