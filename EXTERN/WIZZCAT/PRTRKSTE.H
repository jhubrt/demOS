#ifndef PRTRKSTE_H
#define PRTRKSTE_H

extern s8* WIZbackbuf;
extern s8* WIZfrontbuf;

#define WIZ_MODULE_MARGIN_SIZE 65000UL

void WIZinit    (void);
void WIZmodInit (void* mod_, void* modeend_);
void WIZplay    (void);
void WIZstereo  (void);

#endif
