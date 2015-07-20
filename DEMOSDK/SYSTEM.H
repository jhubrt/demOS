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

/*! @brief @ref SYS @file */

/*! @defgroup  SYS
    
    Implements DemOS core system services.                                                  <br>
        
    System provides:                                                                        <br>
    - interrupt backup / setup / restore system                                             <br>
    - standardized VBL routine                                                              <br>
    - 2 ring buffers:                                                                       <br>
        - one for core system (never freed)                                                 <br>
        - another for transient demo screens data                                           <br>            
    - a bi-threaded system, mixed between cooperative (main => idle thread) 
      and preemptive (idle thread => main)                                                  <br>
        - one main thread: task executed here should be sliced by frame / 
          synchronized with VBL                                                             <br>
          At the end of main thread, main loop yield to idle thread background task 
          with a call to SYSswitchIdle                                                      <br>
        - idle thread: executes a background task not synchronized with the VBL. 
          The task is restored in its last state then it is interrupted by VBL              
          rescheduling the main thread                                                      <br>
        - some simple synchronization primitives                                            <br>
*/

#ifndef SYSTEM_H
#define SYSTEM_H

#include "DEMOSDK\BASTYPES.H"
#include "DEMOSDK\ALLOC.H"

ENUM(SYSemulator)
{
    SYSemulator_NOTDETECTED,
    SYSemulator_STEEM,
    SYSemulator_TOSBOX,
    SYSemulator_PACIFIST
};

STRUCT(SYScore)
{
	RINGallocator	coremem;
	RINGallocator	mem;
	void*			bakGemdos32;
	void*			bakUSP;
	u8				bakvideoMode;
	u8				bakmfpInterruptEnableA;
	u8				bakmfpInterruptMaskA;
	u8				bakmfpInterruptEnableB;
	u8				bakmfpInterruptMaskB;
	u8				bakmfpVectorBase;
	u32				bakvbl;
	u32				bakdma;
	u8				bakvideoAdr[3];
	bool			has2Drives;
	u8				lastKey;
    SYSemulator     emulator;
    u8*             idleThreadStackBase;
    u32             phytop;

    u8*	            font;
    u16	            fontChars[256];

	MEMallocator		allocatorCoreMem;
	MEMallocator		allocatorMem;

#   if !defined(DEMOS_OPTIMIZED) && !defined(DEMOS_USES_BOOTSECTOR)	
	MEMallocator		allocatorStandard;
#	endif

#	ifndef __TOS__
	u32				memoryMapHigh;
#	endif
};

typedef void (*SYSthread)(void);

STRUCT(SYSinitParam)
{
	void*			coreAdr;
	u32				coreSize;
	void*			adr;
	u32				size;
	SYSthread		idleThread;
    u16             idleThreadStackSize;
};

#ifndef SYSTEM_C
	extern SYScore sys;
	extern u16	   SYSbeginFrameNum;

#endif

void SYSinit		        (SYSinitParam* _param);
void SYS_shutdown	        (void);
void SYSvsync		        (void);
void SYScheckHWRequirements (void);

extern volatile u32	SYSvblcount;
extern volatile u16	SYSvblLcount;

#ifdef __TOS__
#	define SYSvsync {	u16 currentvblcount = SYSvblLcount; while (SYSvblLcount == currentvblcount); }
#	define SYSmemoryHighMap	0
#else
#	define SYSvsync
#	define SYSmemoryHighMap sys.memoryMapHigh
#endif

void SYSswitchIdle(void);
bool SYSsetIdlethread(void* _stacklow, void*_stackhigh);
void SYStune(void);
u32	 SYSreadVideoBase (void);
void SYSwriteVideoBase (u32 _val);

#define SYS_kbhit  (sys.lastKey != *HW_KEYBOARD_DATA)
u8 SYSgetKb(void);

void SYSfastPrint(char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch, u32 _fontadr);

#define SYS_1P_BITSHIFT     1
#define SYS_2P_BITSHIFT     2
#define SYS_4P_BITSHIFT     3

#ifdef DEMOS_DEBUG
u16  SYStraceFPS        (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
u16  SYStraceAllocators (void* _image, u16 _pitch, u16 _planePitch, u16 _y);
u16	 SYStraceHW		 (void* _image, u16 _pitch, u16 _planePitch, u16 _y);

void SYSdebugPrint      (void* _screen, u16 _screenPitch, u16 _bitplanPitchShift, u16 _col, u16 _y, char* _s);
#else
#	define SYSdebugPrint(_screen, _screenPitch, _bitplanPitchShift, _col, _y, _s)
#endif

typedef s8 SYSlock;

#define SYSlockInit(_lock) { _lock = 0; }

bool SYSlockMain (SYSlock* _lock); /* when called from main thread => you should manage retry if locked (non blocking strategy) */
void SYSlockIdle (SYSlock* _lock); /* when called from idle thread, it will spin till it is unlocked by main thread (blocking strategy) */

void* SYSgemdosSetMode (void* _adr);
void  SYSreset (void);

#endif
