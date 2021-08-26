#ifndef SYSFNT_H
#define SYSFNT_H

#define SYS_FNT_UNDEFINED   (2*81)
#define SYS_FNT_OFFSETSHIFT 2

#define SYS_FNT_SPACECHAR   (2*80)

struct SYSFont_
{
    u8      charsmap[256];
    u8*     data;
    u16     size;
};
typedef struct SYSFont_ SYSFont;

#ifndef DATA_C
extern SYSFont SYSfont;
#endif

#endif
