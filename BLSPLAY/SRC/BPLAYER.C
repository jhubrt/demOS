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

#include "DEMOSDK\BLSSND.H"

#include "BLSPLAY\SRC\SCREENS.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#include <time.h>

#define BLSPLAY_TITLE "BLSplay"

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


enum TextdisplayState_
{
    TDS_WRITELINE,
    TDS_WAITLINE
};
typedef enum TextdisplayState_ TextdisplayState;

#ifdef __TOS__
#   define TDS_WAITLINECOUNT   50
#   define PLAYER_NB_PANELS    4
#else
#   define TDS_WAITLINECOUNT   100
#   define PLAYER_NB_PANELS    5
#endif

char* g_textdisplayContent[] =
{
    "BLITsnd by Metal Ages from CYBERNETICS sound routine using blitter",
    "BLITsnd provides 4 PCM + 3 YM voices [uses up to 16% of VBL on 8mhz STe]",
    "BLSconvert converts MOD/XM format into BLS format [with constraints...]",
    "BLSplay plays intermediate .BLS files or .BLZ optimized files",
    "Thanks fly to NPomarede for his great advices about DMA sound",
    "Greetings go to all friends and kind people from the demoscene" 
};


static void playerInit (bool _deltadecode, u16 _i, u16 _nb)
{
    u8* framebuffer = (u8*)SYSreadVideoBase();
    char temp[] = "   /   ";


    IGNORE_PARAM(_deltadecode);

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

    
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;

    DEFAULT_CONSTRUCT(&g_player.player);
    
    g_player.play = true;
    
	g_player.lastpanel = 0xFF;

    g_player.currentchannel = 0;
    g_player.currentmask = 0;

    g_player.leftvolume  = 20;
    g_player.rightvolume = 20;

    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes = info.size;

    {
        char lastchar = g_player.filename[ strlen(g_player.filename) - 1];
        void* buffer;

        buffer = STDloadfile (&sys.allocatorMem, g_player.filename, NULL);

        if ((lastchar == 'Z') || (lastchar == 'z'))
        {
            g_player.playerinterface.read       = BLZread;
            g_player.playerinterface.init       = BLSinit;
            g_player.playerinterface.playerInit = BLZplayerInit;
            g_player.playerinterface.update     = g_player.dmaplayonce ? BLZ2update : BLZupdate;
            g_player.playerinterface.updAsync   = BLZupdAsync;
            g_player.playerinterface.playerFree = BLZplayerFree;
            g_player.playerinterface.free       = BLZfree;
            g_player.playerinterface.gotoindex  = BLZgoto;
            g_player.playerinterface.testPlay   = BLZtestPlay;
        }
        else
        {
#           if BLS_SCOREMODE_ENABLE
            g_player.playerinterface.read       = BLSread;
            g_player.playerinterface.init       = BLSinit;
            g_player.playerinterface.playerInit = BLSplayerInit;
            g_player.playerinterface.update     = BLSupdate;
            g_player.playerinterface.updAsync   = BLSupdAsync;
            g_player.playerinterface.playerFree = BLSplayerFree;
            g_player.playerinterface.free       = BLSfree;
            g_player.playerinterface.gotoindex  = BLSgoto;
            g_player.playerinterface.testPlay   = BLStestPlay;
#           else
            ASSERT("Can not play .BLS file with ASM player" == 0);
#           endif
        }

        g_player.playerinterface.read(&sys.allocatorMem, &sys.allocatorMem, buffer, &sndtrack);

        MEM_FREE(&sys.allocatorMem, buffer);
    }

    g_player.playerinterface.init (&sys.allocatorMem, &sys.allocatorMem, sndtrack, playerInit);

    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes -= info.size;
    
    g_player.framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (g_player.framebuffer != NULL);
    STDmset (g_player.framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)g_player.framebuffer);

    g_player.pcmcopy = (s8*) RINGallocatorAlloc ( &sys.mem, PCMCOPYSIZE);
    ASSERT (g_player.pcmcopy != NULL);
    STDmset(g_player.pcmcopy, 0, PCMCOPYSIZE);

    {
        BLZdmaMode mode = BLZ_DMAMODE_LOOP;

        if (g_player.testMode)
            mode = BLZ_DMAMODE_NOAUDIO;
        else if (g_player.dmaplayonce)
            mode = BLZ_DMAMODE_PLAYONCE;

        g_player.playerinterface.playerInit (&sys.allocatorMem, &g_player.player, sndtrack, mode);
    }
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
        if (g_player.dmaplayonce)
            *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYONCE;
        g_player.playerinterface.update (&(g_player.player));
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

        if (g_player.dmaplayonce)
            *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_PLAYONCE;
        g_player.playerinterface.update (&(g_player.player));

        *HW_COLOR_LUT = 0x34;
        g_player.rastermax = TRACmaxraster(g_player.rastermax);
    }

    *HW_COLOR_LUT = 0;
    
#   ifdef __TOS__
	if ((g_player.panel == 0) && (g_player.dmaplayonce == false))
	{
        u16 mask = (g_player.player.dmabufend - g_player.player.dmabufstart) > BLS_NBBYTES_PERFRAME ? 0xFFFF : 0;
        u8* framebuffer = (u8*)g_player.framebuffer;

        *(u16*)(framebuffer +         (74 * 2 + 160 * 18))           = mask;
        *(u16*)(framebuffer + 32000 + (74 * 2 + 160 * 18))           = mask;

        mask = ~mask;

        *(u16*)(framebuffer +         (74 * 2 + 160 * 18 + 160 * 7)) = mask;
        *(u16*)(framebuffer + 32000 + (74 * 2 + 160 * 18 + 160 * 7)) = mask;
    }
#   else
    if ( WINisClosed(EMULgetWindow()) )
    {
        FSMgotoNextState(&g_stateMachine);
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
                g_player.playerinterface.gotoindex(&(g_player.player), g_player.player.trackindex);
                break;

            case HW_KEY_LEFT:
                {
                    s16 index = (s16) g_player.player.trackindex - 1;
                    if (index < 0)
                    {
                        index = 0;
                    }

                    g_player.playerinterface.gotoindex(&(g_player.player), (u8) index);
                }
                break;

            case HW_KEY_RIGHT:
                {
                    u8 index = g_player.player.trackindex + 1;
                    if (index < g_player.player.sndtrack->trackLen)
                    {
                        g_player.playerinterface.gotoindex(&(g_player.player), index);
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
                        g_player.playerinterface.gotoindex(&(g_player.player), index);
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
			case HW_KEY_7:
			case HW_KEY_8:
                g_player.player.voices[scancode - HW_KEY_5].mute ^= 1;
                break;

#if BLS_SCOREMODE_ENABLE
            case HW_KEY_9:
			case HW_KEY_0:
			case HW_KEY_MINUS:
				g_player.player.ymplayer.commands[scancode - HW_KEY_9].mute ^= 1;
				break;
#endif

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
                FSMgotoNextState(&g_stateMachine);
                FSMgotoNextState(&g_stateMachineIdle);
                break;

            case HW_KEY_BACKSPACE:
                g_player.ymemuldisplaysync = !g_player.ymemuldisplaysync;
                g_player.startdisplay = !g_player.startdisplay;
                break;

            case HW_KEY_HOME:
                g_player.playerinterface.gotoindex(&(g_player.player), 0);
                /* NO BREAK HERE !!! */

            case HW_KEY_NUMPAD_0:
                {
                    u16 t;

                    for (t = 0; t < BLS_NBVOICES ; t++)
                    {
                        g_player.player.voices[t].mask = 0xFFFF;
                    }
                    g_player.player.volumeLeft2   = 20 | HW_MICROWIRE_VOLUME_LEFT;
                    g_player.player.volumeRight2  = 20 | HW_MICROWIRE_VOLUME_RIGHT;

#                   if BLS_SCOREMODE_ENABLE
                    SNDYMstop(&g_player.player.ymplayer);
#                   else
                    HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, 0xFF);
                    HW_YM_SET_REG(HW_YM_SEL_LEVELCHA,     0   );
                    HW_YM_SET_REG(HW_YM_SEL_LEVELCHB,     0   );
                    HW_YM_SET_REG(HW_YM_SEL_LEVELCHC,     0   );
#                   endif

                    break;
                }

            case HW_KEY_NUMPAD_MINUS:
			case HW_KEY_X:
                g_player.panel++;
                if (g_player.panel == PLAYER_NB_PANELS)
                    g_player.panel = 0;
                break;
            }
        }
    }

    IGNORE_PARAM(_fsm);
}

void PlayerTest	(u8 _mode)
{
    u16 t, last = 0;
    u16 len = (u16) strlen(g_player.filename);
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
#   ifdef __TOS__
    {
        char *p = strchr(filesample, '.');
        if (p != NULL)
            *p = 0;
    }
#   endif

    strcpy (filetrace, filesample);
    strcpy (fileheap , filesample);

	strcat (filesample, ".RAW");
    strcat (fileheap  , ".HEP");
    if (_mode == 2)
        strcat (filetrace , "2");
    strcat (filetrace , ".TXT");

    *HW_COLOR_LUT = 0;

    BLSdumpHeap (g_player.player.sndtrack, fileheap);
    g_player.playerinterface.testPlay (&(g_player.player), filesample, filetrace, _mode);
}


void TextDisplayUpdate (PlayerText* _text)
{
    u8* frame1 = (u8*) g_player.framebuffer + 2;
    u8* frame2;
    char currentchar;

    
	frame1 += 191*160;
    frame2 = frame1 + 32000;

    switch (_text->state)
    {
    case TDS_WRITELINE:
        
        currentchar = g_textdisplayContent[_text->currentline][_text->currentpos];
        
        if (currentchar != 0)
        {
            char temp[2] = "";

            *temp = currentchar;

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

            _text->animatecount = 0;

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


static void playerDrawPanel0Background(void)
{
	char temp[] = "alloc=        ";
	u8* framebuffer = (u8*)g_player.framebuffer;

	SYSdebugPrint(framebuffer, 160, 2, 0, 0, BLSPLAY_TITLE);
	SYSdebugPrint(framebuffer + 32000, 160, 2, 0, 0, BLSPLAY_TITLE);

	STDutoa(&temp[6], g_player.allocatedbytes, 7);
	SYSdebugPrint(framebuffer, 160, 2, 66, 0, temp);
	SYSdebugPrint(framebuffer + 32000, 160, 2, 66, 0, temp);

    g_player.text.currentpos = 0;

    if (g_player.dmaplayonce == false)
    {
        SYSdebugPrint(framebuffer, 160, 2, 66, 18, "DMAsync");
	    SYSdebugPrint(framebuffer + 32000, 160, 2, 66, 18, "DMAsync");
    }
#   ifndef __TOS__
	printf("bytes allocated= %d\n", g_player.allocatedbytes);
    EMULfbStdEnable();
    EMULcls();
#   endif
}

static void playerDrawPanel0(u8* backframebuffer)
{
	u8* line = (u8*)backframebuffer + (58 * 160);
	char cleared[4];
	BLSplayer* player = &g_player.player;
	BLSvoice* voice;
	BLSsoundTrack* sndtrack = g_player.player.sndtrack;
	static char text [] = "trk=  /   pat=   row=   L=   R=   ";
	static char text2[] = " k=   v=  t=   vo= ";
    static char text3[] = " i=   o=  k=  ";
	u16 t;
	u16* m;


	if (g_player.player.buffertoupdate != NULL)
	{
		s8* bufclearflags = g_player.player.buffertoupdate + BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD;
		STDmcpy2 (g_player.pcmcopy, g_player.player.buffertoupdate, BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD);

		cleared[0] = bufclearflags[0];
		cleared[1] = bufclearflags[1];
		cleared[2] = bufclearflags[3];
		cleared[3] = bufclearflags[2];
	}

	line += 2;
	drawCurve(g_player.pcmcopy    , 160, 12, (u8*)line);
	drawCurve(g_player.pcmcopy + 1, 160, 12, (u8*)line + 40);
	drawCurve(g_player.pcmcopy + 3, 160, 12, (u8*)line + 80);
	drawCurve(g_player.pcmcopy + 2, 160, 12, (u8*)line + 120);
	drawXorPass(line);

	m = (u16*)(line + 16 + 160 * 30);

	for (t = 0; t < 4; t++, m += 20)
	{
		if (g_player.player.voices[t].mute)
		{
			m[0] = 0xFFFF;
			m[80] = PCENDIANSWAP16(0x8001);
			m[160] = PCENDIANSWAP16(0x8661);
			m[240] = PCENDIANSWAP16(0x8181);
			m[320] = PCENDIANSWAP16(0x8181);
			m[400] = PCENDIANSWAP16(0x8661);
			m[480] = PCENDIANSWAP16(0x8001);
			m[560] = 0xFFFF;
		}
	}

	line = (u8*)backframebuffer + (50 * 160);
	STDmset(line, 0, 20 * 160);

    STDuxtoa(&text[4], player->trackindex, 2);
    STDuxtoa(&text[7], player->sndtrack->trackLen, 2);
    STDuxtoa(&text[14], player->sndtrack->track[player->trackindex], 2);

    if (BLSisBlitzSndtrack(player->sndtrack))
    {
        text[18] = '=';
        STDuxtoa(&text[19], player->patternend - player->blizcurrent, 4);
    }
#   if BLS_SCOREMODE_ENABLE
    else
    {
        STDuxtoa(&text[21], player->row, 2);
    }
#   endif

	STDuxtoa(&text[26], g_player.leftvolume, 2);
	STDuxtoa(&text[31], g_player.rightvolume, 2);

    line = (u8*)backframebuffer + (18 * 160);

	SYSdebugPrint(line, 160, 2, 0, 0, text);

	line += 20 * 160;

	for (t = 0; t < 4; t++, line += 40, voice++)
	{
		u16* d = (u16*)(line - 20 * 160);

		voice = &player->voices[t];

		text3[0] = t == g_player.currentchannel ? '>' : ' ';

		if (voice->keys[0] != NULL)
        {
            static char* keynames = "C C+D D+E F F+G G+A A+B ";
            u16 offset      = voice->keys[0] - sndtrack->keys;
            u8  noteinfo    = sndtrack->keysnoteinfo[offset];
            u16 sampleindex = voice->keys[0]->sampleIndex;
            u8  octave      = noteinfo >> 4;
            u8  semitone    = noteinfo & 0xF;

            ASSERT(offset < sndtrack->nbKeys);
            ASSERT(semitone < 12);

            if (BLS_IS_BASEKEY(voice->keys[0]) == false)
            {
                sampleindex = sndtrack->keys[sampleindex].sampleIndex;
            }
                
            STDuxtoa(&text2[3], offset, 2);
                        
            STDuxtoa(&text3[3],  sampleindex    , 2);
            STDuxtoa(&text3[8],  octave         , 1);
            
            semitone <<= 1;

            text3[13] = keynames[semitone];
            text3[14] = keynames[semitone+1];
        }
		else
		{
			text2[3]  = ' ';
			text2[4]  = ' ';
        
            text3[3]  = ' ';
            text3[4]  = ' ';
            text3[8]  = ' ';
            text3[13] = ' ';
            text3[14] = ' ';
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

		STDuxtoa(&text2[18], voice->volumeoffset, 1);

		SYSdebugPrint(line      , 160, 2, 0, 0, text3);
        SYSdebugPrint(line+8*160, 160, 2, 0, 0, text2);

		{
			u32 m = voice->mask;
			if (m != 0xFFFFFFFFUL)
			{
				d[40*80+2] = (u16)m;
			}
			else
			{
				d[40*80+2] = 0;
			}
		}
	}

	TextDisplayUpdate(&g_player.text);

#   if BLS_SCOREMODE_ENABLE
    if (BLSisBlitzSndtrack(g_player.player.sndtrack) == false)
    {
        BLSsoundTrack* sndtrack = (BLSsoundTrack*)g_player.player.sndtrack;
        SNDYMcommand* command = g_player.player.ymplayer.commands;

        m = (u16*)(backframebuffer + 130 * 160 + 2);

        for (t = 0; t < SND_YM_NB_CHANNELS; t++, command++)
        {
            if (command->mute)
            {
                m[0] = 0xFFFF;
                m[80] = PCENDIANSWAP16(0x8001);
                m[160] = PCENDIANSWAP16(0x8661);
                m[240] = PCENDIANSWAP16(0x8181);
                m[320] = PCENDIANSWAP16(0x8181);
                m[400] = PCENDIANSWAP16(0x8661);
                m[480] = PCENDIANSWAP16(0x8001);
                m[560] = 0xFFFF;
            }
            else
            {
                SNDYMchannel* chn = &g_player.player.ymplayer.channels[t];
                u8 soundindex = command->soundindex;
                u16 mask = 0;

                if (soundindex < sndtrack->YMsoundSet.nbSounds)
                {
                    u8 nbcurves = sndtrack->YMsoundSet.sounds[soundindex].nbcurves;
                    u16 c;

                    for (c = 0; c < nbcurves; c++)
                    {
                        if (chn->curvestate[c].running)
                        {
                            mask = 0xFFFF;
                            break;
                        }
                    }
                }

                m[0] = mask;
                m[80] = mask;
                m[160] = mask;
                m[240] = mask;
                m[320] = mask;
                m[400] = mask;
                m[480] = mask;
                m[560] = mask;
            }
    
            m += 19 * 80;
        }
    }

#   endif /* BLS_SCOREMODE_ENABLE */

    {
#       ifdef __TOS__
        u8 regsstate[16];

        void aYMgetState (u8 _regs[14]);
        aYMgetState(regsstate);
#       else
        u8* regsstate = &g_STHardware.reg_HW_YM_REGDATA;
#       endif

        SNDYMdrawYMstate(regsstate, backframebuffer + 130 * 160 + 8);
    }
}



static void playerDrawPanel1Background(void)
{
#   if BLS_SCOREMODE_ENABLE
    if (BLSisBlitzSndtrack(g_player.player.sndtrack))
    { 
        g_player.panel++;
    }
#   else
    g_player.panel++;
#   endif
}

static void playerDrawPanel1(u8* backframebuffer)
{
#   if BLS_SCOREMODE_ENABLE
    if (BLSisBlitzSndtrack(g_player.player.sndtrack) == false)
    {
        static bool active[SND_YM_NB_CHANNELS];

        active[0] = !g_player.player.ymplayer.commands[0].mute;
        active[1] = !g_player.player.ymplayer.commands[1].mute;
        active[2] = !g_player.player.ymplayer.commands[2].mute;

        STDmset(backframebuffer, 0, 160 * 48 * SND_YM_NB_CHANNELS);
        SNDYMdrawSoundCurves(&g_player.player.ymplayer, active, backframebuffer);
    }
#   endif

    IGNORE_PARAM(backframebuffer);
}

static void playerDrawPanel2Background(void)
{
}

static void playerDrawPanel2(u8* backframebuffer)
{
	u8* line = backframebuffer;

	STDmcpy2 (g_player.pcmcopy, g_player.player.buffertoupdate, BLS_NBBYTES_PERFRAME + BLS_NBBYTES_OVERHEAD);
	STDmset(backframebuffer, 0, 160 * 68);

	drawCurve(g_player.pcmcopy, 640, 1, line);
	drawXorPass(line);
}

static void playerDrawPanel3Background(void)
{
	char temp[3];
	u8 t;
	u8* adr = (u8*)g_player.framebuffer;

	temp[2] = 0;

	for (t = 0; t < 20; t++, adr += 5 * 160)
	{
		if (t & 1)
		{
			STDmset(adr + 16, 0x55555555UL, 112);
			STDmset(adr + 32016, 0x55555555UL, 112);
		}
		else
		{
			STDmset(adr + 8, 0xFFFFFFFFUL, 120);
			STDmset(adr + 32008, 0xFFFFFFFFUL, 120);

			STDutoa(temp, t * 5, 2);
			SYSfastPrint(temp, adr, 160, 4, (u32)&SYSfont);
			SYSfastPrint(temp, adr + 32000, 160, 4, (u32)&SYSfont);
		}
	}
}

static void playerDrawPanel3(u8* backframebuffer)
{
	IGNORE_PARAM(backframebuffer);
}


static void playerDrawPanel4Background(void)
{
    EMULfbDisableAll();
    EMULcls();
}

static void playerDrawPanel4(u8* backframebuffer)
{
    IGNORE_PARAM(backframebuffer);

    EMULdrawYMbuffer (-320, 40, g_player.ymemuldisplaysync); 
}


void PlayerBacktask (FSM* _fsm)
{
    static u8 flip = 0;
    u8* backframebuffer = (u8*)g_player.framebuffer;
    
    IGNORE_PARAM(_fsm);

    backframebuffer += flip ? 32000 : 0;

    SYSvsync;

    if (g_player.panel != g_player.lastpanel)
    {
		STDmset (g_player.framebuffer, 0, 64000UL);

		switch (g_player.panel)
		{
		case 0:	playerDrawPanel0Background(); break;
		case 1: playerDrawPanel1Background(); break;
		case 2: playerDrawPanel2Background(); break;
		case 3: playerDrawPanel3Background(); break;
        case 4: playerDrawPanel4Background(); break;
		}

		g_player.lastpanel = g_player.panel;
    }

	switch (g_player.panel)
	{
	case 0:	playerDrawPanel0(backframebuffer); break;
	case 1: playerDrawPanel1(backframebuffer); break;
	case 2: playerDrawPanel2(backframebuffer); break;
	case 3: playerDrawPanel3(backframebuffer); break;
    case 4: playerDrawPanel4(backframebuffer); break;
	}

    SYSwriteVideoBase ((u32)backframebuffer);
    flip ^= 1;
}

void PlayerExit	(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    g_player.playerinterface.free(&sys.allocatorMem, g_player.player.sndtrack);
    g_player.playerinterface.playerFree(&sys.allocatorMem, &g_player.player);

    RINGallocatorFree(&sys.mem, g_player.framebuffer);
    RINGallocatorFree(&sys.mem, g_player.pcmcopy);
    
    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
    
    g_player.play = false;

    FSMgotoNextState (&g_stateMachineIdle);
}
