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

#include "DEMOSDK\PC\EMUL.H"

#include "FX\COLPLANE\COLPLANE.H"

#include "EXTERN\RELOCATE.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\LOADER\LOADER.H"

#include "RELAPSE\RELAPSE1.H"
#include "RELAPSE\RELAPSE2.H"


#ifdef __TOS__
#define LOADER_RASTERIZE() 0
#else
#define LOADER_RASTERIZE() 0
#endif

#if LOADER_RASTERIZE()
#   define LOADER_RASTERIZE_COLOR(COLOR) *HW_COLOR_LUT=COLOR
#else
#   define LOADER_RASTERIZE_COLOR(COLOR)
#endif

#define LOADER_FRAMEBUFFER_SIZE 32000UL

#ifdef __TOS__

static LoaderAsmImport* loaderAsmImport(void* _prxbuffer)
{
    return (LoaderAsmImport*)((DYNbootstrap)_prxbuffer)();
}

#else

static void loaderUpdateVGrid(void* framebuffer_, u16* htone_)
{
    s16 t;

    *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_OP = HW_BLITTER_OP_S;
    *HW_BLITTER_HOP = HW_BLITTER_HOP_HTONE;
    *HW_BLITTER_ADDR_DEST = (u32) framebuffer_;
    *HW_BLITTER_XINC_DEST = 160;
    *HW_BLITTER_YINC_DEST = -199*160 + 8;
    *HW_BLITTER_XSIZE = 200;
    *HW_BLITTER_CTRL2 = 0;

    for (t = 9 ; t >= 0 ; t--)
        HW_BLITTER_HTONE[t] = PCENDIANSWAP16(*htone_++);

    *HW_BLITTER_YSIZE = 10;
    *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY | 9;

    EMULblit();

    for (t = 9 ; t >= 0 ; t--)
        HW_BLITTER_HTONE[t] = PCENDIANSWAP16(*htone_++);

    *HW_BLITTER_YSIZE = 10;
    *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY | 9;

    EMULblit();
}

static void loaderUpdateHGrid(void* framebuffer_, u16 _space, u16 y_)
{
    u16 incadr = _space * 160;
    u16 acc    = 0;
    u32 endadr = (u32) framebuffer_ + 32000;
    u32 adr    = (u32) framebuffer_ + (y_ % _space) * 160;


    *HW_BLITTER_ENDMASK1 = *HW_BLITTER_ENDMASK2 = *HW_BLITTER_ENDMASK3 = -1;
    *HW_BLITTER_OP = HW_BLITTER_OP_BIT1;
    *HW_BLITTER_HOP = HW_BLITTER_HOP_BIT1;
    *HW_BLITTER_XINC_DEST = 8;
    *HW_BLITTER_YINC_DEST = 8;
    *HW_BLITTER_XSIZE = 20;
    *HW_BLITTER_CTRL2 = 0;

    while (adr < endadr)
    {
        *HW_BLITTER_ADDR_DEST = adr;
        *HW_BLITTER_YSIZE = 1;
        *HW_BLITTER_CTRL1 = HW_BLITTER_CTRL1_HOGMODE_BLIT | HW_BLITTER_CTRL1_BUSY;

        EMULblit();

        adr += incadr;
    }
}

static void loadercomputeHTone (void* dest_, u16 inc_, u16 acc_) 
{
    u16  t, i, j;
    u32  acc = acc_;
    u16* htone = (u16*) dest_;


    LOADER_RASTERIZE_COLOR(3);

    for (j = 0 ; j < 2 ; j++)
    {
        for (i = 0; i < 10; i++)
        {
            u16 word = 0;

            for (t = 0; t < 16; t++)
            {
                word <<= 1;

                acc += inc_;
                if (acc > 0xFFFF)
                {
                    acc &= 0xFFFFUL;
                    word |= 1;
                }
            }

            *htone++ = word;
        }
        
        LOADER_RASTERIZE_COLOR(5);
    }

    LOADER_RASTERIZE_COLOR(7);
}

static LoaderAsmImport* loaderAsmImport(void* _prxbuffer)
{
    static LoaderAsmImport asmimport;
    static s16 sintab[512];
    u16 t;

    asmimport.computeHTone      = loadercomputeHTone;
    asmimport.loaderUpdateVGrid = loaderUpdateVGrid;
    asmimport.loaderUpdateHGrid = loaderUpdateHGrid;
    asmimport.sin = sintab;

    for (t = 0 ; t < 512 ; t++)
    {
        sintab[t] = (s16) (sinf((float)t / 512.0f * 2.0f * (float)M_PI) * 32767.0f);
    }

    return &asmimport;
}

#endif

static void loaderUpdateText(Loader* this)
{
    char* text = this->text;
    bool haschanged = false;


    if (this->insertDiskMessage != this->insertDiskMessageDisplayed)
    {
        if (this->insertDiskMessage > 0)
        {
            char* text = "Insert disk   and press SPACE";

            text[12] = (char)('0' + this->insertDiskMessage);

            SYSfastPrint(text, this->framebuffer + 92 * 160 + 6 + 16 + 1, 320, 8, (u32)&SYSfont);
        }
        else
        {
            u16* p = (u16*)(this->framebuffer + 92 * 160 + 6);
            u16 t;

            for (t = 20 * 16; t != 0; t--, p += 4)
                *p = 0;
        }

        this->insertDiskMessageDisplayed = this->insertDiskMessage;
    }

    if (this->countdownactive)
    {
        if (g_screens.loader->lastprogress != g_screens.persistent.initprogress)
        {
            g_screens.loader->lastprogress = g_screens.persistent.initprogress;
            haschanged = true;
        }

        if (g_screens.persistent.initprogress > 0)
            text[0] = (u8)('0' + g_screens.persistent.initprogress);
        else
            text[0] = ' ';

        text[1] = ' ';

        {
            u16 nbsectors = 0;
            u16 t;

            for (t = 0; t < RELAPSE_NBMAX_LOADREQUESTS; t++)
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
                STDuxtoa(text + 2, (u32)nbsectors, 2);
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
            SYSfastPrint(text, this->framebuffer + 182 * 160 + 8 + 6, 320, 8, (u32)&SYSfont);
        }

        if (this->gotomenudisplayed == false)
        {
            if (g_screens.gotomenu)
            {
                this->gotomenudisplayed = true;
                SYSfastPrint("GOTO MENU", this->framebuffer + 182 * 160 + 120 + 6, 320, 8, (u32)&SYSfont);
            }
        }
    }
}


void LoaderEntry (FSM* _fsm)
{
    Loader* this;
    u16 t;


    IGNORE_PARAM(_fsm);

    if (sys.isMegaSTe)
        *HW_MEGASTE_CPUMODE = HW_MEGASTE_CPUMODE_16MHZ | HW_MEGASTE_CPUMODE_CACHED;

    g_screens.persistent.initprogress = 0;

    STDmset (HW_COLOR_LUT, 0UL, 32);
    ScreensSetVideoMode (HW_VIDEO_MODE_4P, 0, 0);

    g_screens.loader = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Loader );
    DEFAULT_CONSTRUCT(this);

    this->prxbuffer = MEM_ALLOCTEMP(&sys.allocatorMem, LOADmetadataOriginalSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_LOADER_ARJX));
    RELAPSE_UNPACK(this->prxbuffer, g_screens.persistent.loaderprx);
    SYSrelocate(this->prxbuffer);

    this->asmimport = loaderAsmImport(this->prxbuffer);

    this->framebuffer = (u8*) MEM_ALLOCTEMP(&sys.allocatorMem, LOADER_FRAMEBUFFER_SIZE);
    STDfastmset(this->framebuffer, 0UL, LOADER_FRAMEBUFFER_SIZE);

    this->size = 4;
    this->incsize = 1;

    {
        u16 startcolor = 0x11;
        u16 endcolor   = 0x7F;

        for (t = 0 ; t < 16 ; t++)
            COLcomputeGradient16Steps(&startcolor, &endcolor, 1, t, &this->gradient[t]);
    }

    SYSwriteVideoBase((u32)this->framebuffer);

    STDmset(this->lut + 8, 0xFFFFFFFFUL, 16);

    this->fadestep = 1;
    this->fadeinc  = 1;

    SYSvblroutines[0] = SYSvbldpakProgress;

    FSMgotoNextState (&g_stateMachine);
    FSMgotoNextState (_fsm);
}


static void loaderCheckDiskInserted(Loader* this, LOADdisk* media_, u16 mediaID_)
{
    if ((sys.has2Drives == false) || (sys.forceUsedDrive >= 0))
    {
        if (media_->mediapreload == NULL)
        {
            while (LOADcheckInsertedMediaID(media_->preferedDrive, mediaID_, this->temp) == false)
            {
                this->insertDiskMessage = media_->preferedDrive + 1;

                while (ScreenGetch() != HW_KEY_SPACEBAR);
                
                this->insertDiskMessage = 0;
            }
        }
    }
}


void LoaderCheckDisk1Inserted (FSM* _fsm)
{
#   ifndef DEMOS_LOAD_FROMHD
    loaderCheckDiskInserted(g_screens.loader, &RSC_RELAPSE1, RELAPSE1_MEDIAID);
#   endif

    g_screens.loader->countdownactive = true;

    FSMgotoNextState (_fsm);
}

void LoaderCheckDisk2Inserted (FSM* _fsm)
{
#   ifndef DEMOS_LOAD_FROMHD
    loaderCheckDiskInserted(g_screens.loader, &RSC_RELAPSE2, RELAPSE2_MEDIAID);

    if (RSC_RELAPSE2.nbEntries == 0)
        LOADinitFAT(1, &RSC_RELAPSE2, RSC_RELAPSE2_NBENTRIES, RSC_RELAPSE2_NBMETADATA);
#   endif

    g_screens.loader->countdownactive = true;
    
    FSMgotoNextState (_fsm);
}


static s16 loaderFindColor(u16* colors_, u16 color_)
{
    s16 t;

    for (t = 15 ; t >= 0 ; t--)
        if (colors_[t] == color_) 
            return t;

    return -1;
}

#define LOADER_SIZE_MIN 4
#define LOADER_SIZE_MAX 120

void LoaderActivity (FSM* _fsm)
{       
    Loader* this = g_screens.loader;
    u8* currentframe = this->framebuffer + (this->plane << 1);
    u16* lut = this->lut;

    u16 y = (this->asmimport->sin[this->angle & 511] >> 8) + 128 + 60;
    this->angle += 2;

    IGNORE_PARAM(_fsm);

    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + 160 * 2 - 1, 40 + 200 - 1, 160, 0);
    EMULfbExEnd();


    if (g_screens.persistent.not1stinto)
    {
        LOADER_RASTERIZE_COLOR(0x7);
        
        this->asmimport->loaderUpdateVGrid(currentframe, this->htone);

        LOADER_RASTERIZE_COLOR(0x5);    

        this->asmimport->loaderUpdateHGrid(currentframe, this->size, y - (this->size >> 1));

        LOADER_RASTERIZE_COLOR(0x300);
   
        {
            u16* colorshistory = this->colorshistory;
            u16* colors = this->gradient;

            s16 c1 = loaderFindColor(colors, colorshistory[0]);
            s16 c2 = loaderFindColor(colors, colorshistory[1]);

            if (c2 >= 0)
                colorshistory[2] = colors[((c2 << 2) + c2) >> 3]; /*(c2+c2+c2)>>2];*/
            if (c1 >= 0)
                colorshistory[1] = colors[((c1 << 2) + c1) >> 3];  /*c1 >> 1*//*(c1+c1+c1)>>2];*/

            colorshistory[0] = colors[1 + ((this->size << 4) - this->size) / (LOADER_SIZE_MAX + 1)];

            switch (this->plane)
            {
            case 0:
                lut[1] = PCENDIANSWAP16(colorshistory[0]);
                lut[4] = PCENDIANSWAP16(colorshistory[1]);
                lut[2] = PCENDIANSWAP16(colorshistory[2]);

                lut[7] = lut[3] = lut[5] = lut[1];
                lut[6] = lut[4];
                break;
            case 1:
                lut[2] = PCENDIANSWAP16(colorshistory[0]);
                lut[1] = PCENDIANSWAP16(colorshistory[1]);
                lut[4] = PCENDIANSWAP16(colorshistory[2]);

                lut[7] = lut[3] = lut[6] = lut[2];
                lut[5] = lut[1];
                break;
            case 2:
                lut[4] = PCENDIANSWAP16(colorshistory[0]);
                lut[2] = PCENDIANSWAP16(colorshistory[1]);
                lut[1] = PCENDIANSWAP16(colorshistory[2]);

                lut[7] = lut[6] = lut[5] = lut[4];
                lut[3] = lut[2];
                break;
            }
        }

        LOADER_RASTERIZE_COLOR(0x3);

        this->asmimport->computeHTone(this->htone, (u16)STDdivu(65536UL, this->size), this->x);

        this->size += this->incsize;
        if ((this->size >= LOADER_SIZE_MAX) || (this->size <= LOADER_SIZE_MIN))
            this->incsize = -this->incsize;

        this->x += 5000;

        this->plane++;
        if (this->plane >= 3)
            this->plane = 0;

        LOADER_RASTERIZE_COLOR(0x70);
    }

    LOADER_RASTERIZE_COLOR(0x50);

    loaderUpdateText(this);

    if (this->fadestep == 16)
    {
        STDmcpy2 (HW_COLOR_LUT, lut, 32);
        this->fadeinc = 0;
    }
    else if (this->fadestep == 0)
    {
        STDmcpy2 (HW_COLOR_LUT, this->black, 32);
        this->fadeinc = 0;
    }
    else
    {
        COLcomputeGradient16Steps(this->black, this->lut, 16, this->fadestep, HW_COLOR_LUT);
        this->fadestep += this->fadeinc;
    }

    LOADER_RASTERIZE_COLOR(0);
}


void LoaderExit (FSM* _fsm)
{
    Loader* this = g_screens.loader;

    
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;

    SYSvblroutines[0] = SYSvblend;

    this->fadestep = 15;
    this->fadeinc  = -1;

#   ifdef __TOS__
    while (this->fadestep != 0);
#   endif

    MEM_FREE (&sys.allocatorMem, this->prxbuffer);
    MEM_FREE (&sys.allocatorMem, this->framebuffer);

    MEM_FREE (&sys.allocatorMem, this);
    g_screens.loader = NULL; 

    if (sys.isMegaSTe)
        *HW_MEGASTE_CPUMODE = HW_MEGASTE_CPUMODE_8MHZ | HW_MEGASTE_CPUMODE_NOCACHE;

    FSMgotoNextState(&g_stateMachine);
    SYSvsync;

    g_screens.persistent.not1stinto = true;

    if (g_screens.gotomenu)
    {
        FSMgoto (_fsm, FSMgetCurrentState(_fsm) + 3);
    }
    else
    {
        g_screens.next = false;
        FSMgotoNextState (_fsm);
    }
}
