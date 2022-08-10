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

extern "C"
{
#   include "DEMOSDK\BASTYPES.H"

#   include "DEMOSDK\STANDARD.H"

#   include "DEMOSDK\PC\SURFACE.H"
#   include "DEMOSDK\PC\WINDOW.H"
}

#include "TOOLS\BINARIZE\HELPERS.H"

extern MEMallocator stdAllocator;


u32 TOOLfileSize (FILE* _file)
{
    u32 filesize;
    u32 pos = ftell(_file);
    
    fseek (_file, 0, SEEK_END);
    filesize = ftell(_file);
    fseek (_file, pos, SEEK_SET);

    return filesize;
}

void TOOLreadcheck (void* _buf, u32 _size, FILE* _file)
{
    if ( fread (_buf, _size, 1, _file) != 1 )
    {
        printf ("#ERROR while reading %d bytes\n", _size);
        throw TOOLerror ("read file error");
    }
}

void TOOLwritecheck (const void* _buf, u32 _size, FILE* _file)
{
    if ( fwrite (_buf, _size, 1, _file) != 1 )
    {
        printf ("#ERROR while writing %d bytes\n", _size);
        throw TOOLerror ("write file error");
    }
}

void TOOLwritecheck(const u32& _val, FILE* _file)
{
    u32 val = PCENDIANSWAP32(_val);
    TOOLwritecheck(&val, sizeof(val), _file);
}

void TOOLwritecheck(const u16& _val, FILE* _file)
{
    u16 val = PCENDIANSWAP16(_val);
    TOOLwritecheck(&val, sizeof(val), _file);
}

void TOOLwritecheck(const u8& _val, FILE* _file)
{
    TOOLwritecheck(&_val, sizeof(_val), _file);
}

void TOOLwritecheck(const u32& _val, FILE* _file);
void TOOLwritecheck(const u16& _val, FILE* _file);


void TOOLtruncateFile (const char* _source, const char* _dest, u32 _size)
{
    u8* buffer = NULL;
    u32 filesize;
    FILE* file = fopen(_source, "rb");
    ASSERT(file != NULL);

    fseek (file, 0, SEEK_END);
    filesize = ftell(file);
    ASSERT(filesize >= _size);

    buffer = (u8*) malloc(_size);

    fseek (file, 0, SEEK_SET);
    fread (buffer, 1, _size, file);
    fclose (file);

    file = fopen (_dest, "wb");
    ASSERT(file != NULL);
    fwrite(buffer, 1, _size, file);
    fclose (file);
}


void TOOLwriteStaticData(char* _filename, char* _sourcefile, char* _symbolName)
{
	u32 filesize;
	FILE* file = fopen(_filename, "rb");
	ASSERT(file != NULL);	
	
	fseek (file, 0, SEEK_END);
	filesize = ftell(file);
	fseek (file, 0, SEEK_SET);

	{	
		u8* buffer = (u8*) malloc(filesize);
		u32 t;
	
		fread (buffer, 1, filesize, file);
		fclose(file);

		file = fopen(_sourcefile, "wt");
		ASSERT(file != NULL);	
		
		fprintf(file, "u8 %s[] = {\n", _symbolName);
		
		for (t = 0 ; t < filesize ; t++)
		{
			fprintf (file, "%u", buffer[t]);

			if ((t + 1) < filesize)
			{
				fprintf (file, ",");
			}

			if ((t & 31) == 31)
			{
				fprintf (file, "\n");
			}
		}

		fprintf(file, "};\n");
		
		fclose(file);		
		free (buffer);
	}
}
