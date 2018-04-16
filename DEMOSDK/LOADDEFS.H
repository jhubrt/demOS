/*------------------------------------------------------------------------------  -----------------
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
------------------------------------------------------------------------------------------------- */

/*! @brief @ref LOAD @file */

/* Common defines shared with imager tool */

#ifndef LOADDEFS_H
#define LOADDEFS_H

#define LOAD_RESOURCE_MASK_TRACK            0x7F
#define LOAD_RESOURCE_MASK_SIDE             0x8000
#define LOAD_RESOURCE_MASK_NBSECTORS        0x3FF
#define LOAD_RESOURCE_RSHIFT_STARTSECTOR    11
#define LOAD_RESOURCE_MASK_STARTSECTOR      0xF
#define LOAD_RESOURCE_RSHIFT_METADATA       7

#define LOAD_METADATA_RSHIFT_OFFSET         11
#define LOAD_METADATA_RSHIFT_ORIGINALSIZE   11
#define LOAD_METADATA_LSHIFT_SIZEH          11
#define LOAD_METADATA_MASK_SIZEH            0x7FF
#define LOAD_METADATA_MASK_SIZEL            0x7FF

#define LOAD_FAT_STARTSECTOR                1
#define LOAD_FAT_NBSECTORS                  2

#define LOAD_SECTORSIZE	                    512UL

#endif
