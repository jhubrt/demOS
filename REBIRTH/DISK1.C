#define DISK1_C

#include "DISK1.H"

LOADdisk RSC_DISK1 = 
{
    NULL, RSC_DISK1_NBENTRIES,    /* FAT */
    NULL, RSC_DISK1_NBMETADATA,   /* Meta data */
    0,                            /* Prefered drive */
    NULL, 760UL * 1024UL,         /* Media preload */
    NULL,                         /* File preload table */
     "DISK1.ST"                   /* Image filename */
};

#undef DISK1_C