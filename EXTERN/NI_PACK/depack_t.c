/*
**
** test program for testing the Ni packer depack code
**
** in: arj file containing the compressed data
**
** the data will be depacked, integrety checked, timed, and thrown away
**
** 2022, Hans Wessels
**
*/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#ifdef __PUREC__
	#define CLOCKS_PER_SEC CLK_TCK
	typedef signed char             int8_t;
	typedef signed short int        int16_t;
	typedef signed long int         int32_t;
	typedef unsigned char           uint8_t;
	typedef unsigned short int      uint16_t;
	typedef unsigned long int       uint32_t;
#else
	#include <stdint.h>
#endif


extern void unstore(unsigned long size, uint8_t *dst, uint8_t *data);
extern void decode_m7(uint8_t *dst, uint8_t *data);
extern void decode_m4(unsigned long size, uint8_t *dst, uint8_t *data);
extern void decode_n0(uint8_t *dst, uint8_t *data);
extern void decode_n1(uint8_t *dst, uint8_t *data);
extern void decode_n2(uint8_t *dst, uint8_t *data);

extern void make_crc32_table(uint32_t crc_table[]);
extern uint32_t crc32(unsigned long count, uint8_t *data, uint32_t crc_table[]);

#define ARJ_HEADER_ID 		0xEA60U

#define STORE 0          /* general store */
#define ARJ_MODE_1 1     /* arj mode 1 */
#define ARJ_MODE_2 2     /* arj mode 2 */
#define ARJ_MODE_3 3     /* arj mode 3 */
#define ARJ_MODE_4 4     /* arj mode 4 */
#define GNU_ARJ_MODE_7 7 /* gnu arj mode 7 */
#define NI_MODE_0 0x10   /* ni packer mode 0 */
#define NI_MODE_1 0x11   /* ni packer mode 1 */
#define NI_MODE_2 0x12   /* ni packer mode 2 */
#define NI_MODE_3 0x13   /* ni packer mode 3 */
#define NI_MODE_4 0x14   /* ni packer mode 4 */
#define NI_MODE_5 0x15   /* ni packer mode 5 */
#define NI_MODE_6 0x16   /* ni packer mode 6 */
#define NI_MODE_7 0x17   /* ni packer mode 7 */
#define NI_MODE_8 0x18   /* ni packer mode 8 */
#define NI_MODE_9 0x19   /* ni packer mode 9 */

uint32_t crc_table[256]; /* CRC32 table to check the results of de depack routines */


uint8_t get_byte(uint8_t *p)
{
	return p[0];
}

uint16_t get_word(uint8_t *p)
{
	uint16_t res;
	res=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

uint32_t get_long(unsigned char *p)
{
	uint32_t res;
	res=p[3];
	res<<=8;
	res|=p[2];
	res<<=8;
	res|=p[1];
	res<<=8;
	res|=p[0];
	return res;
}

static double total_time=0.0;
static unsigned long error_count=0;
static unsigned long total_compressed_size=0;
static unsigned long total_original_size=0;

void decode(int mode, unsigned long size, unsigned long compressed_size, uint32_t crc, uint8_t *data)
{ /* decode the data pointed to data */
	uint8_t *dst;
	uint32_t res_crc=0;
	clock_t start;
	dst=(uint8_t *)malloc(size+1024);
	if(dst==NULL)
	{
		printf("Malloc error, %lu bytes\n", size+1024);
		return;
	}
	start = clock();
	switch(mode)
	{
	case STORE:
		unstore(size, dst, data);
		break;
	case ARJ_MODE_1:
	case ARJ_MODE_2:
	case ARJ_MODE_3:
	case GNU_ARJ_MODE_7:
		{ /* zero last two bytes */
			uint8_t temp_bytes[2];
			temp_bytes[0]=data[compressed_size];
			temp_bytes[1]=data[compressed_size+1];
			data[compressed_size]=0;
			data[compressed_size+1]=0;
			decode_m7(dst, data);
			data[compressed_size]=temp_bytes[0];
			data[compressed_size+1]=temp_bytes[1];
		}
		break;
	case ARJ_MODE_4:
		decode_m4(size, dst, data);
		break;
	case NI_MODE_0:
		decode_n0(dst, data);
		break;
	case NI_MODE_1:
		decode_n1(dst, data);
		break;
	case NI_MODE_2:
	case NI_MODE_9:
		decode_n2(dst, data);
		break;
	default:
		printf("Unknown method: %X", mode);
		break;
	}
	printf(" %7.3f s ", (double)(clock() - start) / CLOCKS_PER_SEC);
	total_time+=(double)(clock() - start) / CLOCKS_PER_SEC;
	if((res_crc=crc32(size, dst, crc_table))==crc)
	{
		printf("CRC OK");
	}
	else
	{
		printf("CRC ERROR! :%08lX", (unsigned long) res_crc);
		error_count++;
	}
	free(dst);
}

int main(int argc, char *argv[])
{
	char *filenaam;
	uint8_t *data;
	unsigned long offset;
	unsigned long file_size;
	FILE* f;
	if(argc==2)
	{
		filenaam=argv[1];
	}
	else
	{
		printf("Usage: %s <file to be tested>\n", argv[0]);
		return -1;
	}
	f=fopen(filenaam, "rb");
	if(f==NULL)
	{
		printf("File open error %s", filenaam);
		return -1;
	}
	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	data = (uint8_t *)malloc(file_size + 1024);
	if (data == NULL)
	{
		printf("Malloc error voor file data!\n");
		fclose(f);
		return -1;
	}
	fseek(f, 0, SEEK_SET);
	(void)!fread(data, 1, file_size, f);
	fclose(f);
	make_crc32_table(crc_table);
	offset=0;
   /*      DATA10.BIN                   7082          2644  10  5944648C */
	printf("File name:               original        packed mode CRC32\n");
	for(;;)
	{
		if(offset>=file_size)
		{
			printf("Unexpected end of data reached, aborting\n");
			break;
		}
		/* zoek header */
		if(get_word(data+offset)==ARJ_HEADER_ID)
		{ /* ARJ header gevonden */
			unsigned int header_size;
			unsigned int header_size_1;
			unsigned long compressed_size;
			unsigned long original_size;
			uint32_t crc32;
			int file_naam_pos;
			int method;
			char* naam;
			header_size=get_word(data+offset+2);
			if(header_size==0)
			{ /* end of archive */
				break;
			}
			header_size_1=get_byte(data+offset+4);
			if(offset+header_size>=file_size)
			{
				printf("Unexpected end of data reached, aborting\n");
				break;
			}
			if(get_byte(data+offset+0xA)!=0)
			{ /* not a compressed binary file, we are not interested */
				offset+=header_size+8;
				while(get_word(data+offset)!=0)
				{
					offset+=get_word(data+offset)+6;
					if(offset+2>=file_size)
					{
						printf("Unexpected end of data reached, aborting\n");
						break;
					}
				}
				offset+=2;
				continue;
			}
			method=get_byte(data+offset+9);
			compressed_size=get_long(data+offset+0x10);
			original_size=get_long(data+offset+0x14);
			crc32=get_long(data+offset+0x18);
			file_naam_pos=get_word(data+offset+0x1C);
			naam=data+offset+header_size_1+4;
			printf("%-20s", naam+file_naam_pos);
			printf(" %12lu ", original_size);
			if((method==ARJ_MODE_1)||(method==ARJ_MODE_2)||(method==ARJ_MODE_3)||(method==GNU_ARJ_MODE_7))
			{
				printf(" %12lu ", compressed_size+2);
				total_compressed_size+=compressed_size+2;
			}
			else
			{
				printf(" %12lu ", compressed_size);
				total_compressed_size+=compressed_size;
			}
			printf(" %2X ", method);
			printf(" %08lX ", (unsigned long)crc32);

			total_original_size+=original_size;

			offset+=header_size+8;
			while(get_word(data+offset)!=0)
			{
				offset+=get_word(data+offset)+6;
				if(offset+2>=file_size)
				{
					printf("Unexpected end of data reached, aborting\n");
					break;
				}
			}
			offset+=2;
			if(offset+compressed_size>=file_size)
			{
				printf("Unexpected end of data reached, aborting\n");
				break;
			}
			decode(method, original_size, compressed_size, crc32, data+offset);
			printf("\n");
			offset+=compressed_size;
		}
		else
		{
			offset++;
		}
	}
	printf("\n%-20s", "totaal");
	printf(" %12lu ", total_original_size);
	printf(" %12lu               ", total_compressed_size);
	printf(" %7.3f s ", total_time);
	if(error_count==0)
	{
		printf("CRC OK\n");
	}
	else
	{
		printf("CRC Errors = %lu\n", error_count);
	}
	free(data);
	return 0;
}

