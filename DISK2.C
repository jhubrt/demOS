#define DISK2_C

#include "DISK2.H"

static LOADresource RSC_DISK2_FAT[] =
{ /* nbSectors @sector   @side     @track   metaDataIndex */
    {        1,        0,        0,        0,      0, NULL } ,  /* lost bytes = 0 */
    {      174,        1,        0,        0,      1, NULL } ,  /* lost bytes = 80 */
    {      174,        5,        1,        8,      2, NULL } ,  /* lost bytes = 80 */
    {       87,        9,        0,       17,      3, NULL } ,  /* lost bytes = 40 */
    {       87,        6,        1,       21,      4, NULL } ,  /* lost bytes = 40 */
    {      109,        3,        0,       26,      5, NULL } ,  /* lost bytes = 88 */
    {      109,        2,        1,       31,      6, NULL } ,  /* lost bytes = 88 */
    {      109,        1,        0,       37,      7, NULL } ,  /* lost bytes = 88 */
    {      109,        0,        1,       42,      8, NULL } ,  /* lost bytes = 88 */
    {       16,        9,        1,       47,      9, NULL } ,  /* lost bytes = 224 */
    {        4,        5,        1,       48,     17, NULL }    /* lost bytes = 348 */
};

static LOADmetaData RSC_DISK2_MetaData[] =
{ /*   offset           size    originalsize */
    {        0UL,      512UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    44504UL,       -1L } ,
    {        0UL,    44504UL,       -1L } ,
    {        0UL,    55720UL,       -1L } ,
    {        0UL,    55720UL,       -1L } ,
    {        0UL,    55720UL,       -1L } ,
    {        0UL,    55720UL,       -1L } ,
    {        0UL,     1904UL,       -1L } ,
    {     1904UL,     2438UL,       -1L } ,
    {     4342UL,     2822UL,       -1L } ,
    {     7164UL,      740UL,       -1L } ,
    {     7904UL,       16UL,       -1L } ,
    {     7920UL,       16UL,       -1L } ,
    {     7936UL,       16UL,       -1L } ,
    {     7952UL,       16UL,       -1L } ,
    {        0UL,     1266UL,     5264L } ,
    {     1266UL,      330UL,     1368L } ,
    {     1596UL,      104UL,       -1L }  
};

LOADdisk RSC_DISK2 = { RSC_DISK2_FAT, RSC_DISK2_NBENTRIES, RSC_DISK2_MetaData, RSC_DISK2_NBMETADATA, 1
#	ifdef DEMOS_DEBUG
     ,"DISK2.ST", NULL
#   endif
};

/* 338432 bytes left on floppy */
