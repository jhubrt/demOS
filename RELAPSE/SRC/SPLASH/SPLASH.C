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
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\BITMAP.H"

#include "DEMOSDK\PC\EMUL.H"

#include "RELAPSE\SRC\SCREENS.H"
#include "RELAPSE\SRC\SPLASH\SPLASH.H"

#include "RELAPSE\RELAPSE1.H"


#define SPLASH_FRAMEBUFFER_SIZE 32000UL
#define SPLASH_PITCH 160


static void splashDisplayText(Splash* this, u32 datasize_)
{
    u8* p = this->framebuffer;
    u8* px = (u8*)p;
    u16 h, t;


    for (t = 0; t < datasize_; t++)
    {
        u8 c = this->data[t];

        u16 offset = 39 * 64;
        u8* p2 = px;
        u8* font = (u8*)this->font;

        if ((u32)px & 1)
            px += 7;
        else
            px++;

        if (c != ' ')
        {
            if ((c >= 'A') && (c <= 'Z'))
                offset = (c - 'A') * 64;
            else if ((c >= '0') && (c <= '9'))
                offset = ((c - '0') + 26) * 64;
            else
            {
                switch (c)
                {
                case '.': offset = 36 * 64; break;
                case ',': offset = 37 * 64; break;
                case '!': offset = 38 * 64; break;
                case 10:                    break;
                case 13:
                    p += SPLASH_PITCH * 10;
                    px = p;
                    break;
                }
            }

            if (offset != 39 * 64)
            {
                font += offset;

                for (h = 0; h < 8; h++)
                {
                    p2[0] = font[0];
                    p2[2] = font[2];
                    p2[4] = font[4];
                    p2[6] = font[6];

                    p2 += SPLASH_PITCH;
                    font += 8;
                }
            }
        }
    }
}


void SplashEntry (FSM* _fsm)
{
    Splash* this;
    void* temp;
    u8* temp2;
    u16 t;

    u32 fontsize = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_LIQUID_FONT_ARJX);
    u32 datasize = LOADmetadataOriginalSize (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_SPLASH_SPLASH_ARJX);
       
    g_screens.splash = this = MEM_ALLOC_STRUCT( &sys.allocatorMem, Splash );
    DEFAULT_CONSTRUCT(this);

    this->framebuffer = (u8*) MEM_ALLOC ( &sys.allocatorMem, SPLASH_FRAMEBUFFER_SIZE );
    this->font        =       MEM_ALLOC ( &sys.allocatorMem, fontsize );   
    this->data        = (u8*) MEM_ALLOC ( &sys.allocatorMem, datasize );

    temp  =       MEM_ALLOCTEMP( &sys.allocatorMem, LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_FONT_ARJX));
    temp2 = (u8*) MEM_ALLOCTEMP( &sys.allocatorMem, LOADresourceRoundedSize(&RSC_RELAPSE1, RSC_RELAPSE1_SPLASH_SPLASH));

    g_screens.persistent.loadRequest[0] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_SPLASH_SPLASH, temp2, LOAD_PRIORITY_INORDER);    
    g_screens.persistent.loadRequest[1] = LOADdata (&RSC_RELAPSE1, RSC_RELAPSE1_LIQUID_FONT_ARJX, temp, LOAD_PRIORITY_INORDER);    
    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[0]);

    STDmset (this->framebuffer, 0UL, SPLASH_FRAMEBUFFER_SIZE);
    SYSwriteVideoBase((u32)this->framebuffer);

    RELAPSE_WAIT_LOADREQUEST_COMPLETED (g_screens.persistent.loadRequest[1]);

    STDmcpy(this->colors, temp2, 32);
    RELAPSE_UNPACK (this->data, temp2 + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_SPLASH_SPLASH_ARJX));
    RELAPSE_UNPACK (this->font, temp);
    BITpl2chunk(this->font, (u16)(STDdivu(fontsize, 160) & 0xFFFF), 20, 0, this->font);
    
    {   /* Store persistent code */
        u32 fastmenusize = LOADmetadataSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_FASTMENU_ARJX);
        u32 loadersize   = LOADmetadataSize(&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_LOADER_ARJX);

        g_screens.persistent.fastmenu = MEM_ALLOC(&sys.allocatorCoreMem, fastmenusize);
        STDmcpy(g_screens.persistent.fastmenu, temp2 + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_FASTMENU_ARJX), fastmenusize);

        g_screens.persistent.loaderprx = MEM_ALLOC(&sys.allocatorCoreMem, loadersize);
        STDmcpy(g_screens.persistent.loaderprx, temp2 + LOADmetadataOffset (&RSC_RELAPSE1, RSC_RELAPSE1_METADATA_______OUTPUT_RELAPSE_LOADER_ARJX), loadersize);
    }

    MEM_FREE(&sys.allocatorMem, temp);
    MEM_FREE(&sys.allocatorMem, temp2);

    splashDisplayText(this, datasize);

    for (t = 0 ; t <= 16 ; t++)
    {
        SYSvsync;
        COLcomputeGradient16Steps (this->black, (u16*)this->colors, 16, t, HW_COLOR_LUT);
    }
   
    FSMgotoNextState (_fsm);
    FSMgotoNextState (&g_stateMachine);
}


void SplashActivity(FSM* _fsm)
{
    EMULfbExStart (HW_VIDEO_MODE_4P, 80, 40, 80 + SPLASH_PITCH * 2 - 1, 40 + 200 - 1, SPLASH_PITCH, 0);
    EMULfbExEnd();

    if (g_screens.next)
    {
        FSMgotoNextState (&g_stateMachineIdle);
        FSMgotoNextState (_fsm);
    }
}


void SplashExit (FSM* _fsm)
{
    Splash* this = g_screens.splash;
    u16 t;

    for (t = 0 ; t <= 16 ; t++)
    {
        SYSvsync;
        COLcomputeGradient16Steps ((u16*)this->colors, this->black, 16, t, HW_COLOR_LUT);
    }

    MEM_FREE ( &sys.allocatorMem, this->font );
    MEM_FREE ( &sys.allocatorMem, this->data );
    MEM_FREE ( &sys.allocatorMem, this->framebuffer );

    MEM_FREE( &sys.allocatorMem, g_screens.splash);
    g_screens.splash = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    g_screens.next = false;

    ScreenNextState(_fsm);
}
