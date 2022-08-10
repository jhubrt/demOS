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

#ifndef BLSZPLAY_H
#define BLSZPLAY_H

/* define wrapping to C or ASM version for each routine */
#if blsUSEASM

#   ifdef BLZupdate
#       define BLZupdAsync              aBLZupdAsync
#   else
#       define blzSetDMABuffer          ablzSetDMABuffer
#       define blzBlitzDecode           ablzBlitzDecode 
#       define blzUpdateAllVoices       ablzUpAllVoices
#   endif

#   define blzUpdateSoundBuffers    ablzUpSoundBuffers
#   define blzUpdateSoundBuffers2   ablzUp2SoundBuffers

#endif

#ifndef BLSupdate
void blzUpdateAllVoices     (struct BLSplayer_* _player);
void blzSetDMABuffer        (struct BLSplayer_* _player);
#endif                                

void blzUpdateSoundBuffers  (struct BLSplayer_* _player);
void blzUpdateSoundBuffers2 (struct BLSplayer_* _player);

#ifndef __TOS__                       
void blzUpdateMicrowireEmul (struct BLSplayer_* _player);
#endif

#endif