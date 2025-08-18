/*-
  GUP N1 decode routines memory to memory (an augmented M4 style depacker)
  (c) 2022 Mr Ni! (the Great of the TOS-crew) a.k.a. Hans Wessels
*/

/*
  call:
  void decode_n1(uint8_t *dst, uint8_t *data)
  dst        = destination to depack to
  data       = packed data
  
  Note: original data size is included in the packed input data (first 4 bytes, Big Endian encoded)
*/

#include <stdint.h>
#include <limits.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint16_t uint16;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */
typedef uint32_t uint32;
typedef int32_t int32;


#define BITBUFSIZE (int)(sizeof(unsigned long int)*CHAR_BIT)  /* number of bits in bitbuffer */

#define TRASHBITS(x)    /* trash  bits from bitbuffer */    \
{                                                           \
  int xbits=(x);                                            \
  bib-=xbits;                                               \
  if(bib<0)                                                 \
  { /* refill bitbuffer */                                  \
    int i;                                                  \
    unsigned long int newbuf=0; /* BITBUFSIZE bits big */   \
    bitbuf<<=(xbits+bib);        /* trash bits */           \
    xbits=-bib;                                             \
    i=(int)sizeof(bitbuf)-2;                                \
    while(--i>=0)                                           \
    {                                                       \
      newbuf<<=8;                                           \
      newbuf+=*data++;                                      \
      bib+=8;                                               \
    }                                                       \
    bitbuf+=newbuf;                                         \
  }                                                         \
  bitbuf<<=xbits;                                           \
}

void decode_n1(uint8_t *dst, uint8_t *data)
{
	/* aanname origsize>0 */
	int bib; /* bits in bitbuf */
	unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits groot */
	unsigned long origsize=0;
	uint8* buffend;
	bitbuf=0;
	bib=0;
	{ /* init bitbuf */
		int i=(int)sizeof(bitbuf);
		while(--i>=0)
		{
			bitbuf<<=8;
			bitbuf+=*data++;
			bib+=8;
		}
		bib-=16;
	}
   { /* get origsize */
		origsize=bitbuf>>(BITBUFSIZE-16);
		origsize<<=16;
		TRASHBITS(16);
		origsize+=bitbuf>>(BITBUFSIZE-16);
		TRASHBITS(16);
   }
	buffend=dst+origsize;
	for(;;)
	{ /* decode loop */
		unsigned long mask=1UL<<(BITBUFSIZE-1);
		if((bitbuf&mask)==0)
		{ /* literal */
			uint32 kar;
			kar=(uint8)(bitbuf>>(BITBUFSIZE-9));
			*dst++=kar;
			TRASHBITS(9);
		}
		else
		{ /* pointer length combinatie */
			int i;
			int tb=15;
			uint32 ptr;
			uint32 kar;
			i=1;
			do
			{
				mask>>=1;
				if((bitbuf&mask)==0)
				{
					tb=i+1;
					break;
				}
				i++;
			} while(i<15);
			TRASHBITS(tb);
			kar=(1<<i)+(bitbuf>>(BITBUFSIZE-i));
			TRASHBITS(i);
			tb=7;
			i=0;
			mask=1UL<<(BITBUFSIZE-1);
			do
			{
				if((bitbuf&mask)==0)
				{
					tb=i+1;
					break;
				}
				mask>>=1;
				i++;
			} while(i<7);
			TRASHBITS(tb);
			ptr=(((1<<i)-1)<<9)+(uint32)(bitbuf>>(BITBUFSIZE-(i+9)));
			TRASHBITS(i+9);
			{
				uint8* q=dst-ptr-1;
				do
				{
					*dst++=*q++;
				} 
				while(--kar>0);
			}
		}
		if(dst>=buffend)
		{
			return;
		}
	}
}
