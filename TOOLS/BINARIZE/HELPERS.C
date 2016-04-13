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
