#ifndef ARJDEP_H
#define ARJDEP_H

extern volatile u8* ARJprogress;

void ARJdepack(void* depack_space, void* packed_data);

#endif
