#define DISK1_C

#include "DISK1.H"

static LOADresource RSC_DISK1_FAT[] =
{ /* nbSectors @sector   @side     @track   metaDataIndex */
    {        1,        0,        0,        0,      0, NULL } ,  /* lost bytes = 0 */
    {       14,        1,        0,        0,      1, NULL } ,  /* lost bytes = 275 */
    {       51,        5,        1,        0,      2, NULL } ,  /* lost bytes = 347 */
    {        2,        6,        0,        3,      3, NULL } ,  /* lost bytes = 376 */
    {        2,        8,        0,        3,      4, NULL } ,  /* lost bytes = 356 */
    {        2,        0,        1,        3,     23, NULL } ,  /* lost bytes = 0 */
    {        3,        2,        1,        3,     24, NULL } ,  /* lost bytes = 214 */
    {        1,        5,        1,        3,     25, NULL } ,  /* lost bytes = 344 */
    {        4,        6,        1,        3,     31, NULL } ,  /* lost bytes = 399 */
    {       87,        0,        0,        4,     32, NULL } ,  /* lost bytes = 40 */
    {       87,        7,        0,        8,     33, NULL } ,  /* lost bytes = 40 */
    {      174,        4,        1,       12,     34, NULL } ,  /* lost bytes = 80 */
    {      174,        8,        0,       21,     35, NULL } ,  /* lost bytes = 80 */
    {      174,        2,        0,       30,     36, NULL } ,  /* lost bytes = 80 */
    {      174,        6,        1,       38,     37, NULL } ,  /* lost bytes = 80 */
    {      174,        0,        1,       47,     38, NULL } ,  /* lost bytes = 80 */
    {      174,        4,        0,       56,     39, NULL } ,  /* lost bytes = 80 */
    {      174,        8,        1,       64,     40, NULL }    /* lost bytes = 80 */
};

static LOADmetaData RSC_DISK1_MetaData[] =
{ /*   offset           size    originalsize */
    {        0UL,      512UL,       -1L } ,
    {        0UL,     6893UL,    32000L } ,
    {        0UL,    25765UL,    51560L } ,
    {        0UL,      648UL,       -1L } ,
    {        0UL,       46UL,       -1L } ,
    {       46UL,       50UL,       -1L } ,
    {       96UL,       22UL,       -1L } ,
    {      118UL,       38UL,       -1L } ,
    {      156UL,       46UL,       -1L } ,
    {      202UL,       50UL,       -1L } ,
    {      252UL,       22UL,       -1L } ,
    {      274UL,       54UL,       -1L } ,
    {      328UL,       26UL,       -1L } ,
    {      354UL,       54UL,       -1L } ,
    {      408UL,       42UL,       -1L } ,
    {      450UL,       14UL,       -1L } ,
    {      464UL,       18UL,       -1L } ,
    {      482UL,       30UL,       -1L } ,
    {      512UL,       46UL,       -1L } ,
    {      558UL,       14UL,       -1L } ,
    {      572UL,       32UL,       -1L } ,
    {      604UL,       32UL,       -1L } ,
    {      636UL,       32UL,       -1L } ,
    {        0UL,     1024UL,       -1L } ,
    {        0UL,     1322UL,     7680L } ,
    {        0UL,       32UL,       -1L } ,
    {       32UL,       32UL,       -1L } ,
    {       64UL,       32UL,       -1L } ,
    {       96UL,       32UL,       -1L } ,
    {      128UL,       32UL,       -1L } ,
    {      160UL,        8UL,       -1L } ,
    {        0UL,     1649UL,    32000L } ,
    {        0UL,    44504UL,       -1L } ,
    {        0UL,    44504UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L } ,
    {        0UL,    89008UL,       -1L }  
};

LOADdisk RSC_DISK1 = { RSC_DISK1_FAT, RSC_DISK1_NBENTRIES, RSC_DISK1_MetaData, RSC_DISK1_NBMETADATA, 0
#	ifdef DEMOS_DEBUG
     ,"DISK1.ST", NULL
#   endif
};

/* 86016 bytes left on floppy */
