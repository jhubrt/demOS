/*
** minimal demo showcasing the embedding of the GUP depacker sources
*/

#include <stdint.h>

/* all GUP depacker sources, bundled */
#include "arj_crc.c"
#include "arj_m4.c"
#include "ni_n0.c"
#include "ni_n1.c"
#include "ni_n2.c"
#include "unstore.c"
#include "arj_m7.c"

/* and main() plus a few utility functions that constitute the depacker demo app itself */
#include "depack_t.c"

