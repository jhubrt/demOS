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

#ifndef CODEGEN_H
#define CODEGEN_H

#define CGEN_OPCODE_NOP 0x4E71
#define CGEN_OPCODE_RTE 0x4E73
#define CGEN_OPCODE_RTS 0x4E75

struct CGENdesc_
{
    u8  opcodelen_div2;    
    u8  cycles_div2;
    u8* codeaddress;
};
typedef struct CGENdesc_ CGENdesc;

u8* aCGENaddnops(u8* _output, u16 _cycles);

#define CGENgenerateSimple(_desc, _output)\
{   s16 len = ((_desc)->opcodelen_div2) << 1;\
    STDmcpy2(_output, (_desc)->codeaddress, len);\
    _output += len; }

#define CGENgenerate(_desc, _cycles, _output) { CGENgenerateSimple(_desc, _output) _cycles += ((_desc)->cycles_div2) << 1; }

#define CGENaddNops(_currentCycles, _targetCycles, _output)\
{   s16  diffCycles = ((s16)(_targetCycles) - (s16)(_currentCycles));\
    ASSERT(diffCycles >= 0);\
    ASSERT((diffCycles & 3) == 0);\
    _output = aCGENaddnops(_output, diffCycles); }

void aCGENpatchWords(void* _address, u16* _offsets, u32 _values, u16 _nbvalues);

#define CGENpatchWords(_adr, _offsets, _values, _nbvalues) {\
    STATIC_ASSERT((_nbvalues) <= 20); \
    aCGENpatchWords(_adr, _offsets, (u32)(_values), _nbvalues); }

u16 CGENfindWords(CGENdesc* _desc, u16* _offsets, u16 _value);

#endif
