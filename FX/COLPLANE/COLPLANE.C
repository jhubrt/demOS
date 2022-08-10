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

#define COLPLANE_C

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"
#include "DEMOSDK\COLORS.H"
#include "DEMOSDK\HARDWARE.H"

#include "FX\COLPLANE\COLPLANE.H"

#define COLP_NBPALETTES    32

/*---------------------------------------------------------------------------
    BUFFER | 0               1
    PLANE  | 0   1   2   3   0   1   2   3
           |
0:         | C                           D         
1:         | D               C           
2:         |     C           D                
3:         |     D               C                
4:         |         C           D       
5:         |         D               C   
6:         |              C          D   
7:         |              D              C

---------------------------------------------------------------------------*/
static u8 COLPstep2Plane4[8] = {3, 0, 0, 1, 1, 2, 2, 3};

/*  Examples of colors
    ----------------------------------
    Raimbow 
         {0x222, 0x007, 0x070, 0x700}
         {0x070, 0x222, 0x007, 0x700}
    ----------------------------------
    Red  {0x777, 0x200, 0xB00, 0x700}
    Blue {0x777, 0x882, 0x88B, 0x8AF}
*/

static void colpInitColors4P(u16* pal_, u16 nbplanes_, bool _combinemax, u16** colors_)
{   
    u16 t, i, p;
    u16* colors = *colors_;


    for (p = nbplanes_; p > 0; p--)       /* for each plane shift case */
    {
        *colors++ = 0;

        for (t = 1; t < 16; t++)  /* for each 16 colors */
        {
            u16 max = 0;

            if (_combinemax)
            {
                static u8  prio[4] = { 4, 1, 2, 3 };
                u16 maxprio = 0;

                for (i = 0; i < 4; i++)   /* for each bit */
                {
                    if (t & (1 << i))
                    {
                        u16 valprio = prio[(i + p) & 3];

                        if (valprio > maxprio)
                        {
                            maxprio = valprio;
                            max = pal_[(i + p) & 3];
                        }
                    }
                }
            }
            else
            {
                for (i = 0; i < 4; i++)   /* for each bit */
                {
                    if (t & (1 << i))
                    {
                        u16 index = i + p;

                        if (index >= nbplanes_)
                            index -= nbplanes_;
                        
                        max |= pal_[index];
                    }
                }
            }

            *colors++ = PCENDIANSWAP16(max);
        } /* colors loop */
    } /* planeshift loop */

    *colors_ = colors;
}

u16* COLPinitColors4Pgradient(MEMallocator* _allocator_, u16 _start[4], u16 _end[4], bool _combinemax)
{   
    u16* colorsbuffer;
    u16* colors;
    u16  pal [4];
    u16  g;


    colorsbuffer = colors = (u16*) MEM_ALLOC ( _allocator_, COLP_NBPALETTES*sizeof(u16)*16*4 );

    for (g = 0 ; g < COLP_NBPALETTES ; g++)
    {
        if (g < 16)
        {
            COLcomputeGradient16Steps (_start, _end, 4, g, pal);
        }
        else
        {
            COLcomputeGradient16Steps (_end, _start, 4, g - 16, pal);
        }

        colpInitColors4P(pal, 4, _combinemax, &colors);
    }

    return colorsbuffer;
}

u16* COLPinitColors4Praimbow(MEMallocator* _allocator_, u16 _colors[4], bool _combinemax)
{   
    u16* colorsbuffer;
    u16* colors;


    colorsbuffer = colors = (u16*) MEM_ALLOC ( _allocator_, sizeof(u16)*16*4 );

    colpInitColors4P(_colors, 4, _combinemax, &colors);

    return colorsbuffer;
}

u16 COLPanimate4P(COLPanimation* animation_)
{
    u16 offset;
    s16 currentplane = animation_->currentplane;
    u16 colorindex   = COLPstep2Plane4[currentplane] << 4;

    
    if (animation_->colormode == COLP_GRADIENT_MODE)
    {
        colorindex += animation_->currentpal;
    }

    ASSERT(currentplane < ARRAYSIZE(COLPstep2Plane4));

    STDmcpy2(HW_COLOR_LUT, &animation_->colors[colorindex], 32);
    offset = currentplane & 6;

    animation_->currentplane++;
    animation_->currentplane &= 7;

    if (animation_->colormode == COLP_GRADIENT_MODE)
    {
        if ((animation_->currentplane) == 0) /* divide colors speed by 4 */
        {
            animation_->currentpal += 16 * 4;

            if (animation_->currentpal >= (COLP_NBPALETTES * 4 * 16))
            {
                animation_->currentpal = 0;
            }
        }
    }

    return offset;
}


/*------------------------------------------------------------------------------
 ONE BUFFER: PLANE currently Cleared & Drawn is invisible like a back buffer

PLANE      | 0   1   2   3   
|
0:         | CD  
1:         |     CD           
2:         |         CD     
3:         |             CD
------------------------------------------------------------------------------*/

void COLPinitColors3P(u16 _colors[3], u16 backcolor_, u16** colors_)
{
    u16 p,t,i;
    u16* colors = *colors_;


#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_SPECIFIC))
    {
        for (t = 0; t < 16; t++)
            printf ("%-2d  ", t);
        printf ("\n");
    }
#   endif

    for (p = 0; p < 4; p++)
    {
        u16 pal[4];

        pal[p] = backcolor_;
        pal[(p + 1) & 3] = _colors[0];
        pal[(p + 2) & 3] = _colors[1];
        pal[(p + 3) & 3] = _colors[2];

        for (t = 0; t < 16; t++)
        {
            u16 color = 0;
            u16 bit = 1;

            for (i = 0; i < 4; i++)
            {
                if (t & bit)
                {
                    color |= pal[i];
                }

                bit <<= 1;
            }

#           ifndef __TOS__
            if (TRACisSelected(TRAC_LOG_SPECIFIC))
            {
                if (p == 0)
                    printf ("%-3x ", color);
            }
#           endif

            *colors++ = PCENDIANSWAP16(color);
        }
    }

#   ifndef __TOS__
    if (TRACisSelected(TRAC_LOG_SPECIFIC))
    {
        printf ("\n");
    }
#   endif

    *colors_ = colors;
}


void COLPinitColors3Praimbow(u16 _colors[3], u16 backcolor_, u16 colorsbuffer[16*4])
{
    u16* colors = colorsbuffer;

    COLPinitColors3P(_colors, backcolor_, &colors);
}


u16* COLPinitColors3Pgradient(MEMallocator* _allocator_, u16 _start[3], u16 _end[3], u16 backcolor_)
{
    u16* colors;
    u16* colorsbuffer;
    u16 g;
    u16 pal[4];


    colorsbuffer = colors = (u16*) MEM_ALLOC ( _allocator_, COLP_NBPALETTES*sizeof(u16)*16*4 );

    for (g = 0 ; g < COLP_NBPALETTES ; g++)
    {
        if (g < 16)
        {
            COLcomputeGradient16Steps (_start, _end, 3, g, pal);
        }
        else
        {
            COLcomputeGradient16Steps (_end, _start, 4, g - 16, pal);
        }

        COLPinitColors3P(pal, backcolor_, &colors);
    }

    return colorsbuffer;
}


u16 COLPanimate3P(COLPanimation* animation_)
{
    s16 currentplane = animation_->currentplane;
    u16 offset;
    u16 colorindex = currentplane << 4;
   

    if (animation_->colormode == COLP_GRADIENT_MODE)
    {
        colorindex += animation_->currentpal;
    }

    STDmcpy2(HW_COLOR_LUT, &animation_->colors[colorindex], 32);
    offset = currentplane << 1;

    animation_->currentplane++;
    animation_->currentplane &= 3;

    if (animation_->colormode == COLP_GRADIENT_MODE)
    {
        if ((animation_->currentplane) == 0) /* divide colors speed by 4 */
        {
            animation_->currentpal += 16 * 4;

            if (animation_->currentpal >= (COLP_NBPALETTES * 4 * 16))
            {
                animation_->currentpal = 0;
            }
        }
    }

    return offset;
}
