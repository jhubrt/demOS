#include "DEMOSDK\BASTYPES.H"

#define BLITZIK1_C

#include "BLITZWAV.H"

LOADdisk RSC_BLITZWAV = 
{
    NULL, RSC_BLITZWAV_NBENTRIES,          /* FAT */
    NULL, RSC_BLITZWAV_NBMETADATA,         /* Meta data */
    0,                                     /* Prefered drive */
    NULL, 0,                               /* Media preload */
    NULL,                                  /* File preload table */
    "BLITZWAV.ST"                          /* Image filename */
};

#undef BLITZIK1_C
