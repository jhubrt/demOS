/*
** unstore routine
*/

#include <stdint.h>

void unstore(unsigned long size, uint8_t *dst, uint8_t *data)
{
	while(size--)
	{
		*dst++=*data++;
	}
	return;
}
