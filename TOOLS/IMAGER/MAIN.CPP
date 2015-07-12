/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of rebirth demo

  rebirth demo is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  rebirth demo is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with rebirth demo.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#include "TOOLS\IMAGER\IMAGER.H"

// Script your content here
static char* g_disk1[] = 
{
    "..\\\\OUTPUT\\\\BOOTSTRA.BOT", NULL,
    "BOOT\\\\BOOT.ARJ"     , NULL,
    "..\\\\DEMOSOPT.ARJ"   , NULL,
	"SYSTFONT.BIN"		   , NULL,
	"POLYZOOM\\\\CYBERVECTOR.BIN"	, 
						  "POLYZOOM\\C.DAT;"
						  "POLYZOOM\\Y_1.DAT;"
						  "POLYZOOM\\Y_2.DAT;"
						  "POLYZOOM\\B_1.DAT;"
						  "POLYZOOM\\B_2.DAT;"
						  "POLYZOOM\\E_1.DAT;"
						  "POLYZOOM\\E_2.DAT;"
						  "POLYZOOM\\R_1.DAT;"
						  "POLYZOOM\\R_2.DAT;"
						  "POLYZOOM\\N.DAT;"
						  "POLYZOOM\\T_1.DAT;"
						  "POLYZOOM\\T_2.DAT;"
						  "POLYZOOM\\I_1.DAT;"
						  "POLYZOOM\\I_2.DAT;"
						  "POLYZOOM\\S_1.DAT;"
						  "POLYZOOM\\S_2.DAT;"
						  "POLYZOOM\\PAL1.PAL;"
						  "POLYZOOM\\PAL2.PAL;"
						  "POLYZOOM\\PAL3.PAL;",	
	"POLYZOOM\\\\SIN.BIN"  , NULL,
	"POLYZOOM\\\\REBIRTH.ARJ", NULL,
	"VISUALIZ\\\\PAL.BIN"	, 
						  "VISUALIZ\\PAL0.PAL;"
						  "VISUALIZ\\PAL1.PAL;"
						  "VISUALIZ\\PAL2.PAL;"
						  "VISUALIZ\\PAL3.PAL;"
						  "VISUALIZ\\PAL4.PAL;"
                          "VISUALIZ\\TEXTOS.PAL;",
    "VISUALIZ\\TEXTOS.ARJ"  , NULL,
	"ZIK\\\\4190.RAW"		, NULL,
	"ZIK\\\\360222.RAW"		, NULL,
	"ZIK\\\\1606334.RAW"	, NULL,
	"ZIK\\\\1784382.RAW"	, NULL,
	"ZIK\\\\3475534.RAW"	, NULL,
	"ZIK\\\\3564542.RAW"	, NULL,
	"ZIK\\\\5967758.RAW"	, NULL,
	"ZIK\\\\6056766.RAW"	, NULL,
	"ZIK\\\\7391886.RAW"	, NULL,
	NULL
};


static char* g_disk2[] = 
{
    "..\\\\OUTPUT\\\\BOOTSEC2.BOT", NULL,
	"ZIK\\\\7569902.RAW"	, NULL,
	"ZIK\\\\12465342.RAW"	, NULL,
	"ZIK\\\\14245502.RAW"	, NULL,
	"ZIK\\\\14646038.RAW"	, NULL,
    "SLIDES\\\\PHOTF.HC"    , NULL,
	"SLIDES\\\\PHOT5.HC"    , NULL,
	"SLIDES\\\\PHOT8.HC"    , NULL,
	"SLIDES\\\\PHOT6.HC" 	, NULL,

    "SLIDES\\\\MASKS.PT"    , "SLIDES\\PHOTFMSK.PT;"
                              "SLIDES\\PHOT5MSK.PT;"
                              "SLIDES\\PHOT8MSK.PT;"
                              "SLIDES\\PHOT6MSK.PT;"

                              "SLIDES\\PHOTF.PA8;"
                              "SLIDES\\PHOT5.PA8;"
                              "SLIDES\\PHOT8.PA8;"
                              "SLIDES\\PHOT6.PA8;",

    "FUGIT\\\\FONT.ARJ"     , "FUGIT\\FONT.ARJ;"
                              "FUGIT\\FONTSCAN.ARJ;"
                              "FUGIT\\FONTOFFS.BIN;",
	NULL
};


void main (int argc, char* argv)
{
	bool result;

	result = writeST ("DISK1", 10, 2, 82, g_disk1, 0, argc <= 2);	

    if ( result )
    {
	    result = writeST ("DISK2", 10, 2, 82, g_disk2, 1, argc <= 1);
    }

    if (result == false)
    {
        printf ("IMAGER error\n");
    }
}
