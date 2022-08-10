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

#include "DEMOSDK\BLSSND.H"

#if BLS_ENABLE_OPTIMIZED_DUMP

struct BLSoptDump_* BLSoptDumpInit      (FILE* _output, BLSsoundTrack* _sndtrack);
void                BLSoptDumpShutdown  (struct BLSoptDump_* _dump);

void blsOptDump_newframe        (struct BLSoptDump_* _dump, struct YMregs_* _ymregs, u16 _frame);
void blsOptDump_endframe        (struct BLSoptDump_* _dump);
void blsOptDump_voiceactivation (struct BLSoptDump_* _dump, BLSvoice* _voice, bool _active);
void blsOptDump_startkey        (struct BLSoptDump_* _dump, BLSvoice* _voice);
void blsOptDump_mask            (struct BLSoptDump_* _dump, BLSvoice* _voice, u16 _mask);
void blsOptDump_nullkeypost     (struct BLSoptDump_* _dump, BLSvoice* _voice);
void blsOptDump_volume          (struct BLSoptDump_* _dump, BLSvoice* _voice, u8 _volume);
void blsOptDump_retriggersample (struct BLSoptDump_* _dump, BLSvoice* _voice);
void blsOptDump_startarpegio    (struct BLSoptDump_* _dump, BLSvoice* _voice, BLSprecomputedKey* _key1, BLSprecomputedKey* _key2);
void blsOptDump_stoparpegio     (struct BLSoptDump_* _dump, BLSvoice* _voice);
void blsOptDump_setsampleoffset (struct BLSoptDump_* _dump, BLSvoice* _voice, u32 _offset);
void blsOptDump_newpattern      (struct BLSoptDump_* _dump, u8 _patternindex);

#else

#   define blsOptDump_newframe(_dump, _ymregs, _frame)
#   define blsOptDump_endframe(_dump)
#   define blsOptDump_voiceactivation(_dump, _voice,_active)
#   define blsOptDump_startkey(_dump, _voice)
#   define blsOptDump_mask(_dump, _voice, _mask)
#   define blsOptDump_retriggersample(_dump, _voice)
#   define blsOptDump_nullkeypost(_dump, _voice)
#   define blsOptDump_volume(_dump, _voice, _volume)
#   define blsOptDump_startarpegio(_dump, _voice, key1, key2)
#   define blsOptDump_setsampleoffset(_dump, _voice, _offset)
#   define blsOptDump_stoparpegio(_dump, _voice)
#   define blsOptDump_newpattern(_dump, _patternindex)

#endif
