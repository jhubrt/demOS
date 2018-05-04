/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2018 J.Hubert

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


#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\ALLOC.H"
#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\SOUND.H"
#include "DEMOSDK\BITMAP.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\TRACE.H"

#include "DEMOSDK\PC\WINDOW.H"
#include "DEMOSDK\PC\EMUL.H"

#include "REBIRTH\SRC\SCREENS.H"
#include "REBIRTH\SRC\POLYZOOM.H"
#include "REBIRTH\SRC\SNDTRACK.H"

#include "EXTERN\ARJDEP.H"

#include "REBIRTH\DISK1.H"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200
#define NBFRAMEBUFFERS 3

#define NBCOLORS 48L

static u16 fileoffsets[] = 
{
	RSC_DISK1_METADATA_POLYZOOM_C_DAT,
	RSC_DISK1_METADATA_POLYZOOM_Y_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_Y_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_B_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_B_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_E_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_E_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_R_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_R_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_N_DAT,
	RSC_DISK1_METADATA_POLYZOOM_E_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_E_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_T_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_T_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_I_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_I_2_DAT,
	RSC_DISK1_METADATA_POLYZOOM_C_DAT,
	RSC_DISK1_METADATA_POLYZOOM_S_1_DAT,
	RSC_DISK1_METADATA_POLYZOOM_S_2_DAT
};

u16* pzprecompute (u16* poly, s16 cs, s16 sn, u16 coef, s16 offsetx, s16 offsety, s16 centerx, s16 centery, u16* dlist);	/* ASM version lot faster (x5) */

void CybervectorEntry (FSM* _fsm)
{
	u16 polysDataMaxSize  = 690;
	u16 polysDataTempSize = (u16) LOADresourceRoundedSize ( &RSC_DISK1, RSC_DISK1_POLYZOOM__CYBERVECTOR_BIN);
	u32 polysDispListSize = 400UL * 1024UL;
	u8* temp;
	u16* pald;
	u16 i;
	LOADrequest* loadRequest;
	LOADrequest* loadRequest2;


	IGNORE_PARAM(_fsm);

	g_screens.cybervector = MEM_ALLOC_STRUCT( &sys.allocatorMem, Cybervector );	

	g_screens.cybervector->polygonsList = (u16*) RINGallocatorAlloc ( &sys.mem, ARRAYSIZE(fileoffsets) * 6 + 4 );
	g_screens.cybervector->polygonsData = (u8*) RINGallocatorAlloc ( &sys.mem, polysDataMaxSize );
	g_screens.cybervector->sin = (u16*) RINGallocatorAlloc ( &sys.mem, 1024 + 256);

	{
		u16* p   = (u16*) g_screens.cybervector->polygonsList;

		p += 2;

		*p = 50;		/* C */			p += 3;
		*p = 0;			/* Y */			p += 3;
		*p = 50;		/* Y */ 		p += 3;
		*p = 0;			/* B */			p += 3;
		*p = 50;		/* B */			p += 3;
		*p = 0;			/* E */			p += 3;
		*p = 50;		/* E */			p += 3;
		*p = 0;			/* R */			p += 3;
		*p = 50;		/* R */			p += 3;
		*p = 50;		/* N */			p += 3;
		*p = 0;			/* E */			p += 3;
		*p = 50;		/* E */			p += 3;
		*p = 0;			/* T */			p += 3;
		*p = 50;		/* T */			p += 3;
		*p = 0;			/* I */			p += 3;
		*p = 20;		/* I */			p += 3;
		*p = 50;		/* C */			p += 3;
		*p = 0;			/* S */			p += 3;
		*p = 0;			/* S */			;
	}

	g_screens.cybervector->pal = (u16*) RINGallocatorAlloc (&sys.mem, NBCOLORS*2+64*2);

	temp = (u8*) RINGallocatorAlloc ( &sys.mem, polysDataTempSize );

	loadRequest  = LOADdata (&RSC_DISK1, RSC_DISK1_POLYZOOM__CYBERVECTOR_BIN, temp, LOAD_PRIORITY_INORDER);
	loadRequest2 = LOADdata (&RSC_DISK1, RSC_DISK1_POLYZOOM__SIN_BIN, g_screens.cybervector->sin, LOAD_PRIORITY_INORDER);

    LOADwaitRequestCompleted ( loadRequest );	
	
	{
		u16* p   = (u16*) g_screens.cybervector->polygonsList;
		u8*  adr = (u8*)  g_screens.cybervector->polygonsData;

		for ( i = 0 ; i < ARRAYSIZE(fileoffsets) ; i++ )
		{
			u16 rscOffsetId = fileoffsets[i];
            u16 offset = (u16) LOADmetadataOffset (&RSC_DISK1, rscOffsetId);
			u16 size   = (u16) LOADmetadataSize   (&RSC_DISK1, rscOffsetId);

			*(void**)p = adr;
			p += 3;

			STDmcpy (adr, temp + offset, size);
			adr += size;
		}

		*(void**)p = NULL;

		ASSERT( (adr - g_screens.cybervector->polygonsData) <= polysDataMaxSize );
	}

    pald = (u16*)(temp + LOADmetadataOffset (&RSC_DISK1, RSC_DISK1_METADATA_POLYZOOM_PAL1_PAL));

	for (i = 0 ; i < NBCOLORS ; i++)
	{
		g_screens.cybervector->pal[i] = *pald++ & ~RASstopMask;
	}

	for (i = 0 ; i < 64 ; i++)
	{
		g_screens.cybervector->pal[i+NBCOLORS] = 0x700;
	}

	RINGallocatorFree (&sys.mem, temp);

	LOADwaitRequestCompleted ( loadRequest2 );

	g_screens.cybervector->cos = g_screens.cybervector->sin + 128;
	STDmcpy (g_screens.cybervector->sin + 512, g_screens.cybervector->sin, 256);

	/* COMPUTE NB TOTAL EDGES 
	{
	u16 nbTotalEdges = 0;
	u16* p = g_screens.cybervector->polygonsList;

	while ( *(u32*)p )
	{
	u16	nbEdges = **(u16**)p;
	nbTotalEdges += PCENDIANSWAP16(nbEdges);
	p += 3;
	}

	printf ("nbTotalEdges = %d\n", nbTotalEdges);
	}*/

	g_screens.cybervector->displist = (u16*) RINGallocatorAlloc ( &sys.mem, polysDispListSize );
	g_screens.cybervector->displistp = g_screens.cybervector->displist;
	g_screens.cybervector->currentFrame = 0;
	g_screens.cybervector->nbFrames = 0;

	/* PRECOMPUTE VERTICES POSITIONS */	
	{
		u16 coef = 0;
		s16 xdep = 0;
		u16 ic = 127;
		u16 icx = 32;
		u16 angle = 0;
		u16* dlist = g_screens.cybervector->displist;

		do
		{
			u16* plist = g_screens.cybervector->polygonsList;
			u16* poly = *(u16**) plist;
			s16  offsetx = -250;
			s16  offsety = -27;
			s16* dlistbegin = (s16*) dlist;

			s16 cs = g_screens.cybervector->cos[angle];
			s16 sn = g_screens.cybervector->sin[angle];

			cs = PCENDIANSWAP16 ( cs );
			sn = PCENDIANSWAP16 ( sn );

			coef += ic;
			if ( coef == (127 << 8) )
			{
				ic = -ic;
			}
			else if ( coef == 0 )
			{
				break;
			}

			xdep += icx;
			if ((xdep == (-256 * 16)) || (xdep == (256 * 16)))
			{
				icx = -icx;
			}

			plist += 2;

			/* reserve some space for minx / maxx / miny / maxy */
			*dlist++ = 0x7FFF;	
			*dlist++ = 0x8000;
			*dlist++ = 0x7FFF;
			*dlist++ = 0x8000;

			while (poly != NULL)
			{
				u16* dlistbackup = dlist;

#				ifdef __TOS__
				dlist = pzprecompute (poly, cs, sn, coef, xdep + (offsetx << 4), offsety << 4, SCREEN_WIDTH >> 1, SCREEN_HEIGHT >> 1, dlist);
#				else
				u16 nbEdges = *poly++;
				s16	minx = 0x7FFF;
				s16	maxx = 0x8000;
				s16	miny = 0x7FFF;
				s16	maxy = 0x8000;

				nbEdges = PCENDIANSWAP16(nbEdges);

				*dlist++ = nbEdges - 1;

				while (nbEdges-- > 0)
				{
					s16 x, y;
					s32 x2, y2;

					x = *poly++;
					x = PCENDIANSWAP16(x);
					x += offsetx;
					x <<= 4;
					x += xdep;

					y = *poly++;
					y = PCENDIANSWAP16(y);
					y += offsety;
					y <<= 4;

					x2  = (s32) x * cs;
					x2 -= (s32) y * sn;
					y2  = (s32) x * sn;
					y2 += (s32) y * cs;

					x2 >>= 16;
					y2 >>= 16;

					x2 *= coef;
					y2 *= coef;

					x2 >>= 16;
					y2 >>= 16;

					x2 += SCREEN_WIDTH >> 1;
					y2 += SCREEN_HEIGHT >> 1;

					*dlist++ = (u16) x2;
					*dlist++ = (u16) y2;

					if ( x2 < minx )
						minx = (s16) x2;

					if ( x2 > maxx )
						maxx = (s16) x2;

					if ( y2 < miny )
						miny = (s16) y2;

					if ( y2 > maxy )
						maxy = (s16) y2;
				} /* edges loop */

				dlist[0] = minx;
				dlist[1] = maxx;
				dlist[2] = miny;
				dlist[3] = maxy;
#				endif

				{
					s16* minmax = (s16*)dlist;

					if ( dlistbegin[0] > minmax[0] )	/* min x */
						dlistbegin[0] = minmax[0];

					if ( dlistbegin[1] < minmax[1] )	/* max x */
						dlistbegin[1] = minmax[1];

					if ( dlistbegin[2] > minmax[2] )	/* min y */
						dlistbegin[2] = minmax[2];

					if ( dlistbegin[3] < minmax[3] )	/* max y */
						dlistbegin[3] = minmax[3];

					if (( minmax[3] < 0 ) || 
						( minmax[2] >= SCREEN_HEIGHT ) ||
						( minmax[1] < 0 ) ||
						( minmax[0] >= SCREEN_WIDTH ))
					{
						dlist = dlistbackup;
						*dlist++ = 0xFFFF;
					}
				}

				offsetx += *plist;
				plist++;

				poly = *(u16**) plist;
				plist += 2;
			} /* poly loop */


			/* converts dlistbegin[0] and dlistbegin[1] into word numbers and offset for blitter pass */
			if ( dlistbegin[0] < 0 )
				dlistbegin[0] = 0;

			if ( dlistbegin[1] >= SCREEN_WIDTH )
				dlistbegin[1] = SCREEN_WIDTH - 1;

			dlistbegin[0] &= 0xFFF0;
			dlistbegin[0] >>= 1;

			dlistbegin[1] &= 0xFFF0;
			dlistbegin[1] >>= 1;

			dlistbegin[1] -= dlistbegin[0];
			dlistbegin[1] >>= 3;
			dlistbegin[1]++;

			/* increment angle */
			angle += 2;
			angle &= 511;

			g_screens.cybervector->nbFrames++;
		}
		while (1);

		{
			u32 displistsize = ((u8*)dlist - (u8*)g_screens.cybervector->displist);
			ASSERT (displistsize <= polysDispListSize);
		}
	}

	g_screens.cybervector->rastersop[0] = (u16*) RINGallocatorAlloc ( &sys.mem, 2048UL );
	g_screens.cybervector->rastersop[1] = g_screens.cybervector->rastersop[0] + 512;
	g_screens.cybervector->fliprasterop = 0;

	g_screens.cybervector->framebuffers[0] = RINGallocatorAlloc ( &sys.mem, 32000UL * ((u32)NBFRAMEBUFFERS) );
    STDmset (g_screens.cybervector->framebuffers[0], 0, 32000UL * ((u32)NBFRAMEBUFFERS));
	
	for (i = 1 ; i < NBFRAMEBUFFERS ; i++)
	{
		g_screens.cybervector->framebuffers[i] = (u8*)g_screens.cybervector->framebuffers[i-1] + 32000UL;
	}

	g_screens.cybervector->flipbuffer = 0;

	SYSwriteVideoBase ((u32) g_screens.cybervector->framebuffers[0]);

	RASsetColReg (0x8242);

    SNDwaitDMALoop();

    SYSvsync;

	FSMgotoNextState (&g_stateMachineIdle);
	FSMgotoNextState (&g_stateMachine);
}

#define GRIDMAXSIZE 3000


static u16 CybervectorGenerateRandomBuffer( u8* _bitmap, u16* _random, u16 _height )
{
	u16 x, y, i;
	u16 size = 0;


	for (y = 0 ; y < _height ; y++)
	{
		for (x = 0 ; x < 40 ; x++)
		{
			u16 offset;

			offset = (x & 0xFFFE) << 2;
			if (x & 1)
				offset++;
			offset += y * 160;

			if ( _bitmap[offset] != 0 )
				_random[size++] = offset;
			if ( _bitmap[offset + 2] != 0 )
				_random[size++] = offset + 2;
			if ( _bitmap[offset + 4] != 0 )
				_random[size++] = offset + 4;
		}
	}

	ASSERT( size <= GRIDMAXSIZE );

	/*{
		static u16 y = 0;
		char text[] = "        ";
		STD_uxtoa (text, size, 8);

		TRAC_debugPrint((u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L)) + 1, 160, 8, 0, 8, text);
		y += 8;
	}*/

	{
		/* replicate offsets buffer for each frame buffer */
		u16 t = size;

		for (i = 1 ; i < NBFRAMEBUFFERS ; i++, t += size)
		{
			u16 p;

			for (p = 0 ; p < size ; p++)
			{
				_random[t+p] = _random[p];
			}
		}
	}

	{   /* invert randomly the offsets buffer differently for each each frame buffer */
		u16* p1 = _random;
		u16* p2 = _random;

		ASSERT(size > 255);

		for (i = 0 ; i < NBFRAMEBUFFERS ; i++)
		{
			s16 r;

			for (r = size ; r > 0 ; r--)
			{
				s16 index = STDifrnd() & 255;	/* and mask instead of modulo size (with mask <= modulo) is faster */
				u16* pi = p2 + index;			/* and gives visually interesting result */ 				
				STD_SWAP(u16, *p1, *pi);
				p1++;
			}

			p2 += size;
		}
	}

	return size;
}

static void CybervectorAnimateLogo (u16 _size, u16* _random, u8* _bitmap, u8** _framebuffers)
{
    s16 i;

	
    for (i = 0 ; i < _size ; i++)
	{
        register s16 t = NBFRAMEBUFFERS - 1;
		u8** fb	= _framebuffers;
        u16* p  = _random + i;	
		
        do
		{
            s16 offset = *p;

            u8* s = _bitmap + offset;
            u8* d = offset + *fb++;

			*d = *s;

            p += _size;
            t--;
		}
        while (t >= 0);
	
        if ((i & 0x3F) == 0)
		{
			SYSvsync;
		}
	}
}


static void CybervectorEraseLogo (u16 _size, u16* _random, u8** _framebuffers)
{
    s16 i;

	
    for (i = 0 ; i < _size ; i++)
	{
        register s16 t = NBFRAMEBUFFERS - 1;
		u8** fb	= _framebuffers;
        u16* p  = _random + i;	
		
        do
		{
            u8* d = *p + *fb++;

			*d = 0;

            p += _size;
            t--;
		}
        while (t >= 0);
	
        if ((i & 0x3F) == 0)
		{
			SYSvsync;
		}
	}}


STRUCT(CybervectorText)
{
	s16 height;
	s16 offsetsource;
	s16 offsetdest;
};

#define NBTEXTS 2

static CybervectorText g_texts[NBTEXTS] = 
{
	24, 6400, 14082,
	39, 0,	  12802,
};

u32 pzloop(void* _ecran, u16* _dlist, u16 _polycount)
#ifndef __TOS__
{
	WINsetColor  ( EMULgetWindow(), 0, 0, 0);
	WINclear     ( EMULgetWindow() );
	WINsetColor  ( EMULgetWindow(), 255, 0, 0);

	while (_polycount-- > 0)
	{
		s16 e = *_dlist++;

		if ( e >= 0 )
		{
			s16 x1 = *_dlist++;
			s16 y1 = *_dlist++;

			s16 firstx = x1;
			s16 firsty = y1;

			s16 x2;
			s16 y2;

			while (e-- > 0)
			{
				x2 = *_dlist++;
				y2 = *_dlist++;

				WINline ( EMULgetWindow(), x1, y1, x2, y2);

				x1 = x2;
				y1 = y2;
			}

			WINline ( EMULgetWindow(), firstx, firsty, x2, y2);
		}
	}

	WINsetColor  ( EMULgetWindow(), 255, 255, 255);
	WINrectangle ( EMULgetWindow(), -1, -1, SCREEN_WIDTH, SCREEN_HEIGHT);
	WINrender ( EMULgetWindow(), 40);

	return (u32) _dlist;
}
#endif
; 

void Clinetest(void* _image, u16* _coord);

void pzsetrasters(void* _rasteropbuf, void* _pal, s16 _miny, s16 _maxy)
#ifndef __TOS__
{
    /* test rasters */
	u8* p = (u8*) _rasteropbuf;
	u16* pal = (u16*) _pal;
	s16 dy = _maxy - _miny;
	s16 firstline;
	u32 acc = 0;

	u32 inc = ((NBCOLORS-3) << 16) / dy;	    /* *2 because we put a raster every 2 scanlines */


	*((RASinterupt*)p) = RASvbl2;
	p += sizeof(RASinterupt);

	firstline = 3;

	if (_miny < 0)
	{
		s16 off = (-(NBCOLORS-3) * (s32)_miny) / (s32)dy;
		if ( off < 0 )
			off = 0;
		pal += off;

		{
			RASopVbl2* s = (RASopVbl2*) p;
			s->backgroundColor			= 0;
			s->color						= *pal++;
			s->scanLinesTo1stInterupt	= 1;
			s->nextRasterRoutine			= RASmid1;
			p += sizeof(RASopVbl2);
		}

		{
			RASopMid1* s = (RASopMid1*) p;
			s->color						= *pal++ | RASstopMask;
			s->scanLineToNextInterupt	= 2;
			s->nextRasterRoutine			= RASmid1;
			p += sizeof(RASopMid1);
		}
	}
	else
	{
		firstline += _miny > 0 ? _miny : 1;

		{
			RASopVbl2* s = (RASopVbl2*) p;
			s->backgroundColor			= 0;
			s->color						= *pal++;
			s->scanLinesTo1stInterupt	= firstline;
			s->nextRasterRoutine			= RASmid1;
			p += sizeof(RASopVbl2);
		}

		{
			RASopMid1* s = (RASopMid1*) p;
			s->color						= *pal++ | RASstopMask;
			s->scanLineToNextInterupt	= 2;
			s->nextRasterRoutine			= RASmid1;
			p += sizeof(RASopMid1);
		}
	}

	{
		u32 acc2 = 0;
		u16 t,nb;
		s16	affdy;

		if ( _maxy < 198 )
		{
			affdy = _maxy - firstline;
		}
		else
		{
			affdy = 198 - firstline;
		}

		if ( affdy < 0 )
		{
			affdy = 0;
		}

		nb = affdy; /* >> 1; */

		ASSERT(dy > 0);

		for (t = 0 ; t < nb ; t++)
		{
			u16 off = 0;

			acc2 += acc & 0xFFFFUL;
			if (acc2 > 0xFFFFUL)
			{
				acc2 &= 0xFFFFUL;
				off = 1;
			}

			*((u16*)p) = pal[(acc >> 16) + off] & ~RASstopMask;
			p += 2;
			acc += inc;
		}

		{
			RASopMid1* s = (RASopMid1*) p;
			s->color					= pal[acc >> 16] | RASstopMask;
			s->scanLineToNextInterupt	= 255;
			s->nextRasterRoutine		= 0;
			p += sizeof(RASopMid1);
		}
	}

	/* printf ("%d %d %d %d %f %d\n", _miny, _maxy, firstline, dy, (float)inc / 256.0f, (-NBCOLORS * _miny) / dy); */

	RASnextOpList = _rasteropbuf;

    (*HW_COLOR_LUT) = 0;
    STDmset (&HW_COLOR_LUT[1], 0x07000700UL, 30);
}
#endif
;

void CybervectorActivity (FSM* _fsm)
{
	s16 miny, maxy;
	void* image = g_screens.cybervector->framebuffers[g_screens.cybervector->flipbuffer];
    u16 vblcount = SYSvblLcount;        /* I really dont get why it does not work with SYSbeginFrameNum... */

	g_screens.cybervector->flipbuffer++;
	if (g_screens.cybervector->flipbuffer == 3)
	{
		g_screens.cybervector->flipbuffer = 0;
	}

	miny = g_screens.cybervector->displistp[2];
	maxy = g_screens.cybervector->displistp[3];

	IGNORE_PARAM(_fsm);
		
	g_screens.cybervector->displistp = (u16*) pzloop (image, g_screens.cybervector->displistp, ARRAYSIZE(fileoffsets));

	pzsetrasters(g_screens.cybervector->rastersop[g_screens.cybervector->fliprasterop], g_screens.cybervector->pal, miny, maxy);

	g_screens.cybervector->fliprasterop ^= 1;

	g_screens.cybervector->currentFrame++;

	if ( g_screens.cybervector->currentFrame == g_screens.cybervector->nbFrames )
	{
		g_screens.cybervector->displistp = g_screens.cybervector->displist;
		g_screens.cybervector->currentFrame = 0;
	}

    while (SYSvblLcount == vblcount);

	SYSwriteVideoBase ((u32) image);
}

void CybervectorBacktask (FSM* _fsm)
{
	u8*		packed;
	u8*		data;
	u16*	random;
	u16		i, x, y, size;
    LOADrequest* loadRequest;
    u16 logoSize = (u16) LOADmetadataOriginalSize(&RSC_DISK1, RSC_DISK1_METADATA_POLYZOOM__REBIRTH_ARJX);


	IGNORE_PARAM(_fsm);

	packed = (u8*) RINGallocatorAlloc ( &sys.mem, LOADresourceRoundedSize(&RSC_DISK1, RSC_DISK1_POLYZOOM__REBIRTH_ARJX) );
    data   = (u8*) RINGallocatorAlloc ( &sys.mem, (logoSize / 3) * 4 );
	random = (u16*) RINGallocatorAlloc  ( &sys.mem, GRIDMAXSIZE * sizeof(u16) * NBFRAMEBUFFERS );

	loadRequest = LOADdata (&RSC_DISK1, RSC_DISK1_POLYZOOM__REBIRTH_ARJX, packed, LOAD_PRIORITY_INORDER);

    for (i = 1 ; i < 8 ; i++)
	{
		u16 color = (i << 8) | (i << 4) | i;
		*(HW_COLOR_LUT + (i << 1)) = color;

		if ( i < 4 )
		{
			*(HW_COLOR_LUT + (i << 1) + 1) = 0x444;
		}
		else
		{
			*(HW_COLOR_LUT + (i << 1) + 1) = color;
		}
	}

	LOADwaitRequestCompleted ( loadRequest );

    ARJdepack (data, packed);

	x = (80*64)-4;
	y = (60*64)-3;

	/* unpack ignored plane */
	{
		u16* b = (u16*) data;
		
		for (i = 0 ; i < 1280 ; i++) /* 1280 = 64 * 160 / 8 */
		{
            u16 p0 = b [ y ];
            u16 p1 = b [ y + 1 ];
            u16 p2 = b [ y + 2 ];
			
            b [ x	  ] = p0;
			b [ x + 1 ] = p1; 
			b [ x + 2 ] = p2;
			/*b [ x + 3 ] = 0;*/

			x -= 4;
			y -= 3;
		}
	}

	for (i = 0 ; i < NBTEXTS ; i++)
	{
		u8* framebuffers[NBFRAMEBUFFERS];
		u8* logo = data + g_texts[i].offsetsource;

		for (x = 0 ; x < NBFRAMEBUFFERS ; x++)
		{
			framebuffers[x] = ((u8*)g_screens.cybervector->framebuffers[x]) + g_texts[i].offsetdest;
		}

		size = CybervectorGenerateRandomBuffer (logo, random, g_texts[i].height);

		/*BIT_pl2chunk(buffer,200,20,0,buffer);*/

		SNDwaitClientStep(STEP_START_POLYZOOM);
		snd.playerClientStep = STEP_POLYZOOM_STARTED;
		SNDwaitDMALoop();

		CybervectorAnimateLogo(size, random, logo, framebuffers);

		for (x = 0 ; x < 180 ; x++)
		{
			SYSvsync;
		}

		CybervectorEraseLogo(size, random, framebuffers);
	}

	/*	for (i = 1 ; i < 16 ; i++)
	{
		*(HW_COLOR_LUT + i) = 0x70;
	}*/

	RINGallocatorFree ( &sys.mem, random );
	RINGallocatorFree ( &sys.mem, packed );
	RINGallocatorFree ( &sys.mem, data );

    while (g_screens.cybervector->currentFrame != 0);

    if ( FSMisLastState(_fsm) )
    {
	    while(1);
    }
    else
    {
        snd.playerClientStep = STEP_POLYZOOM_STOP;
        RASnextOpList = NULL;               
        FSMgotoNextState (&g_stateMachine);
        ScreenWaitMainDonothing();
    }
}


void CybervectorExit (FSM* _fsm)
{
	IGNORE_PARAM(_fsm);

	RINGallocatorFree (&sys.mem, g_screens.cybervector->framebuffers[0]);
	RINGallocatorFree (&sys.mem, g_screens.cybervector->rastersop[0]);
	RINGallocatorFree (&sys.mem, g_screens.cybervector->displist);
    RINGallocatorFree (&sys.mem, g_screens.cybervector->pal);
	RINGallocatorFree (&sys.mem, g_screens.cybervector->sin);
	RINGallocatorFree (&sys.mem, g_screens.cybervector->polygonsData);
	RINGallocatorFree (&sys.mem, g_screens.cybervector->polygonsList);

	MEM_FREE (&sys.allocatorMem, g_screens.cybervector);
	g_screens.cybervector = NULL;

    ASSERT( RINGallocatorIsEmpty(&sys.mem) );

    FSMgotoNextState (&g_stateMachineIdle);
}
