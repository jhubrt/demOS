/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of demOS

  demOS is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with demOS.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#include "DEMOSDK\BASTYPES.H"

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\COLORS.H"

#include "DEMOSDK\PC\SURFACE.H"
#include "DEMOSDK\PC\BITCONVR.H"


typedef void (*BITconvertionFunc) (void* _source, u16 _pitchSource, u32* _lut, void* _dest, u16 _w, u16 _h, u16 _pitchDest);

#define BIT_CONVERTION_ALLOWS_INPLACE 1

STRUCT(BITconvertion)
{
    BITconvertionFunc func;
    u32               flags;
};

static BITconvertion convert[BITformat_NBMAX][16];
static bool isInit = false;

static bool         trueColor       [BITformat_NBMAX];
static BITlutFormat defaultFormat   [BITformat_NBMAX];

STATIC_ASSERT(BITformat_NBMAX <= 16);



void BITlutDestroy(BITlut* _lut)
{
    if (_lut->allocator != NULL)
    {
        if (_lut->data.p != NULL )
        {
            if ( _lut->bufferowner )
            {
                MEM_FREE(_lut->allocator, _lut->data.p);
                _lut->data.p = NULL;
                _lut->bufferowner = false;
            }
        }

        _lut->allocator = NULL;
    }
    else
    {
        ASSERT( (_lut->data.p == NULL) || ( _lut->bufferowner == false) );
    }

    _lut->format = BITlutFormat_BnW;
}


void BITsurfaceDestroy(BITsurface* _this)
{
    BITlutDestroy (&_this->lut);

    if (_this->allocator != NULL)
    {
        if (_this->bufferowner)
        {
            if (_this->buffer != NULL)
            {
                MEM_FREE(_this->allocator, _this->buffer);
                _this->buffer = NULL;
                _this->allocator = NULL;
                _this->bufferowner = false;
            }
        }
    }
    else
    {
        ASSERT ((_this->buffer == NULL) || (_this->bufferowner == false));
    }
}

void BITsurfaceInit(MEMallocator* _allocator, BITsurface* _surface, BITformat _format, u16 _w, u16 _h, u16 _pitch)
{
	u16 autoPitch;
	u16 nbPlanes = 1;
    u32 size;


	switch(_format)
	{
	case BITformat_Plane1P:
	case BITformat_Plane2P:
	case BITformat_Plane3P:
	case BITformat_Plane4P:
		autoPitch = (_w + 7) >> 3; 
		
		switch(_format)
		{
		case BITformat_Plane2P:
			nbPlanes = 2;
			break;
		case BITformat_Plane3P:
			nbPlanes = 3;
			break;
		case BITformat_Plane4P:
			nbPlanes = 4;
			break;
		}
		break;

	case BITformat_Chunk3P:
		autoPitch = ((_w + 7) >> 3) * 3; 
		break;
	
	case BITformat_Chunk2P:
		autoPitch = (_w + 3) >> 2; 
		break;
	
	case BITformat_4bits:
	case BITformat_Chunk4P:
		autoPitch = (_w + 1) >> 1; 
		break;

	case BITformat_888:
		autoPitch = _w * 3;
		break;

	case BITformat_x888:
		autoPitch = _w * 4;
		break;

	case BITformat_8bits:
		autoPitch = _w;
		break;
	};

	_surface->pitch = _pitch == BIT_DEFAULT_PITCH ? autoPitch : _pitch;

    size = _surface->pitch * nbPlanes * _h;

    if (( _surface->buffer == NULL ) || ( _surface->size != size ))
    {
        BITsurfaceDestroy (_surface);

        _surface->allocator = _allocator;
        _surface->buffer    = (u8*) MEM_ALLOC(_allocator,size);
        _surface->bufferowner = true;
        ASSERT(_surface->buffer != NULL);
    }

    _surface->size	    = size;
	_surface->width     = _w;
	_surface->height    = _h;
	_surface->format    = _format;
	_surface->nbPlanes  = nbPlanes;
}

void BITsurfaceSetExternal (BITsurface* _surface, void* _buffer, BITformat _format, u16 _w, u16 _h, u16 _pitch)
{	
    ASSERT(_pitch != BIT_DEFAULT_PITCH);

    BITsurfaceDestroy (_surface);

    _surface->buffer    = (u8*) _buffer;
	_surface->width     = _w;
	_surface->height    = _h;
	_surface->format    = _format;
	_surface->pitch     = _pitch;
	_surface->size	    = _surface->pitch * _h;
	_surface->nbPlanes  = 1;
	_surface->allocator = NULL;
    _surface->bufferowner = false;
}


void BITinit (void)
{
    if ( isInit == false ) 
    {
        STDmset (trueColor, 0, sizeof(trueColor));

        trueColor [BITformat_888 ] = true;
        trueColor [BITformat_x888] = true;

        STDmset (defaultFormat, 0, sizeof(defaultFormat));
   
        defaultFormat [BITformat_Chunk4P] = BITlutFormat_STe;
        defaultFormat [BITformat_Chunk3P] = BITlutFormat_STe;
        defaultFormat [BITformat_Chunk2P] = BITlutFormat_STe;
        defaultFormat [BITformat_Plane1P] = BITlutFormat_BnW;
        defaultFormat [BITformat_Plane2P] = BITlutFormat_444;
        defaultFormat [BITformat_Plane3P] = BITlutFormat_444;
        defaultFormat [BITformat_Plane4P] = BITlutFormat_444;
        defaultFormat [BITformat_4bits  ] = BITlutFormat_x888;
        defaultFormat [BITformat_8bits  ] = BITlutFormat_x888;
        defaultFormat [BITformat_888    ] = BITlutFormat_x888;
        defaultFormat [BITformat_x888   ] = BITlutFormat_x888;

        STDmset (convert, 0, sizeof(convert));

        /* ----- source ------------ dest  -------- */        
        convert [BITformat_888]       [BITformat_x888]      .func  = BITfrom888Tox888;
        convert [BITformat_x888]      [BITformat_8bits]     .func  = BITfromx888To8b;

        convert [BITformat_8bits]     [BITformat_888]       .func =  BITfrom8bTo888;
        convert [BITformat_8bits]     [BITformat_x888]      .func  = BITfrom8bTox888;
        convert [BITformat_8bits]     [BITformat_4bits]     .func  = BITfrom8bTo4b;

        convert [BITformat_4bits]     [BITformat_8bits]     .func  = BITfrom4bTo8b;

        convert [BITformat_4bits]     [BITformat_Plane1P]   .func  = BITfrom4bToChunk1P;
        convert [BITformat_4bits]     [BITformat_Chunk2P]   .func  = BITfrom4bToChunk2P;
        convert [BITformat_4bits]     [BITformat_Chunk3P]   .func  = BITfrom4bToChunk3P;
        convert [BITformat_4bits]     [BITformat_Chunk4P]   .func  = BITfrom4bToChunk4P;

        convert [BITformat_Plane1P]   [BITformat_4bits]     .func  = BITfromChunk1PTo4b;
        convert [BITformat_Chunk2P]   [BITformat_4bits]     .func  = BITfromChunk2PTo4b;
        convert [BITformat_Chunk3P]   [BITformat_4bits]     .func  = BITfromChunk3PTo4b;
        convert [BITformat_Chunk4P]   [BITformat_4bits]     .func  = BITfromChunk4PTo4b;

        convert [BITformat_Plane1P]   [BITformat_8bits]     .func  = BITfromChunk1PTo8b;
        convert [BITformat_Chunk2P]   [BITformat_8bits]     .func  = BITfromChunk2PTo8b;
        convert [BITformat_Chunk3P]   [BITformat_8bits]     .func  = BITfromChunk3PTo8b;
        convert [BITformat_Chunk4P]   [BITformat_8bits]     .func  = BITfromChunk4PTo8b;

        isInit = true;
    }
}

void BITsurfaceConvert(MEMallocator* _allocator, BITsurface* _source, BITsurface* _dest, BITformat _destFormat)
{
    BITconvertion* converter = &convert [_source->format][_destFormat];

    
    BITinit();

    if ( converter->func != NULL )
    {
        ASSERT( ( _source != _dest) || ((converter->flags & BIT_CONVERTION_ALLOWS_INPLACE) != 0) );

        BITsurfaceInit (_allocator, _dest, _destFormat, _source->width, _source->height, BIT_DEFAULT_PITCH);

        if ( trueColor[_destFormat] == false )
        {
            BITlutConvert (_allocator, &_source->lut, &_dest->lut, defaultFormat[_destFormat]);
        }

        converter->func(_source->buffer, _source->pitch, _source->lut.data.p32, _dest->buffer, _dest->width, _dest->height, _dest->pitch);
    }
    else
    {
        BITsurface temp;
        BITlut     tempLut;

        BITsurfaceConstruct(&temp);
        BITlutConstruct(&tempLut);

        if (( convert[_source->format][BITformat_8bits].func != NULL ) && 
            ( convert[BITformat_8bits][_destFormat].func != NULL     ))
        {
            BITsurfaceConvert( _allocator, _source, &temp, BITformat_8bits);
            BITlutConvert (_allocator, &_source->lut, &tempLut, BITlutFormat_x888);

            BITsurfaceConvert( _allocator, &temp, _dest, _destFormat);
            BITlutConvert (_allocator, &tempLut, &_dest->lut, defaultFormat[_destFormat]);
        }
        else if (( convert[_source->format][BITformat_4bits].func != NULL ) && 
                 ( convert[BITformat_4bits][_destFormat].func != NULL     ))
        {
            BITsurfaceConvert( _allocator, _source, &temp, BITformat_4bits);
            BITlutConvert (_allocator, &_source->lut, &tempLut, BITlutFormat_x888);

            BITsurfaceConvert( _allocator, &temp, _dest, _destFormat);
            BITlutConvert (_allocator, &tempLut, &_dest->lut, defaultFormat[_destFormat]);
        }
        else
        {
            ASSERT(0);
        }

        BITlutDestroy(&tempLut);
        BITsurfaceDestroy(&temp);
    }
}

void BITsurfaceClear(BITsurface* _surface)
{
	STDmset(_surface->buffer,0,_surface->size);
}

void BITlutSet(MEMallocator* _allocator, BITlut* _lut, BITlutFormat _format, void* _lutData, u16 _lutSize)
{
	u16 sizeOfColor = 0;
	u16 sizeOfColorSource = 0;

    
	switch(_lut->format)
	{
	case BITlutFormat_x888:
		sizeOfColorSource = sizeof(u32);
		break;
    case BITlutFormat_STe:
    case BITlutFormat_444:
        sizeOfColorSource = sizeof(u16);
        break;
    }

	switch(_format)
	{
	case BITlutFormat_x888:
		sizeOfColor = sizeof(u32);
		break;
    case BITlutFormat_STe:
    case BITlutFormat_444:
        sizeOfColor = sizeof(u16);
        break;
    }

    if ( sizeOfColor > 0 )
    {
        u32 size = sizeOfColor * _lutSize;
        u32 sizeSource = sizeOfColorSource * _lut->size;

        if ( size != sizeSource )
        {
            BITlutDestroy(_lut);
            _lut->data.p = MEM_ALLOC(_allocator, size);
            _lut->allocator = _allocator;
            _lut->bufferowner = true;
        }

        _lut->size = _lutSize;

        if ( _lutData != NULL )
        {
            STDmcpy (_lut->data.p, _lutData, size);
        }
        else
        {
            STDmset (_lut->data.p, 0, size);
        }
    }
    else
    {
        BITlutDestroy(_lut);    
    }

    _lut->format = _format;
}

void BITlutSetExternal(BITlut* _lut, BITlutFormat _format, void* _lutData, u16 _lutSize)
{
    ASSERT(_lutData != NULL);

    _lut->format = _format;
	_lut->data.p = _lutData;
    _lut->size = _lutSize;
    _lut->allocator = NULL;
    _lut->bufferowner = false;
}

void BITlutInit(MEMallocator* _allocator, BITlut* _lut, BITlutFormat _format, u16 _lutSize)
{
    BITlutSet(_allocator, _lut, _format, NULL, _lutSize);
}

void BITlutConvert (MEMallocator* _allocator, BITlut* _source, BITlut* _dest, BITlutFormat _destFormat)
{
    u32 lut[256];
    u16 t;
    u16 size = _source->size;


    ASSERT(size <= 256);

    if (( _source == _dest ) && ( _source->format == _destFormat ))
    {
        return;
    }

    STDmset(lut, 0, sizeof(lut));

    switch ( _source->format )
    {
    case BITlutFormat_BnW:
        lut[0] = 0xFFFFFF;
        break;

    case BITlutFormat_STe:
    case BITlutFormat_444:
        {
            u16* p = _source->data.p16;

            for (t = 0 ; t < size ; t++)
            {
                u16 c = PCENDIANSWAP16(*p++);
                u16 r = (c & 0xF00) >> 8;
                u16 g = (c & 0x0F0) >> 4;
                u16 b = (c & 0x00F);

                if ( _source->format == BITlutFormat_STe )
                {
                    r = COLST24b [r];
                    g = COLST24b [g];
                    b = COLST24b [b];
                }

                lut[t] = ((u32)r << 20) | ((u32)g << 12) | ((u32)b << 4);
            }
        }
        break;

    case BITlutFormat_x888:
        STDmcpy(lut, _source->data.p, size*sizeof(u32));
        break;
    }
    
    switch ( _destFormat )
	{
    case BITlutFormat_STe:
    case BITlutFormat_444:
        size = 16;
        break;
    case BITlutFormat_x888:
        size = 256;
        break;
    default:
        size = 0;
    }

    BITlutInit(_allocator, _dest, _destFormat, size);

    switch ( _destFormat )
    {
    case BITlutFormat_STe:
    case BITlutFormat_444:
        {
            u16* p = _dest->data.p16;

            for (t = 0 ; t < size ; t++)
            {
                u16 r = (u16)(lut[t] >> 20);
                u16 g = (u16)(lut[t] >> 12);
                u16 b = (u16)(lut[t] >> 4);

                r &= 0xF;
                g &= 0xF;
                b &= 0xF;

                if ( _destFormat == BITlutFormat_STe )
                {
                    r = COL4b2ST [r];
                    g = COL4b2ST [g];
                    b = COL4b2ST [b];
                }

                *p++ = PCENDIANSWAP16((r << 8) | (g << 4) | b);
            }
        }
        break;

    case BITlutFormat_x888:
        STDmcpy(_dest->data.p, lut, size*sizeof(u32));
        break;
	}
}


void BITsurfaceFSErrorDiffuse (MEMallocator* _allocator, BITsurface* _surface, u16 _rbits, u16 _gbits, u16 _bbits)
{
    typedef u8 DeltaBuffer[3][2];

	int last = 0;
	int current = 1;
	DeltaBuffer* delta;
	u16 deltasize;
	u16 y;
	u16 w = _surface->width;
	u16 h = _surface->height;
	u16 offset = 0;
    u8* buf;
	u8  rmask = ~((1 << (8 - _rbits)) - 1);
	u8  gmask = ~((1 << (8 - _gbits)) - 1);
	u8  bmask = ~((1 << (8 - _bbits)) - 1);
	

	ENUM(Component)
	{
		_R = 0,
		_G = 1,
		_B = 2
	};	
	
	deltasize = (w + 2) * 3 * 2;
	delta = (DeltaBuffer*) MEM_ALLOC(_allocator, deltasize);
	memset (delta, 0, deltasize);

	switch (_surface->format)
	{
	case BITformat_888:
		break;
	case BITformat_x888:
		offset = 1;
		break;
	default:
		ASSERT(0);
	}

    buf	= _surface->buffer;

	for (y = 0 ; y < h ; y++)
	{
        u16 r,g,b;
		s16 x;
		u8* s = buf;
		buf += _surface->pitch;

        if ( y & 1 )
        {
            s += (w - 1) * (3 + offset);

            for (x = w - 1 ; x >= 0 ; x--)
            {
                b = s[0];
                g = s[1];
                r = s[2];

                b += delta[x+2][_B][current] * 7/16;
                g += delta[x+2][_G][current] * 7/16;
                r += delta[x+2][_R][current] * 7/16;

                b += delta[x][_B][last]    * 3/16;
                g += delta[x][_G][last]    * 3/16;
                r += delta[x][_G][last]    * 3/16;

                b += delta[x+1][_B][last]  * 5/16;
                g += delta[x+1][_G][last]  * 5/16;
                r += delta[x+1][_R][last]  * 5/16;

                b += delta[x+2][_B][last]  * 1/16;
                g += delta[x+2][_G][last]  * 1/16;
                r += delta[x+2][_R][last]  * 1/16;

                b = STD_MIN(b, 255);
                g = STD_MIN(g, 255);
                r = STD_MIN(r, 255);

                delta[x+1][_R][current] = r & ~rmask;
                delta[x+1][_G][current] = g & ~gmask;
                delta[x+1][_B][current] = b & ~bmask;

                r &= rmask;
                g &= gmask;
                b &= bmask;

                s[0] = (u8) b;
                s[1] = (u8) g;
                s[2] = (u8) r;	

                s -= 3 + offset;
            }
        }
        else
        {
            for (x = 0 ; x < w ; x++)
            {
                s += offset;

                b = s[0];
                g = s[1];
                r = s[2];

                b += delta[x][_B][current] * 7/16;
                g += delta[x][_G][current] * 7/16;
                r += delta[x][_R][current] * 7/16;

                b += delta[x+2][_B][last]  * 3/16;
                g += delta[x+2][_G][last]  * 3/16;
                r += delta[x+2][_G][last]  * 3/16;

                b += delta[x+1][_B][last]  * 5/16;
                g += delta[x+1][_G][last]  * 5/16;
                r += delta[x+1][_R][last]  * 5/16;

                b += delta[x][_B][last]    * 1/16;
                g += delta[x][_G][last]    * 1/16;
                r += delta[x][_R][last]    * 1/16;

                b = STD_MIN(b, 255);
                g = STD_MIN(g, 255);
                r = STD_MIN(r, 255);

                delta[x+1][_R][current] = r & ~rmask;
                delta[x+1][_G][current] = g & ~gmask;
                delta[x+1][_B][current] = b & ~bmask;

                r &= rmask;
                g &= gmask;
                b &= bmask;

                *s++ = (u8) b;
                *s++ = (u8) g;
                *s++ = (u8) r;		
            }
        }
		
		last 	= current;
		current = !current;
	}

    MEM_FREE(_allocator, delta);
}
