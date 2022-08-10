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

#include "EXTERN\ARJDEP.H"

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\DATA\SYSTFNT.H"

#include "DEMOSDK\PC\EMUL.H"

#include "BLITZIK\BLITZWAV.H"

#include "BLITZIK\SRC\SCREENS.H"


#define BLITZ_LOADER_CHAR_YSPACING  4

#define BLITZ_LOADER_LOADFILE_LEN   20
#define BLITZ_LOADER_UNPACK_LEN     50
#define BLITZ_LOADER_SAMPLEINIT_LEN 30

#define BLITZ_LOADER_READERROR_COLOR PCENDIANSWAP16(0x200)

#define BLITZ_LOADER_MOVE_OPCODE    0x1140  /* move.b d0,X(a0) */
#define BLITZ_LOADER_GENCODE_SIZE   7700

typedef void (*blitZLoaderCharFunc)(u8 _value, u8* _adr);

#ifdef DEMOS_DEBUG
static u32 g_blitzLoaderNbAllocs = 0;
#endif

#if BLZ_DEVMODE()
#   define BLZ_STACKS_DIAGNOSTIC() 0
#else
#   define BLZ_STACKS_DIAGNOSTIC() 0
#endif


#if BLZ_STACKS_DIAGNOSTIC()

#ifdef DEMOS_USES_BOOTSECTOR
ASMIMPORT u16 __text;
ASMIMPORT u8* _StkLim;
#endif


static void blitzPrint(char* s, u8* adr)
{
    SYSfastPrint (s, adr,     160, 4, (u32)&SYSfont);
    SYSfastPrint (s, adr + 2, 160, 4, (u32)&SYSfont);
}


static void blitzLoaderDebugInfos(BlitZLoader* this)
{
    static char stackusage    [] = "idleStack        >       =>      ";
    static char mainstackusage[] = "mainStack        >       =>      ";


    STDuxtoa(&stackusage [10], (u32)sys.idleThreadStackBase, 6);
    STDuxtoa(&stackusage [18], (u32)sys.idleThreadStackBase + sys.idleThreadStackSize, 6);
    STDutoa (&stackusage [28], SYSevaluateStackUsage(sys.idleThreadStackBase, sys.idleThreadStackSize), 4);

#   ifdef DEMOS_USES_BOOTSECTOR
    {
        u32 aStkSize = *(u32*)(&__text+3);

        STDuxtoa(&mainstackusage[10], (u32)(_StkLim+256)         , 6);
        STDuxtoa(&mainstackusage[18], (u32)(_StkLim+256+aStkSize), 6);
        STDutoa (&mainstackusage[28], SYSevaluateStackUsage(_StkLim+256, aStkSize), 4);
    }
#   endif    

    blitzPrint (stackusage    , this->framebuffer);
    blitzPrint (mainstackusage, this->framebuffer + 10*160);
}
#else
#   define blitzLoaderDebugInfos(THIS)
#endif


static u16 blitZLoaderGenerateCode(u8* _code, u16* _table)
{
    u16  t;
    u16* code = (u16*)_code;


    *code++ = CGEN_OPCODE_RTS;

    for (t = '.' ; t <= '9' ; t++)
    {
        if (SYSfont.charsmap[t] == SYS_FNT_UNDEFINED)
        {
            _table[t] = 0;
        }
        else
        {
            u8*  bits = SYSfont.data + (SYSfont.charsmap[t] << SYS_FNT_OFFSETSHIFT);
            s16  offset = 0;
            u8   ys;


            _table[t] = (u8*) code - _code;

            for (ys = 8; ys > 0; ys--)
            {
                u8  v = *bits++;
                u8  xs;

                for (xs = 4; xs > 0; xs--)
                {
                    if (v & 0x80)
                    {
                        *code++ = BLITZ_LOADER_MOVE_OPCODE;
                        *code++ = offset;
                    }

                    offset++;
                    v <<= 1;

                    if (v & 0x80)
                    {
                        *code++ = BLITZ_LOADER_MOVE_OPCODE;
                        *code++ = offset;
                    }
                    offset += 3;
                    v <<= 1;
                }

                offset += BLITZ_LOADER_CHAR_YSPACING*160 - 16;
            }

            *code++ = CGEN_OPCODE_RTS;
        }
    }

    return (u8*)code - _code;
}

static void blitZLoaderDisplay(BlitZLoader* this, char* _text, u8 _value, u8* _adr)
{
    u8*  code  = this->codebuffer;
    u16* table = this->charFunc;

    while (*_text != 0)
    {
        char c  = *_text++;
        u8*  call = code + table[c];

#       ifdef __TOS__
        ((blitZLoaderCharFunc)call) (_value, _adr);
#       else
        {
            u16* p = (u16*)call;

            while (*p != CGEN_OPCODE_RTS)
            {
                u16 offset = p[1];
                p += 2;

                _adr[offset] = _value;
            }
        }
#       endif

        _adr += 16;
    }
}

static BlitZLoader* blitZLoaderInit (void)
{
    BlitZLoader* this;
    u16 size;

    SYSvsync;
    *HW_VIDEO_MODE = HW_VIDEO_MODE_2P;
    STDmset(HW_COLOR_LUT, 0, 32);

    this = g_screens.loader = MEM_ALLOC_STRUCT(&sys.allocatorMem, BlitZLoader);
    DEFAULT_CONSTRUCT(this);

    this->framebuffer = MEM_ALLOC(&sys.allocatorMem, 32000);
    STDfastmset(this->framebuffer, 0, 32000);
    SYSwriteVideoBase((u32)this->framebuffer);

    this->codebuffer = MEM_ALLOC(&sys.allocatorMem, BLITZ_LOADER_GENCODE_SIZE);
    size = blitZLoaderGenerateCode(this->codebuffer, this->charFunc);
    ASSERT(size <= BLITZ_LOADER_GENCODE_SIZE);

    this->blsinitprogress = this->lastprogress = -1;
    ARJprogress = NULL;

    blitzLoaderDebugInfos(this);

    return this;
}

static void BlitZLoaderShutdown(void)
{
    BlitZLoader* this = g_screens.loader;

    STDmset(HW_COLOR_LUT, 0, 32);

    MEM_FREE(&sys.allocatorMem, this->codebuffer);
    MEM_FREE(&sys.allocatorMem, this->framebuffer);
    MEM_FREE(&sys.allocatorMem, g_screens.loader);

    this = g_screens.loader = NULL;

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorCoreMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorCoreMem.allocator, stdout);
    }
#   endif
}

/*ASMIMPORT u16 loaddebug;*/

void BlitZLoaderActivity(FSM* _fsm)
{
    BlitZLoader* this = g_screens.loader;

    /*    u16 colors[2];*/
    u16 planeflip = this->planeflip;

    IGNORE_PARAM(_fsm);

    if (this->colorcount > 0)
    {
        this->colorcount--;

        {
            u8 color4b = this->colorcount >> 1;
            u8 g,b;
            u8 val     = COL4b2ST[color4b];
            u8 halfval = COL4b2ST[color4b >> 1];


            b = val;
            g = halfval;

            HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16((g << 4) | b);
            HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0x4F);
            HW_COLOR_LUT[3] = PCENDIANSWAP16(0x05F);

            /*switch (g_screens.persistent.menu.colorchoice)
            {
            default:
            case 0:
            case 1:
                b = val;
                g = halfval;

                HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16((g << 4) | b);
                HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0xBF);
                HW_COLOR_LUT[3] = PCENDIANSWAP16(0xBF);
                break;

            case 3:
                g = val;

                HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16(g << 4);
                HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0xF0);
                HW_COLOR_LUT[3] = PCENDIANSWAP16(0xF0);
                break;

            case 2:
                g = b = val;

                HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16((g << 4) | b);
                HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0xFF);
                HW_COLOR_LUT[3] = PCENDIANSWAP16(0xFF);
                break;

            case 4:
                r = val;
                g = halfval;

                HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16((r << 8) | (g << 4));
                HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0xFB0);
                HW_COLOR_LUT[3] = PCENDIANSWAP16(0xFB0);
                break;

            case 5:
                r= g = b = val;

                HW_COLOR_LUT[this->planeflip + 1]       = PCENDIANSWAP16((r << 8) | (g << 4) | b);
                HW_COLOR_LUT[(this->planeflip ^ 1) + 1] = PCENDIANSWAP16(0xFFF);
                HW_COLOR_LUT[3] = PCENDIANSWAP16(0xFFF);
                break;
            }*/
        }
    }

    if (this->updateprogresscallback(this))
    {
        char* text = this->text[planeflip];
        u16   offset;
        u16   progress = this->lastprogress;


        blitZLoaderDisplay(this, text, 0, this->framebuffer + this->offset[planeflip]);

        if (progress < 10)
        {
            text[0] = '0' + progress;
            text[1] = 0;
            offset = (640 - 64) / 2;
        }
        else if (progress < 100)
        {
            u32 div = STDdivu(progress, 10);
            text[0] = (char)('0' + (div & 0xFFFF));
            text[1] = (char)('0' + (div >> 16));
            text[2] = 0;
            offset = (640 - 128) / 2;
        }
        else
        {
            text[0] = '1';
            text[1] = '0';
            text[2] = '0';
            text[3] = 0;
            offset = (640 - 192) / 2;
        }

        offset &= ~15;
        offset >>= 2;
        offset += ((200 - 32) / 2) * 160;

        offset += planeflip << 1;

        blitZLoaderDisplay(this, text, 0x10, this->framebuffer + offset);

        this->offset[planeflip] = offset;
        this->planeflip ^= 1;

        this->colorcount = 32;
    }

    if (this->loadRequest != NULL)
    {
        if (this->loadRequest->processed == LOADrequestState_RETRYING)
            *HW_COLOR_LUT = BLITZ_LOADER_READERROR_COLOR;
        else
            *HW_COLOR_LUT = 0;
    }

    /*{
        bool pressed  = (sys.key & HW_KEYBOARD_KEYRELEASE) == 0;
        u8   scancode = sys.key & ~(HW_KEYBOARD_KEYRELEASE);

        if (pressed)
            if (scancode == HW_KEY_F8)
                loaddebug = 1;
    }*/
}


void BlitZLoaderBacktask(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    FSMgotoNextState (_fsm);
}



/* ----------------------------------------------------------------------------------------------------------
    SPECIFIC TO PRELOAD
---------------------------------------------------------------------------------------------------------- */

#ifndef DEMOS_LOAD_FROMHD
static bool blitZPreloadUpdateProgress (BlitZLoader* this)
{
    bool updated = false;
    s16 progress;


    ASSERT(this->preloadresourcecount < ARRAYSIZE(this->diskpreloadprogress));

    progress = (u16) STDdivu(STDmulu((this->diskpreloadprogress[this->preloadresourcecount] - this->resourcesectorstoload), 100), this->preloadtotalsectors);
    progress = 100 - progress;

    /*TRAClogNumber10(TRAC_LOG_ALL, "rsccount", this->preloadresourcecount, 4);
    TRAClogNumber10(TRAC_LOG_ALL, "rscsects", this->resourcesectorstoload, 4);
    TRAClogNumber10(TRAC_LOG_ALL, "progress", this->diskpreloadprogress[this->preloadresourcecount], 4);
    TRAClogNumber10(TRAC_LOG_ALL, "total", this->preloadtotalsectors, 4);
    TRAClogNumber10S(TRAC_LOG_ALL, "result", progress, 4, '\n');*/

    updated = progress != this->lastprogress;

    if (updated)
    {
        this->lastprogress = progress;
    }

    return updated;
}


static void blitzPreloadDisplay (u16 _preloadResourceIndex, LOADrequest* _request, void* _clientData)
{
    BlitZLoader* this = (BlitZLoader*) _clientData;

    this->preloadresourcecount  = _preloadResourceIndex;
    this->resourcesectorstoload = _request->nbsectors;
}
#endif


void BlitZPreloadEnter (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

#   if DEMOS_MEMDEBUG
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRACmemDump(&sys.allocatorCoreMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorCoreMem.allocator, stdout);
    }
#   endif

#   ifndef DEMOS_LOAD_FROMHD

    if (g_screens.preload != NULL)
    {
        u8 disk1Preload[RSC_BLITZWAV_NBENTRIES - RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC];
        u8 t;
        EMUL_STATIC BlitZLoader* this;


        this = blitZLoaderInit();
        this->updateprogresscallback = blitZPreloadUpdateProgress;
    
        for (t = RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC; t < RSC_BLITZWAV_NBENTRIES; t++)
        {
            disk1Preload[t - RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC] = t;
            this->preloadtotalsectors += LOADresourceNbSectors(&RSC_BLITZWAV, t);
            this->diskpreloadprogress [t - RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC] = this->preloadtotalsectors;
        }

        FSMgotoNextState(&g_stateMachine);

        SYSfastPrint ("xtra RAM preload", this->framebuffer + 61 + 192 * 160, 160, 4, (u32)&SYSfont);

        /*current =*/ LOADpreload(g_screens.preload, RSC_BLITZWAV.mediapreloadsize, g_screens.preload, &RSC_BLITZWAV, disk1Preload, ARRAYSIZE(disk1Preload), blitzPreloadDisplay, g_screens.loader);       

        FSMgotoNextState(_fsm);
    }
    else
    {
        FSMgoto(_fsm, _fsm->activeState + 2);
    }
#   else
    FSMgoto(_fsm, _fsm->activeState + 2);
#   endif
}


void BlitZPreloadExit(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

#   ifndef DEMOS_LOAD_FROMHD
    if (g_screens.preload != NULL)
    {
        BlitZLoaderShutdown();
    }
#   endif

    FSMgoto(_fsm           , g_screens.fsm.stateIdleEntryPoints [BLZ_EP_LOADER]);
    FSMgoto(&g_stateMachine, g_screens.fsm.stateEntryPoints     [BLZ_EP_LOADER]);
}



/* ----------------------------------------------------------------------------------------------------------
    SPECIFIC TO LOAD MODULE 
   ---------------------------------------------------------------------------------------------------------- */

static void blitZLoadModuleBLSInitCallback(bool _deltadecode, u16 _index, u16 _nb)
{  
    BlitZLoader* this= g_screens.loader;

    if (_deltadecode)
    {
        this->blsinitprogress = (s8) STDdivu((_index + 1) * BLITZ_LOADER_SAMPLEINIT_LEN / 4, _nb);
    }
    else
    {
        this->blsinitprogress = (s8)(BLITZ_LOADER_SAMPLEINIT_LEN / 4 + (u16) STDdivu((_index + 1) * (BLITZ_LOADER_SAMPLEINIT_LEN * 3 / 4), _nb));
    }
}


static void blitzLoaderExtract(BLSsoundTrack* _sndtrack)
{
    u16 i;


    g_screens.sampleToSourceSample = (u8*) MEM_ALLOC(&sys.allocatorCoreMem, _sndtrack->nbSamples);

    for (i = 0; i < _sndtrack->nbSamples; i++)
    {
        u16 k = _sndtrack->sampleAllocOrder[i] & BLS_STORAGE_ORDER_MASK;
        BLSprecomputedKey* key = &_sndtrack->keys[k];

        ASSERT(k < _sndtrack->nbKeys);
        g_screens.sampleToSourceSample[i] = key->sampleIndex;
    }
}


static bool blitZLoadModuleUpdateProgress (BlitZLoader* this)
{
    s16 progress = 0;
    bool updated = false;


    /**HW_COLOR_LUT = 0x0;*/

    if (this->loadRequest != NULL)
    {
        progress = BLITZ_LOADER_LOADFILE_LEN - (u16)(STDmulu(this->loadmul, this->loadRequest->nbsectors) >> 8);
    }
    else if (ARJprogress != NULL)
    {
        u16 diff = (u16)((ARJprogress - this->depackstart) >> 8);

        progress = BLITZ_LOADER_LOADFILE_LEN + (u16)(STDmulu(diff, this->depackmul) >> 8);
    }
    else if (this->blsinitprogress != -1)
    {
        progress = BLITZ_LOADER_LOADFILE_LEN + BLITZ_LOADER_UNPACK_LEN + this->blsinitprogress;
    }

    progress = 100 - progress;
    if (progress < 0)
        progress = 0;

    updated = progress != this->lastprogress;

    if (updated)
    {
        this->lastprogress = progress;
    }

    return updated;
}




void BlitZLoadModuleEnter (FSM* _fsm)
{
    u8  nextmodule = RSC_BLITZWAV_ZIKS_LOADER_ARJX + g_screens.persistent.menu.currentmodule;


    IGNORE_PARAM(_fsm);
    EMULfbExDisable();

    if (g_screens.runningmodule == nextmodule)
    {
        SYSvsync;
        STDmset(HW_COLOR_LUT, 0, 32);

        BlitZrestartMod();

        if (g_screens.persistent.menu.playmode != BLZ_PLAY_FROM_MENU)
        {
            g_screens.runningphase = BLZ_PHASE_FX;
        }

        FSMgoto(_fsm           , g_screens.fsm.stateIdleEntryPoints[g_screens.runscreen]);
        FSMgoto(&g_stateMachine, g_screens.fsm.stateEntryPoints    [g_screens.runscreen]);
    }
    else
    {
        g_screens.runningmodule = nextmodule;

#       ifdef DEMOS_DEBUG
        g_blitzLoaderNbAllocs = RINGallocatorCheck(sys.allocatorMem.allocator);
#       endif

        if (SYSvblroutines[0] == g_screens.blzupdateroutine) /* module currently running */
        {
            s8 t;

            for (t = 40; t >= 0; t--)
            {
                *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | t;
                SYSvsync;
            }

            SYSvblroutines[0] = RASvbldonothing;
            *HW_DMASOUND_CONTROL = HW_DMASOUND_CONTROL_OFF;
            g_screens.player.framenum = 0;

            if (g_screens.persistent.menu.playmode != BLZ_PLAY_FROM_MENU)
            {
                g_screens.runningphase = BLZ_PHASE_FX;
            }

            MEM_FREE(&sys.allocatorCoreMem, g_screens.sampleToSourceSample); 
            g_screens.sampleToSourceSample = NULL;

            BLZfree(&sys.allocatorCoreMem, g_screens.player.sndtrack);
            BLZplayerFree(&sys.allocatorCoreMem, &g_screens.player);

            ASSERT (g_screens.fxsequence.framesmap != NULL);

            MEM_FREE(&sys.allocatorCoreMem, g_screens.fxsequence.framesmap);
            g_screens.fxsequence.framesmap = NULL;
            g_screens.fxsequence.seq = NULL;
            g_screens.fxsequence.current = NULL;
        }

        {
            BlitZLoader* this = blitZLoaderInit();

            this->updateprogresscallback = blitZLoadModuleUpdateProgress;
        }

        FSMgotoNextState(_fsm);
        FSMgotoNextState(&g_stateMachine);
    }
}

void BlitZLoadModule(FSM* _fsm)	
{ 
    BlitZLoader* this = g_screens.loader;
    BLSsoundTrack* sndtrack;
    u8  currentzik    = g_screens.runningmodule;
    u16 metadataIndex = LOADresourceMetaDataIndex(&RSC_BLITZWAV, currentzik);

    
    EMUL_BEGIN_ASYNC_REGION

    DEFAULT_CONSTRUCT(&g_screens.player);

    {
        u8* blsfilebuffer = (u8*) MEM_ALLOCTEMP( &sys.allocatorMem, LOADmetadataOriginalSize( &RSC_BLITZWAV, metadataIndex ));
        u8* packeddata    = (u8*) MEM_ALLOCTEMP( &sys.allocatorMem, LOADresourceRoundedSize ( &RSC_BLITZWAV, currentzik));
        u16 t;

        /* Load */
        this->loadmul     = (u16) STDdivu (BLITZ_LOADER_LOADFILE_LEN << 8, LOADresourceNbSectors(&RSC_BLITZWAV, currentzik));
        this->loadRequest = LOADdata (&RSC_BLITZWAV, currentzik, packeddata, LOAD_PRIORITY_INORDER);
        LOADwaitRequestCompleted(this->loadRequest);

        /* Unpack */
        this->depackstart = packeddata;
        this->depackmul   = (u16) STDdivu(256 * BLITZ_LOADER_UNPACK_LEN, (u16)(LOADmetadataSize( &RSC_BLITZWAV, metadataIndex ) >> 8));
        this->loadRequest = NULL;

        ARJprogress = packeddata;
        ARJdepack(blsfilebuffer, packeddata);
        this->blsinitprogress = 0;
        ARJprogress = NULL;

        {
            u8* src = packeddata + LOADmetadataOffset( &RSC_BLITZWAV, metadataIndex + 1 );
            
            ASSERT (LOADresourceMetaDataNbs(&RSC_BLITZWAV, currentzik) == 2);

            g_screens.fxsequence.size = (u16) LOADmetadataSize( &RSC_BLITZWAV, metadataIndex + 1 );
            g_screens.fxsequence.framesmap = (u16*) MEM_ALLOC( &sys.allocatorCoreMem, g_screens.fxsequence.size);
            STDmcpy(g_screens.fxsequence.framesmap, src, g_screens.fxsequence.size);
        }

        MEM_FREE(&sys.allocatorMem, packeddata);

        /* Init module */
        
        BLZread         (&sys.allocatorCoreMem, &sys.allocatorMem, blsfilebuffer, &sndtrack);

        blitzLoaderExtract(sndtrack);

        BLSinit         (&sys.allocatorCoreMem, &sys.allocatorMem, sndtrack, blitZLoadModuleBLSInitCallback);
        BLZplayerInit   (&sys.allocatorCoreMem, &g_screens.player, sndtrack, g_screens.dmaplayoncemode ? BLZ_DMAMODE_PLAYONCE : BLZ_DMAMODE_LOOP);
        
        g_screens.fxsequence.seq = (u8*)&g_screens.fxsequence.framesmap[g_screens.player.sndtrack->trackLen];
        g_screens.fxsequence.current = g_screens.fxsequence.seq;

        g_screens.sndtrackKeyMin = 255;
        g_screens.sndtrackKeyMax = 0;

        for (t = 0; t < g_screens.player.sndtrack->nbKeys; t++)
        {
            u8 key = g_screens.player.sndtrack->keysnoteinfo[t];

            key = (key >> 4) * 12 + (key & 0xF);

            if (key > g_screens.sndtrackKeyMax)
                g_screens.sndtrackKeyMax = key;

            if (key < g_screens.sndtrackKeyMin)
                g_screens.sndtrackKeyMin = key;
        }

        MEM_FREE(&sys.allocatorMem, blsfilebuffer);

        this->blsinitprogress = 100 - (BLITZ_LOADER_LOADFILE_LEN + BLITZ_LOADER_UNPACK_LEN);
    }

    #ifndef __TOS__
    this->blsinitprogress = 0;

    EMUL_REENTER_POINT

    EMUL_EXIT_IF(this->blsinitprogress++ <= 100)
    #endif

    /* play module */
    *HW_MICROWIRE_DATA = HW_MICROWIRE_VOLUME | 40;

    aBLZplayer = &g_screens.player;
    if (g_screens.runscreen !=  BLZ_EP_INTRO)
        SYSvblroutines[0] = g_screens.blzupdateroutine;

    FSMgotoNextState (_fsm);

    EMUL_END_ASYNC_REGION
}


void BlitZLoadModuleExit(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    BlitZLoaderShutdown();

#   ifdef DEMOS_DEBUG
    ASSERT (g_blitzLoaderNbAllocs == RINGallocatorCheck(sys.allocatorMem.allocator));
#   endif

    FSMgoto(_fsm           , g_screens.fsm.stateIdleEntryPoints[g_screens.runscreen]);
    FSMgoto(&g_stateMachine, g_screens.fsm.stateEntryPoints    [g_screens.runscreen]);
}


void BlitZLoadModuleIdleActivity(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    while (BLZ_COMMAND_AVAILABLE)
    {
        u8 cmd = BLZ_CURRENT_COMMAND;

        ASSERT((g_screens.persistent.menu.playmode == BLZ_PLAY_INTERACTIVE) || ((cmd & BLZ_CMD_CATEGORY_MASK) == BLZ_CMD_FUNC_CATEGORY) || (cmd & BLZ_CMD_CATEGORY_MASK) == BLZ_CMD_CURSOR_CATEGORY);
            
        BLZ_ITERATE_COMMAND;

        if (cmd != BLZ_CMD_SELECT)
            if (ScreensManageScreenChoice(BLZ_EP_WAIT_FOR_FX, cmd))
                return;
    }
}


void BlitZLoadModuleIdleBacktask(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);
}


void BlitZLoadModuleIdleExit(FSM* _fsm)
{
    IGNORE_PARAM(_fsm);
    ScreensGotoScreen();
}


