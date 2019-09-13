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

#define SYS_NBMAX_VBLROUTINES 5

typedef void(*SYSinterupt)(void);

enum SYSemulator_
{
    SYSemulator_NOTDETECTED,
    SYSemulator_STEEM,
    SYSemulator_TOSBOX,
    SYSemulator_PACIFIST
};
typedef enum SYSemulator_ SYSemulator;

STRUCT(SYScore)
{
    /* should be set into main start */
	void*			bakGemdos32;
    u8*             membase;
    u8*             coreHeapbase;
    u32             coreHeapsize;
    u8*             mainHeapbase;
    u32             mainHeapsize;

    /* init after */
	RINGallocator	coremem;
	RINGallocator	mem;
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
    u8				key;
	u8				lastKey;
	bool			has2Drives;
    bool            isMegaSTe;
    bool            OSneedRestore;
    SYSemulator     emulator;
    u8*             idleThreadStackBase;
    u32             phytop;

	MEMallocator	allocatorCoreMem;
	MEMallocator	allocatorMem;

#   if !defined(DEMOS_OPTIMIZED) && !defined(DEMOS_USES_BOOTSECTOR)	
	MEMallocator	allocatorStandard;
#	endif

#	ifndef __TOS__
	u32				memoryMapHigh;
#	endif
};

typedef void (*SYSthread)(void);

STRUCT(SYSinitThreadParam)
{
    SYSthread       idleThread;
    u16             idleThreadStackSize;
};

#ifndef SYSTEM_C
	extern SYScore      sys;
    extern SYSinterupt  SYSvblroutines[SYS_NBMAX_VBLROUTINES];
#   ifdef DEMOS_DEBUG
    extern u16	        SYSbeginFrameNum;
#   endif
#endif

void SYSinit		        (void);
void SYSinitHW              (void);
void SYSinitThreading       (SYSinitThreadParam* _param);
void SYSshutdown	        (void);
void SYSvsync		        (void);
void SYScheckHWRequirements (void);
bool SYSgetCookie           (u32 _id, u32* _value);
void SYSdbgBreak            (void);
void SYSvbldonothing        (void); 
void SYSvblend              (void);
void SYSvblrunRTSroutine    (void);

extern volatile u32	SYSvblcount;
extern volatile u16	SYSvblLcount;

#ifdef __TOS__
#	define SYSvsync {	u16 currentvblcount = SYSvblLcount; while (SYSvblLcount == currentvblcount); }
#	define SYSmemoryHighMap	0
#else
#	define SYSvsync
#	define SYSmemoryHighMap sys.memoryMapHigh
#endif

void SYSswitchIdle      (void);
bool SYSsetIdlethread   (void* _stacklow, void*_stackhigh);
u32	 SYSreadVideoBase   (void);
void SYSwriteVideoBase  (u32 _val);
u32  SYSvidCountRead    (void);
u16  SYSwmovep          (void* _adr);
u32  SYSlmovep          (void* _adr);

#define SYSkbAcquire  sys.key = *HW_KEYBOARD_DATA
#define SYSkbHit  (sys.lastKey != sys.key)
void SYSkbReset(void);

void SYSfastPrint(char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch);

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
void* SYSmalloc (size_t _size);
void  SYSreset (void);

#endif
