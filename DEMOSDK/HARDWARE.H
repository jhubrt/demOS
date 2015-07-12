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

/*! @brief @ref HARDWARE @file */

/*! @defgroup  HARDWARE
   
    HARDWARE maps most of STe registers and some specific constant values 
*/


#ifndef HARDWARE_H
#define HARDWARE_H

#include "DEMOSDK\BASTYPES.H"

#define HW_BLACK		0x000
#define HW_LIGHT_RED	0xF00 
#define HW_LIGHT_GREEN  0x0F0 
#define HW_LIGHT_BLUE   0x00F 
#define HW_RED			0x400 
#define HW_GREEN		0x040 
#define HW_BLUE			0x004 
#define HW_DARK_RED		0x200
#define HW_DARK_GREEN	0x020
#define HW_DARK_BLUE	0x002
#define HW_YELLOW		0x0FF
#define HW_MAGENTA		0xF0F
#define HW_CYAN   		0x0FF
#define HW_ORANGE		0x4F0
#define HW_WHITE		0xFFF

#define HW_YM_SEL_FREQCHA_L         0
#define HW_YM_SEL_FREQCHA_H         1
#define HW_YM_SEL_FREQCHB_L         2
#define HW_YM_SEL_FREQCHB_H         3
#define HW_YM_SEL_FREQCHC_L         4
#define HW_YM_SEL_FREQCHC_H         5
#define HW_YM_SEL_FREQNOISE         6
#define HW_YM_SEL_IO_AND_MIXER      7
#define HW_YM_SEL_LEVELCHA          8
#define HW_YM_SEL_LEVELCHB          9
#define HW_YM_SEL_LEVELCHC          10
#define HW_YM_SEL_FREQENVELOPE_L    11
#define HW_YM_SEL_FREQENVELOPE_H    12
#define HW_YM_SEL_ENVELOPESHAPE     13
#define HW_YM_SEL_IO_PORTA          14
#define HW_YM_SEL_IO_PORTB          15

#ifdef __TOS__

#define HW_VECTOR_INIT_PC   ((u32*)0x4)
#define HW_VECTOR_VBL		((u32*)0x70)
#define HW_VECTOR_HBL		((u32*)0x68)
#define HW_VECTOR_DMA		((u32*)0x11C)
#define HW_VECTOR_TIMERB	((volatile u32*)0x120)

/* Shifter */

#define HW_COLOR_LUT		((u16*)0xFFFF8240UL)

#define HW_VIDEO_MODE		((u8*)0xFFFF8260UL)

#define HW_VIDEO_BASE_H		((u8*)0xFFFF8201UL)
#define HW_VIDEO_BASE_M		((u8*)0xFFFF8203UL)
#define HW_VIDEO_BASE_L		((u8*)0xFFFF820DUL)  /* STe */

#define HW_VIDEO_COUNT_H	((u8*)0xFFFF8205UL)	 /* STe */
#define HW_VIDEO_COUNT_M	((u8*)0xFFFF8207UL)  /* STe */
#define HW_VIDEO_COUNT_L	((u8*)0xFFFF8209UL)  /* STe */

#define HW_VIDEO_SYNC   	((u8*)0xFFFF820AUL)

#define HW_VIDEO_OFFSET				((u8*)0xFFFF820FUL)	 /* STe */
#define HW_VIDEO_PIXOFFSET			((u8*)0xFFFF8265UL)	 /* STe */
#define HW_VIDEO_PIXOFFSET_HIDDEN	((u8*)0xFFFF8264UL)	 /* STe */

#define HW_YM_REGSELECT         ((u8*)0xFFFF8800UL)
#define HW_YM_REGDATA           ((u8*)0xFFFF8802UL)

/* DMA sound - STe only */

#define HW_DMASOUND_CONTROL			((u8*)0xFFFF8901UL)	

#define HW_DMASOUND_STARTADR_H		((u8*)0xFFFF8903UL)	
#define HW_DMASOUND_STARTADR_M		((u8*)0xFFFF8905UL)	
#define HW_DMASOUND_STARTADR_L		((u8*)0xFFFF8907UL)	

#define HW_DMASOUND_ENDADR_H		((u8*)0xFFFF890FUL)
#define HW_DMASOUND_ENDADR_M		((u8*)0xFFFF8911UL)
#define HW_DMASOUND_ENDADR_L		((u8*)0xFFFF8913UL)

#define HW_DMASOUND_COUNTER_H		((u8*)0xFFFF8909UL)	
#define HW_DMASOUND_COUNTER_M		((u8*)0xFFFF890BUL)	
#define HW_DMASOUND_COUNTER_L		((u8*)0xFFFF890DUL)	

#define HW_DMASOUND_MODE     		((u16*)0xFFFF8920UL)	

#define HW_MICROWIRE_DATA    		((u16*)0xFFFF8922UL)
#define HW_MICROWIRE_MASK    		((u16*)0xFFFF8924UL)

/* Keyboard ACIA */

#define HW_KEYBOARD_STATE 	((u8*)0xFFFFFC00UL)
#define HW_KEYBOARD_DATA 	((u8*)0xFFFFFC02UL)

/* MFP */

#define HW_MFP_INTERRUPT_ENABLE_A		((u8*)0xFFFFFA07UL)
#define HW_MFP_INTERRUPT_ENABLE_B		((u8*)0xFFFFFA09UL)
#define HW_MFP_INTERRUPT_PENDING_A		((u8*)0xFFFFFA0BUL)
#define HW_MFP_INTERRUPT_PENDING_B		((u8*)0xFFFFFA0DUL)
#define HW_MFP_INTERRUPT_INSERVICE_A	((u8*)0xFFFFFA0FUL)
#define HW_MFP_INTERRUPT_INSERVICE_B	((u8*)0xFFFFFA11UL)
#define HW_MFP_INTERRUPT_MASK_A     	((u8*)0xFFFFFA13UL)
#define HW_MFP_INTERRUPT_MASK_B     	((u8*)0xFFFFFA15UL)
#define HW_MFP_VECTOR_BASE				((u8*)0xFFFFFA17UL)  
#define HW_MFP_TIMER_A_CONTROL      	((u8*)0xFFFFFA19UL)
#define HW_MFP_TIMER_B_CONTROL		  	((u8*)0xFFFFFA1BUL)
#define HW_MFP_TIMER_CD_CONTROL		  	((u8*)0xFFFFFA1DUL)
#define HW_MFP_TIMER_A_DATA         	((u8*)0xFFFFFA1FUL)
#define HW_MFP_TIMER_B_DATA   		  	((u8*)0xFFFFFA21UL)
#define HW_MFP_TIMER_C_DATA         	((u8*)0xFFFFFA23UL)
#define HW_MFP_TIMER_D_DATA   		  	((u8*)0xFFFFFA25UL)

#define OS_FLOPVBL		((u16*)0x43e)
#define OS_NFLOPS		((u16*)0x4a6)

#else

STRUCT(SThardware)
{
    u32 vectorInitPC;
	u32 vectorVBL;
	u32 vectorHBL;
	u32 vectorDMA;
	u32 vectorTimerB;

	u8	keyboardState;
	u8	keyboardData;

	/* color */
	u16 colorLUT[16];

	/* video */
	u8  videoMode;
	u8	videoBaseH;
	u8	videoBaseM;
	u8	videoBaseL;

	u8	videoCountH;
	u8	videoCountM;
	u8	videoCountL;

	u8  videoOffset;
	u8	videoPixOffset;
	u8	videoPixOffsetHidden;

	u8  videoSync;

	u8  dmaSoundControl;
	u8  dmaSoundStartAdrH;
	u8  dmaSoundStartAdrM;
	u8  dmaSoundStartAdrL;
	u8  dmaSoundEndAdrH;
	u8  dmaSoundEndAdrM;
	u8  dmaSoundEndAdrL;
	u8  dmaSoundCounterH;
	u8  dmaSoundCounterM;
	u8  dmaSoundCounterL;
	u8  dmaSoundMode;
	u16 microwireData;
	u16 microwireMask;

	u8	mfpInterruptEnableA;
	u8	mfpInterruptEnableB;
	u8	mfpInterruptPendingA; 	
	u8	mfpInterruptPendingB; 	
	u8	mfpInterruptInServiceA;
	u8	mfpInterruptInServiceB;
	u8	mfpInterruptMaskA;
	u8	mfpInterruptMaskB;
	u8	mfpVectorBase;
	u8	mfpTimerAControl;
	u8	mfpTimerBControl;
	u8	mfpTimerCDControl;
	u8	mfpTimerAData;
	u8	mfpTimerBData;
	u8	mfpTimerCData;
	u8	mfpTimerDData;

    u8  ymRegSelect;
    u8  ymRegData;

	u16 _flopvbl;
	u16 _nflops;
};

#	ifndef HARDWARE_C
	extern SThardware g_STHardware;
#	endif

#   define HW_VECTOR_INIT_PC (&g_STHardware.vectorInitPC)

#	define HW_VECTOR_VBL	(&g_STHardware.vectorVBL)
#	define HW_VECTOR_HBL	(&g_STHardware.vectorHBL)
#	define HW_VECTOR_DMA	(&g_STHardware.vectorDMA)
#	define HW_VECTOR_TIMERB (&g_STHardware.vectorTimerB)

#	define HW_COLOR_LUT		(g_STHardware.colorLUT)
#	define HW_VIDEO_MODE	(&g_STHardware.videoMode)

#	define HW_KEYBOARD_DATA		(&g_STHardware.keyboardData)
#	define HW_KEYBOARD_STATE 	(&g_STHardware.keyboardState)

#	define HW_VIDEO_BASE_H	(&g_STHardware.videoBaseH)
#	define HW_VIDEO_BASE_M	(&g_STHardware.videoBaseM)
#	define HW_VIDEO_BASE_L	(&g_STHardware.videoBaseL)

#	define HW_VIDEO_COUNT_H	(&g_STHardware.videoBaseH)
#	define HW_VIDEO_COUNT_M	(&g_STHardware.videoBaseM)
#	define HW_VIDEO_COUNT_L	(&g_STHardware.videoBaseL)

#	define HW_VIDEO_SYNC   	(&g_STHardware.videoSync)

#	define HW_VIDEO_OFFSET				(&g_STHardware.videoOffset)
#	define HW_VIDEO_PIXOFFSET			(&g_STHardware.videoPixOffset)
#	define HW_VIDEO_PIXOFFSET_HIDDEN	(&g_STHardware.videoPixOffsetHidden)

#	define HW_DMASOUND_CONTROL			(&g_STHardware.dmaSoundControl)

#	define HW_DMASOUND_STARTADR_H		(&g_STHardware.dmaSoundStartAdrH)	
#	define HW_DMASOUND_STARTADR_M		(&g_STHardware.dmaSoundStartAdrM)	
#	define HW_DMASOUND_STARTADR_L		(&g_STHardware.dmaSoundStartAdrL)	

#	define HW_DMASOUND_ENDADR_H			(&g_STHardware.dmaSoundEndAdrH)
#	define HW_DMASOUND_ENDADR_M			(&g_STHardware.dmaSoundEndAdrM)
#	define HW_DMASOUND_ENDADR_L			(&g_STHardware.dmaSoundEndAdrL)

#	define HW_DMASOUND_COUNTER_H		(&g_STHardware.dmaSoundCounterH)	
#	define HW_DMASOUND_COUNTER_M		(&g_STHardware.dmaSoundCounterM)	
#	define HW_DMASOUND_COUNTER_L		(&g_STHardware.dmaSoundCounterL)	

#	define HW_DMASOUND_MODE     		(&g_STHardware.dmaSoundMode)	

#	define HW_MICROWIRE_DATA    		(&g_STHardware.microwireData)
#	define HW_MICROWIRE_MASK    		(&g_STHardware.microwireMask)

#   define HW_YM_REGSELECT              (&g_STHardware.ymRegSelect)
#   define HW_YM_REGDATA                (&g_STHardware.ymRegData)

#	define HW_MFP_INTERRUPT_ENABLE_A	(&g_STHardware.mfpInterruptEnableA)
#	define HW_MFP_INTERRUPT_ENABLE_B	(&g_STHardware.mfpInterruptEnableB)
#	define HW_MFP_INTERRUPT_PENDING_A	(&g_STHardware.mfpInterruptPendingA)
#	define HW_MFP_INTERRUPT_PENDING_B	(&g_STHardware.mfpInterruptPendingB)
#	define HW_MFP_INTERRUPT_INSERVICE_A	(&g_STHardware.mfpInterruptInServiceA)
#	define HW_MFP_INTERRUPT_INSERVICE_B	(&g_STHardware.mfpInterruptInServiceB)
#	define HW_MFP_INTERRUPT_MASK_A     	(&g_STHardware.mfpInterruptMaskA)
#	define HW_MFP_INTERRUPT_MASK_B     	(&g_STHardware.mfpInterruptMaskB)
#	define HW_MFP_VECTOR_BASE			(&g_STHardware.mfpVectorBase)
#	define HW_MFP_TIMER_A_CONTROL      	(&g_STHardware.mfpTimerAControl)
#	define HW_MFP_TIMER_B_CONTROL		(&g_STHardware.mfpTimerBControl)
#	define HW_MFP_TIMER_CD_CONTROL		(&g_STHardware.mfpTimerCDControl)
#	define HW_MFP_TIMER_A_DATA         	(&g_STHardware.mfpTimerAData)
#	define HW_MFP_TIMER_B_DATA   		(&g_STHardware.mfpTimerBData)
#	define HW_MFP_TIMER_C_DATA         	(&g_STHardware.mfpTimerCData)
#	define HW_MFP_TIMER_D_DATA   		(&g_STHardware.mfpTimerDData)

#	define OS_FLOPVBL					(&g_STHardware._flopvbl)
#	define OS_NFLOPS					(&g_STHardware._nflops)

#endif

#define HW_VIDEO_SYNC_50HZ	 2
#define HW_VIDEO_SYNC_60HZ	 0
#define HW_VIDEO_SYNC_EXTERN 1

#define HW_VIDEO_MODE_4P	0
#define HW_VIDEO_MODE_2P	1
#define HW_VIDEO_MODE_1P	2

#define HW_DMASOUND_CONTROL_OFF			0
#define HW_DMASOUND_CONTROL_PLAYONCE	1
#define HW_DMASOUND_CONTROL_PLAYLOOP	3

#define HW_DMASOUND_MODE_6258HZ		0
#define HW_DMASOUND_MODE_12517HZ	1
#define HW_DMASOUND_MODE_25033HZ	2
#define HW_DMASOUND_MODE_50066HZ	3

#define HW_DMASOUND_MODE_MONO		0x80
#define HW_DMASOUND_MODE_STEREO		0

#define HW_MICROWIRE_MASK_SOUND		0x7FF

#define HW_MICROWIRE_VOLUME			0x4C0	/* 0 1100 0000	values 0 to 40 */
#define HW_MICROWIRE_VOLUME_LEFT	0x540	/* 1 0100 0000  values 0 to 20 */
#define HW_MICROWIRE_VOLUME_RIGHT	0x500	/* 1 0000 0000  values 0 to 20 */
#define HW_MICROWIRE_VOLUME_TREBLE	0x480	/* 0 1000 0000  values 0 to 12 */
#define HW_MICROWIRE_VOLUME_BASS  	0x440	/* 0 0100 0000  values 0 to 12 */

#define HW_MICROWIRE_MIXER_YM_12DB	0x400
#define HW_MICROWIRE_MIXER_YM		0x401
#define HW_MICROWIRE_MIXER_NO_YM	0x402

#define HW_KEYBOARD_KEYRELEASE		0x80
#define HW_KEY_F1					0x3B
#define HW_KEY_F2					0x3C
#define HW_KEY_F3					0x3D
#define HW_KEY_F4					0x3E
#define HW_KEY_F5					0x3F
#define HW_KEY_F6					0x40
#define HW_KEY_F7					0x41
#define HW_KEY_F8					0x42
#define HW_KEY_F9					0x43
#define HW_KEY_F10					0x44
#define HW_KEY_SPACEBAR				0x39
#define HW_KEY_BACKSPACE			0xE
#define HW_KEY_TAB					0xF
#define HW_KEY_S					0x1F
#define HW_KEY_V					0x2F

#define HW_68KOP_RTS                0x4E75

#endif
