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

#ifndef BLITZMENU_H
#define BLITZMENU_H

#include "DEMOSDK\FSM.H"
#include "DEMOSDK\RASTERS.H"
#include "DEMOSDK\CODEGEN.H"


void BlitZMenuInitStatic    (FSM* _fsm);

void BlitZMenuEnter	        (FSM* _fsm);
void BlitZMenuBacktask      (FSM* _fsm);
void BlitZMenuExiting       (FSM* _fsm);
void BlitZMenuExit		    (FSM* _fsm);

void BlitZMenuActivity	    (FSM* _fsm);


struct BlitZMenuPersistentData_
{
    u8  colorchoice;
    u8  currentmodule;
    u8  playmode;

    s8  selectx;
    
    u16 autorunindex;
    u16 autorunframes;

    u16  plasx;
    u16  plasx2;
    u16  plasy1;
    u16  plasy2;
    u16  plasyinc;
    
    bool plasmashiftmode;
};
typedef struct BlitZMenuPersistentData_ BlitZMenuPersistentData;


enum BlitZMenuASMopcodes_
{
    BMpOp_Begin,
    BMpOp_End,
    BMpOp_PreBlit1,
    BMpOp_PreBlit2,
    BMpOp_Blit,
    BMpOp_C0_back,
    BMpOp_C2_000,
    BMpOp_C1_555,
    BMpOp_C1_666,
    BMpOp_C1_FFF,
    BMpOp_C3_777,
    BMpOp_C3_R,
    BMpOp_Pitch160,
    BMpOp_PitchIcons,
    BMpOp_PitchTips,
    BMpOp_PixOffset0,
    BMpOp_PixOffsetIcons,
    BMpOp_PixOffsetTips,
    BMpOp_AdrEmpty,
    BMpOp_AdrScroller1,
    BMpOp_AdrScroller2,
    BMpOp_AdrScroller2Shift,
    BMpOp_AdrScrollerIcons,
    BMpOp_AdrScrollerTips,
    BMpOp_SIZE
};
typedef enum BlitZMenuASMopcodes_ BlitZMenuASMopcodes;


typedef void* (*FuncBlitzMenuCopyIcons)(void* _src, void* _dst, u16 _pitchdst);
typedef void  (*FuncBlitzMenuXor)      (void* _adr, u16 _nbwords, u16 _h, u16 _pitch);
typedef void  (*FuncBlitzMenuClear)    (void* _adr, u16 _nbwords, u16 _h, u16 _pitch);


struct BlitZMenuASMimport_
{
    u16     color1;
    u16     color2;
    u16     waitloop;
    u16     waitloop2;

    u8      pitchIcons;
    u8      pixelOffsetIcons;

    u8      pitchTips;
    u8      pixelOffsetTips;

    /* these adresses should be left shifted by 8 */
    u8*     empty;             
    u8*     scroller1;
    u8*     scroller2;
    u8*     scroller2shift;
    u8*     iconscroller;
    u8*     tipscroller;

    /*  */
    void*   rasters;
    void*   p1plasma;
    void*   p2plasma;
    void*   plasmacurve1;
    void*   plasmacurve2;
    
    void*   reserved1;
    void*   reserved2;

    FuncBlitzMenuCopyIcons  blitzMenuCopyIcons;
    FuncBlitzMenuXor        blitzMenuXor;    
    FuncBlitzMenuClear      blitzMenuClear;

    CGENdesc opcodes[BMpOp_SIZE];
};
typedef struct BlitZMenuASMimport_ BlitZMenuASMimport;



enum BlitZMenuState_
{
    BZMS_Hidden,
    BZMS_Drawing,
    BZMS_Drawn,
    BZMS_Undrawing
};
typedef enum BlitZMenuState_ BlitZMenuState;

enum BlitZMenuAction_
{
    BZMA_DONOTHING,
    BZMA_SELECT_MUSIK,
    BZMA_SELECT_PLAYMODE,
    BZMA_SELECT_COLORS, 
    BZMA_SELECT_INFO,
};
typedef enum BlitZMenuAction_ BlitZMenuAction;


struct BlitZMenuCell_
{
    bool    validated;
    u8      state;

    u8      nbwords;
    u8      h;
    u8      action;
    u8      param;
    u8      pitch_div2;
    u8      tipslen;
    
    char*   tips;
    u8*     adr;
};
typedef struct BlitZMenuCell_ BlitZMenuCell;

#define BLITZ_MENU_WIDTH  5
#define BLITZ_MENU_HEIGHT 9

struct BlitZMenuBarState_
{
    s16  bardisplayx;
    s16  bardisplayy;
    s8   selecty;

    s8   backtaskGridx;

    s16  tipsdisplayx;
    s16  tipstargetx;

    u16  tipid;
    u16  tipiddisplay;
    u16  waitcounter;

    BlitZMenuCell grid[BLITZ_MENU_WIDTH][BLITZ_MENU_HEIGHT];
};
typedef struct BlitZMenuBarState_ BlitZMenuBarState;

struct BlitZMenu_
{
    u16* colors;
    u8*  plasma;
    u16* plasmasin;
    u16* plasmasindelta;

    u8*  tempunpacked;

    u8*  charshadows;
    u8*  tipstext;

    u16* flashcolors;
    u8*  automaton;
    u16  automatonsize;

    u8   updatebasstreble;
    u8   displayedcolor;
    bool interactive;
    
    BlitZMenuBarState  menubarstate;

    char* textbase;
    u8*   empty;
    u8*   iconscrollerbuffer;
    u16   iconscrollerbuffersize;
    u8*   tipscrollerbuffer;
    u16   tipscrollerbuffersize;
    u8*   basstreblebuffer;
    u8*   gridscrollerbuffer;
    u32   gridscrollerbuffersize;

    u8*   gridscrollers[BLITZ_MENU_WIDTH];
    u8    gridheight[BLITZ_MENU_WIDTH];

    void* code;
    void* plasmacode;

    RASinterupt rasterBootFunc;
    RASopVbl1	rasterBootOp;

    BlitZMenuASMimport* asmimport;
};
typedef struct BlitZMenu_ BlitZMenu;

#endif
