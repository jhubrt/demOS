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

#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\SOUND.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\FUGIT.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "EXTERN\ARJDEP.H"

#include "REBIRTH\REBIRTH2.H"


#define FUG_FIRST_SCANLINE 50

void FUGsetData         (u32 _fontoffs, u32 _fontscan, u32 _fontbitmap, void* _asmbuf) PCSTUB;
void FUGinitSequence    (u8* auxiliaire, u8* deltabuffer,u32 _textpos) PCSTUB;
u8*  FUGdtDpack         (u8* _deltabuffer, u8* _auxiliaire) PCSTUBRET;
void FUGcopyAux2Screen  (u8* _auxiliaire, u8* _screenlineadr) PCSTUB;
void FUGxorPass         (u8* _screenlineadr) PCSTUB;

ASMIMPORT u16 FUGbufsize;

STRUCT(FUGtext)
{
    u16  effect;
    u16  nbsteps;
    char text [14];
    u16  pause;
};

ENUM(FUGfx)
{
    APPEAR_L2R,
    APPEAR_SIMULT,
    APPEAR_R2L,
    DISAPPEAR_L2R,
    DISAPPEAR_SIMULTANEOUS,
    DISAPPEAR_R2L,
};

static FUGtext FUGtexts[] =
{
    {APPEAR_L2R     , 60,"LIBERTY     ",0},
    {DISAPPEAR_L2R  , 60,"LIBERTY     ",0},
    {APPEAR_R2L     , 75,"  CREATIVITY",0},
    {DISAPPEAR_R2L  , 75,"  CREATIVITY",0},
    {APPEAR_L2R     , 71,"GENEROSITY  ",0},
    {DISAPPEAR_L2R  , 71,"GENEROSITY  ",40},
    {APPEAR_SIMULT  , 26,"END LESS END",0},
    {DISAPPEAR_L2R  , 82,"END LESS END",0}
};


static void FugitSetGradient(u16 c1, u16 c2, u16 g1, u16 g2)
{
    u16  nb;
    u16* pc;
    u16* pc2;       

    u16 grey[16];
    u16 col[16];

    COLcomputeGradient (&g1, &g2, 1, 16, grey);
    COLcomputeGradient (&c1, &c2, 1, 16, col);

    g_screens.fugit->rasters[0].topOp.color = g_screens.fugit->rasters[1].topOp.color = grey[15];

    pc  = g_screens.fugit->rasters[0].colors;
    pc2 = g_screens.fugit->rasters[1].colors;

    pc  = COLcomputeGradient (&grey[12], &grey[4] , 1, 3 , pc);
    pc2 = COLcomputeGradient (&grey[12], &grey[4] , 1, 3 , pc2);

    nb = COLcomputeGradientEx (&grey[2], &grey[15], 1, 43, pc, pc2);
    pc  += nb;
    pc2 += nb;

    nb = COLcomputeGradientEx (&grey[15], &grey[15], 1, 3, pc, pc2);
    pc  += nb;
    pc2 += nb;

    pc  = COLcomputeGradient (&grey[15], &grey[1] , 1, 7 , pc);
    pc2 = COLcomputeGradient (&grey[15], &grey[1] , 1, 7 , pc2);

    pc  = COLcomputeGradient (&col[0], &col[8] , 1, 6 , pc);
    pc2 = COLcomputeGradient (&col[0], &col[8] , 1, 6 , pc2);

    {
        u16 remaining = 85 + (((u8*)g_screens.fugit->rasters[0].colors - (u8*)pc) >> 1);    /* pointer arithmetic generates ldiv with Pure C if sizeof != 1 */
        nb = COLcomputeGradientEx (&col[8], &col[15], 1, remaining, pc, pc2);
    }
    pc  += nb;
    pc2 += nb;

    pc  = COLcomputeGradient (&col[15], &col[15], 1, 3, pc);
    pc2 = COLcomputeGradient (&col[15], &col[15], 1, 3, pc2);

    pc  = COLcomputeGradient (&col[15], &grey[15], 1, 6, pc);
    pc2 = COLcomputeGradient (&col[15], &grey[15], 1, 6, pc2);

    pc  = COLcomputeGradient (&grey[13], &col[4], 1, 4, pc);
    pc2 = COLcomputeGradient (&grey[13], &col[4], 1, 4, pc2);

    g_screens.fugit->rasters[0].midLastOp.color |= RASstopMask;
    g_screens.fugit->rasters[1].midLastOp.color |= RASstopMask;
}


void FugitEntry (FSM* _fsm)
{
    LOADrequest* loadRequest;
    u32 fontoffsetsize = LOADmetadataSize         (&RSC_REBIRTH2, RSC_REBIRTH2_METADATA_FUGIT_FONTOFFS_BIN);
    u32 fontsize       = LOADmetadataOriginalSize (&RSC_REBIRTH2, RSC_REBIRTH2_METADATA_FUGIT_FONT_ARJX );
    u32 fontscansize   = LOADmetadataOriginalSize (&RSC_REBIRTH2, RSC_REBIRTH2_METADATA_FUGIT_FONTSCAN_ARJX);

    
    IGNORE_PARAM(_fsm);

    g_screens.fugit = MEM_ALLOC_STRUCT( &sys.allocatorMem, Fugit );	
    DEFAULT_CONSTRUCT(g_screens.fugit);

    g_screens.fugit->auxbuffer       = (u8*) RINGallocatorAlloc ( &sys.mem, 20000 );

    loadRequest = LOADdata (&RSC_REBIRTH2, RSC_REBIRTH2_FUGIT_FONT_ARJX, g_screens.fugit->auxbuffer, LOAD_PRIORITY_INORDER);
    
    g_screens.fugit->deltapacked     = (u8*) RINGallocatorAlloc ( &sys.mem, 40000UL );
    g_screens.fugit->framebuffers[0] = (u8*) RINGallocatorAlloc ( &sys.mem, 32000UL * 2UL );
    g_screens.fugit->framebuffers[1] = g_screens.fugit->framebuffers[0] + 32000;

    g_screens.fugit->font            = (u8*) RINGallocatorAlloc ( &sys.mem, fontsize + fontscansize + fontoffsetsize );
    g_screens.fugit->fontscan        = g_screens.fugit->font + fontsize;
    g_screens.fugit->fontoffset      = g_screens.fugit->fontscan + fontscansize;

    g_screens.fugit->asmbuf          = (u8*) RINGallocatorAlloc ( &sys.mem, FUGbufsize );

    ASSERT(LOADresourceRoundedSize(&RSC_REBIRTH2, RSC_REBIRTH2_FUGIT_FONT_ARJX) < 16000);

    STDmset (g_screens.fugit->framebuffers[0], 0, 64000UL);

    SYSwriteVideoBase ((u32) g_screens.fugit->framebuffers[0]);

    RASsetColReg(0x8242);

    g_screens.fugit->bootFunc = RASvbl1;

    g_screens.fugit->rasters[0].bootOp.scanLinesTo1stInterupt = FUG_FIRST_SCANLINE;
    g_screens.fugit->rasters[0].bootOp.backgroundColor        = 0;
    g_screens.fugit->rasters[0].bootOp.nextRasterRoutine      = RAStop1;

    g_screens.fugit->rasters[0].topOp.backgroundColor         = 0;
    g_screens.fugit->rasters[0].topOp.color                   = 0x700;
    g_screens.fugit->rasters[0].topOp.scanLinesToNextInterupt = 1;
    g_screens.fugit->rasters[0].topOp.nextRasterRoutine       = RASmid1;

    g_screens.fugit->rasters[0].midLastOp.color              = 0x700 | RASstopMask;
    g_screens.fugit->rasters[0].midLastOp.nextRasterRoutine  = NULL;
    g_screens.fugit->rasters[0].midLastOp.scanLineToNextInterupt = 200;

    STDmcpy(&g_screens.fugit->rasters[1], &g_screens.fugit->rasters[0], sizeof(FugitRasters));

    /* blue theme */
    FugitSetGradient(0x112, 0x56F, 0x00, 0xFFF);

    LOADwaitRequestCompleted ( loadRequest );	

    ARJdepack(g_screens.fugit->font    , g_screens.fugit->auxbuffer);
    ARJdepack(g_screens.fugit->fontscan, g_screens.fugit->auxbuffer + LOADmetadataOffset (&RSC_REBIRTH2, RSC_REBIRTH2_METADATA_FUGIT_FONTSCAN_ARJX));   
    STDmcpy  (g_screens.fugit->fontoffset, g_screens.fugit->auxbuffer + LOADmetadataOffset (&RSC_REBIRTH2, RSC_REBIRTH2_METADATA_FUGIT_FONTOFFS_BIN), fontoffsetsize );

    FUGsetData((u32) g_screens.fugit->fontoffset, (u32) g_screens.fugit->fontscan, (u32) g_screens.fugit->font, g_screens.fugit->asmbuf);

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}

void FugitActivity (FSM* _fsm)
{
    RASnextOpList = &g_screens.fugit->rasters[g_screens.fugit->currentrasterbuffer];
    SYSvblroutines[0] = g_screens.fugit->bootFunc;

    g_screens.fugit->currentrasterbuffer ^= 1;

    if (g_screens.fugit->currentdeltabuffer != NULL)
    {
        u8* framebuffer;

        /* *HW_COLOR_LUT = 0x20; */

        g_screens.fugit->currentframebuffer ^= 1;
        framebuffer = g_screens.fugit->framebuffers[g_screens.fugit->currentframebuffer];

        SYSwriteVideoBase ((u32) framebuffer);

        framebuffer += FUG_FIRST_SCANLINE * 160;
        
        g_screens.fugit->currentdeltabuffer = (volatile u8*) FUGdtDpack ((u8*)g_screens.fugit->currentdeltabuffer, g_screens.fugit->auxbuffer);

        if ( g_screens.fugit->currentdeltabuffer != NULL )
        {
            FUGcopyAux2Screen (g_screens.fugit->auxbuffer, framebuffer);

            while ( (void*)(*HW_VECTOR_TIMERB) != NULL );

            FUGxorPass (framebuffer);
        }
    }

    IGNORE_PARAM(_fsm);
}


void FugitBacktask (FSM* _fsm)
{
    u16 t, p;

    for (t = 0 ; t < ARRAYSIZE(FUGtexts) ; t++)
    {
        /* *HW_COLOR_LUT = 0x200; */

        FUGinitSequence (g_screens.fugit->auxbuffer, g_screens.fugit->deltapacked, (u32) &FUGtexts[t]);

        SNDwaitClientStep(STEP_START_FUGIT);

        g_screens.fugit->currentdeltabuffer = g_screens.fugit->deltapacked;

        while (g_screens.fugit->currentdeltabuffer != NULL);

        for (p = 0 ; p < FUGtexts[t].pause ; p++)
        {
            SYSvsync;
        }

        if ( t == 5 )
        {
            /* green theme */
            FugitSetGradient(0x122, 0x5F6, 0x00, 0xFFF);
        }
    }

    if ( FSMisLastState(_fsm) == false )
    {
        FSMgotoNextState (&g_stateMachine);
        ScreenWaitMainDonothing();
    }
}


void FugitExit (FSM* _fsm)
{
    IGNORE_PARAM(_fsm);

    SYSvblroutines[0] = RASvbldonothing;

    RINGallocatorFree ( &sys.mem, g_screens.fugit->auxbuffer );    
    RINGallocatorFree ( &sys.mem, g_screens.fugit->deltapacked );
    RINGallocatorFree ( &sys.mem, g_screens.fugit->framebuffers[0] );
    RINGallocatorFree ( &sys.mem, g_screens.fugit->font );
    RINGallocatorFree ( &sys.mem, g_screens.fugit->asmbuf );

    MEM_FREE(&sys.allocatorMem, g_screens.fugit);
    g_screens.fugit = NULL; 

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
