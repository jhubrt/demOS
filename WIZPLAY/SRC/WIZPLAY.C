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

#include "EXTERN\WIZZCAT\PRTRKSTE.H"

#include "WIZPLAY\SRC\SCREENS.H"

#define WIZPLAY_TITLE "WIZplay 1.0.0"

#   define drawXorPass  adrawXorPass
#   define drawCurve    adrawCurve
void adrawXorPass (void* _screen);
void drawCurve (void* _sample, u16 _nbsamples, u16 _incx, void* _screen);

enum TextdisplayState_
{
    TDS_WRITELINE,
    TDS_WAITLINE
};
typedef enum TextdisplayState_ TextdisplayState;

#define PLAYER_NB_PANELS    2

/* 25khz (5 div) 100 + 63  */
/* 25khz (4 div) 105 + 63  */
/* 12.5khz (2 div) 38 + 63 */


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


void PlayerEntry (void)
{
    (*HW_VIDEO_MODE) = HW_VIDEO_MODE_2P;
   
    g_player.play = true;
    
	g_player.lastpanel = 0xFF;

   {
        void* buffer = NULL;
        u32   length = 0;

        FILE* file = fopen(g_player.filename, "rb");
        ASSERT(file != NULL);

        fseek (file, 0, SEEK_END);
        length = ftell(file);
        fseek (file, 0, SEEK_SET);

        buffer = MEM_ALLOCTEMP(&sys.allocatorMem, length + 64UL*1024UL);
        ASSERT(buffer != NULL);

        fread(buffer, 1, length, file);

        fclose(file);

        g_player.modulebuffer    = buffer;
        g_player.modulebufferend = g_player.modulebuffer + length + 128UL*1024UL;
    }
   
    g_player.framebuffer = RINGallocatorAlloc ( &sys.mem, 64000UL );
    ASSERT (g_player.framebuffer != NULL);
    STDmset (g_player.framebuffer, 0, 64000UL);
    SYSwriteVideoBase ((u32)g_player.framebuffer);

    g_player.pcmcopy = (s8*) RINGallocatorAlloc ( &sys.mem, PCMCOPYSIZE);
    ASSERT (g_player.pcmcopy != NULL);
    STDmset(g_player.pcmcopy, 0, PCMCOPYSIZE);

    WIZinit();    
    WIZmodInit(g_player.modulebuffer, g_player.modulebufferend);
    WIZplay();

    SYSvblroutines[0] = (SYSinterupt)WIZstereo;
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
    *HW_COLOR_LUT = 0x33;
    g_player.rastermax = TRACmaxraster(g_player.rastermax);
    *HW_COLOR_LUT = 0;

    if ( SYSkbHit )
    {
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;

        if (pressed)
        {
            u8 scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);

            switch (scancode)
            {
            case HW_KEY_RETURN:
                g_player.rastermax = 0;
                break;

            case HW_KEY_NUMPAD_MINUS:
                g_player.panel++;
                if (g_player.panel == PLAYER_NB_PANELS)
                    g_player.panel = 0;
                break;
            }
        }
    }

    IGNORE_PARAM(_fsm);
}

static void playerDrawPanel0Background(void)
{
    u8* framebuffer = (u8*)g_player.framebuffer;
    
    SYSfastPrint(WIZPLAY_TITLE,framebuffer, 160, 4, (u32)&SYSfont);
    SYSfastPrint(WIZPLAY_TITLE,framebuffer + 32000, 160, 4, (u32)&SYSfont);
}

static void playerDrawPanel0(u8* backframebuffer)
{
	u8* line = (u8*)backframebuffer + (58 * 160);

    STDmcpy2 (g_player.pcmcopy, WIZbackbuf, 2000);

	line += 2;
	drawCurve(g_player.pcmcopy    , 160, 12, (u8*)line);
	drawCurve(g_player.pcmcopy + 1, 160, 12, (u8*)line + 40);
	drawCurve(g_player.pcmcopy + 3, 160, 12, (u8*)line + 80);
	drawCurve(g_player.pcmcopy + 2, 160, 12, (u8*)line + 120);
	drawXorPass(line);
}


static void playerDrawPanel1(u8* backframebuffer)
{
	u8* line = backframebuffer;

	STDmcpy2 (g_player.pcmcopy, WIZbackbuf, 2000);
	
    line += (199-70) * 160;
    
    drawCurve(g_player.pcmcopy, 640, 1, line);
	drawXorPass(line);
}

static void playerDrawPanel1Background(void)
{
    char temp[4];
    u8 t;
    u8* adr = (u8*)g_player.framebuffer;

    temp[3] = 0;

    for (t = 0; t < 30; t++, adr += 5 * 160)
    {
        if (t & 1)
        {
            STDmset(adr + 96, 0x55555555UL,    64);
            STDmset(adr + 32096, 0x55555555UL, 64);
        }
        else
        {
            STDmset(adr + 88, 0xFFFFFFFFUL, 72);
            STDmset(adr + 32088, 0xFFFFFFFFUL, 72);

            STDutoa(temp, t * 5, 3);
            SYSfastPrint(temp, adr + 80, 160, 4, (u32)&SYSfont);
            SYSfastPrint(temp, adr + 32080, 160, 4, (u32)&SYSfont);
        }
    }
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
        case 1:	playerDrawPanel1Background(); break;
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

void PlayerExit	(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;

    RINGallocatorFree(&sys.mem, g_player.framebuffer);
    RINGallocatorFree(&sys.mem, g_player.pcmcopy);
    
    ASSERT( RINGallocatorIsEmpty(&sys.mem) );
    
    g_player.play = false;

    FSMgotoNextState (&g_stateMachineIdle);
}
