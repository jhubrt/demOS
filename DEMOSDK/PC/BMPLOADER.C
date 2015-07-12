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

#include "DEMOSDK\PC\SURFACE.H"
#include "DEMOSDK\PC\BMPLOADER.H"


#define getWord(P)  (*(u16*)(P))
#define getDWord(P) (*(u32*)(P))


STRUCT(BMPPixel)
{
    u8 b;
    u8 g;
    u8 r;
};


static bool readSurface (FILE* _file, BITsurface* _surface)
{
    u8*	  destPixel; 
    int   line;
    u32   bytesToRead;
    u32   bytesToSkip;


    ASSERT(_file != NULL );
    ASSERT(_surface != NULL );

    bytesToRead = _surface->width;

    switch ( _surface->format )
    {
    case BITformat_888:
        bytesToRead *= 3;
        break;

    case BITformat_4bits:
        bytesToRead >>= 1;
        break;

	case BITformat_Plane1P:
        bytesToRead >>= 3;
        break;
    }

    bytesToSkip = 0;
    if ( bytesToRead & 3 )
    {
        bytesToSkip = 4 - (bytesToRead & 3);
    }

    for (line = _surface->height - 1 ; line >= 0 ; line--)
    {
        /* start with the bottom of the surface because BMP is X Flipped */
        destPixel = _surface->buffer + _surface->pitch * line;

        /* Read one line */
        if ( fread (destPixel, bytesToRead, 1, _file) != 1 )
        {
            return false;
        }
        else
        {
            if ( fseek (_file, bytesToSkip, SEEK_CUR) != 0 )
            {
                return false;
            }
        }
    }

    return true;
}


BITloadResult BITbmpLoad (BITsurface* _surface, MEMallocator* _allocator, char* _filename)
{
    u8			bmpFileHeader[14];    
    u8			bmpInfoHeader[64];    
    u32			headerSize;              
    u16			bitPerPixel;      
    u16			width, height;
    u16			paletteSize = 0;
    BITformat	format;
	BITloadResult  returnCode = BITloadResult_READERROR;


    FILE* file = fopen (_filename, "rb");

    if ( file == NULL )
        goto Error;
    
	if ( fread (bmpFileHeader, sizeof(bmpFileHeader), 1, file) != 1 )
        goto Error;

	returnCode = BITloadResult_UNKNOWN_FORMAT;

	if ( (bmpFileHeader[0] != 'B') || (bmpFileHeader[1] != 'M') )
        goto Error;

    /* The infoheader might be 12 bytes (OS/2 1.x), 40 bytes (Windows),
       or 64 bytes (OS/2 2.x).  Check the first 4 bytes to find out which */
    if ( fread (bmpInfoHeader, 4, 1, file) != 1 )
        goto Error;

    headerSize = getDWord(bmpInfoHeader);
    if ((headerSize < 12) || (headerSize > 64))
        goto Error;

    /* Load rest of the header */
    if ( fread (bmpInfoHeader+4, headerSize-4, 1, file) != 1 )
        goto Error;

    /* Get infos from the header */
    switch(headerSize)
    {
    case 12:
        /* Decode OS/2 1.x header => MS BITMAPCOREHEADER */
        width       = getWord(bmpInfoHeader + 4);
        height      = getWord(bmpInfoHeader + 6);
        bitPerPixel = getWord(bmpInfoHeader + 10);

        if (getWord(bmpInfoHeader+6) != 1)
            goto Error;	 /* Number of planes is not 1 -> not supported */
        break;

    case 40:
    case 64:
        // Decode Windows 3.x header (Microsoft calls this a BITMAPINFOHEADER)
        // or OS/2 2.x header, which has additional fields that we ignore
        width       = (u16) getDWord(bmpInfoHeader  + 4);
        height      = (u16) getDWord(bmpInfoHeader  + 8);
        bitPerPixel = getWord (bmpInfoHeader  + 14);
        paletteSize = (u16) getDWord(bmpInfoHeader  + 32);
        if ( (getWord(bmpInfoHeader+12) != 1) || (getDWord(bmpInfoHeader+16) != 0) )
            goto Error;  /* number of planes is not 1 or compressed format -> not supported */
        break;

    default:
        /* header of incoherent size -> failure */
        goto Error;
    }

    switch (bitPerPixel)
    {
    case 1:
		format = BITformat_Plane1P;
        break;
    case 4:
		format = BITformat_4bits;
        break;
    case 8:
        format = BITformat_8bits;
        break;
    case 24:
        format = BITformat_888;
        break;
    default:
        goto Error;
    }

	BITsurfaceInit (_allocator, _surface, format, width, height, BIT_DEFAULT_PITCH);

    if (paletteSize)
    {
        u32 lut[256];


        if ( fread (lut, paletteSize * sizeof(u32), 1, file) != 1 )
			goto Error;

        BITlutConstruct(&_surface->lut);
		BITlutSet(_allocator, &_surface->lut, BITlutFormat_x888, lut, paletteSize);
    }

    /* Move to the begining of bitmap data */
    if ( fseek (file, getDWord(bmpFileHeader+10), SEEK_SET) != 0 )
        goto Error;

    readSurface(file, _surface);

    fclose (file);

	return BITloadResult_OK;

Error:
    if ( file != NULL )
        fclose (file);
   
    return returnCode;
}


BITloadResult BITbmpLoadLUT (BITlut* _lut, MEMallocator* _allocator, char* _filename)
{
    u8 bmpFileHeader[14];   
    u8 bmpInfoHeader[64];   
    u32 headerSize;    
    u16 lutSize = 0;
	BITloadResult returnCode = BITloadResult_READERROR;


    FILE* file = fopen (_filename, "rb");

    if ( file == NULL )
        goto Error;

    if ( fread (bmpFileHeader, sizeof(bmpFileHeader), 1, file) != 1 )
        goto Error;

	returnCode = BITloadResult_UNKNOWN_FORMAT;

	if ( (bmpFileHeader[0] != 'B') || (bmpFileHeader[1] != 'M') )
        goto Error;

    /* The infoheader might be 12 bytes (OS/2 1.x), 40 bytes (Windows),
       or 64 bytes (OS/2 2.x).  Check the first 4 bytes to find out which */
    if ( fread (bmpInfoHeader, 4, 1, file) != 1 )
        goto Error;

	headerSize = getDWord(bmpInfoHeader);
    if ((headerSize < 12) || (headerSize > 64))
        goto Error;
    
	/* Load the rest of header */
    if ( fread (bmpInfoHeader+4, headerSize-4, 1, file) != 1 )
        goto Error;
    
    switch(headerSize)
    {
    case 12:
        /* Decode OS/2 1.x header (Microsoft calls this a BITMAPCOREHEADER) */
        if (getWord(bmpInfoHeader+6) != 1)           
            goto Error; /* The number of Planes is too high -> not supported */
        break;

    case 40:
    case 64:
        /* Decode Windows 3.x header (MS BITMAPINFOHEADER)
           or OS/2 2.x header, which has additional fields that we ignore */
        lutSize = (u16) getDWord(bmpInfoHeader + 32);

        if ( (getWord(bmpInfoHeader+12) != 1) || (getDWord(bmpInfoHeader+16) != 0) )
            goto Error;
        break;

    default:
        goto Error;
    }

    if (lutSize > 0)
    {
        u32 lut[256];

		ASSERT(lutSize <= 256);
        if ( fread (lut, lutSize * sizeof(u32), 1, file) != 1 )
            goto Error;
 
		BITlutSet(_allocator, _lut, BITlutFormat_x888, lut, lutSize);
    }
    else
    {
        goto Error;
    }

    fclose (file);

	return BITloadResult_OK;

Error:
	if ( file != NULL )
        fclose (file);
    
    return returnCode;
}



STRUCT(BITbmpInfoHeader)
{
    u32     biSize;
    s32     biWidth;
    s32     biHeight;
    s16     biPlanes;
    s16     biBitCount;
    u32     biCompression;
    u32     biSizeImage;
    s32     biXPelsPerMeter;
    s32     biYPelsPerMeter;
    u32     biClrUsed;
    u32     biClrImportant;
};



bool BITbmpSave (BITsurface* _surface, char* _filename)
{
    u8               bmpFileHeader[14];   
    BITbmpInfoHeader bmpInfoHeader;
    u32 paletteSize = 0;
    u32 bitmapSize  = _surface->width * 3 * _surface->height;
    FILE* file = NULL;


    ASSERT(_surface->format == BITformat_888);

    memset (bmpFileHeader, 0, sizeof(bmpFileHeader));

    getWord (bmpFileHeader)         = 0x4d42;  /* "BM" */
    getDWord(&bmpFileHeader[2])     = sizeof(bmpFileHeader) + bitmapSize + paletteSize;
    getDWord(&bmpFileHeader[10])    = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader);

    file = fopen (_filename, "wb");

    if ( file == NULL )
        goto Error;
    
	if ( fwrite (&bmpFileHeader, sizeof(bmpFileHeader), 1, file) != 1 )
        goto Error;

    bmpInfoHeader.biSize         = sizeof(bmpInfoHeader);
    bmpInfoHeader.biWidth        = _surface->width;
    bmpInfoHeader.biHeight       = _surface->height;
    bmpInfoHeader.biPlanes       = 1;
    bmpInfoHeader.biCompression  = 0;
    bmpInfoHeader.biBitCount     = 24;
    bmpInfoHeader.biClrImportant = 0;
    bmpInfoHeader.biClrUsed      = 0;
    bmpInfoHeader.biSizeImage    = bitmapSize;
    bmpInfoHeader.biXPelsPerMeter = 72;
    bmpInfoHeader.biYPelsPerMeter = 72;

	if ( fwrite (&bmpInfoHeader, sizeof(bmpInfoHeader), 1, file) != 1 )
        goto Error;

    {
        u16 i;
        u8* p = _surface->buffer + bitmapSize;
    
        for (i = 0 ; i < _surface->height ; i++)
        {
            p -= _surface->width * 3;
            fwrite (p, _surface->width * 3, 1, file);
        }
    }

    fclose (file);

	return true;

Error:
    if ( file != NULL )
        fclose (file);
   
    return false;
}


BITloadResult BITneoLoad (BITsurface* _surface, MEMallocator* _allocator, char* _filename)
{
	BITloadResult returnCode = BITloadResult_READERROR;

    FILE* file = fopen (_filename, "rb");

    if ( file == NULL )
        goto Error;
    
    BITsurfaceInit (_allocator, _surface, BITformat_Chunk4P, 320, 200, BIT_DEFAULT_PITCH);
    
    BITlutConstruct(&_surface->lut);
    BITlutInit(_allocator, &_surface->lut, BITlutFormat_STe, 16);

    fseek (file, 4, SEEK_SET);
    fread (_surface->lut.data.p, 16, sizeof(u16), file);

    fseek (file, 128, SEEK_SET);
    fread (_surface->buffer, 32000, 1, file);

    fclose(file);
    
	return BITloadResult_OK;

Error:
    if ( file != NULL )
        fclose (file);
   
    return returnCode;
}
