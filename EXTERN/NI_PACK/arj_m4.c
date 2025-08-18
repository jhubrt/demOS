/*-
  ARJ decode routines memory to memory
  (c) 1995-1997 Mr Ni! (the Great of the TOS-crew)
*/

/*
** arj m4 decoder
**
*/

/*
  call:
  void decode_m4(unsigned long size, uint8_t *dst, uint8_t *data)
  size       = original size
  dst        = destination to depack to
  data       = packed data
*/

#include <stdint.h>
#include <limits.h>

typedef uint8_t uint8;         /* unsigned 8 bit */
typedef uint16_t uint16;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */
typedef uint32_t uint32;

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

void decode_m4(unsigned long size, uint8_t *dst, uint8_t *data)
{
  /* aanname origsize>0 */

  int bib; /* bits in bitbuf */
  unsigned long bitbuf; /* shift buffer, BITBUFSIZE bits groot */
  uint8* buff=dst;
  uint8* buffend;

  buffend=dst+size;
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

	for(;;)
	{ /* decode loop */
		unsigned long mask=1UL<<(BITBUFSIZE-1);
		if((bitbuf&mask)==0)
		{ /* literal */
			uint32 kar;
			kar=(uint8)(bitbuf>>(BITBUFSIZE-9));
			*buff++=kar;
			TRASHBITS(9);
		}
		else
		{ /* pointer length combinatie */
			int i;
			int tb=7;
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
			} while(i<7);
			TRASHBITS(tb);
			kar=(1<<i)+(bitbuf>>(BITBUFSIZE-i))+1;
			TRASHBITS(i);
			tb=4;
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
			} while(i<4);
			TRASHBITS(tb);
			ptr=(((1<<i)-1)<<9)+(uint32)(bitbuf>>(BITBUFSIZE-(i+9)));
			TRASHBITS(i+9);
			{
				uint8* q=buff-ptr-1;
				do
				{
					*buff++=*q++;
				} 
				while(--kar>0);
			}
		}
		if(buff>=buffend)
		{
			return;
		}
	}
}
