extern "C"
{
    #include "DEMOSDK\BASTYPES.H"
    #include "DEMOSDK\STANDARD.H"
}

#include <assert.h>


int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf ("#ERROR: wrong parameters\n"
                "ARJ2ARJX v1.0.0\n"                
                "  Usage:\n"
                "    ARJ2ARJX file.ARJ\n");

        return 1;
    }

    char* filename = argv[1];
    u8* buffer;
    u32 filesize;

    {
        FILE* data = fopen(filename, "rb");

        if (data == NULL)
        {
            printf("ERROR: can not open file %s\n", filename);
            return 1;
        }

        fseek(data, 0, SEEK_END);
        filesize = ftell(data);
        fseek(data, 0, SEEK_SET);

        buffer = (u8*)malloc(filesize);
        assert(buffer != NULL);

        u32 result = fread(buffer, 1, filesize, data);
        assert(result == filesize);
        fclose(data);
    }

    if ((buffer[0] != 0x60) || (buffer[1] != 0xEA))
    {
        printf("Error: '%s' is not an ARJ file\n", filename);
        return 1;
    }

    u8* p = &buffer[2];

    u16 headerSize = *(u16*)p;
    p += sizeof(headerSize);
    p += headerSize;
    p += 4;     // skip crc

    u16 exHeaderSize = *(u16*)p;
    p += sizeof(exHeaderSize);

    if (exHeaderSize > 0)
    {
        p += exHeaderSize;
        p += 4;     // skip crc
    }

    if ((p[0] != 0x60) || (p[1] != 0xEA))
    {
        printf("Error: '%s' is not an ARJ file\n", filename);
        return 1;
    }

    p += 2;

    if (p[7] != 4)
    {
        printf("demOS expect to have ARJ file compressed in mode 4 (current is %d)\n", p[7]);
    }

    u32 compressedSize = *(u32*)&p[14];
    s32 originalsize = *(u32*)&p[18];

    printf("sizes of ARJ : compressed=%d - uncompressed=%d (ratio=%d%%)\n", compressedSize, originalsize, compressedSize * 100 / originalsize);

    u16 localHeaderSize = *(u16*)p;
    p += sizeof(localHeaderSize);

    if (localHeaderSize > 0)
    {
        p += localHeaderSize;
        p += 4;     // skip crc
    }

    u16 exLocalHeaderSize = *(u16*)p;
    p += sizeof(exLocalHeaderSize);

    if (exLocalHeaderSize > 0)
    {
        p += localHeaderSize;
        p += 4;     // skip crc
    }

    u32 shrinkSize = p - buffer;

    shrinkSize -= 4; // leave place to store size

    printf("image strip headers for %d bytes\n", shrinkSize);

    memmove(&buffer[4], p, filesize - shrinkSize);

    filesize = 4 + compressedSize;

    *(u32*)buffer = STDswap32(originalsize);

    char destfilename[256];

    sprintf(destfilename, "%sX", filename);

    {
        FILE* data = fopen(destfilename, "wb");

        if (data == NULL)
        {
            printf("ERROR: can not open file %s\n", filename);
            return 1;
        }

        u32 result = fwrite(buffer, 1, filesize, data);
        assert(result == filesize);
        fclose(data);
    }

    return 0;
}

