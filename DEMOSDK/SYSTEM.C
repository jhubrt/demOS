/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

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

#include "DEMOSDK\BASTYPES.H"

#define SYSTEM_C

#include "DEMOSDK\STANDARD.H"
#include "DEMOSDK\SYSTEM.H"
#include "DEMOSDK\HARDWARE.H"

#include "DEMOSDK\DATA\SYSTFNT.H"

SYScore sys;

#ifdef DEMOS_DEBUG
u16	SYSbeginFrameNum = 0;
#endif

#ifdef __TOS__

u32*   SYSdetectEmu (void);
extern SYSinterupt SYSvblroutines[SYS_NBMAX_VBLROUTINES];
void   SYSvblend (void);

#else

void EMULnewFrame(void);
bool EMULupdateRequested(void);

SYSinterupt  SYSvblroutines[SYS_NBMAX_VBLROUTINES] = {SYSvblend, SYSvblend, SYSvblend, SYSvblend, SYSvblend};
volatile u32 SYSvblcount;
volatile u16 SYSvblLcount;

static SYSthread SYSidleThread = NULL;

void SYSvbldonothing    (void) {}
void SYSvblrunRTSroutine(void) {}
void SYSvblend          (void) {}
void SYSdbgBreak        (void) {}

u32* SYSdetectEmu (void)
{
    static u32 dummy[2] = {0,0};
    return dummy;
}

void SYSswitchIdle(void) 
{
	if ( SYSidleThread != NULL )
	{
        #ifndef __TOS__
        EMULnewFrame();
        if (EMULupdateRequested())
        #endif
        {
		    SYSidleThread();
            
            SYSvblcount++;
            SYSvblLcount++;
        }
    }

#   ifndef __TOS__
    {
        u16 i;

        for (i = 0 ; i < ARRAYSIZE(SYSvblroutines) ; i++)
        {
            if (SYSvblroutines[i] == SYSvblend)
            {
                break;
            }
            SYSvblroutines[i]();
        }
    }
#   endif

#   ifdef DEMOS_DEBUG
    SYSbeginFrameNum = SYSvblLcount;
#   endif
}

u32 SYSvidCountRead (void) 
{ 
    u32 val = ((u32)*HW_VIDEO_COUNT_H << 16) | ((u32)*HW_VIDEO_COUNT_M << 8) | (u32)*HW_VIDEO_COUNT_L;
	return val; 
}

u32 SYSreadVideoBase (void) 
{ 
    u32 val = (sys.memoryMapHigh) | ((u32)*HW_VIDEO_BASE_H << 16) | ((u32)*HW_VIDEO_BASE_M << 8) | (u32)*HW_VIDEO_BASE_L;
	return val; 
}

void SYSwriteVideoBase (u32 _val) 
{
	*HW_VIDEO_BASE_H = (u8)(_val >> 16);
	*HW_VIDEO_BASE_M = (u8)(_val >> 8);
	*HW_VIDEO_BASE_L = (u8) _val;
}

u32 SYSlmovep (void* _adr)
{
    return 0;
}

#endif

/*------------------------------------------------------------------
    SYSTEM FONT
--------------------------------------------------------------------*/
void SYSfastPrint(char* _s, void* _screenprintadr, u16 _screenPitch, u16 _bitplanPitch, u32 _fontadr)
#ifdef __TOS__
;
#else
{
    u8* adr = (u8*)_screenprintadr;
    SYSFont* font = (SYSFont*) _fontadr;

    _bitplanPitch--;

	while (*_s)
	{
		u8  c = *_s++;
		u8* d = adr;
        u16 index  = font->charsmap[c];
        u8* bitmap = font->data + (index << SYS_FNT_OFFSETSHIFT);

        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;
        *d = *bitmap++;	d += _screenPitch;

		if (1 & (u32) adr)
		{
			adr += _bitplanPitch;
		}
		else
		{
			adr++;
		}
	}
}
#endif


#ifdef DEMOS_DEBUG
void SYSdebugPrint(void* _screen, u16 _screenPitch, u16 _bitplanPitchShift, u16 _col, u16 _y, char* _s)
{
    u16 bitplanPitch = 1 << _bitplanPitchShift;
	u8* adr = (u8*)_screen;


	if  (_y != 0)	
	{
		adr += _y * _screenPitch;
	}

	adr += (_col & 0xFFFE) << (_bitplanPitchShift - 1);
	adr += _col & 1;

    SYSfastPrint (_s, adr, _screenPitch, bitplanPitch, (u32) &SYSfont);
}
#endif


void  SYSvblinterrupt   (void)       PCSTUB;
void* SYSgemdosSetMode  (void* _adr) PCSTUBRET;
void  SYSemptyKb        (void)       PCSTUB;

#ifndef DEMOS_USES_BOOTSECTOR
static void* SYSstdAlloc(void* _alloc, u32 _size)
{
	IGNORE_PARAM(_alloc);
	return malloc(_size);
}

static void SYSstdFree(void* _alloc, void* _adr)
{
	IGNORE_PARAM(_alloc);
	free(_adr);
}
#endif
   
struct SYScookie_
{
    u32 id;
	u32 value;
};
typedef struct SYScookie_ SYScookie;


bool SYSgetCookie(u32 _id, u32* _value)
{
#   ifdef __TOS__
    SYScookie *cookie = *(SYScookie**) OS_COOKIEJAR;

    if (cookie != NULL)
    {
        while (cookie->id != 0)
        {
            if ( cookie->id == _id )
            {
                *_value = cookie->value;
                return true;
            }
            cookie++;
        }
    } 
#   endif
    
    return false;
}

void SYSinitDbgBreak(void) PCSTUB;

#ifndef DEMOS_USES_BOOTSECTOR
void SYSinitStdAllocator(void)
{
    sys.allocatorStandard.alloc     = SYSstdAlloc;
    sys.allocatorStandard.alloctemp = SYSstdAlloc;
    sys.allocatorStandard.free      = SYSstdFree;
    sys.allocatorStandard.allocator = NULL;
}
#endif


void SYSinit(void)
{
    void* bakGemdos32 = sys.bakGemdos32;
    u32   offsetofcoremem = (u32)&(((SYScore*)0)->coremem);

    ASSERT(sys.coreHeapbase != NULL);
    ASSERT(sys.mainHeapbase != NULL);

    SYSinitDbgBreak ();

    STDmset (&sys.coremem, 0, sizeof(sys) - offsetofcoremem);

    sys.bakGemdos32 = bakGemdos32;

    RINGallocatorInit ( &sys.mem, sys.mainHeapbase, sys.mainHeapsize );

    {
#   ifdef DEMOS_USES_BOOTSECTOR
        u32* emudetect = (u32*) 0x600;
#   else
        u32* emudetect = SYSdetectEmu();
#   endif

        if (( emudetect[0] == 0x53544565UL ) && ( emudetect[1] == 0x6d456e67UL ))
        {
            sys.emulator = SYSemulator_STEEM;
        }
        else if (( emudetect[0] == 0x456D753FUL ) && ( emudetect[1] == 0x456D753FUL ))    
        {
            sys.emulator = SYSemulator_PACIFIST;
        }
        else if ( emudetect[0] == 0x54426f78UL ) 
        {
            sys.emulator = SYSemulator_TOSBOX;
        }
        else
        {
            sys.emulator = SYSemulator_NOTDETECTED;
        }
    }

#   ifdef __TOS__
    {   /* detect mega ste */
        u32 machineType = 0;
        u8  id[4] = {'_','M','C','H'};

        bool cookieFound = SYSgetCookie( *(u32*)id, &machineType );        
        sys.isMegaSTe = machineType == 0x10010UL;
        if (cookieFound)
        {
            if ( sys.isMegaSTe )
            {
                *HW_MEGASTE_CPUMODE = HW_MEGASTE_CPUMODE_8MHZ | HW_MEGASTE_CPUMODE_NOCACHE;
            }
        }
    }
#   else
    sys.memoryMapHigh = ((u32)sys.coreHeapbase) & 0xFF000000;
#   endif

    sys.invertDrive = DEMOS_INVERT_DRIVE;

    *HW_YM_REGSELECT = HW_YM_SEL_IO_PORTA;
    if ((HW_YM_GET_REG() & 0x4) == 0) /* if drive B selected into YM */
        sys.invertDrive = true;

    sys.has2Drives  = *OS_NFLOPS >= 2;
    sys.phytop      = *OS_PHYTOP;

	sys.bakUSP = STDgetUSP();

    RINGallocatorInit ( &sys.coremem, sys.coreHeapbase, sys.coreHeapsize );

    sys.allocatorMem.alloc     = (MEMallocFunc) RINGallocatorAlloc;
    sys.allocatorMem.alloctemp = (MEMallocFunc) RINGallocatorAllocTemp;
	sys.allocatorMem.free      = (MEMfreeFunc)  RINGallocatorFree;
	sys.allocatorMem.allocator = &sys.mem;

	sys.allocatorCoreMem.alloc     = (MEMallocFunc) RINGallocatorAlloc;
    sys.allocatorCoreMem.alloctemp = (MEMallocFunc) RINGallocatorAllocTemp;
	sys.allocatorCoreMem.free      = (MEMfreeFunc)  RINGallocatorFree;
	sys.allocatorCoreMem.allocator = &sys.coremem;
}

void SYSinitHW (void)
{
    sys.OSneedRestore = true;

	sys.bakvideoMode = *HW_VIDEO_MODE;
	sys.bakvbl	     = *HW_VECTOR_VBL;
	sys.bakdma		 = *HW_VECTOR_DMA;

	sys.bakvideoAdr[0] = *HW_VIDEO_BASE_H;
	sys.bakvideoAdr[1] = *HW_VIDEO_BASE_M;
	sys.bakvideoAdr[2] = *HW_VIDEO_BASE_L;

	sys.bakmfpInterruptEnableA = *HW_MFP_INTERRUPT_ENABLE_A;
	sys.bakmfpInterruptMaskA   = *HW_MFP_INTERRUPT_MASK_A;
	sys.bakmfpInterruptEnableB = *HW_MFP_INTERRUPT_ENABLE_B;
	sys.bakmfpInterruptMaskB   = *HW_MFP_INTERRUPT_MASK_B;

	STDcpuSetSR(0x2700);

	*HW_MFP_INTERRUPT_ENABLE_A = 0;
	*HW_MFP_INTERRUPT_ENABLE_B = 0;
	
	*HW_MICROWIRE_MASK = 0x7FF;
	*HW_VECTOR_VBL = (u32) SYSvblinterrupt;

	sys.bakmfpVectorBase	   = *HW_MFP_VECTOR_BASE;

	*OS_FLOPVBL = 1;

	*HW_MFP_INTERRUPT_ENABLE_B |= 0x80;
	*HW_MFP_INTERRUPT_MASK_B   |= 0x80;
	*HW_MFP_VECTOR_BASE		   &= ~0x8;

	*HW_MFP_INTERRUPT_MASK_A   |= 0x1;	/* enable Timer B mask */

	sys.lastKey = sys.key = 0;
}

void SYSinitThreading(SYSinitThreadParam* _param)
{
    sys.idleThreadStackBase = (u8*) RINGallocatorAlloc ( &sys.coremem, _param->idleThreadStackSize );
    ASSERT(sys.idleThreadStackBase != NULL);
    
    sys.idleThreadStackSize = _param->idleThreadStackSize;

    STDmset(sys.idleThreadStackBase, 0xFAFAFAFAUL, _param->idleThreadStackSize);

#	ifdef __TOS__
    if ( SYSsetIdlethread (sys.idleThreadStackBase, sys.idleThreadStackBase + _param->idleThreadStackSize) )
    {
        _param->idleThread ();
        ASSERT(0);	/* you should not return from this function */
    }
#	else
    SYSidleThread = _param->idleThread;
#	endif
}

void SYSshutdown(void)
{
    if (sys.OSneedRestore)
    {
        *HW_VIDEO_BASE_H = sys.bakvideoAdr[0];
        *HW_VIDEO_BASE_M = sys.bakvideoAdr[1];
        *HW_VIDEO_BASE_L = sys.bakvideoAdr[2];

        *HW_VIDEO_MODE = sys.bakvideoMode;
        STDcpuSetSR(0x2700);
        *HW_VECTOR_VBL = sys.bakvbl;
        *HW_VECTOR_DMA = sys.bakdma;
        STDcpuSetSR(0x2300);

        *HW_MFP_INTERRUPT_ENABLE_A = sys.bakmfpInterruptEnableA;
        *HW_MFP_INTERRUPT_MASK_A   = sys.bakmfpInterruptMaskA;

        *HW_MFP_INTERRUPT_ENABLE_B = sys.bakmfpInterruptEnableB;
        *HW_MFP_INTERRUPT_MASK_B   = sys.bakmfpInterruptMaskB;
        *HW_MFP_VECTOR_BASE		   = sys.bakmfpVectorBase;

        *OS_FLOPVBL = 0;

        STDsetUSP (sys.bakUSP);
    }
}

void SYSkbReset(void)
{
    sys.lastKey = sys.key;
    SYSemptyKb();
}

#ifdef __TOS__

#ifdef DEMOS_ASSERT

#define SYS_ASSERT_COLOR 0xA00

void SYSassert(char* _message, char* _file, int _line)
{
    static char line[] = "line=       ";
#   ifdef __TOS__
    u8 buffer[1];
#   else
    u8 buffer[40000];
#   endif


    STDcpuSetSR(0x2700);

    _message[79] = 0;
    _file   [79] = 0;

    STDutoa (&line[5], _line, 6);

    if ( sys.mem.buffer == NULL )
    {
        sys.mem.buffer = buffer + 4096;
    }

    SYSwriteVideoBase((u32) sys.mem.buffer);
    STDmset(sys.mem.buffer, 0, 32000);

    *HW_COLOR_LUT = SYS_ASSERT_COLOR;
    *HW_VIDEO_OFFSET = 0;
    *HW_VIDEO_PIXOFFSET = 0;
    *HW_VIDEO_MODE = HW_VIDEO_MODE_2P;

    STDmset (HW_COLOR_LUT + 1, 0xFFFFFFFFUL, 30);

    SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0,  0, "Assertion failed:");
    SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0,  8, _message);
    SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0, 16, _file);
    SYSdebugPrint(sys.mem.buffer, 160, SYS_2P_BITSHIFT, 0, 24, line);
    while(1);
}
#endif /* DEMOS_ASSERT */

#endif /* !__TOS__ */

u16 SYSevaluateStackUsage(u8* p, u16 size_)
{
    u16 t, freebytes = 0;

    for (t = 0 ; t < size_ ; t++)
    {
        if (*p++ == 0xFA)
            freebytes++; 
        else
            break;
    }

    return size_ - freebytes;
}

#ifdef DEMOS_DEBUG

u16 SYStraceFPS (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
	char temp[2];

    temp[0] = SYSvblLcount - SYSbeginFrameNum + '1';	/* frame rate */
	temp[1] = 0;
	SYSdebugPrint ( _image, _pitch, _planePitch, 39, _y, temp);
    
    return 12;
}

static u16 SYS_traceRingAlloc (void* _image, u16 _pitch, u16 _planePitch, u16 _y, RINGallocator* _allocator)
{
    {
        static char line[] = "buf =       end =       size=      ";

        STDuxtoa(&line[5] , (u32) _allocator->buffer   , 6);
        STDuxtoa(&line[17], (u32) _allocator->bufferEnd, 6);
        STDuxtoa(&line[29], _allocator->size     , 6);

        SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y, line);
    }

    {
        static char line[] = "head=       tail=       last=      ";

        STDuxtoa(&line[5] , (u32)_allocator->head     , 6);
        STDuxtoa(&line[17], (u32)_allocator->tail     , 6);
        STDuxtoa(&line[29], (u32)_allocator->last     , 6);

        SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y + 8, line);
    }

    return 20;
}

u16 SYStraceAllocators (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
    u16 h;
        
    h =  SYS_traceRingAlloc (_image, _pitch, _planePitch, _y    , &sys.coremem);
    h += SYS_traceRingAlloc (_image, _pitch, _planePitch, _y + h, &sys.mem);

    return h;
}

u16 SYStraceHW (void* _image, u16 _pitch, u16 _planePitch, u16 _y)
{
	static char line[] = "vmod= |   key=   mach=     vbl=    |    ";
    
	STDuxtoa (&line[5] , *HW_VIDEO_MODE, 1);
	STDuxtoa (&line[7] , *HW_VIDEO_SYNC == HW_VIDEO_SYNC_50HZ ? 0x50 : 0x60, 2);
	STDuxtoa (&line[14], *HW_KEYBOARD_DATA, 2);
    line[22] = sys.emulator  ? 'e' : 'h';
    line[23] = sys.isMegaSTe ? 'm' : 's';
    STDuxtoa (&line[31], SYSvblLcount, 4);
    STDuxtoa (&line[36], SYSbeginFrameNum, 4);

	SYSdebugPrint ( _image, _pitch, _planePitch, 0, _y, line); 

	return 12;
}

static void sysStackTestRecurs (int _level, int _max, int* _values)
{
    if (_level < _max)
    {
        _values[_level] = _level;
        sysStackTestRecurs (_level + 1, _max, _values);
    }
}

void SYSstackTest (void)
{
    while (1)
    {
        int values [64];
        u16 max = STDifrnd() & 63;
        u16 t;


        *HW_COLOR_LUT ^= 0x2;

        sysStackTestRecurs(0, max, values);

        for (t = 0 ; t < max ; t++)
        {
            ASSERT(values[t] == t);
        }
    }
}

#endif
