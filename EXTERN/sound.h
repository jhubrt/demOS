/*-----------------------------------------------------------------------------------------------
   This file is part of Hatari, the Atari ST/TT/Falcon emulator ; 
   permission was granted on March 2022 to Jerome Hubert by the author Nicolas Pomarède to re-license this file under The MIT License (MIT)
   to use it in "demOS" Atari STe demos framework 


   The MIT License (MIT)
   
   Copyright (c) N.Pomarède
   
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

#ifndef HATARI_SOUND_H
#define HATARI_SOUND_H

/* definitions common for all sound rendering engines */

/* Internal data types */

typedef     __int64			yms64;

typedef		s8			    yms8;
typedef		s16             yms16;
typedef		s32 			yms32;

typedef		u8			    ymu8;
typedef		u16		    	ymu16;
typedef		u32			    ymu32;

typedef		yms16			ymsample;	/* Output samples are mono 16bits signed PCM */


#define YM_LINEAR_MIXING		1		/* Use ymout1c5bit[] to build ymout5[] */
#define YM_TABLE_MIXING			2		/* Use volumetable_original to build ymout5[] */
#define YM_MODEL_MIXING			3		/* Use circuit analysis model to build ymout5[] */

                                        /* Constants for YM2149_Normalise_5bit_Table */
#define	YM_OUTPUT_LEVEL			32767   /* amplitude of the final signal (0..65535 if centered, 0..32767 if not) */
#define YM_OUTPUT_CENTERED		false

extern int	YmVolumeMixing;

void Sound_WriteReg( int reg , ymu8 data );

#endif  /* HATARI_SOUND_H */
