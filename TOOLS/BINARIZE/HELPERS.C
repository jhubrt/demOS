/*------------------------------------------------------------------------------  -----------------
  The MIT License (MIT)

  Copyright (c) 2015-2016 J.Hubert

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
------------------------------------------------------------------------------------------------- */

#include "DEMOSDK\BASTYPES.H"
#include "TOOLS\BINARIZE\HELPERS.H"

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
