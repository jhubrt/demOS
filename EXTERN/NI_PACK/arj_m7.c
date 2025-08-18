/*-
  ARJ decode routines memory to memory
  (c) 1995-1997 Mr Ni! (the Great of the TOS-crew)
*/

/*
  call:
  void decode_m7(uint8_t *dst, uint8_t *data)
  dst        = destination to depack to
  data       = packed data
*/

#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t byte;         /* unsigned 8 bit */
typedef uint16_t word;    /* unsigned 16 bit */
typedef int16_t kartype;   /* signed 16 bit */

#undef DEBUG_MODE      /* debug mode on, undef for no debuging */

#ifdef DEBUG_MODE
  #include <stdio.h>
#endif

#define BITBUFSIZE (int)(sizeof(unsigned long int)*CHAR_BIT)  /* number of bits in bitbuffer */
#define CHARS 256       /* number of characters */
#define MAXMATCH 258    /* maximum lenth of a match */
#define DEPACK_MALLOC_SIZE ((sizeof(kartype)*65536UL)+CHARS+MAXMATCH+1UL+65536UL+19UL) /* size of memory needed for the depacker to work */

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

void decode_m7(uint8_t *dst, uint8_t *data)
{
  int karshlvl;             /* size of character shift */
  int ptrshlvl;             /* size of pointer shift */
  int bib;                  /* bits in bitbuf */
  unsigned long int bitbuf; /* shift buffer, BITBUFSIZE bits big */
  word huffcount=0;         /* size of huffmanblock */
  kartype *huff2kar;
  byte *karlen;
  byte *huff2ptr;
  byte *ptrlen;
  byte *base;
  byte *base_base; /* so I can free it at the end */
  base=(byte *)malloc(DEPACK_MALLOC_SIZE);
  if(base==NULL)
  { /* malloc error */
  	 return;
  }
  base_base=base;
  huff2kar=(void*)base;
  base+=sizeof(kartype)*65536UL;
  karlen=base+CHARS;
  base+=CHARS+MAXMATCH+1;
  huff2ptr=base;
  base+=65536UL;
  ptrlen=base;
  bitbuf=0;
  bib=0;
  karshlvl=0;
  ptrshlvl=0;
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
    while(huffcount--) /* start of the main depack loop, from here it has to be optimized like hell */
    {
      kartype kar;
      if((kar=huff2kar[bitbuf>>karshlvl])>0) /* most of the time this if won't be true, but it is very depending on the type of packed data. Data with very much repeating sequences may make this if true */
      { /* pointer length combination */
        word ptr;
        
        TRASHBITS(karlen[kar]);               /* make this code inline, lots of nice register optimisations should be possible, maximum size of the trash bits is 16 */
        ptr=huff2ptr[bitbuf>>ptrshlvl];
        TRASHBITS(ptrlen[ptr]);               /* same true here, but usually pointerlen won't be bigger then 8, but maximum is 16 */
        if(ptr>1)
        {
          int tmp;
          ptr--;
          tmp=ptr;
          ptr=(1<<ptr) + (word)(bitbuf>>(BITBUFSIZE-ptr));
          TRASHBITS(tmp);                     /* maximum value for this trash bits is 15 */
        }
        {
          byte* q=dst-ptr-1;
          do
          {
              *dst++=*q++;
          } while(--kar>0);
        }
      }
      else
      {
        *dst++=(byte)kar;      /* use lower eight bits of negative number as literal character */
        TRASHBITS(karlen[kar]); /* maximum size of trashbits will be 16, small values (6-9) are more common */
      }
    }
    /* end of heavily used main loop */
    { /* read new huffman codes */
      /* this loop will be use less often, the loop above will typicaly be exectued 4000+ 
         times before this code is executed again. This loop creates new huffman tables.
         it will be used at least one time to read the initial huffman table
       */
      huffcount=bitbuf>>(BITBUFSIZE-16);
      if(huffcount==0)
      { /* stream end code */
        free(base_base);
        return; /* exit succes */
      }
      TRASHBITS(16);
      { /* read huffman codes for the character lengths */
        int count=(word)(bitbuf>>(BITBUFSIZE-5));
        TRASHBITS(5);
        if(count)
        { /* read count for the huffman lengths */
          byte *p=ptrlen;
          int i=3;
          while(--i>=0)
          { /* get length 0, 2-18 times 0 and 20+ times 0 */
            int tmp=(word)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* optimize: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          /* it's possible to skip lenth 1, 2 and 3 */
          i=(int)(-((word)(bitbuf>>(BITBUFSIZE-2))));
          *p=0;
          p[1]=0;
          p[2]=0;
          p-=i; /* skip i positions over */
          i+=count-3; /* 3 because of the loop */
          TRASHBITS(2);
          while(--i>=0)
          {
            int tmp=(word)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* optimalize: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          { /* cahr lengths have been read make huff2ptr table */
            unsigned long clenct[17]; /* there are 17 different charlenths at maximum 0 to including 16 */
            int maxlen;
            unsigned long *q=clenct;
            i=17;
            while(--i>=0)
            {
              *q++=0;
            }
            i=count;
            p=ptrlen;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* zoek maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            ptrshlvl=BITBUFSIZE-maxlen;
            /* calculate start adresses of lengths */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* skip 0 */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
#ifdef DEBUG_MODE
                printf("First huffman table is bad!\n");
#endif
                free(base_base);
                return;
              }
            }
            p=ptrlen;
            for(i=0;i<count;i++)
            { /* create huff2ptr */
              unsigned long j;
              int tmp;
              if((tmp=*p++)!=0)
              {
                j=1<<(maxlen-tmp);
                memset(huff2ptr+clenct[tmp], i, j);
                clenct[tmp]+=j;
              }
            }
          }
        }
        else
        { /* there is only one charlenth (very rare) */
          int ptr=(word)(bitbuf>>(BITBUFSIZE-5));
          TRASHBITS(5);
          ptrshlvl=BITBUFSIZE-1; /* those dammed intel processors can't shift a 32 bit register over 32 bits... */
          *huff2ptr=ptr;  /* so when charlenth is zero whe need two characters in the lookup table */
          huff2ptr[1]=ptr;
          ptrlen[ptr]=0;
        }
      }
      {
        /*
          read characters
        */
        int count=(word)(bitbuf>>(BITBUFSIZE-9));
        TRASHBITS(9);
        if(count)
        {
          int i=count;
          byte* p=karlen-CHARS;
          while(--i>=0)
          {
            unsigned int tmp=huff2ptr[bitbuf>>ptrshlvl];
            TRASHBITS(ptrlen[tmp])
            if(tmp>2) /* echte len */
            {
              *p++=(byte)(tmp-2);
            }
            else
            {
              if(tmp)
              {
                if(tmp==1)
                {
                  tmp=3+(word)(bitbuf>>(BITBUFSIZE-4));
                  TRASHBITS(4);
                }
                else
                {
                  tmp=20+(word)(bitbuf>>(BITBUFSIZE-9));
                  TRASHBITS(9);
                }
                memset(p, 0, tmp);
                p+=tmp;
                i-=tmp-1;
              }
              else
              {
                *p++=0;
              }
            }
          }
          memset(p, 0, CHARS+MAXMATCH+1-count); /* clear rest of karlen */
          memmove(karlen+3, karlen, 256); /* fix for special table, 256 because of gnuzip */
          *karlen=0;    /* otherwise wrong huffman table */
          karlen[1]=0;
          karlen[2]=0;
          count+=3;
          { /* create huff2kar */
            unsigned long clenct[17]; /* charlenths are 0 t/m 16 */
            int maxlen;
            unsigned long *q=clenct;
            memset(q, 0, 17*sizeof(unsigned long));
            i=count;
            p=karlen-CHARS;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* find maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            karshlvl=BITBUFSIZE-maxlen;
            /* calc start adresses for all lengths */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* 0 skip zero */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
#ifdef DEBUG_MODE
                printf("Second huffman table is bad!\n");
#endif
                free(base_base);
                return;
              }
            }
            p=karlen-CHARS;
            count-=CHARS;
            for(i=-CHARS;i<count;i++)
            { /* create huff2ptr */
              long j;
              int tmp;
              if((tmp=*p++)!=0)
              {
                kartype *q=huff2kar+clenct[tmp];
                j=1<<(maxlen-tmp);
                clenct[tmp]+=j;
                while(--j>=0)
                {
                  *q++=i;
                }
              }
            }
          }
        }
        else
        { /* there is only one character */
          int kar=(word)(bitbuf>>(BITBUFSIZE-9));
          TRASHBITS(9);
          kar-=CHARS;
          if(kar>=0)
          { /* charatcter is a pointer-len */
            kar+=3;
          }
          *huff2kar=kar;
          huff2kar[1]=kar;
          karshlvl=BITBUFSIZE-1; /* intel is still shitty, we can't shift over BITBUFSIZE bits */
          karlen[kar]=0;
        }
      }
      { /* characters have been read, now the pointers */
        /* this is about the same as above */
        int count=(word)(bitbuf>>(BITBUFSIZE-5));
        TRASHBITS(5);
        if(count)
        { /* read count for huffmanodes */
          byte *p=ptrlen;
          int i=count;
          while(--i>=0)
          {
            int tmp=(word)(bitbuf>>(BITBUFSIZE-3));
            TRASHBITS(3);
            if(tmp==7)
            {
              while(((signed long) bitbuf) <0)
              {
                tmp++;
                TRASHBITS(1); /* optimize: bitbuf+=bitbuf... */
              }
              TRASHBITS(1);
            }
            *p++=tmp;
          }
          { /* make huff2ptr tabel */
            unsigned long clenct[17]; /* ; */
            int maxlen;
            unsigned long *q=clenct;
            i=17;
            while(--i>=0)
            {
              *q++=0;
            }
            i=count;
            p=ptrlen;
            while(--i>=0)
            {
              clenct[*p++]++;
            }
            q=clenct+17;
            /* zoek maxlen */
            while(*--q==0)
            {
              ;
            }
            maxlen=(int)(q-clenct);
            ptrshlvl=BITBUFSIZE-maxlen;
            /* calculate start adresses */
            {
              unsigned long start=0;
              int j=maxlen;
              q=clenct;
              *q++=0; /* wis 0 */
              while(--j>=0)
              {
                unsigned long tmp=*q;
                *q++=start;
                start+=tmp<<j;
              }
              if(start != (1UL<<maxlen))
              {
#ifdef DEBUG_MODE
                printf("Third huffman table is bad!\n");
#endif
                free(base_base);
                return;
              }
            }
            p=ptrlen;
            for(i=0;i<count;i++)
            { /* create huff2ptr */
              unsigned long j;
              int tmp;
              if((tmp=*p++)!=0)
              {
                j=1<<(maxlen-tmp);
                memset(huff2ptr+clenct[tmp], i, j);
                clenct[tmp]+=j;
              }
            }
          }
        }
        else
        { /* there is only 1 pointer */
          int ptr=(word)(bitbuf>>(BITBUFSIZE-5));
          TRASHBITS(5);
          ptrshlvl=BITBUFSIZE-1; /* shitty intel; we mogen niet over BITBUFSIZE schuiven */
          *huff2ptr=ptr;  /* zet dus de pointer voor nul en een */
          huff2ptr[1]=ptr;
          ptrlen[ptr]=0;
        }
      }
    }
  }
}


