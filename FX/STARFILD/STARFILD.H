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

#ifndef STARFILD_H
#define STARFILD_H

#define STAR_ZMAX       127
#define STAR_ZSHIFT     7
#define STAR_INCMAX     3
#define STAR_WIDTH      336
#define STAR_PERSPEC    32


struct STARparam_
{
    s16*    prex;
    s16*    prey;
    void**  rndx;
    void**  rndy;

    u16*  pixdrawoffset;
    s16*  starx;
    s16*  stary;
    u32*  starz;
    u8*   erasebuffers[2];

    u16   pitch;
    u16   nbstars;
    s16*  pitchmul;

    s16*  copyposx;
    u8*   copyposy;

    u16*  xval;

    u16   inc;
    u16   height;

    void**  currentrndx;
    void**  currentrndy;
};
typedef struct STARparam_ STARparam;


u8*  STARstaticIniField1P (MEMallocator* _allocator);
void STARinit             (MEMallocator* _allocator, STARparam* _param);
void STARinit1P           (MEMallocator* _allocator, STARparam* _param, u8* _starfielddiv);
void STARerase            (void* _erasebuffer, u16 _nbstars);
void STARerase1P          (void* _erasebuffer, u16 _nbstars);
void STARdraw             (void* _framebuffer, u32* z, u16 _nbstars, u32 _erasebuffer, STARparam* _param);
void STARdraw1P           (void* _framebuffer, u32* z, u16 _nbstars, u32 _erasebuffer, u32 _param);
void STARshutdown         (MEMallocator* _allocator, STARparam* _param);

#endif
