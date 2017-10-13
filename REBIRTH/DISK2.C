#define DISK2_C

#include "DISK2.H"

LOADdisk RSC_DISK2 = 
{ 
    NULL, RSC_DISK2_NBENTRIES,    /* FAT */
    NULL, RSC_DISK2_NBMETADATA,   /* Meta data */
    1,                            /* Prefered drive */
    NULL, 520UL * 1024UL,         /* Media preload */
    NULL,                         /* File preload table */
    "DISK2.ST"                    /* Image filename */
};

#undef DISK2_C
