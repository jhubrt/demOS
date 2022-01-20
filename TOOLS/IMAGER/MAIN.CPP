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

#include "DEMOSDK\BASTYPES.H"

#include "TOOLS\IMAGER\IMAGER.H"
#include "TOOLS\IMAGER\SCRIPT.H"


void main (int argc, char** argv)
{
	bool result;

    FloppyImageScript floppyimagescript;

    if (argc == 3)
    {        
        if (floppyimagescript.loadScript(argv[1]))
        {
            char name [256];
            char scriptpath [256];
            char subdir [256];


            _splitpath(argv[1], scriptpath, subdir, name, NULL);
            strcat(scriptpath, subdir);

            result = writeST(
                scriptpath,
                argv[2],
                name,
                floppyimagescript.m_nbsectors,
                floppyimagescript.m_nbsides,
                floppyimagescript.m_nbtracks,
                floppyimagescript.m_preferedunit, 
                floppyimagescript.m_executablebootsector,
                floppyimagescript.getFilesList()
                );

            if (result == false)
            {
                printf("#ERROR: while creating image\n");
            }
        }
        else
        {
            printf ("#ERROR: floppy image script not loaded '%s'\n", argv[1]);
        }
    }
    else
    {
        printf ("Wrong argument\n" 
                "Usage:\n"
                "  imager imagescript.txt imagedestdir\n");
    }
}
