/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2021 J.Hubert

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

#define HARDWARE_C

#include "DEMOSDK\HARDWARE.H"

#ifndef __TOS__

SThardware g_STHardware;

void hwSetYMReg(u8 _regnum, u8 _data)
{
    *HW_YM_REGSELECT = _regnum; 

    /* emulate unused bit of ym for further reads */
    switch (_regnum)
    {
    case HW_YM_SEL_FREQCHA_H: 
    case HW_YM_SEL_FREQCHB_H: 
    case HW_YM_SEL_FREQCHC_H: 
    case HW_YM_SEL_ENVELOPESHAPE:
        _data &= 0xF;
        break;

    case HW_YM_SEL_FREQNOISE:
    case HW_YM_SEL_LEVELCHA:
    case HW_YM_SEL_LEVELCHB:
    case HW_YM_SEL_LEVELCHC:
        _data &= 0x1F;
        break;
    }

    (&g_STHardware.reg_HW_YM_REGDATA)[_regnum] = _data;
}

#endif
