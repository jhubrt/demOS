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
#include "DEMOSDK\SYNTHYMZ.H"


u8* SNDblitzDecodeYM (u8* blitzdata)
{
    /* YM  1 byte : Mixer (FreqA | restart sqr A) (FreqB | restart sqr B) (FreqC | restart sqr C) FreqEnvelop (LevelA | Freq noise) (LevelB | EnvelopShape) LevelC
    
    if Mixer
        1 byte mixer 
    
    if (FreqX | restart sqr X)
        1 byte FreqX low
        1 byte FreqX hight (4 low bits) | 0x80 if square X need to be restarted
    
    if FreqEnvelop
        1 byte FreqEnvelopL
        1 byte FreqEnvelopH
    
    if LevelA | freqnoise
        1 byte Level A (5 bits low) | 0x80 if freqnoise changed
    if Freqnoise changed
        1 byte freqnoise
    
    if LevelB | Envshape
        1 byte Level A (5 bits low) | 0x80 if envshape changed
    if envshape changed
        1 byte envshape
    
    if LevelC
        1 byte Level C
    */

    u8 ymvoicedata = *blitzdata++;

    if (ymvoicedata != 0)
    {
        u8 t;

        if (ymvoicedata & 0x80)
        { 
            u8 mixer = *blitzdata++;
            HW_YM_SET_REG(HW_YM_SEL_IO_AND_MIXER, mixer);
        }
        ymvoicedata <<= 1;

        for (t = 0 ; t < SND_YM_NB_CHANNELS ; t++)
        {
            if (ymvoicedata & 0x80)
            { 
                u8 freqL = *blitzdata++;
                s8 freqH = *blitzdata++;

                if (freqH < 0)
                {
                    HW_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (t << 1), 0);
                    HW_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (t << 1), 0);
                }

                HW_YM_SET_REG(HW_YM_SEL_FREQCHA_L + (t << 1), freqL);
                HW_YM_SET_REG(HW_YM_SEL_FREQCHA_H + (t << 1), freqH);
            }
            ymvoicedata <<= 1;
        }

        if (ymvoicedata & 0x80)
        {
            u8 freqL = *blitzdata++;
            u8 freqH = *blitzdata++;

            HW_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_L, freqL);
            HW_YM_SET_REG(HW_YM_SEL_FREQENVELOPE_H, freqH);
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            s8 levelA = *blitzdata++;

            if (levelA < 0)
            {
                u8 noisefreq = *blitzdata++;
                HW_YM_SET_REG(HW_YM_SEL_FREQNOISE, noisefreq);
            }
            HW_YM_SET_REG(HW_YM_SEL_LEVELCHA, levelA);
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            s8 levelB = *blitzdata++;

            if (levelB < 0)
            {
                u8 envshape = *blitzdata++;
                HW_YM_SET_REG(HW_YM_SEL_ENVELOPESHAPE, envshape);
            }
            HW_YM_SET_REG(HW_YM_SEL_LEVELCHB, levelB);
        }
        ymvoicedata <<= 1;

        if (ymvoicedata & 0x80)
        {
            u8 levelC = *blitzdata++;

            HW_YM_SET_REG(HW_YM_SEL_LEVELCHC, levelC);
        }
        ymvoicedata <<= 1;
    }

    return blitzdata;
}
