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


/*! @brief @ref BIT @file */
   
#ifndef BMPLOADER_H
#define BMPLOADER_H

#include "DEMOSDK\PC\SURFACE.H"

ENUM(BITloadResult)
{
    BITloadResult_OK,    
    BITloadResult_READERROR,    
    BITloadResult_UNKNOWN_FORMAT
} ;

BITloadResult BITbmpLoad	(BITsurface* _surface, MEMallocator* _allocator, char* _filename);
BITloadResult BITbmpLoadLUT (BITlut* _lut, MEMallocator* _allocator, char* _filename);
bool          BITbmpSave    (BITsurface* _surface, char* _filename);
bool          BITdegasSave  (BITsurface* _surface, char* _filename);

BITloadResult BITneoLoad    (BITsurface* _surface, MEMallocator* _allocator, char* _filename);
BITloadResult BITdegasLoad  (BITsurface* _surface, MEMallocator* _allocator, char* _filename);

#endif
