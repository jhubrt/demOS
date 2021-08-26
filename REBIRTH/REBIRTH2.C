#include "DEMOSDK\BASTYPES.H"

#define REBIRTH2_C

#include "REBIRTH2.H"

LOADdisk RSC_REBIRTH2 = 
{ 
    NULL, RSC_REBIRTH2_NBENTRIES, /* FAT */
    NULL, RSC_REBIRTH2_NBMETADATA,/* Meta data */
    1,                            /* Prefered drive */
    NULL, 520UL * 1024UL,         /* Media preload */
    NULL,                         /* File preload table */
    "REBIRTH2.ST"                 /* Image filename */
};

#undef REBIRTH2_C
