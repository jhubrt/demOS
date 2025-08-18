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
#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\BITMAP.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "EXTERN\ARJDEP.H"

#include "FX\COLPLANE\COLPLANE.H"

#include "BLITZIK\SRC\SCREENS.H"
#include "BLITZIK\SRC\INTRO\INTRO.H"

#include "BLITZIK\BLITZWAV.H"


#define INTRO_DISPLIST_SIZE  420000UL

#define NBFRAMEBUFFERS      2
#define NBMAXEDGES_PERPOLY  32
#define SIZEMAX_PERPOLY     ((NBMAXEDGES_PERPOLY * 4 + 4) * sizeof(u16))

#define BENCHMARK() 0

#if BENCHMARK()
static u16 g_benchColorMask = 0x303;
#endif

static bool CybervectorAnimation_Scene0 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    state->animState.angle = (state->animState.angle + 4) & 511;
    
    if (state->animState.angle == 0)
    {
        return true;
    }

    if (g_screens.cybervector->exit)
    {
        return true;
    }

    return false;
}

static bool CybervectorAnimation_Scene1 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    state->animState.coef += state->ic;
    if ( state->animState.coef == (127 << 8) )
    {
        state->ic = -state->ic;
    }
    else if ( state->animState.coef == 0 )
    {
        return true;
    }

    state->animState.xdep += state->icx;
    if ((state->animState.xdep == (-256 * 16)) || (state->animState.xdep == (256 * 16)))
    {
        state->icx = -state->icx;
    }

    /* increment angle */
    state->animState.angle = (state->animState.angle + 2) & 511;

    if (g_screens.cybervector->exit)
    {
        return true;
    }

    return false;
}

static bool CybervectorAnimation_Scene2 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    if ( state->animState.coef >= 32767 )
    {
        return true;
    }

    switch (state->state)
    {
    case 0:     /* init */
        state->ic      = 2;
        state->state   = 1;
        /* no break here ! */

    case 1:
        /*_state->ic += 3;  */
        state->animState.coef += state->ic;

        if (( state->animState.coef > 2000 ) && (state->animState.angle == 0))
        {
            state->state = 2;
        }
        else
        {
            state->animState.angle = (state->animState.angle + 4) & 511;
            if ((state->animState.angle & 63) == 0)
            {
                state->ic++;
            }
        }

        break;

    case 2:
        state->animState.coef += 16;

        if ( state->animState.coef > 3400 )
        {
            state->state = 3;
            state->ic    = 64;
        }
        break;

    case 3:
        state->animState.coef += state->ic;
        state->ic++;

        if ( state->animState.coef > 8000 )
        {
            state->state = 4;
        }
        break;

    case 4:
        state->animState.coef += state->ic;
        state->ic++;
        state->animState.angle = (state->animState.angle + 4) & 511;

        break;
    }

    if (g_screens.cybervector->exit)
    {
        return true;
    }

    return false;
}

static bool CybervectorAnimation_Scene3 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    if ( state->animState.coef >= 32767 )
    {
        return true;
    }

    switch (state->state)
    {
    case 0:     /* init */
        state->ic      = 1;
        state->state   = 1;
        state->count   = 0;
        
        state->animState.angle = 500;
        /* no break here ! */

    case 1:
        state->ic++;
        state->animState.coef += state->ic;
        state->count++;

        if (state->count >= 19)
        {
            state->count = 0;
            state->state = 2;
        }

        state->animState.angle = (state->animState.angle + 1) & 511;

        break;

    case 2:
        state->animState.coef--;
        state->count++;

        if (state->count >= 18)
        {
            state->count   = 0;

            if ( state->animState.coef > 8000 )
            {
                state->state = 3;
            }
            else
            {
                state->state = 1;
            }
        }

        state->animState.angle = (state->animState.angle + 511) & 511;

        break;

    case 3:
        state->animState.coef += state->ic;
        state->ic++;
        state->animState.angle = (state->animState.angle + 1) & 511;
    }

    if (g_screens.cybervector->exit)
    {
        return true;
    }

    return false;
}

static bool CybervectorAnimation_Scene4 (VECanimationState* _state)
{
    PZanimationState* state = (PZanimationState*) _state;

    if ( state->animState.coef >= 26000 )
    {
        return true;
    }

    state->animState.coef += state->ic >> 2;
    state->ic++;
    state->animState.angle = (state->animState.angle + 3) & 511;

    if (g_screens.cybervector->exit)
    {
        return true;
    }

    return false;
}


static void pzcheckList (u16* _dlist)
{
#   if BENCHMARK()
    *HW_COLOR_LUT ^= g_benchColorMask;
#   endif

#   ifdef DEMOS_ASSERT
    {
        u32 displistsize = ((u8*)_dlist - (u8*)g_screens.cybervector->displist);
        ASSERT (displistsize <= INTRO_DISPLIST_SIZE);
    }
#   endif
}

#define GREYSCALE_MODE  2
#define GREYSCALE_MODE2 3 


static u16 Intro_firstColors[] = 
{
    0x7, 0x0, 0x7, 0x0, 0x7, 0x0, 0x7, 0x0, 0x7, 0x0, 0x7, 0x0, 0x7, 0x0, 0x7
};

static u16 Intro_firstColors2[] = 
{
    0x7, 0x111, 0x111, 0x222, 0x222, 0x333, 0x333, 0x444, 0x444, 0x555, 0x555, 0x666, 0x660, 0xFFF, 0xFFF,
    
    0x666, 0x666, 0x555, 0x555, 0x444, 0x444, 0x222, 0x222, 0x444, 0x444, 0x666, 0x666, 0xFFF, 0xFFF,
    0x666, 0x666, 0x555, 0x555, 0x444, 0x444, 0x222, 0x222, 0x444, 0x444, 0x666, 0x666, 0xFFF, 0xFFF
};

void CybervectorEnter (FSM* _fsm)
{
    Cybervector *this = g_screens.cybervector; /* for async reentrance on PC */
	u16 polysDataTempSize = (u16) LOADresourceRoundedSize ( &RSC_BLITZWAV, RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC);
    u8* temp;
	LOADrequest* loadRequest;
    EMUL_STATIC u16* dlist;
    EMUL_STATIC u16 sceneindex;


    EMUL_BEGIN_ASYNC_REGION
        
    IGNORE_PARAM(_fsm);

    ScreensLogFreeArea("CybervectorEntry");

    this = g_screens.cybervector = MEM_ALLOC_STRUCT( &sys.allocatorMem, Cybervector );	
    DEFAULT_CONSTRUCT(this);

    /* Trick to save memory : use the fact 1: intro is played once at the begining 2: the muzik for intro is smaller than other ones */
    /* => allocate buffer into coremem in this case to lower max memory use for sys.mem to enlarge sys.coremem for other muziks      */
    this->framebuffers[0] = (u8*) MEM_ALLOC ( &sys.allocatorCoreMem, (u32)VEC_PITCH * 200UL * (u32)NBFRAMEBUFFERS );  
    this->framebuffers[1] = this->framebuffers[0] + (u32)VEC_PITCH * 200UL;
    /*STDmset (this->framebuffers[0], 0, (u32)VEC_PITCH * 200UL * (u32)NBFRAMEBUFFERS); logo will clear this */

	temp = (u8*) MEM_ALLOCTEMP ( &sys.allocatorMem, polysDataTempSize );

	loadRequest  = LOADdata (&RSC_BLITZWAV, RSC_BLITZWAV_POLYZOOM_POLYZOOM_VEC, temp, LOAD_PRIORITY_INORDER);
    LOADwaitRequestCompleted ( loadRequest );	

    SYSvblroutines[0] = g_screens.blzupdateroutine; /* start only there to have a determinist time (to depending on floppy access) */
    g_screens.runscreen = BLZ_EP_MENU;

    BlitZsetVideoMode(HW_VIDEO_MODE_4P, 0, BLITZ_VIDEO_16XTRA_PIXELS);

    ARJdepack(this->framebuffers[0] + 2, temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_PARTY_ARJX));
    BITpl2chunk(this->framebuffers[0] + 2, 200, 21, 0, this->framebuffers[0] + 2);
    *(u16*)this->framebuffers[0] = 0;
    STDmcpy2 (this->framebuffers[1], this->framebuffers[0], (u32)VEC_PITCH * 200UL);

    VECsceneConstruct(&this->scenes[0], temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC), (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC));
    this->colormodes[0] = GREYSCALE_MODE;

    VECsceneConstruct(&this->scenes[2], temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_CYBERNET_VEC), (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_CYBERNET_VEC));
    this->colormodes[2] = COLP_GRADIENT_MODE;

    VECsceneConstruct(&this->scenes[3], temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_PRESENT_VEC ), (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_PRESENT_VEC));
    this->colormodes[3] = COLP_GRADIENT_MODE;

    VECsceneConstruct(&this->scenes[4], temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZWAV_VEC), (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZWAV_VEC));
    this->colormodes[4] = COLP_RAIMBOW_MODE;

    VECsceneConstruct(&this->scenes[5], temp + LOADmetadataOffset(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC), (u16) LOADmetadataSize(&RSC_BLITZWAV, RSC_BLITZWAV_METADATA_POLYZOOM_BLITZ_VEC));
    this->colormodes[5] = COLP_RAIMBOW_MODE;

    {
        static u16 start [4] = {0xFFF, 0x200, 0xB00, 0x700};    /* Red */ 
        static u16 end   [4] = {0xFFF, 0x882, 0x88B, 0x8AF};    /* Blue */ 

        this->colors[COLP_GRADIENT_MODE] = COLPinitColors4Pgradient(&sys.allocatorMem, start, end, false);
    }

    {
        /* Raimbow  */
        static u16 colors [4] = {0x222, 0x007, 0x070, 0x700};
        /*static u16 end   [4] = {0x070, 0x222, 0x007, 0x700};*/

        this->colors[COLP_RAIMBOW_MODE] = COLPinitColors4Praimbow(&sys.allocatorMem, colors, false);
    }

    {
        /* Greyscale */
        static u16 colors [4] = {0x666, 0x222, 0x111, 0x888};

        this->colors[GREYSCALE_MODE] = COLPinitColors4Praimbow(&sys.allocatorMem, colors, false);
    }

    {
        /* Greyscale 2 */
        static u16 colors [4] = {0xEEE, 0x444, 0x222, 0x111};

        this->colors[GREYSCALE_MODE2] = COLPinitColors4Praimbow(&sys.allocatorMem, colors, false);
    }

    this->coloranimation.colormode = this->colormodes[0];
    this->coloranimation.colors    = this->colors[this->coloranimation.colormode];

    this->cycling = 2;

    {   
        PZanimationState* state;
        
        /* INIT SCENES */
        this->animationCallback[0] = CybervectorAnimation_Scene0;
        this->scenes[0].nbrepeat   = 11;

        state = &this->animationState[0];
        state->ic      = 0;
        state->icx     = 0;
        state->animState.coef = 4900;

        this->scenes[1].nbframes = 20;

        this->animationCallback[2] = CybervectorAnimation_Scene1;
        state = &this->animationState[2];
        state->ic      = 127;
        state->icx     = 32;

        this->animationCallback[3] = CybervectorAnimation_Scene2;

        this->animationCallback[4] = CybervectorAnimation_Scene3;

        state = &this->animationState[4];
        state->ic      = 1;
        state->icx     = 0;

        this->animationCallback[5] = CybervectorAnimation_Scene4;
        state = &this->animationState[5];
        state->ic      = 4;
    }

	MEM_FREE (&sys.allocatorMem, temp);

	/* COMPUTE NB TOTAL EDGES 
	{
	u16 nbTotalEdges = 0;
	u16* p = this->polygonsList;

	while ( *(u32*)p )
	{
	u16	nbEdges = **(u16**)p;
	nbTotalEdges += PCENDIANSWAP16(nbEdges);
	p += 3;
	}

	printf ("nbTotalEdges = %d\n", nbTotalEdges);
	}*/
   
    this->polylinestmp = (u16*) MEM_ALLOC ( &sys.allocatorMem, SIZEMAX_PERPOLY );

    this->displist     = (u16*) MEM_ALLOC ( &sys.allocatorMem, INTRO_DISPLIST_SIZE );
	this->displistp    = this->displist;
	this->currentFrame = 0;

    {
        u16 i;

        for (i = 0 ; i < ARRAYSIZE(this->erase) ; i++)
            this->erase[i] = this->displist;
    }

    /* PRECOMPUTE VERTICES POSITIONS */	
    dlist = this->displist;

    for (sceneindex = 0 ; (sceneindex < PZ_NBSCENES) && (this->exit == false) ; )
    {
        EMUL_REENTER_POINT;
        {
            VECscene* scene = &this->scenes[sceneindex];
            VECanimationCallback animate = this->animationCallback[sceneindex];

            if (animate != NULL)
            {
                VECanimationState* animationState = &(this->animationState[sceneindex].animState);
                /* CybervectorAnimationState* animationState = &(scene->animationState);  MAKES A FUCKING COMPILER ERROR :( */
                u16* scenedisplist = dlist;


                scene->displist = scenedisplist;

                dlist = VECscenePrecompute(scene, dlist, pzcheckList, g_screens.base.cos, g_screens.base.sin, animate, animationState, this->polylinestmp, SIZEMAX_PERPOLY);

                TRAClogNumber10S(TRAC_LOG_SPECIFIC, "animsize", ((u8*)dlist - (u8*)this->displist), 8, '\n');
 
                if (sceneindex == 0)
                {
                    SYSvsync;
                    SYSwriteVideoBase((u32)this->framebuffers[0]);
                    FSMgotoNextState(&g_stateMachine);
                }
                else if (sceneindex == 3)
                {
                    do
                    {
                        EMUL_BACKUP_REENTER_POINT;
                        EMUL_REENTER_POINT;
                        if (this->exit) goto Stop;
                        EMUL_EXIT_IF(this->currentScene < 3);
                        EMUL_RESTORE_REENTER_POINT;
                    } 
                    while (this->currentScene < 3);
                    dlist = (u16*)this->scenes[0].displist; /* reuse buffer of scene 0 to save ram */
                }
            }
        }

#       if BENCHMARK()
        g_benchColorMask ^= 0x333;
#       endif

        sceneindex++;
        EMUL_EXIT_IF(sceneindex < PZ_NBSCENES)

Stop: ;

    } /* foreach scene */    

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_MEMORY))
    {
        TRAClog(TRAC_LOG_MEMORY, "Intro memallocator dump", '\n');
        TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
        RINGallocatorDump(sys.allocatorMem.allocator, stdout);
    }
#   endif

    MEM_FREE(&sys.allocatorMem, this->polylinestmp);
    this->polylinestmp = NULL;

    FSMgotoNextState (&g_stateMachineIdle);

    EMUL_END_ASYNC_REGION
}


void CybervectorActivity(FSM* _fsm)
{
    Cybervector* this = g_screens.cybervector;


    IGNORE_PARAM(_fsm);

    this->currentFrame++;

    if (this->currentFrame >= 50)
    {
        this->currentFrame = 0;
        FSMgotoNextState(&g_stateMachine);
    }
}


void CybervectorActivity2 (FSM* _fsm)
{
	/*s16 miny, maxy;*/
    Cybervector* this           = g_screens.cybervector;
    u16       currentplane      = this->coloranimation.currentplane;
    void*     framebuffer       = this->framebuffers[currentplane & 1];
	u8*       image             = (u8*) framebuffer;
    u16*      currentlist       = this->displistp;
    u16       currentsceneindex = this->currentScene;
    VECscene* scene             = &this->scenes[currentsceneindex];

  
    IGNORE_PARAM(_fsm);

#   ifndef __TOS__
    EMULfbExStart(HW_VIDEO_MODE_4P, 64, 63, 64 + VEC_PITCH * 2 - 1, 63 + 200 - 1, VEC_PITCH, 0);
    EMULfbExEnd();
#   endif

    if (scene->nbPolygons > 0)
    {
        if (this->currentScene == 0)
        {
            u16 colors[15];

            enum Scene0State
            {
                S0_FADEIN_VECTOR,
                S0_WAITFOR_FADEIN_LOGO,
                S0_FADEIN_LOGO,
                S0_WAITFOR_CYCLING,
                S0_FADETO_CYCLINGCOLORS,
                S0_CYCLINGCOLORS,
                S0_FADETO_LOGOCOLORS,
                S0_WAITFOR_FADEOUT_LOGO,
                S0_FADEOUT_LOGO, 
                S0_WAITFOR_FADEOUT_VECTOR,
                S0_FADEOUT_VECTOR,
                S0_END
            };

            switch (this->scene0State)
            {
            case S0_FADEIN_VECTOR:
                if (this->currentFrame <= 32)
                    COLcomputeGradient16Steps(this->black, Intro_firstColors, 15, this->currentFrame >> 1, this->cyclingcolors);
                else 
                {
                    this->scene0State = S0_WAITFOR_FADEIN_LOGO;
                    this->scene0FXframe = 0;
                }                    
                break;

            case S0_WAITFOR_FADEIN_LOGO:                
                if (g_screens.player.trackindex == 1)
                {
                    this->scene0FXframe++;
                    if (this->scene0FXframe >= 40)
                    {
                        this->scene0State   = S0_FADEIN_LOGO;
                        this->scene0FXframe = 0;
                    }
                }
                break;

            case S0_FADEIN_LOGO:
                if (this->scene0FXframe <= 32)
                {
                    COLcomputeGradient16Steps(Intro_firstColors, Intro_firstColors2, 15, this->scene0FXframe >> 1, this->cyclingcolors);
                    this->scene0FXframe++;
                }
                else
                {
                    this->scene0State = S0_WAITFOR_CYCLING;
                }
                break;

            case S0_WAITFOR_CYCLING:
                if (g_screens.player.trackindex == 2)
                {
                    this->scene0State = S0_FADETO_CYCLINGCOLORS;
                    this->scene0FXframe = 0;
                }
                break;

            case S0_FADETO_CYCLINGCOLORS:
            case S0_CYCLINGCOLORS:
            case S0_FADETO_LOGOCOLORS:

                colors[0] = Intro_firstColors2[0];
                STDmcpy2(colors + 1, Intro_firstColors2 + (this->cycling >> 1), 14 * sizeof(u16));

                switch (this->scene0State)
                {
                case S0_FADETO_CYCLINGCOLORS:
                    if (this->scene0FXframe <= 32)
                    {
                        COLcomputeGradient16Steps(Intro_firstColors2, colors, 15, this->scene0FXframe >> 1, this->cyclingcolors);
                        this->scene0FXframe++;
                    }                    
                    else
                    {
                        STDmcpy2(this->cyclingcolors, colors, 15 * sizeof(u16));
                        this->scene0State = S0_CYCLINGCOLORS;
                        this->scene0FXframe = 0;
                    }
                    break;

                case S0_CYCLINGCOLORS:

                    STDmcpy2(this->cyclingcolors, colors, 15 * sizeof(u16));

                    if (g_screens.player.trackindex >= 3)
                    {
                        this->scene0FXframe++;
                        if (this->scene0FXframe > 250)
                        {
                            this->scene0State = S0_FADETO_LOGOCOLORS;
                            this->scene0FXframe = 0;
                        }
                    }
                    break;

                case S0_FADETO_LOGOCOLORS:
                       
                    if (this->scene0FXframe <= 32)
                    {
                        COLcomputeGradient16Steps(colors, Intro_firstColors2, 15, this->scene0FXframe >> 1, this->cyclingcolors);
                        this->scene0FXframe++;
                    }
                    else
                    {
                        STDmcpy2(this->cyclingcolors, Intro_firstColors2, 15 * sizeof(u16));
                        this->scene0State = S0_WAITFOR_FADEOUT_LOGO;
                        this->scene0FXframe = 0;
                    }

                    break;
                }

                this->cycling++;

                if (this->cycling >= (15 + 14) * 2)
                    this->cycling = 15 * 2;

                break;

            case S0_WAITFOR_FADEOUT_LOGO:
                if (g_screens.player.trackindex >= 4)
                {
                    this->scene0FXframe++;
                    if (this->scene0FXframe >= 40)
                    {
                        this->scene0State   = S0_FADEOUT_LOGO;
                        this->scene0FXframe = 0;
                    }
                }
                break;

            case S0_FADEOUT_LOGO:
                if (this->scene0FXframe <= 32)
                {
                    COLcomputeGradient16Steps(Intro_firstColors2, Intro_firstColors, 15, this->scene0FXframe >> 1, this->cyclingcolors);
                    this->scene0FXframe++;
                }
                else
                {
                    this->scene0State   = S0_WAITFOR_FADEOUT_VECTOR;
                    this->scene0FXframe = 0;
                }
                break;

            case S0_WAITFOR_FADEOUT_VECTOR:
                if ((this->repeatcount + 1) >= scene->nbrepeat)
                    if (this->currentFrame >= (scene->nbframes - 33))
                    {
                        this->scene0State   = S0_FADEOUT_VECTOR;
                        this->scene0FXframe = 0;
                    }
                break;

            case S0_FADEOUT_VECTOR:
                if (this->scene0FXframe <= 32)
                {
                    COLcomputeGradient16Steps(Intro_firstColors, this->black, 15, this->scene0FXframe >> 1, this->cyclingcolors);
                    this->scene0FXframe++;
                }
                else
                    this->scene0State = S0_END;
                break;
            }

            STDmcpy2(HW_COLOR_LUT + 1, this->cyclingcolors, 15 * sizeof(u16));
            PCENDIANSWAPBUFFER16(HW_COLOR_LUT + 1, 15);

            this->coloranimation.currentplane ^= 1;
        }
        else if ((this->currentScene == 4) || (this->currentScene == 5))
        {
            image += COLPanimate4P(&this->coloranimation);

            if ((this->repeatcount + 1) >= scene->nbrepeat)
                if (this->currentFrame >= (scene->nbframes - 32))
                {
                    COLcomputeGradient16Steps(this->black, HW_COLOR_LUT + 1, 15, (scene->nbframes - this->currentFrame) >> 1, HW_COLOR_LUT + 1);
                    PCENDIANSWAPBUFFER16(HW_COLOR_LUT + 1, 15);
                }
        }
        else
        {
            image += COLPanimate4P (&this->coloranimation);
        }

        /*ASSERT( this->frames[this->currentFrame] == currentlist );*/
        VECclrpass();
        VECclr(image, this->erase[currentplane]);

        this->displistp = VECloop(image, currentlist + 4, scene->nbPolygons);

        if (this->currentScene == 0)
        {
            VECcircle(image, 168, 100, 80);
            
            currentlist[0] = 32;
            currentlist[1] = 13;
            currentlist[2] = 20;
            currentlist[3] = 180;
        }

        VECxorpass(0x206);
        VECxor(image, currentlist);

        this->erase[currentplane] = currentlist;
    }
    
    this->currentFrame++;

    if ( this->currentFrame >= scene->nbframes )
    {
        this->currentFrame = 0;
        this->repeatcount++;

        if (this->repeatcount >= scene->nbrepeat)
        {
            this->repeatcount = 0;

            if (this->currentScene < PZ_NBSCENES)
            {
                VECscene* nextScene = scene + 1;

                if ((nextScene->displist != NULL) || (nextScene->nbPolygons == 0))
                {
                    STDfastmset(this->framebuffers[0], 0, (u32)VEC_PITCH * 200UL * (u32) NBFRAMEBUFFERS);
                    this->currentScene++;
                    scene++;
                }
            }
            else
            {
                this->currentScene = 0;
                scene = this->scenes;
                this->exit = true;
            }
        }

        this->coloranimation.colormode = this->colormodes[this->currentScene];
        this->coloranimation.colors    = this->colors[this->coloranimation.colormode];

        this->displistp = (u16*)scene->displist;
	}

    SYSwriteVideoBase((u32)framebuffer);

    while(BLZ_COMMAND_AVAILABLE)
    {
        if (BLZ_CURRENT_COMMAND == BLZ_CMD_SELECT)
        {
            if ((g_screens.compomode == false) || BLZ_DEVMODE())
            {
                this->exit = true;
            }
        }

        BLZ_ITERATE_COMMAND;
    }

#   if BENCHMARK()
    *HW_COLOR_LUT = 4;
#   endif
}


void CybervectorBacktask (FSM* _fsm)
{
    Cybervector *this = g_screens.cybervector;

    IGNORE_PARAM(_fsm);

    if (this->exit)
    {
        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (&g_stateMachine);
    }
}


void CybervectorExit (FSM* _fsm)
{
    Cybervector *this = g_screens.cybervector;

	IGNORE_PARAM(_fsm);

	MEM_FREE (&sys.allocatorCoreMem, this->framebuffers[0]);

	MEM_FREE (&sys.allocatorMem, this->displist);
    MEM_FREE (&sys.allocatorMem, this->colors[COLP_GRADIENT_MODE]);
    MEM_FREE (&sys.allocatorMem, this->colors[COLP_RAIMBOW_MODE]);
    MEM_FREE (&sys.allocatorMem, this->colors[GREYSCALE_MODE]);
    MEM_FREE (&sys.allocatorMem, this->colors[GREYSCALE_MODE2]);

    VECsceneDestroy(&this->scenes[0]);
    VECsceneDestroy(&this->scenes[2]);
    VECsceneDestroy(&this->scenes[3]);
    VECsceneDestroy(&this->scenes[4]);
    VECsceneDestroy(&this->scenes[5]);

	MEM_FREE (&sys.allocatorMem, g_screens.cybervector);
	this = g_screens.cybervector = NULL;

/*
#   if DEMOS_MEMDEBUG
    TRACmemDump(&sys.allocatorMem, stdout, ScreensDumpRingAllocator);
    RINGallocatorDump(sys.allocatorMem.allocator, stdout);
#   endif
*/

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
    FSMgotoNextState (&g_stateMachine);
}
