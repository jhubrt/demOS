/*
** ni packer mode n2 depacker
** 2022 Hans Wessels
*/

#include <stdint.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint32_t uint32;
typedef int32_t int32;

#define MATCH_2_CUTTOFF 1024

#define N2_GET_BIT(bit)								\
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

#define N2_GET_LEN(len)								\
{ /* get length from data stream */				\
	int bit;												\
	N2_GET_BIT(bit);									\
	len=bit;												\
	N2_GET_BIT(bit);									\
	len+=len+bit;										\
	if(len<3)											\
	{ /* short len */									\
		len+=2;											\
	}														\
	else													\
	{ /* long len */									\
		len=1;											\
		do													\
		{													\
			N2_GET_BIT(bit);							\
			len+=len+bit;								\
			N2_GET_BIT(bit);							\
		} while(bit==0);								\
		len+=3;											\
	}														\
	if(ptr>=MATCH_2_CUTTOFF)						\
	{														\
		len++;											\
	}														\
}
			


#define N2_GET_PTR(ptr)								\
{ /* get pointer from data stream */			\
	int32 tmp=-2;										\
	int bit;												\
	do														\
	{														\
		N2_GET_BIT(bit);								\
		tmp+=tmp+bit;									\
		N2_GET_BIT(bit);								\
	} while(bit==0);									\
	if(tmp<=-65537)									\
	{ /* eof token */									\
		return;											\
	}														\
	tmp+=3;												\
	if(tmp==0)											\
	{														\
		ptr=last_ptr1;									\
	}														\
	else													\
	{														\
		tmp<<=8;											\
		tmp|=*data++;									\
		ptr=~tmp;										\
	}														\
}

void decoden2(uint8_t *dst, uint8_t *data)
{
	uint8 bitbuf=0;
	uint32 last_ptr0=1;
	uint32 last_ptr1=0;
	int bits_in_bitbuf=0;
	for(;;)
	{
		int bit;
		do
		{ /* literal */
			*dst++=*data++;
			N2_GET_BIT(bit);
		} while(bit==0);
		do
		{ /* ptr len */
			uint32 len;
			uint32 ptr;
			N2_GET_PTR(ptr);
			N2_GET_LEN(len);
			{ /* copy */
				uint8* q=dst-ptr-1;
				do
				{
					*dst++=*q++;
				} 
				while(--len>0);
			}
			last_ptr1=last_ptr0;
			last_ptr0=ptr;
			N2_GET_BIT(bit);
		} while(bit==1);
		{
			uint32 ptr=last_ptr1;
			last_ptr1=last_ptr0;
			last_ptr0=ptr;
		}

	}
}
