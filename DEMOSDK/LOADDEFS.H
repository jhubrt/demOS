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
