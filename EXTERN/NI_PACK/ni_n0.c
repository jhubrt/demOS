/*
** ni packer mode n0 depacker
** 2022 Hans Wessels
*/

#include <stdint.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint16_t uint16;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */
typedef uint32_t uint32;
typedef int32_t int32;


#define GET_N0_BIT(bit)								\
{ /* get a bit from the data stream */			\
 	if(bits_in_bitbuf==0)							\
 	{ /* fill bitbuf */								\
  		bitbuf=*data++;								\
  		bits_in_bitbuf=8;								\
	}														\
	bit=(bitbuf&0x80)>>7;							\
	bitbuf+=bitbuf;									\
	bits_in_bitbuf--;									\
}

#define DECODE_N0_LEN(val)							\
{ /* get value 2 - 2^32-1 */						\
	int bit;												\
	val=1;												\
	do														\
	{														\
		GET_N0_BIT(bit);								\
		val+=val+bit;									\
		GET_N0_BIT(bit);								\
	} while(bit==0);									\
}


void decode_n0(uint8_t *dst, uint8_t *data)
{
	uint8 bitbuf=0;
	int bits_in_bitbuf=0;
	*dst++=*data++;
	for(;;)
	{
		int bit;
		GET_N0_BIT(bit);
		if(bit==0)
		{ /* literal */
			*dst++=*data++;
		}
		else
		{ /* ptr len */
			int32 ptr;
			uint8* src;
			uint8 c;
			int len;
			ptr=-1;
			ptr<<=8;
			c=*data++;
			GET_N0_BIT(bit);
			if(bit==0)
			{
				ptr|=c;
			}
			else
			{ /* 16 bit pointer */
				if(c==0)
				{
					return; /* end of stream */
				}
				ptr|=~c;
				ptr<<=8;
				ptr|=*data++;
			}
			DECODE_N0_LEN(len);
			len++;
			src=dst+ptr;
			do
			{
  				*dst++=*src++;
			} while(--len!=0);
		}
	}
}

