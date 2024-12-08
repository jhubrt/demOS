/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2024 J.Hubert

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

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\LOAD.H"

#include "DEMOSDK\PC\EMUL.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\LOADER\PRELOAD.H"

#include "RELAPSE\RELAPSE1.H"
#include "RELAPSE\RELAPSE2.H"


#ifdef DEMOS_LOAD_FROMHD

void PreLoadEntry (FSM* _fsm)
{
    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState (_fsm);
}

void PreLoadActivity (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);
}

void PreLoadExit (FSM* _fsm)
{
    FSMgotoNextState(&g_stateMachine);
    FSMgotoNextState(_fsm);
}

#else

#define PRELOAD_FRAMEBUFFER_SIZE 32000UL

static void preloadUpdateText(PreLoad* this)
{
    char* text = this->text;
    bool haschanged = false;

    if (g_screens.loader->lastprogress != g_screens.persistent.initprogress)
    {
        g_screens.loader->lastprogress = g_screens.persistent.initprogress;
        haschanged = true;
    }

    if (g_screens.persistent.initprogress > 0)
        STDuxtoa (text, (u32)g_screens.persistent.initprogress, 1);
    else
        text[0] = ' ';

    text[1] = ' ';

    {
        u16 nbsectors = 0;
        u16 t;

        for (t = 0 ; t < RELAPSE_NBMAX_LOADREQUESTS ; t++)
        {
            if (g_screens.persistent.loadRequest[t] != NULL)
            {
                nbsectors = g_screens.persistent.loadRequest[t]->nbsectors >> 2;

                if (nbsectors != 0)
                    break;
            }
        }

        if (g_screens.loader->lastnbsectors != nbsectors)
        {
            g_screens.loader->lastnbsectors = nbsectors;
            haschanged = true;
        }

        if (nbsectors > 0)
            STDuxtoa (text + 2, (u32)nbsectors, 2);
        else 
        {
            text[2] = text[3] = ' ';
        }
    }

    text[4] = ' ';

    {
        u32 delta = 0;

        if (SYSdpakActive)
        {
            delta = (SYSdpakProgress - g_screens.persistent.dpackbase) >> 11;
            STDuxtoa(text + 5, delta, 2);
        }
        else
        {
            text[5] = text[6] = ' ';
        }

        if (g_screens.loader->lastdpakprogress != delta)
        {
            g_screens.loader->lastdpakprogress = delta;
            haschanged = true;
        }
    }

    {
        u8 sec = (u8)(this->nbframes / 50);

        text[7] = ' ';

        if (sec != this->lastsec)
        {
            STDuxtoa(text + 8, sec, 2);
            this->lastsec = sec;
            haschanged = true;
        }        
        this->nbframes++;

        text[10] = 0;
    }

    if (haschanged)
    {
        SYSfastPrint(text, this->framebuffer + 182*160 + 8 + 6, 320, 8, (u32)&SYSfont);
    }
}


static void preloadPrint(PreLoad* this, char* text_, u16 x_, u16 line_, u16 color_)
{
    u8* d = (u8*) this->line;
    u8* s;
    u16 t;

    STDmset(this->line, 0UL, sizeof(this->line));

    color_ <<= 1;

    d += (x_ & 0xFFFE) << 2;
    d += x_ & 1;

    for (t = 1 ; t < PRELOAD_NBPLANES ; t++)
    {
        d += 2;
        if ((1 << t) & color_)
            SYSfastPrint(text_, d, 160, PRELOAD_NBPLANES * 2, (u32)&SYSfont);
    }

    s = (u8*) this->line;
    d = this->framebuffer + line_ * 160;

    for (t = 0 ; t < 8 ; t++)
    {
        STDmcpy2(d    , s, 160);
        STDmcpy2(d+160, s, 160);

        {
            u16 i;
            u16* sl = (u16*) s;
            u16* dl = (u16*) (d + 160);

            for (i = 0; i < 20; i++)
            {
                *dl = sl[1] | sl[2] | sl[3];

                sl += PRELOAD_NBPLANES;
                dl += PRELOAD_NBPLANES;
            }
        }

        s += 160;
        d += 480;
    }
}

static void preloadWait(u16 nbframes_)
{
    while (--nbframes_)
    {
        SYSvsync;
    }
}

static void preloadCallback(LOADrequest* _request, void* _clientData)
{
    PreLoad* this = (PreLoad*) _clientData;
    char temp[] = "       ";


    STDutoa(temp    , _request->track, 2);
    STDutoa(temp + 3, _request->nbsectors, 4);

    preloadPrint(this, temp, 16, this->y, this->printcolor);
}

static void preloadFadeIn(PreLoad* this, u16 color_)
{
    this->inc[color_ - 1] = 1;
}

static void preloadFadeOut(PreLoad* this, u16 color_)
{
    this->inc[color_ - 1] = -1;
}

void PreLoadEntry (FSM* _fsm)
{
    PreLoad* this;
    u16 t;
    u8 scancode;
    bool allowpreload = true;


    IGNORE_PARAM(_fsm);

    g_screens.preload = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, PreLoad );
    DEFAULT_CONSTRUCT(this);

    STDmset (HW_COLOR_LUT, 0UL, 32);
    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 0, 0);

    this->framebuffer = (u8*) MEM_ALLOCTEMP(&sys.allocatorMem, PRELOAD_FRAMEBUFFER_SIZE);
    STDfastmset(this->framebuffer, 0UL, PRELOAD_FRAMEBUFFER_SIZE);

    {
        u16 startcolor = 0x0;
        u16 endcolor   = 0x0CF;   /* 0x172; */

        for (t = 0 ; t <= 16 ; t++)
            COLcomputeGradient16Steps(&startcolor, &endcolor, 1, t, &this->gradient[t]);
    }

    SYSwriteVideoBase((u32)this->framebuffer);

    SYSvblroutines[0] = SYSvbldpakProgress;

    FSMgotoNextState (&g_stateMachine);

    if (sys.has2Drives)
    {
        this->y = 24;

        preloadPrint(this, "        2 FLOPPY DRIVES DETECTED"        , 0, this->y, 1);     this->y += 48;

        preloadPrint(this, "    insert disk 2 in other drive and"    , 0, this->y, 2);     this->y += 24;
        preloadPrint(this, "              press SPACE"               , 0, this->y, 2);     this->y += 32;
        /*                 |                                        | */
        preloadPrint(this, "       or go on with 1 drive and"        , 0, this->y, 3);     this->y += 24;
        preloadPrint(this, "               press ESC"                , 0, this->y, 3);

        preloadFadeIn(this, 1); preloadWait(10);
        preloadFadeIn(this, 2); preloadWait(10);
        preloadFadeIn(this, 3);

        do
        {
            scancode = ScreenGetch();

            if (scancode == HW_KEY_ESC)
            {
                preloadFadeOut(this, 1);
                preloadFadeOut(this, 2);

                sys.forceUsedDrive = sys.invertDrive;

                break;
            }

            if ((scancode == HW_KEY_SPACEBAR) || ((scancode == HW_KEY_RETURN)))
            {
                preloadFadeOut(this, 1);
                preloadFadeOut(this, 3);

                if (LOADcheckInsertedMediaID(RSC_RELAPSE2.preferedDrive, RELAPSE2_MEDIAID, this->temp)) /* check floppy inserted */
                {
                    allowpreload = scancode == HW_KEY_SPACEBAR;

                    preloadFadeOut(this, 3);

                    LOADinitFAT (1, &RSC_RELAPSE2, RSC_RELAPSE2_NBENTRIES, RSC_RELAPSE2_NBMETADATA);

                    g_screens.use2drives = true;
                    break;
                }

                preloadFadeIn(this, 1);
                preloadFadeIn(this, 2);
                preloadFadeIn(this, 3);
            }
        }
        while (1);
    }

    preloadFadeOut(this, 1);
    preloadFadeOut(this, 2);
    preloadFadeOut(this, 3);

    if (allowpreload)
    {
        TRAClogNumber10S(TRAC_LOG_ALL, "media1 preload:", RSC_RELAPSE1.mediapreloadsize, 6, '\n');
        if ((sys.coreHeapsize - demOS_COREHEAPSIZE) >= RSC_RELAPSE1.mediapreloadsize)
        {
            this->y = 50;
            preloadPrint(this, "  Extra RAM detected > Preload disk 1", 0, this->y, 4);
            this->y += 28;

            preloadFadeIn(this, 4);
            this->printcolor = 4;
            LOADmediaPreload(&RSC_RELAPSE1, &sys.allocatorCoreMem, RSC_RELAPSE1_SPLASH_SPLASH, preloadCallback, this);
            preloadFadeOut(this, 4);
            this->y += 32;

            if (g_screens.use2drives == false)
            {
                bool success;

                do
                {
                    preloadPrint(this, "Insert disk 2 in drive and press SPACE", 0, this->y, 5);
                    preloadFadeIn(this, 5);

                    while (ScreenGetch() != HW_KEY_SPACEBAR);
                    preloadFadeOut(this, 5);

                    success = LOADcheckInsertedMediaID(RSC_RELAPSE2.preferedDrive, RELAPSE2_MEDIAID, this->temp);
                } while (success == false);

                LOADinitFAT(sys.forceUsedDrive, &RSC_RELAPSE2, RSC_RELAPSE2_NBENTRIES, RSC_RELAPSE2_NBMETADATA);

                this->y += 24;
            }
        }

        if ((sys.coreHeapsize - demOS_COREHEAPSIZE) >= (RSC_RELAPSE1.mediapreloadsize + RSC_RELAPSE2.mediapreloadsize))
        {
            this->y = 100;
            preloadPrint(this, "   More RAM detected > Preload disk 2", 0, this->y, 6);
            this->y += 28;

            preloadFadeIn(this, 6);
            this->printcolor = 6;
            LOADmediaPreload(&RSC_RELAPSE2, &sys.allocatorCoreMem, RSC_RELAPSE2_ZIKS_DELOS_04_ARJX, preloadCallback, this);
            preloadFadeOut(this, 6);
        }
    }

    FSMgotoNextState (_fsm);
}


void PreLoadActivity (FSM* _fsm)
{       
    PreLoad* this = g_screens.preload;
    u16 t;

    IGNORE_PARAM(_fsm);

    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + 160 * 2 - 1, 40 + 200 - 1, 160, 0);
    EMULfbExEnd();
 
    for (t = 0; t < ARRAYSIZE(this->counter) ; t++)
    {
        if (this->inc[t] != 0)
        {
            this->counter[t] += this->inc[t];
            
            if (this->inc[t] > 0)
            {
                if (this->counter[t] >= 16)
                {
                    this->counter[t] = 16;
                    this->inc[t] = 0;
                }
            }
            else
            {
                if (this->counter[t] <= 0)
                { 
                    this->counter[t] = 0;
                    this->inc[t] = 0;
                }
            }
        }

        HW_COLOR_LUT[(t + 1) * 2    ] = this->gradient [  this->counter[t]          ];
        HW_COLOR_LUT[(t + 1) * 2 + 1] = this->gradient [ (this->counter[t] + 1) / 2 ];
    }

    /*preloadUpdateText(this);*/
}


void PreLoadExit (FSM* _fsm)
{
    PreLoad* this = g_screens.preload;

    SYSvblroutines[0] = SYSvblend;

    STDmset(this->inc, -1UL, sizeof(this->inc));

    preloadWait(17);

    FSMgotoNextState(&g_stateMachine);
    SYSvsync;

    MEM_FREE (&sys.allocatorMem, this->framebuffer);
    MEM_FREE (&sys.allocatorMem, this);
    g_screens.preload = NULL; 
    g_screens.next = false;
    g_screens.gotomenu = false;

    FSMgotoNextState(_fsm);
}

#endif /* DEMOS_LOAD_FROMHD */
