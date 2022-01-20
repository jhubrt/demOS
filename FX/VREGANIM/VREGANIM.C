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

#include "DEMOSDK\STANDARD.H"

#include "FX\VREGANIM\VREGANIM.H"


u16* VANIanimate(VANIanimation** _animations, u16 _nbAnimations, u16 _nbLines, u16* _p, u16* _bufferend)
{
    u16 t;
    bool run;
    bool changed;
    u16* lastnbframes = _p;

    do
    {
        run = false;
        changed = false;

        *_p = 0; /* nb frames */

        STDmset(_p, 0, _nbLines*sizeof(u16));

        for (t = 0 ; t < _nbAnimations ; t++)
        {
            /*char temp[16];
            
            STDuxtoa (temp, _logos[t]->nbFrames, 10);
            SYSdebugPrint ( g_screens.sshade->bitmaps, VIS_SCREEN_PITCH, SYS_2P_BITSHIFT, _logos[t]->ysrc >> 3, 0, temp );*/

            changed |= _animations[t]->animateFunc(_animations[t], _p + 1);
            run     |= (_animations[t]->nbFrames > 0);
        }

        if ( changed )
        {
            lastnbframes = _p;
            _p[_nbLines] = 0;
            _p += _nbLines + 1;
        }
        else
        {
            ASSERT(lastnbframes != _p);
            (*lastnbframes)++;
        }

        ASSERT (_p < _bufferend);

        /* *HW_COLOR_LUT ^= 0x30; */
    }
    while (run);

    return _p;
}


