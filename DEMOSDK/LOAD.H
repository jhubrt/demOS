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
#include "DEMOSDK\LOADDEFS.H"

/*! @struct LOADdisk

    contains information for a floppy disk                                                      <br>
    - LOADresource entries table                                                                <br>
    - offsets table: different small data can be packed into the same sectors                   
      (in the same LOADresource entry). 
      offsets allows you to address them separately once loaded in memory                       <br>

    @struct LOADresource
    
    defines a FAT entry to address of a specific data 
    the resource table is generated with IMAGER tool

    @struct LOADmetadata

    defines detailed info for each file section (one or more file section(s) may exist 
    by entry)
*/

STRUCT(LOADmetadata)
{
    u32     offsetsizeh;                /* 21 bits offset        | 11 bits size high bits */
    u32     originalsizesizel;          /* 21 bits original size | 11 bits size low bits  */ 
};

STRUCT(LOADresource)
{
	u16	    startsectorsidenbsectors;	/* 1 bit side | 4 bits start sector | 11 bits nb sectors */
	u16	    metadataindextrack;		    /* 9 bits metadataindex | 7 bits track num */
};

STRUCT(LOADdisk)
{
	LOADresource* FAT;
    u16           nbEntries;
    LOADmetadata* metaData;  
    u16           nbMetaData;
	u8		      preferedDrive;
    void**        preload;

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

#define LOADroundBufferSize(SIZE)	((SIZE + 511UL) & 0xFFFFFE00UL)

#define LOAD_PRIOTITY_HIGH      0
#define LOAD_PRIORITY_INORDER   (LOADorder++)

#define LOAD_NOTPACKED -1L

#ifndef LOAD_C
extern u16 LOADorder;
#endif

void          LOADinit          		(LOADdisk* _firstmedia, u16 _nbEntries, u16 _nbMetaData);
void          LOADinitFAT               (u8 _drive, LOADdisk* _media, u16 _nbEntries, u16 _nbMetaData);
LOADrequest*  LOADrequestLoad           (LOADdisk* _media, u16 _resource, void* _buffer, u16 _order);
LOADrequest*  LOADdata                  (LOADdisk* _media, u16 _resource, void* _buffer, u16 _order);
void          LOADwaitRequestCompleted  (LOADrequest* request);
void		  LOADwaitFDCIdle			(void);
#define       LOADfreeRequest(_request) if (_request != NULL) _request->allocated = false;
void*         LOADpreload               (void* _framebuffer, u16 _pitch, u16 _planepitch, void* _preload, u32 _preloadsize, void* _current, LOADdisk* _disk, u8* _resources, u16 _nbResources);


u32           LOADresourceRoundedSize   (LOADdisk* _media, u16 _entryIndex);
u16           LOADresourceMetaDataIndex (LOADdisk* _media, u16 _entryIndex);
#define LOAD_RESOURCE_NOMETADATA        0x1FF

u32           LOADmetadataOffset        (LOADdisk* _media, u16 _metaDataIndex);
u32           LOADmetadataSize          (LOADdisk* _media, u16 _metaDataIndex);
u32           LOADmetadataOriginalSize  (LOADdisk* _media, u16 _metaDataIndex);
#define LOAD_METADATA_NOTPACKEDSIZE     0x1FFFFF


#ifdef DEMOS_DEBUG
u16 LOADtrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
#endif

#ifdef DEMOS_UNITTEST

#include "DEMOSDK\FSM.H"

void LOADunitTestInit   (FSM* _fsm);
void LOADunitTestUpdate (FSM* _fsm);
#endif

#endif
