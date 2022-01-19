/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2020 J.Hubert

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

#include "DEMOSDK\CODEGEN.H"



u16 CGENfindWords(CGENdesc* _desc, u16* _offsets, u16 _value)
{
    u16* p   = (u16*) _desc->codeaddress;
    u16  len = _desc->opcodelen_div2 << 1;
    u16  nb  = 0;
    u16  t;


    for (t = 0 ; t < len ; t += 2)
    {
        if (*p++ == _value)
        {
            _offsets[nb++] = t;
        }
    }

    return nb;
}


#ifndef __TOS__

u8* aCGENaddnops (u8* _output, u16 _cycles)
{
    u16* p = (u16*)_output;
    u16 t;

    for (t = 0; t < _cycles; t += 4)
    {
        *p++ = CGEN_OPCODE_NOP;
    }

    return (u8*)p;
}

void aCGENpatchWords(void* _address, u16* _offsets, u32 _values, u16 _nbvalues)
{
}

#endif