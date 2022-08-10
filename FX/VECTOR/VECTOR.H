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

#ifndef VECTOR_H
#define VECTOR_H

#define VEC_PITCH 168

#define VEC_S16_MAX 0x7FFF
#define VEC_S16_MIN 0x8000 

#define VEC_SCREEN_WIDTH    336
#define VEC_SCREEN_HEIGHT   200

#define VEC_MINX_IDX 0
#define VEC_MAXX_IDX 1
#define VEC_MINY_IDX 2
#define VEC_MAXY_IDX 3

#define VEC_NOEDGE 0xFFFF

struct VECanimationState_
{
    u16 coef;
    s16 xdep;
    s16 ydep;
    u16 angle;
};
typedef struct VECanimationState_ VECanimationState;

typedef bool (*VECanimationCallback)(VECanimationState*);
typedef void (*VECprecomputeCheckCallback)(u16* _dlist);


struct VECscene_
{
    u16**               polygons;
    u16                 nbPolygons;
    u16                 nbframes;
    u8                  nbrepeat;
    u8*                 data;
    volatile u16*       displist;
};
typedef struct VECscene_ VECscene;

void VECsceneConstruct  (VECscene* _this, u8* _data, u16 _size);
void VECsceneDestroy    (VECscene* _this);
u16* VECscenePrecompute (VECscene* _scene, u16* _dlist, VECprecomputeCheckCallback _checkcallback, s16* _cosTable, s16* _sinTable, VECanimationCallback _animate, VECanimationState* _animationState, u16* _temp, u16 _tempsize);

u16* VECclipline(u16* dlist, u16* coord, u32 _pnbedges);

void VECclrpass (void);
void VECclr     (void* _ecran, u16* _list);
void VECxorpass (u16 hogOpMask);
void VECxor     (void* _ecran, u16* _list);
u16* VECloop    (void* _ecran, u16* _dlist, u16 _polycount);
void VECcircle  (void* screen_, u16 centerx_, u16 centery_, u16 radius_);

void VECdisplayBar (u16 xa_, u16 xb_, u16 xc_, u16 xd_, u16 yab_, u16 ycd_, u16* _dlist);

#endif
