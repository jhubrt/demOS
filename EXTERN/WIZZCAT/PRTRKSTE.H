#ifndef PRTRKSTE_H
#define PRTRKSTE_H

extern s8* WIZbackbuf;
extern s8* WIZfrontbuf;

struct WIZplayerInfo_
{
    u16* m_audioPeriods;
    u16  m_audioVarSize;
    u16* m_wizVars;
    u16  m_wizVarSize;
};
typedef struct WIZplayerInfo_ WIZplayerInfo;

struct WIZinfo_
{
    u8  songpos;
    u8  pattpos;
    u16 dmacom;
};
typedef struct WIZinfo_ WIZinfo;

#define WIZ_MODULE_MARGIN_SIZE 65000UL

void WIZinit    (WIZplayerInfo* playerInfo_);
void WIZmodInit (void* mod_, void* modeend_);
void WIZplay    (void);
void WIZrundma  (void);
void WIZstereo  (void);
void WIZstop    (void);
void WIZgetInfo (WIZinfo* info_);
void WIZjump    (u8 pos_);

#endif
