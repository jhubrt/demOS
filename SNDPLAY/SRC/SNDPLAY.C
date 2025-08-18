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

#define SNDPLAY_C

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYNTHYMZ.H"
#include "DEMOSDK\YMDISP.H"

#ifndef __TOS__
#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"
#endif

#include "SNDPLAY\SRC\SNDPLAY.H"

#include <time.h>

#define SNDPLAY_TITLE "SNDplay"


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


static void playerDebugPrint(void* _screen, u16 _col, u16 _y, char* _s)
{
    SYSdebugPrint(_screen             , 160, 2, _col, _y, _s);
    SYSdebugPrint((u8*)_screen + 32000, 160, 2, _col, _y, _s);
}

static void playerInit (bool _deltadecode, u16 _i, u16 _nb, u32 _sampleHeapSize, u32 _sampleheap[2])
{
    u8* framebuffer = (u8*)SYSreadVideoBase();
    char temp[] = "   /   ";


    IGNORE_PARAM(_deltadecode);

    STDuxtoa(temp, _i, 3);
    STDuxtoa(&temp[4], _nb, 3);
    SYSdebugPrint(framebuffer, 160, 2, 0, 16, temp);
}


void PlayerEntry (void)
{
    RINGallocatorFreeArea info;

    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;
   
    g_player.run = true;
    g_player.play = true;
    g_player.playframe = 0;
    
	g_player.lastpanel = 0xFF;

    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes = info.size;

    g_player.data = STDloadfile (&sys.allocatorMem, g_player.filename, &g_player.datasize);

    RINGallocatorFreeSize(&sys.mem, &info);
	g_player.allocatedbytes -= info.size;
    
    g_player.framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (g_player.framebuffer != NULL);
    STDmset (g_player.framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)g_player.framebuffer);

    HW_COLOR_LUT[0] = 0;
    HW_COLOR_LUT[1] = PCENDIANSWAP16(0xFFF);
}


void PlayerActivity	(void)
{  
    if ( SYSkbHit )
    {
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;

        if (pressed)
        {
            u8 scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);

            switch (scancode)
            {
            case HW_KEY_SPACEBAR:
                if (g_player.play == false)
                {
                    g_player.play = true;
                    g_player.dataindex = 0;
                    g_player.playframe = 0;
                }
                break;

            case HW_KEY_ESC:
                g_player.run = false;
                break;

            case HW_KEY_NUMPAD_MINUS:
            case HW_KEY_X:
                g_player.panel++;
                if (g_player.panel == PLAYER_NB_PANELS)
                    g_player.panel = 0;
                break;
            }
        }
    }

    {
#       ifndef __TOS__
        static u8 buffer[1000 * 2];
        u8 playBuffer = EMULgetPlayOffset () >= (1000 * 2);
        {
            static u8 oldBuffer = 0;

            if (oldBuffer == playBuffer)
                return;

            oldBuffer = playBuffer;
        }

        EMULplaysound (buffer, 1000 * 2, playBuffer != 0 ? 0 : 1000 * 2);

        if ( WINisClosed(EMULgetWindow()) )
            g_player.run = false;

#       endif
    }

    if (g_player.play)
    {
        u8* p = g_player.data + g_player.dataindex;
        u16 frame = (p[0] << 8) | p[1];

        if (frame == g_player.playframe)
        {
            p += 2;

            printf ("frame=%d index=%d\n", g_player.playframe, g_player.dataindex);

            g_player.dataindex = SNDblitzDecodeYM(p) - g_player.data;

            if (g_player.dataindex >= g_player.datasize)
            {
                g_player.play = false;
            }
        }

        g_player.playframe++;
    }
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
}


static void playerDrawPanel0Background(void)
{
	char temp[] = "alloc=        ";
	u8* framebuffer = (u8*)g_player.framebuffer;

    playerDebugPrint(framebuffer, 0, 0, SNDPLAY_TITLE);

	STDutoa(&temp[6], g_player.allocatedbytes, 7);
    playerDebugPrint (framebuffer, 66, 0, temp);

    g_player.text.currentpos = 0;

#   ifndef __TOS__
	printf("bytes allocated= %d\n", g_player.allocatedbytes);
    EMULfbStdEnable();
    EMULcls();
#   endif
}

static void playerDrawPanel0(u8* backframebuffer)
{
#   ifdef __TOS__
    u8 regsstate[16];

    void aYMgetState (u8 _regs[14]);
    aYMgetState(regsstate);
#   else
    u8* regsstate = &g_STHardware.reg_HW_YM_REGDATA;
#   endif

    SNDYMdrawYMstate(regsstate, backframebuffer + 130 * 160 + 8);
}

static void playerDrawPanel1Background(void)
{
    EMULfbDisableAll();
    EMULcls();
}

static void playerDrawPanel1(u8* backframebuffer)
{
    IGNORE_PARAM(backframebuffer);

    EMULdrawYMbuffer (-320, 40, g_player.ymemuldisplaysync); 
}


void PlayerBacktask (void)
{
    static u8 flip = 0;
    u8* backframebuffer = (u8*)g_player.framebuffer;
    

    backframebuffer += flip ? 32000 : 0;

    if (g_player.panel != g_player.lastpanel)
    {
		STDmset (g_player.framebuffer, 0, 64000UL);

		switch (g_player.panel)
		{
		case 0:	playerDrawPanel0Background(); break;
		case 1: playerDrawPanel1Background(); break;
		}

		g_player.lastpanel = g_player.panel;
    }

	switch (g_player.panel)
	{
	case 0:	playerDrawPanel0(backframebuffer); break;
	case 1: playerDrawPanel1(backframebuffer); break;
	}

    SYSwriteVideoBase ((u32)backframebuffer);
    flip ^= 1;
}


void PlayerExit	(void)
{
    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    MEM_FREE(&sys.allocatorMem, g_player.data);

    RINGallocatorFree(&sys.mem, g_player.framebuffer);
    
    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
}

