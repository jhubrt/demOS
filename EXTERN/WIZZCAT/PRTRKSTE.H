#ifndef PRTRKSTE_H
#define PRTRKSTE_H

extern s8* WIZbackbuf;
extern s8* WIZfrontbuf;

struct WIZinfo_
{
    u8  songpos;
    u8  pattpos;
};
typedef struct WIZinfo_ WIZinfo;

#define WIZ_MODULE_MARGIN_SIZE 65000UL

void WIZinit    (void);
void WIZmodInit (void* mod_, void* modeend_);
void WIZplay    (void);
void WIZrundma  (void);
void WIZstereo  (void);
void WIZgetInfo (WIZinfo* info_);
void WIZjump    (u8 pos_);

#endif
