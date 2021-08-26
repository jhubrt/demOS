#include "DEMOSDK\BASTYPES.H"

#define REBIRTH1_C

#include "REBIRTH1.H"

LOADdisk RSC_REBIRTH1 = 
{
    NULL, RSC_REBIRTH1_NBENTRIES, /* FAT */
    NULL, RSC_REBIRTH1_NBMETADATA,/* Meta data */
    0,                            /* Prefered drive */
    NULL, 760UL * 1024UL,         /* Media preload */
    NULL,                         /* File preload table */
    "REBIRTH1.ST"                 /* Image filename */
};

#undef REBIRTH1_C
