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

#define LOAD_C

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\LOAD.H"
#include "DEMOSDK\HARDWARE.H"
#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"


u16 LOADorder = 1;

ASMIMPORT volatile u8				LOADinprogress;
ASMIMPORT volatile u16				LOADsecpertrack;
ASMIMPORT		   LOADrequest		LOADreq;
ASMIMPORT volatile LOADrequest*	LOADcurrentreq;
ASMIMPORT          u16				LOADreqNb;
ASMIMPORT volatile u32              LOADdeconnectAndStartRequest;

LOADrequest* LOADpush (void* _buffer, u16 _startsector, u32 _starttracknside, u32 _loadnbsectorsnorder);
void LOADidle(void);

#define LOAD_SECTORS_PER_TRACK 10

#ifdef DEMOS_DEBUG
/* #	define LOAD_CHECK_CONTENT */ /* LOAD_CHECK_CONTENT needs DEMOS_DEBUG */
#endif

void LOADinit (void)
{
#	ifdef __TOS__
	*HW_VECTOR_DMA = (u32) LOADidle;
	LOADsecpertrack = LOAD_SECTORS_PER_TRACK;
	STDcpuSetSR(0x2300);
#	endif
}

#ifdef DEMOS_DEBUG
static void* loadUsingStdlib (void* _buffer, LOADdisk* _media, u16 _resourceid)
{
	LOADresource* rsc = &_media->FAT[_resourceid];
    u32 offset = ((rsc->track * 2 + rsc->side) * LOAD_SECTORS_PER_TRACK + rsc->startsector) * 512UL;
    u32 result;
    u32 size = LOADgetEntrySize(_media, _resourceid);
	u8* buf;

	if ( _buffer == NULL )
	{
		buf = (u8*) malloc (size);
	}
	else
	{
		buf = (u8*) _buffer;
	}

    ASSERT(buf != NULL);

    if ( _media->file == NULL )
    {
        _media->file = fopen(_media->filename, "rb");
        ASSERT(_media->file != NULL);
    }

    fseek  ( _media->file, offset , SEEK_SET );
    result  = fread ( buf, 1, size, _media->file);
    ASSERT (result == size);

    return buf;
}
#endif

LOADrequest* LOADrequestLoad (LOADdisk* _media, u16 _resourceid, void* _buffer, u16 _order)
{
	LOADresource* rsc = &_media->FAT[_resourceid];

#	ifndef DEMOS_LOAD_FROMHD

	u8 drive = sys.has2Drives & (_media->preferedDrive != DEMOS_INVERT_DRIVE);

	LOADrequest* loadRequest = LOADpush (_buffer, rsc->startsector + 1, (u32) rsc->track | ((u32) rsc->side << 16) | ((u32) drive << 17), ((u32)_order << 16) | (u32) rsc->nbsectors);

#	ifdef LOAD_CHECK_CONTENT
	if ( loadRequest != NULL )
	{
		u16* image = (u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L));
		static y = 40;
		u32 i;
        u8* buf;
		u32 size = rsc->size;


        LOADwaitRequestCompleted (loadRequest);
        LOADwaitFDCIdle();

        buf = (u8*) loadUsingStdlib (NULL, _media, _resourceid);

		for (i = 0 ; i < size ; i++)
		{
            if ( *(buf + i) != *(((u8*)_buffer) + i) )
			{
                u16* image = (u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L));
                static char line[] = "            ";

                STDuxtoa(line, i, 6);
                SYSdebugPrint ( image + 2, 160, SYS_4P_BITSHIFT, 0, 100, line);
                break;
			}
		}

		free (buf);
	}
#	endif /* LOAD_CHECK_CONTENT */

    return loadRequest;

#	else /* DEMOS_LOAD_FROMHD */

    static LOADrequest request[3];
	static u16 requestnum = 0;
	LOADrequest* curReq = &request[requestnum++];

	if ( requestnum >= 3 )
	{
		requestnum = 0;
	}

	loadUsingStdlib (_buffer, _media, _resourceid);

	curReq->allocated = true;
	curReq->processed = true;
    curReq->address   = _buffer;

    return curReq;

#	endif
}


u32 LOADgetEntrySize (LOADdisk* _media, u16 _entryIndex)
{
    ASSERT(_entryIndex < _media->nbEntries);
    return (u32)_media->FAT[_entryIndex].nbsectors * 512UL;
}


void LOADwaitFDCIdle (void)
{
	while ( LOADinprogress );
}

void LOADwaitRequestCompleted (LOADrequest* _request)
{
    ASSERT (_request->allocated);
	while ( _request->processed == 0 );
	_request->allocated = false;
}

#ifdef DEMOS_DEBUG
u16 LOADtrace (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
	u16 t, h = 8;

	{
	    static char line[] = "FDCbusy=   request=       desel=       ";

		line[8] = '0' + (LOADinprogress != 0);
		STDuxtoa(&line[19], (u32) LOADcurrentreq, 6);
        STDuxtoa(&line[32], LOADdeconnectAndStartRequest, 6);

		SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y, line);
	}
	
	{
		static char line[] = "            ";
		LOADrequest* r = &LOADreq;		

		for (t = 0 ; t < LOADreqNb ; t++, r++)
		{
            line[0] = ( r == LOADcurrentreq ) ? '>' : ' ';

			if ( r->allocated )
			{
                if ( r->processed == false )
                {
					line[2] = '*';
				    STDuxtoa(&line[8], r->nbsectors, 3);
			    }
				else
				{
					line[2] = '+';
				}
				STDuxtoa(&line[4], r->order		, 3);
			}
			else
			{
				STDmset(line, 0x20202020UL, sizeof(line) - 1);
				line[2] = '-';
			}

    		SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + h, line);
            h += 8;
		}
	}

    if (LOADcurrentreq != NULL)
	{
		static char line[] = "tk=   sc=   si=  dv=  nb=    a=      ";

		STDuxtoa(&line[3], LOADcurrentreq->track, 2);
		STDuxtoa(&line[9], LOADcurrentreq->sector, 2);
		line[15] = '0' + ((LOADcurrentreq->side_drive & 0x0100) == 0);
		line[20] = '0' + ((LOADcurrentreq->side_drive & 0x0400) == 0);
		STDuxtoa(&line[25], LOADcurrentreq->nbsectors, 3);
		STDuxtoa(&line[31], (u32) LOADcurrentreq->address, 6);

		SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + h, line);
		h += 8;
	}

	return h;
}
#endif


#ifdef DEMOS_UNITTEST

#define NBREQUESTS 2
#define RESOURCEOFFSET 6

u8*				g_bufReference[NBREQUESTS];
u8*				g_bufLoaded[NBREQUESTS];
LOADrequest*	g_requests[NBREQUESTS];
LOADdisk*		g_media = NULL;

#include "DISK2.H"

void LOADunitTestInit (FSM* _fsm)
{
	u16 t;


    IGNORE_PARAM(_fsm);
    
    g_media = (&RSC_DISK2);

	for (t = 0 ; t < NBREQUESTS ; t++)
	{
#       ifndef DEMOS_USES_BOOTSECTOR
        g_bufReference[t] = (u8*) loadUsingStdlib (NULL, g_media, t + RESOURCEOFFSET);
#       endif
        g_bufLoaded   [t] = (u8*) MEM_ALLOC (&sys.allocatorMem, 512UL * 255UL);
		
		g_requests[t] = NULL;
	}

    FSMgotoNextState (_fsm);
}

void LOADunitTestUpdate (FSM* _fsm)
{
	u16 num = STDmfrnd();
	u16 t;
	

    IGNORE_PARAM(_fsm);

	if ((num & 0x1FFF) == 16)
	{
		for (t = 0 ; t < NBREQUESTS ; t++)
		{
			if ( g_requests[t] == NULL )
			{
				g_requests[t] = LOADrequestLoad(g_media, t + RESOURCEOFFSET, g_bufLoaded[t], (STDmfrnd() & 7) == 0 ? LOAD_PRIOTITY_HIGH : LOAD_PRIORITY_INORDER);
			}
		}
	}

	for (t = 0 ; t < NBREQUESTS ; t++)
	{
		if ( g_requests[t] != NULL )
		{
			if ( g_requests[t]->processed )
			{
				u32 size = LOADgetEntrySize (g_media, t + RESOURCEOFFSET);
				u32 i;

#               ifndef DEMOS_USES_BOOTSECTOR
				for (i = 0 ; i < size ; i++)
				{
					if ( *(g_bufReference[t] + i) != *(g_bufLoaded[t] + i) )
					{
						break;
					}
				}

				if ( i < size )
				{
					u16* image = (u16*)(((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | ((u32)*HW_VIDEO_BASE_L));
					static char line[] = "            ";

					STDuxtoa(line, i, 6);
					SYSdebugPrint ( image, 160, SYS_4P_BITSHIFT, 0, 100, line);

                    ASSERT(0);
				}
#               endif

				g_requests[t]->allocated = false;
				g_requests[t] = NULL;
			}
		}
	}
}

#endif
