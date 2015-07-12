#define DISK1_C

#include "DISK1.H"

static LOADresource RSC_DISK1_FAT[] =
{ /* nbSectors @sector   @side     @track   metaDataIndex */
    {        1,        0,        0,        0,      0 } ,  /* lost bytes = 6 */
    {       14,        1,        0,        0,      1 } ,  /* lost bytes = 275 */
    {       49,        5,        1,        0,      2 } ,  /* lost bytes = 118 */
    {        2,        4,        0,        3,      3 } ,  /* lost bytes = 376 */
    {        2,        6,        0,        3,      4 } ,  /* lost bytes = 356 */
    {        2,        8,        0,        3,     23 } ,  /* lost bytes = 0 */
    {        3,        0,        1,        3,     24 } ,  /* lost bytes = 214 */
    {        1,        3,        1,        3,     25 } ,  /* lost bytes = 344 */
    {        4,        4,        1,        3,     31 } ,  /* lost bytes = 399 */
    {       87,        8,        1,        3,     32 } ,  /* lost bytes = 40 */
    {       87,        5,        0,        8,     33 } ,  /* lost bytes = 40 */
    {      174,        2,        1,       12,     34 } ,  /* lost bytes = 80 */
    {      174,        6,        0,       21,     35 } ,  /* lost bytes = 80 */
    {      174,        0,        0,       30,     36 } ,  /* lost bytes = 80 */
    {      174,        4,        1,       38,     37 } ,  /* lost bytes = 80 */
    {      174,        8,        0,       47,     38 } ,  /* lost bytes = 80 */
    {      174,        2,        0,       56,     39 } ,  /* lost bytes = 80 */
    {      174,        6,        1,       64,     40 }    /* lost bytes = 80 */
};

static LOADmetaData RSC_DISK1_MetaData[] =
{ /*   offset           size    originalsize */
    {        0UL,      506UL,       -1L } ,
    {        0UL,     6893UL,    32000L } ,
    {        0UL,    24970UL,    49934L } ,
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

/* 87040 bytes left on floppy */
