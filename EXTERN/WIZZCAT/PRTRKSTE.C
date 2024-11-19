#include "DEMOSDK\BASTYPES.H"
#include "PRTRKSTE.H"

#ifndef __TOS__
s8* WIZbackbuf;
s8* WIZfrontbuf;

void WIZinit    (WIZplayerInfo* playerInfo_) {}
void WIZmodInit (void* mod_, void* modeend_) {}
void WIZplay    (void) {}
void WIZstop    (void) {}
void WIZrundma  (void) {}
void WIZstereo  (void) {}
void WIZgetInfo(WIZinfo* info)
{
    info->songpos = 64;
    info->pattpos = 100;
}

void WIZjump(u8 _pos) {}

#endif
