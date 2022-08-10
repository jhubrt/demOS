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

/*! HARDWARE maps most of STe registers and some specific constant values 
*/

#ifndef HARDWARE_H
#define HARDWARE_H

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

#define HW_MICROWIRE_MIXER_YM_12DB	0x410   /* for fixed hardware this is 0x10 not 0x00 */
#define HW_MICROWIRE_MIXER_YM		0x401
#define HW_MICROWIRE_MIXER_NO_YM	0x402

#define HW_KEYBOARD_KEYRELEASE		0x80

/* reference for a QWERTY keyboard -------------------------- */

/* functions key */
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

/* line 1 */
#define HW_KEY_ESC          		0x1     
#define HW_KEY_1            		0x2
#define HW_KEY_2            		0x3
#define HW_KEY_3            		0x4
#define HW_KEY_4            		0x5
#define HW_KEY_5            		0x6
#define HW_KEY_6            		0x7
#define HW_KEY_7            		0x8
#define HW_KEY_8            		0x9
#define HW_KEY_9            		0xA
#define HW_KEY_0               		0xB
#define HW_KEY_MINUS           		0xC
#define HW_KEY_EQUAL           		0xD 
#define HW_KEY_QUOTE           		0x29
#define HW_KEY_BACKSPACE			0xE

/* line 2 */
#define HW_KEY_TAB					0xF
#define HW_KEY_Q            		0x10    
#define HW_KEY_W            		0x11
#define HW_KEY_E            		0x12
#define HW_KEY_R            		0x13
#define HW_KEY_T            		0x14
#define HW_KEY_Y            		0x15
#define HW_KEY_U            		0x16
#define HW_KEY_I            		0x17
#define HW_KEY_O            		0x18
#define HW_KEY_P            		0x19
#define HW_KEY_BRACKET_LEFT    		0x1A
#define HW_KEY_BRACKET_RIGHT   		0x1B
#define HW_KEY_RETURN               0x1C
#define HW_KEY_DELETE               0x53

/* line 3 */
#define HW_KEY_CONTROL              0x1D
#define HW_KEY_A					0x1E
#define HW_KEY_S					0x1F
#define HW_KEY_D					0x20
#define HW_KEY_F					0x21
#define HW_KEY_G					0x22
#define HW_KEY_H					0x23
#define HW_KEY_J					0x24
#define HW_KEY_K					0x25
#define HW_KEY_L					0x26
#define HW_KEY_SEMICOLON			0x27
#define HW_KEY_AT         	        0x28
#define HW_KEY_SHARP     		    0x2B    

/* line 4 */
#define HW_KEY_SHIFT_LEFT           0x2A
#define HW_KEY_ANTI_SLASH           0x60
#define HW_KEY_Z					0x2C
#define HW_KEY_X					0x2D
#define HW_KEY_C					0x2E
#define HW_KEY_V					0x2F
#define HW_KEY_B					0x30
#define HW_KEY_N					0x31
#define HW_KEY_M					0x32
#define HW_KEY_COMMA               	0x33
#define HW_KEY_DOT                  0x34
#define HW_KEY_SLASH            	0x35
#define HW_KEY_SHIFT_RIGHT			0x36

/* line 5 */
#define HW_KEY_ALTERNATE			0x38
#define HW_KEY_SPACEBAR				0x39
#define HW_KEY_CAPS_LOCK			0x3A

/* cursors pad */
#define HW_KEY_HOME                 0x47
#define HW_KEY_UP				    0x48
#define HW_KEY_LEFT     			0x4B
#define HW_KEY_RIGHT			    0x4D
#define HW_KEY_DOWN					0x50
#define HW_KEY_INSERT               0x52
#define HW_KEY_HELP                 0x62
#define HW_KEY_UNDO                 0x61

/* numpad */
#define HW_KEY_NUMPAD_PARENT_LEFT	0x63
#define HW_KEY_NUMPAD_PARENT_RIGHT  0x64
#define HW_KEY_NUMPAD_DIVIDE		0x65
#define HW_KEY_NUMPAD_MULTIPLY      0x66

#define HW_KEY_NUMPAD_MINUS         0x4A
#define HW_KEY_NUMPAD_PLUS  		0x4E

#define HW_KEY_NUMPAD_7     		0x67
#define HW_KEY_NUMPAD_8     		0x68
#define HW_KEY_NUMPAD_9     		0x69

#define HW_KEY_NUMPAD_4     		0x6A
#define HW_KEY_NUMPAD_5     		0x6B
#define HW_KEY_NUMPAD_6     		0x6C

#define HW_KEY_NUMPAD_1     		0x6D
#define HW_KEY_NUMPAD_2     		0x6E
#define HW_KEY_NUMPAD_3     		0x6F

#define HW_KEY_NUMPAD_0             0x70
#define HW_KEY_NUMPAD_DOT           0x71
#define HW_KEY_NUMPAD_ENTER         0x72

/* mega STe CPU freq and cache */
#define HW_MEGASTE_CPUMODE_8MHZ     0
#define HW_MEGASTE_CPUMODE_16MHZ    1
#define HW_MEGASTE_CPUMODE_NOCACHE  0
#define HW_MEGASTE_CPUMODE_CACHED   2

/* blitter */
#define HW_BLITTER_HOP_BIT1             0     
#define HW_BLITTER_HOP_HTONE            1
#define HW_BLITTER_HOP_SOURCE           2
#define HW_BLITTER_HOP_SOURCE_AND_HTONE 3

#define HW_BLITTER_OP_BIT0              0     
#define HW_BLITTER_OP_S_AND_D           1
#define HW_BLITTER_OP_S_AND_NOT_D       2
#define HW_BLITTER_OP_S                 3
#define HW_BLITTER_OP_NOT_S_AND_D       4
#define HW_BLITTER_OP_D                 5
#define HW_BLITTER_OP_S_XOR_D           6
#define HW_BLITTER_OP_S_OR_D            7
#define HW_BLITTER_OP_NOT_S_AND_NOT_D   8
#define HW_BLITTER_OP_NOT_S_XOR_D       9
#define HW_BLITTER_OP_NOT_D             10
#define HW_BLITTER_OP_S_OR_NOT_D        11
#define HW_BLITTER_OP_NOT_S             12
#define HW_BLITTER_OP_NOT_S_OR_D        13
#define HW_BLITTER_OP_NOT_S_OR_NOT_D    14  
#define HW_BLITTER_OP_BIT1              15

#define HW_BLITTER_CTRL1_BUSY           0x80
#define HW_BLITTER_CTRL1_HOGMODE_BLIT   0x40
#define HW_BLITTER_CTRL1_SMUDGE         0x20
/* bit 0..3 : half tone pattern index to start with */

#define HW_BLITTER_CTRL2_FORCE_XTRA_SRC     0x80
#define HW_BLITTER_CTRL2_NO_FINAL_SRC_READ  0x40
/* bit 0..3 : skew (right bit shift) */

/*         OP
    HOP    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
       0   4  8  8  4  8  8  8  8  8  8  8  8  4  8  8  4
       1   4  8  8  4  8  8  8  8  8  8  8  8  4  8  8  4
       2   4 12 12  8 12  8 12 12 12 12  8 12  8 12 12  4
       3   4 12 12  8 12  8 12 12 12 12  8 12  8 12 12  4
    
    (Actually, ENDMASKs differing from $FFFF are rumoured to also
     delay the BLiTTER, i have not yet verified this.) */

#ifdef __TOS__

#   define HW_DEFINE_REGISTER(TYPE,ADR,NAME)   ((volatile TYPE*)ADR)
#   define HW_YM_SET_REG(REGNUM,DATA)   { *HW_YM_REGSELECT = (REGNUM); *HW_YM_REGDATA = (DATA); }
#   define HW_YM_GET_REG()              (*HW_YM_REGSELECT)

#   define HW_DISABLE_MOUSE()                   { *HW_KEYBOARD_DATA = 0x12; } /* deactivate mouse management on ACIA */

#else

#   define HW_DEFINE_REGISTER(TYPE,ADR,NAME)   (&g_STHardware.reg_##NAME)
#   define HW_DECLARE_REGISTER(TYPE,NAME)      TYPE reg_##NAME

#   define HW_DISABLE_MOUSE()

void emulYMselectReg(u8 _regnum);
void emulYMwriteReg(u8 _data);
void hwSetYMReg(u8 _regnum, u8 _data);

#   define HW_YM_SET_REG(REGNUM,DATA)   { emulYMselectReg(REGNUM); emulYMwriteReg(DATA); hwSetYMReg(REGNUM,DATA); }
#   define HW_YM_GET_REG()              ((&g_STHardware.reg_HW_YM_REGDATA)[*HW_YM_REGSELECT])

struct SThardware_
{
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_HTONE          ); 
    u16 reg_HW_BLITTER_HTONE_1TO15 [15];

    HW_DECLARE_REGISTER(u16 , HW_BLITTER_XINC_SOURCE    );
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_YINC_SOURCE    );       
    HW_DECLARE_REGISTER(u32 , HW_BLITTER_ADDR_SOURCE    );       
                           
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_ENDMASK1       );   
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_ENDMASK2       );  
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_ENDMASK3       );   
                           
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_XINC_DEST      );  
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_YINC_DEST      );   
    HW_DECLARE_REGISTER(u32 , HW_BLITTER_ADDR_DEST      );   
                           
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_XSIZE          );   
    HW_DECLARE_REGISTER(u16 , HW_BLITTER_YSIZE          );   
                           
    HW_DECLARE_REGISTER(u8  , HW_BLITTER_HOP            );   
    HW_DECLARE_REGISTER(u8  , HW_BLITTER_OP             );   
    HW_DECLARE_REGISTER(u8  , HW_BLITTER_CTRL1          );   
    HW_DECLARE_REGISTER(u8  , HW_BLITTER_CTRL2          );   

    HW_DECLARE_REGISTER(u32 , HW_VECTOR_INIT_PC         );   
    HW_DECLARE_REGISTER(u32 , HW_VECTOR_VBL		        ); 
    HW_DECLARE_REGISTER(u32 , HW_VECTOR_HBL		        ); 
    HW_DECLARE_REGISTER(u32 , HW_VECTOR_DMA		        ); 
    HW_DECLARE_REGISTER(u32 , HW_VECTOR_TIMERB          );

    HW_DECLARE_REGISTER(u8  , HW_YM_REGSELECT           );
    HW_DECLARE_REGISTER(u8  , HW_YM_REGDATA             );
    u8 HW_YM_REGDATA_1TO15[15];

    HW_DECLARE_REGISTER(u16, HW_COLOR_LUT		        );
    u16 reg_HW_COLOR_LUT_1TO15 [15];

    HW_DECLARE_REGISTER(u8 , HW_VIDEO_MODE		        );  

    HW_DECLARE_REGISTER(u8 , HW_VIDEO_BASE_H		    );      
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_BASE_M		    );      
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_BASE_L		    ); 

    HW_DECLARE_REGISTER(u8 , HW_VIDEO_COUNT_H	        ); 
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_COUNT_M	        ); 
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_COUNT_L	        ); 

    HW_DECLARE_REGISTER(u8 , HW_VIDEO_SYNC   	        ); 

    HW_DECLARE_REGISTER(u8 , HW_VIDEO_OFFSET			); 
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_PIXOFFSET	        ); 
    HW_DECLARE_REGISTER(u8 , HW_VIDEO_PIXOFFSET_HIDDEN  );

    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_CONTROL		);

    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_STARTADR_H	    );
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_STARTADR_M     );
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_STARTADR_L	    );

    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_ENDADR_H		); 
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_ENDADR_M		);  
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_ENDADR_L		);  

    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_COUNTER_H	    );  	
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_COUNTER_M	    );  	
    HW_DECLARE_REGISTER(u8 , HW_DMASOUND_COUNTER_L	    );  	

    HW_DECLARE_REGISTER(u16, HW_DMASOUND_MODE     	    );

    HW_DECLARE_REGISTER(u16, HW_MICROWIRE_DATA    	    ); 
    HW_DECLARE_REGISTER(u16, HW_MICROWIRE_MASK    	    ); 

    HW_DECLARE_REGISTER(u8 , HW_KEYBOARD_STATE 	        );        
    HW_DECLARE_REGISTER(u8 , HW_KEYBOARD_DATA 	        ); 

    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_ENABLE_A	    );
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_ENABLE_B	    );
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_PENDING_A     );	 
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_PENDING_B     );	 
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_INSERVICE_A   );
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_INSERVICE_B   );
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_MASK_A        );
    HW_DECLARE_REGISTER(u8 , HW_MFP_INTERRUPT_MASK_B        );
    HW_DECLARE_REGISTER(u8 , HW_MFP_VECTOR_BASE			    );  
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_A_CONTROL         );
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_B_CONTROL		    ); 
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_CD_CONTROL		); 
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_A_DATA            );
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_B_DATA   		    );  
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_C_DATA            ); 
    HW_DECLARE_REGISTER(u8 , HW_MFP_TIMER_D_DATA   		    );  

    HW_DECLARE_REGISTER(u8 , HW_MEGASTE_CPUMODE             ); 

    HW_DECLARE_REGISTER(u16 , OS_FLOPVBL		            );
    HW_DECLARE_REGISTER(u16 , OS_BOOTDEV                    );
    HW_DECLARE_REGISTER(u16 , OS_NFLOPS		                );
    HW_DECLARE_REGISTER(u32 , OS_PHYTOP                     );
    HW_DECLARE_REGISTER(u32 , OS_COOKIEJAR                  );
};
typedef struct SThardware_ SThardware;

#	ifndef HARDWARE_C
	extern SThardware g_STHardware;
#	endif

#endif  /* ! __TOS__ */

/* Blitter */

#define HW_BLITTER_HTONE                HW_DEFINE_REGISTER(u16 , 0xFFFF8A00UL , HW_BLITTER_HTONE          ) 
#define HW_BLITTER_XINC_SOURCE          HW_DEFINE_REGISTER(u16 , 0xFFFF8A20UL , HW_BLITTER_XINC_SOURCE    )
#define HW_BLITTER_YINC_SOURCE          HW_DEFINE_REGISTER(u16 , 0xFFFF8A22UL , HW_BLITTER_YINC_SOURCE    )       
#define HW_BLITTER_ADDR_SOURCE          HW_DEFINE_REGISTER(u32 , 0xFFFF8A24UL , HW_BLITTER_ADDR_SOURCE    )       

#define HW_BLITTER_ENDMASK1             HW_DEFINE_REGISTER(u16 , 0xFFFF8A28UL , HW_BLITTER_ENDMASK1       )   
#define HW_BLITTER_ENDMASK2             HW_DEFINE_REGISTER(u16 , 0xFFFF8A2AUL , HW_BLITTER_ENDMASK2       )  
#define HW_BLITTER_ENDMASK3             HW_DEFINE_REGISTER(u16 , 0xFFFF8A2CUL , HW_BLITTER_ENDMASK3       )   

#define HW_BLITTER_XINC_DEST            HW_DEFINE_REGISTER(u16 , 0xFFFF8A2EUL , HW_BLITTER_XINC_DEST      )  
#define HW_BLITTER_YINC_DEST            HW_DEFINE_REGISTER(u16 , 0xFFFF8A30UL , HW_BLITTER_YINC_DEST      )   
#define HW_BLITTER_ADDR_DEST            HW_DEFINE_REGISTER(u32 , 0xFFFF8A32UL , HW_BLITTER_ADDR_DEST      )   

#define HW_BLITTER_XSIZE                HW_DEFINE_REGISTER(u16 , 0xFFFF8A36UL , HW_BLITTER_XSIZE          )   
#define HW_BLITTER_YSIZE                HW_DEFINE_REGISTER(u16 , 0xFFFF8A38UL , HW_BLITTER_YSIZE          )   

#define HW_BLITTER_HOP                  HW_DEFINE_REGISTER(u8 , 0xFFFF8A3AUL ,  HW_BLITTER_HOP            )
#define HW_BLITTER_OP                   HW_DEFINE_REGISTER(u8 , 0xFFFF8A3BUL ,  HW_BLITTER_OP             )   
#define HW_BLITTER_CTRL1                HW_DEFINE_REGISTER(u8 , 0xFFFF8A3CUL ,  HW_BLITTER_CTRL1          )   
#define HW_BLITTER_CTRL2                HW_DEFINE_REGISTER(u8 , 0xFFFF8A3DUL ,  HW_BLITTER_CTRL2          )   

/* 68k / mfp vectors */

#define HW_VECTOR_INIT_PC               HW_DEFINE_REGISTER(u32 , 0x4 ,        HW_VECTOR_INIT_PC         )   
#define HW_VECTOR_VBL		            HW_DEFINE_REGISTER(u32 , 0x70 ,       HW_VECTOR_VBL		        ) 
#define HW_VECTOR_HBL		            HW_DEFINE_REGISTER(u32 , 0x68 ,       HW_VECTOR_HBL		        ) 
#define HW_VECTOR_DMA		            HW_DEFINE_REGISTER(u32 , 0x11C ,      HW_VECTOR_DMA		        ) 
#define HW_VECTOR_TIMERB	            HW_DEFINE_REGISTER(u32 , 0x120,       HW_VECTOR_TIMERB          )

/* Shifter */

#define HW_COLOR_LUT		            HW_DEFINE_REGISTER(u16, 0xFFFF8240UL ,HW_COLOR_LUT		        )  

#define HW_VIDEO_MODE		            HW_DEFINE_REGISTER(u8 , 0xFFFF8260UL ,HW_VIDEO_MODE		        )  

#define HW_VIDEO_BASE_H		            HW_DEFINE_REGISTER(u8 , 0xFFFF8201UL ,HW_VIDEO_BASE_H		    )      
#define HW_VIDEO_BASE_M		            HW_DEFINE_REGISTER(u8 , 0xFFFF8203UL ,HW_VIDEO_BASE_M		    )      
#define HW_VIDEO_BASE_L		            HW_DEFINE_REGISTER(u8 , 0xFFFF820DUL ,HW_VIDEO_BASE_L		    ) /* STe */

#define HW_VIDEO_COUNT_H	            HW_DEFINE_REGISTER(u8 , 0xFFFF8205UL ,HW_VIDEO_COUNT_H	        ) /* STe */
#define HW_VIDEO_COUNT_M	            HW_DEFINE_REGISTER(u8 , 0xFFFF8207UL ,HW_VIDEO_COUNT_M	        ) /* STe */
#define HW_VIDEO_COUNT_L	            HW_DEFINE_REGISTER(u8 , 0xFFFF8209UL ,HW_VIDEO_COUNT_L	        ) /* STe */

#define HW_VIDEO_SYNC   	            HW_DEFINE_REGISTER(u8 , 0xFFFF820AUL ,HW_VIDEO_SYNC   	        )  

#define HW_VIDEO_OFFSET				    HW_DEFINE_REGISTER(u8 , 0xFFFF820FUL ,HW_VIDEO_OFFSET			) /* STe */
#define HW_VIDEO_PIXOFFSET			    HW_DEFINE_REGISTER(u8 , 0xFFFF8265UL ,HW_VIDEO_PIXOFFSET	    ) /* STe */
#define HW_VIDEO_PIXOFFSET_HIDDEN	    HW_DEFINE_REGISTER(u8 , 0xFFFF8264UL ,HW_VIDEO_PIXOFFSET_HIDDEN	) /* STe */

#define HW_YM_REGSELECT                 HW_DEFINE_REGISTER(u8 , 0xFFFF8800UL ,HW_YM_REGSELECT           )
#define HW_YM_REGDATA                   HW_DEFINE_REGISTER(u8 , 0xFFFF8802UL ,HW_YM_REGDATA             )    

/* DMA sound - STe only */

#define HW_DMASOUND_CONTROL			    HW_DEFINE_REGISTER(u8 , 0xFFFF8901UL ,HW_DMASOUND_CONTROL		)

#define HW_DMASOUND_STARTADR_H		    HW_DEFINE_REGISTER(u8 , 0xFFFF8903UL ,HW_DMASOUND_STARTADR_H	)
#define HW_DMASOUND_STARTADR_M		    HW_DEFINE_REGISTER(u8 , 0xFFFF8905UL ,HW_DMASOUND_STARTADR_M    )
#define HW_DMASOUND_STARTADR_L		    HW_DEFINE_REGISTER(u8 , 0xFFFF8907UL ,HW_DMASOUND_STARTADR_L	)

#define HW_DMASOUND_ENDADR_H		    HW_DEFINE_REGISTER(u8 , 0xFFFF890FUL ,HW_DMASOUND_ENDADR_H		) 
#define HW_DMASOUND_ENDADR_M		    HW_DEFINE_REGISTER(u8 , 0xFFFF8911UL ,HW_DMASOUND_ENDADR_M		)  
#define HW_DMASOUND_ENDADR_L		    HW_DEFINE_REGISTER(u8 , 0xFFFF8913UL ,HW_DMASOUND_ENDADR_L		)  

#define HW_DMASOUND_COUNTER_H		    HW_DEFINE_REGISTER(u8 , 0xFFFF8909UL ,HW_DMASOUND_COUNTER_H		)  	
#define HW_DMASOUND_COUNTER_M		    HW_DEFINE_REGISTER(u8 , 0xFFFF890BUL ,HW_DMASOUND_COUNTER_M		)  	
#define HW_DMASOUND_COUNTER_L		    HW_DEFINE_REGISTER(u8 , 0xFFFF890DUL ,HW_DMASOUND_COUNTER_L		)  	

#define HW_DMASOUND_MODE     		    HW_DEFINE_REGISTER(u16 , 0xFFFF8920UL ,HW_DMASOUND_MODE     	)

#define HW_MICROWIRE_DATA    		    HW_DEFINE_REGISTER(u16 , 0xFFFF8922UL ,HW_MICROWIRE_DATA    	) 
#define HW_MICROWIRE_MASK    		    HW_DEFINE_REGISTER(u16 , 0xFFFF8924UL ,HW_MICROWIRE_MASK    	) 

/* Keyboard ACIA */

#define HW_KEYBOARD_STATE 	            HW_DEFINE_REGISTER(u8 , 0xFFFFFC00UL ,HW_KEYBOARD_STATE 	    )        
#define HW_KEYBOARD_DATA 	            HW_DEFINE_REGISTER(u8 , 0xFFFFFC02UL ,HW_KEYBOARD_DATA 	        ) 

/* MFP */

#define HW_MFP_INTERRUPT_ENABLE_A		HW_DEFINE_REGISTER(u8 , 0xFFFFFA07UL ,HW_MFP_INTERRUPT_ENABLE_A	    )
#define HW_MFP_INTERRUPT_ENABLE_B		HW_DEFINE_REGISTER(u8 , 0xFFFFFA09UL ,HW_MFP_INTERRUPT_ENABLE_B	    )
#define HW_MFP_INTERRUPT_PENDING_A		HW_DEFINE_REGISTER(u8 , 0xFFFFFA0BUL ,HW_MFP_INTERRUPT_PENDING_A    )	 
#define HW_MFP_INTERRUPT_PENDING_B		HW_DEFINE_REGISTER(u8 , 0xFFFFFA0DUL ,HW_MFP_INTERRUPT_PENDING_B    )	 
#define HW_MFP_INTERRUPT_INSERVICE_A	HW_DEFINE_REGISTER(u8 , 0xFFFFFA0FUL ,HW_MFP_INTERRUPT_INSERVICE_A  )
#define HW_MFP_INTERRUPT_INSERVICE_B	HW_DEFINE_REGISTER(u8 , 0xFFFFFA11UL ,HW_MFP_INTERRUPT_INSERVICE_B  )
#define HW_MFP_INTERRUPT_MASK_A     	HW_DEFINE_REGISTER(u8 , 0xFFFFFA13UL ,HW_MFP_INTERRUPT_MASK_A       )
#define HW_MFP_INTERRUPT_MASK_B     	HW_DEFINE_REGISTER(u8 , 0xFFFFFA15UL ,HW_MFP_INTERRUPT_MASK_B       )
#define HW_MFP_VECTOR_BASE				HW_DEFINE_REGISTER(u8 , 0xFFFFFA17UL ,HW_MFP_VECTOR_BASE			)  
#define HW_MFP_TIMER_A_CONTROL      	HW_DEFINE_REGISTER(u8 , 0xFFFFFA19UL ,HW_MFP_TIMER_A_CONTROL        )
#define HW_MFP_TIMER_B_CONTROL		  	HW_DEFINE_REGISTER(u8 , 0xFFFFFA1BUL ,HW_MFP_TIMER_B_CONTROL		) 
#define HW_MFP_TIMER_CD_CONTROL		  	HW_DEFINE_REGISTER(u8 , 0xFFFFFA1DUL ,HW_MFP_TIMER_CD_CONTROL		) 
#define HW_MFP_TIMER_A_DATA         	HW_DEFINE_REGISTER(u8 , 0xFFFFFA1FUL ,HW_MFP_TIMER_A_DATA           )
#define HW_MFP_TIMER_B_DATA   		  	HW_DEFINE_REGISTER(u8 , 0xFFFFFA21UL ,HW_MFP_TIMER_B_DATA   		)  
#define HW_MFP_TIMER_C_DATA         	HW_DEFINE_REGISTER(u8 , 0xFFFFFA23UL ,HW_MFP_TIMER_C_DATA           ) 
#define HW_MFP_TIMER_D_DATA   		  	HW_DEFINE_REGISTER(u8 , 0xFFFFFA25UL ,HW_MFP_TIMER_D_DATA   		)  

#define HW_MEGASTE_CPUMODE              HW_DEFINE_REGISTER(u8 , 0xFFFF8E21UL ,HW_MEGASTE_CPUMODE            ) 

#define OS_FLOPVBL		                HW_DEFINE_REGISTER(u16 , 0x43e ,      OS_FLOPVBL		            )
#define OS_BOOTDEV  	                HW_DEFINE_REGISTER(u16 , 0x446 ,      OS_BOOTDEV	                )
#define OS_NFLOPS		                HW_DEFINE_REGISTER(u16 , 0x4a6 ,      OS_NFLOPS		                )
#define OS_PHYTOP                       HW_DEFINE_REGISTER(u32 , 0x42e ,      OS_PHYTOP                     )
#define OS_COOKIEJAR                    HW_DEFINE_REGISTER(void, 0x5A0 ,      OS_COOKIEJAR                  )

#endif   /* HARDWARE_H */

