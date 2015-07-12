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

/*! @defgroup LOAD

    Manages loading requests on floppy disks

    LOAD system is designed to be cheap in terms of CPU usage (DMA + interrupts)                <br>

    - lock less system to post loading requests                                                 <br>
    - manages up to 3 loading requests :                                                        <br>
       - one should be reserved for the soundtrack system as high priority                      <br>
       - client has 2 remaining requests (in-order priority), so he can interlace the requests   
         to obtain maximum loading performance (no wait time when current request is processed, 
         next request is executed straight on by the interrupt code)                            <br>
    - data are organized on floppy disks by @see IMAGER tool 
      (it generates disks image data + C code to use it from demo code)                         <br>

    @see LOADdisk LOADresource LOADrequest 
*/

#ifndef LOAD_H
#define LOAD_H

#include "DEMOSDK\BASTYPES.H"

/*! @struct LOADdisk

    contains information for a floppy disk                                                      <br>
    - LOADresource entries table                                                                <br>
    - offsets table: different small data can be packed into the same sectors                   
      (in the same LOADresource entry). 
      offsets allows you to address them separately once loaded in memory                       <br>

    @struct LOADresource
    
    defines a FAT entry to address of a specific data 
    the resource table is generated with IMAGER tool

    @struct LOADmetaData

    defines detailed info for each file section (one or more file section(s) may exist 
    by entry)
*/

STRUCT(LOADmetaData)
{
    u32     offset;
    u32     size;
    s32     originalsize;
};

STRUCT(LOADresource)
{
	u16	nbsectors;		/* : 10; */
	u8  startsector;	/* : 5;  */
	u8	side;			/* : 1;  */
	u8	track;			/* : 8;	 */
    s16 metadataIndex;  /* : 16; */
};

STRUCT(LOADdisk)
{
	LOADresource* FAT;
    u16           nbEntries;
    LOADmetaData* metaData;  
    u16           nbMetaData;
	u8		      preferedDrive;

#	ifdef DEMOS_DEBUG
	char*	filename;
	FILE*	file;
#	endif
};

/*! @struct LOADrequest

    this structure is allocated by the LOADrequestLoad function (up to 3 requests at the same time)
    it allows you to determine if the request has been processed (processed member goes to 'true')
    you can also have information about the current progress of the loading (side_drive, track, sector)

    ! do not forget to free the request with LOADfreeRequest when you have acknowledged the end of the request
*/

STRUCT(LOADrequest)
{
    bool            allocated;
    volatile bool   processed;
    volatile u16	sector;
	u16		        reserved;
    volatile u16    side_drive;
    volatile u16    track;
    u16		        nbsectors;
    void*           address;
    u16		        order;
};


#define LOAD_SECTORSIZE	512UL
#define LOADroundBufferSize(SIZE)	((SIZE + 511UL) & 0xFFFFFE00UL)

#define LOAD_PRIOTITY_HIGH      0
#define LOAD_PRIORITY_INORDER   (LOADorder++)

#define LOAD_NOTPACKED -1L

#ifndef LOAD_C
extern u16 LOADorder;
#endif

void          LOADinit          		(void);
LOADrequest*  LOADrequestLoad           (LOADdisk* _media, u16 _resource, void* _buffer, u16 _order);
void          LOADwaitRequestCompleted  (LOADrequest* request);
void		  LOADwaitFDCIdle			(void);
#define       LOADfreeRequest(_request) _request->allocated = false;
u32           LOADgetEntrySize          (LOADdisk* _media, u16 _entryIndex);

#ifdef DEMOS_DEBUG
u16 LOADtrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#ifdef DEMOS_UNITTEST

#include "DEMOSDK\FSM.H"

void LOADunitTestInit   (FSM* _fsm);
void LOADunitTestUpdate (FSM* _fsm);
#endif

#endif
