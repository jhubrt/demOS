#ifndef DEMOS_INVERT_DRIVE
#   define DEMOS_INVERT_DRIVE 1
#endif

#define DEMOS_LOAD_FROMHD

#include "BLSPLAY\SRC\DEMOS.C"

#include "DEMOSDK\ALLOC.C"
#include "DEMOSDK\BITMAP.C"
#include "DEMOSDK\COLORS.C"
#include "DEMOSDK\FSM.C"
#include "DEMOSDK\STANDARD.C"
#include "DEMOSDK\BLSZIO.C"
#include "DEMOSDK\BLSZPLAY.C"
#include "DEMOSDK\BLSTEST.C"

#if !blsUSEASM
#include "DEMOSDK\BLSPLAY.C"
#include "DEMOSDK\BLSIO.C"
#include "DEMOSDK\SYNTHYM.C"
#include "DEMOSDK\SYNTHYMD.C"
#endif

#include "DEMOSDK\YMDISP.C"
#include "DEMOSDK\SYSTEM.C"
#include "DEMOSDK\TRACE.C"
#include "DEMOSDK\DATA\DATA.C"

#include "BLSPLAY\SRC\SCREENS.C"
#include "BLSPLAY\SRC\BPLAYER.C"
