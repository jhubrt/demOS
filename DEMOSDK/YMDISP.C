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

#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\YMDISP.H"

#ifndef SND_YM_NB_CHANNELS
#   define SND_YM_NB_CHANNELS 3
#endif


void SNDYMdrawYMstate (u8 _ymregs[16], void* _display)
{
    u8   t;
    u8*  pinstr = (u8*) _display + 8;
    u16* m      = (u16*)_display;


    /* Acquire YM state once at vbl */
    u8  envfreql  = _ymregs[HW_YM_SEL_FREQENVELOPE_L];
    u8  envfreqh  = _ymregs[HW_YM_SEL_FREQENVELOPE_H];
    u16 envfreq   = ((u16)envfreqh << 8) | envfreql;
    u8  noisefreq = _ymregs[HW_YM_SEL_FREQNOISE];
    u8  mix       = _ymregs[HW_YM_SEL_IO_AND_MIXER];
    u8  envshape  = _ymregs[HW_YM_SEL_ENVELOPESHAPE];
    u8  level   [SND_YM_NB_CHANNELS];
    u16 chfreq  [SND_YM_NB_CHANNELS];
    u8  mask = 0x9;

       
    for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
    {
        u8 freqh, freql;


        level[t]  = _ymregs[HW_YM_SEL_LEVELCHA  + t];
        freql     = _ymregs[HW_YM_SEL_FREQCHA_L + (t << 1)];
        freqh     = _ymregs[HW_YM_SEL_FREQCHA_H + (t << 1)];
        chfreq[t] = (((u16) (freqh & 0xF)) << 8) | freql;
    }

    for (t = 0; t < SND_YM_NB_CHANNELS; t++, mask <<= 1)
    {
        bool active;
        u8   clevel = level[t];
        u16  bits;


        active = ((mix & mask) != 0) && (clevel != 0);
        active |= clevel == 15;

        bits = active ? 0xFFFF : 0;

        m[0]   = bits;
        m[80]  = bits;
        m[160] = bits;
        m[240] = bits;
        m[320] = bits;
        m[400] = bits;
        m[480] = bits;
        m[560] = bits;

        m += 19 * 80;
    }

    {
        u16 y;
        static char noise[] = "NOISE (  )";
        static char env[]   = "ENV   (  )(    )";


        STDuxtoa(&noise[7], noisefreq, 2);
        STDuxtoa(&env  [7], envshape , 1);
        STDuxtoa(&env [11], envfreq  , 4);

        SYSfastPrint(noise, pinstr + ((2 * 8) + 3) * 160, 160, 4, (u32)&SYSfont);
        SYSfastPrint(env  , pinstr + ((3 * 8) + 3) * 160, 160, 4, (u32)&SYSfont);

        for (t = 0, y = 0 ; t < SND_YM_NB_CHANNELS ; t++, y += (2 * 8 + 3) * 160)
        {
            static char sqr[]       = "SQR A (   ) ---- |";
            static char noisefeed[] = "NOISE       ---- |";
            static char lev[]       = "LEV ( )";
            char c;

            STDuxtoa(&sqr[7], chfreq[t], 3);

            sqr[4] = 'A' + t; 

            c = mix & (1 << t) ? ' ' : '-';

            sqr[12] = c;
            sqr[13] = c;
            sqr[14] = c;
            sqr[15] = c;

            c = mix & (1 << (t + 3)) ? ' ' : '-';

            noisefeed[12] = c;
            noisefeed[13] = c;
            noisefeed[14] = c;
            noisefeed[15] = c;

            SYSfastPrint(sqr      , pinstr + y + 40          , 160, 4, (u32)&SYSfont);            
            SYSfastPrint(noisefeed, pinstr + y + 40 + 160 * 8, 160, 4, (u32)&SYSfont);            

            if (level[t] < 16)
            {
                STDuxtoa(&lev[5], level[t], 1);
                SYSfastPrint(lev, pinstr + y + 77, 160, 4, (u32)&SYSfont);            
            }
            else
            {
                SYSfastPrint("ENV", pinstr + y + 77, 160, 4, (u32)&SYSfont);            
            }
        }
    }
}
