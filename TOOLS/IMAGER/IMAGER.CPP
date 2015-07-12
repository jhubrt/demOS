/*------------------------------------------------------------------------------  -----------------
  Copyright J.Hubert 2015

  This file is part of demOS

  demOS is free software: you can redistribute it and/or modify it under the terms of 
  the GNU Lesser General Public License as published by the Free Software Foundation, 
  either version 3 of the License, or (at your option) any later version.

  demOS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY ;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with demOS.  
  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------- */

#include "TOOLS\IMAGER\IMAGER.H"

#include "DEMOSDK\LOAD.H"

#include <vector>
#include <string>

struct FileEntry
{
	u32	    nbsectors;
	u32     startsector;
	u32     side;
	u32     track;
    s32     metadataIndex;
    u32     padding;
    std::string name;
};

struct FileMetaData
{
	u32			offset;
	u32			size;
	s32			originalsize;
	std::string name;
    std::string filename;
};


class OutputFile
{
protected:
    FILE*           m_file;
    std::string     m_fileid;
    std::string     m_filename;


    OutputFile(const char* _filename, const char* _ext, const char* _fileid)
        : m_fileid      (_fileid)
        , m_filename    (_filename)
    {
    	char filename[256];
    	
        sprintf(filename, "%s.%s", _filename, _ext);
	    m_file = fopen (filename, "w");
    }
    
    ~OutputFile()
    {
        if (m_file != NULL)
        {
            fclose(m_file);
            m_file = NULL;
        }
    }

public:
    
    bool isValid() const { return m_file != NULL; }
};


class HeaderFile : public OutputFile
{
public:
    HeaderFile (const char* _filename, const char* _fileid) 
        : OutputFile(_filename, "H", _fileid)
  
    {}

    void write(std::vector<FileEntry>& _fileEntries, std::vector<FileMetaData>& _metaData)
    {
        assert (m_file != NULL);
        const char* fileid = m_fileid.c_str();

		fprintf (m_file, "#ifndef %s_H\n#define %s_H\n\n#include \"DEMOSDK\\LOAD.H\"\n\n", fileid, fileid);
		fprintf (m_file, "ENUM(RSC_%s_ENTRIES)\n{\n", m_fileid.c_str());

        for (u32 t = 0 ; t < _fileEntries.size() ; t++)
        {
			fprintf (m_file, "    RSC_%s_%s,   /* %d bytes */\n", 
                m_fileid.c_str(), 
                _fileEntries[t].name.c_str(),
                _fileEntries[t].nbsectors * 512);
        }

		fprintf (m_file, "    RSC_%s_NBENTRIES\n};\n\n", fileid);

		// Files metadata list

		fprintf (m_file, "ENUM(RSC_%s_METADATA)\n{\n", fileid);

		for (std::vector<FileMetaData>::const_iterator metaData = _metaData.begin() ; metaData != _metaData.end() ; metaData++)
		{
			fprintf (m_file, "    RSC_%s_METADATA_%s", fileid, metaData->name.c_str());
			fprintf (m_file, ",");
			fprintf (m_file, "    /* @%ld %ld bytes */\n", metaData->offset, metaData->size);
		}

		fprintf (m_file, "    RSC_%s_NBMETADATA\n", fileid);
		fprintf (m_file, "};\n\n");

		fprintf (m_file, "#ifndef %s_C\n", fileid);
		fprintf (m_file, "extern LOADdisk RSC_%s;\n", fileid);
		fprintf (m_file, "#endif\n\n");

		fprintf (m_file, "#endif\n");
    }
};


class BodyFile : public OutputFile
{
public:
    BodyFile (const char* _filename, const char* _fileid) 
        : OutputFile(_filename, "C", _fileid)
    {}

    void write(std::vector<FileEntry>& _fileEntries, std::vector<FileMetaData>& _metaData, s32 _delta, u16 _preferedUnit)
    {
        assert (m_file != NULL);
        const char* fileid = m_fileid.c_str();

		fprintf (m_file, "#define %s_C\n\n", fileid);
		fprintf (m_file, "#include \"%s.H\"\n\n", m_filename.c_str());

		fprintf (m_file, "static LOADresource RSC_%s_FAT[] =\n", fileid); 
		fprintf (m_file, "{ /* nbSectors @sector   @side     @track   metaDataIndex */\n"); 

        for (std::vector<FileEntry>::const_iterator fileEntry = _fileEntries.begin() ; fileEntry != _fileEntries.end() ; fileEntry++)
		{
            fprintf (m_file, "    { %8ld, %8ld, %8ld, %8ld, %6ld } %c  /* lost bytes = %d */\n",
                fileEntry->nbsectors,
                fileEntry->startsector,
                fileEntry->side,
                fileEntry->track,
                fileEntry->metadataIndex,
                ((fileEntry + 1) != _fileEntries.end()) ? ',' : ' ',
                fileEntry->padding
                );
		}
		
        fprintf (m_file, "};\n\n");
        fprintf (m_file, "static LOADmetaData RSC_%s_MetaData[] =\n", fileid);
        fprintf (m_file, "{ /*   offset           size    originalsize */\n");

		for (std::vector<FileMetaData>::const_iterator metaData = _metaData.begin() ; metaData != _metaData.end() ; metaData++)
		{
            fprintf (m_file, "    { %8ldUL, %8ldUL, %8ldL } %c\n", 
                metaData->offset,
                metaData->size, 
                metaData->originalsize, 
                ((metaData + 1) != _metaData.end()) ? ',' : ' ');
		}

		fprintf (m_file, "};\n\n");

		fprintf (m_file, "LOADdisk RSC_%s = { RSC_%s_FAT, RSC_%s_NBENTRIES, RSC_%s_MetaData, RSC_%s_NBMETADATA, %d\n", fileid, fileid, fileid, fileid, fileid, _preferedUnit);
		fprintf (m_file, "#	ifdef DEMOS_DEBUG\n     ,\"%s.ST\", NULL\n#   endif\n};\n", m_filename.c_str()); 

        fprintf (m_file, "\n/* %d bytes left on floppy */\n", _delta);
    }
};


static u32 STDswap32(u32 _v)
{
    u32 v;
    u8* s = (u8*) &_v;
    u8* d = (u8*) &v;

    d[0] = s[3];
    d[1] = s[2];
    d[2] = s[1];
    d[3] = s[0];

    return v;
}

static u16 STDswap16(u16 _v)
{
    u16 v;
    u8* s = (u8*) &_v;
    u8* d = (u8*) &v;

    d[0] = s[1];
    d[1] = s[0];
    
    return v;
}

static void makeId (char* _source, char* _dest)
{
	while (*_source)
	{
		if ((((*_source) >= 'A') && ((*_source) <= 'Z')) ||
			(((*_source) >= 'a') && ((*_source) <= 'z')) ||
			(((*_source) >= '0') && ((*_source) <= '9')))
		{
			*_dest = *_source;
		}
		else
		{
			*_dest = '_';
		}

		_source++;
		_dest++;
	}

	(*_dest) = 0;
}

struct FileType
{
    enum Enum
    {
        NORMAL,
        ARJ,
        BOOTSECTOR
    };
};

static void printPrgInfo (const char* _subdir, const char* _name)
{
    char temp[256];

    sprintf(temp, "%s%s%s%s", "DATABIN\\", _subdir, _name, ".PRG");

    FILE* file = fopen (temp, "rb");

    if ( file != NULL )
    {
        u32 value = 0;
        u32 total = 0;

        fseek(file, 2, SEEK_SET);

        printf ("Program file detected : %s\n", temp);

        fread (&value, sizeof(value), 1, file);
        value = PCENDIANSWAP32(value);
        printf ("text size:    %d\n", value);
        total += value;

        fread (&value, sizeof(value), 1, file);
        value = PCENDIANSWAP32(value);
        printf ("data size:    %d\n", value);
        total += value;

        fread (&value, sizeof(value), 1, file);
        value = PCENDIANSWAP32(value);
        printf ("bss size:     %d\n", value);
        total += value;

        fread (&value, sizeof(value), 1, file);
        value = PCENDIANSWAP32(value);
        printf ("symbols size: %d\n", value);
        total += value;

        printf ("total size:   %d bytes\n", total);

        fclose(file);
    }
}

static void* loadDataFile( const char* _currentName, u32& _filesize, s32& _originalsize, bool _executableBootSector)
{
    char filename[256];
    u8* buffer = NULL;

    sprintf (filename, "DATABIN\\%s", _currentName);
    
    {
        FILE* data = fopen (filename, "rb");

        if ( data == NULL )
        {
            printf ("ERROR: can not open file %s\n", filename);
            goto Error;
        }

        fseek (data, 0, SEEK_END);
        _filesize = ftell(data);
        fseek (data, 0, SEEK_SET);

        buffer = (u8*) malloc (_filesize);
        assert(buffer != NULL);

        u32 result = fread (buffer, 1, _filesize, data);
        assert(result == _filesize);
        fclose (data);
    }

    {
        char ext[256];
        char subdir[256];
        char name[256];

        _splitpath (_currentName, NULL, subdir, name, ext);

        printPrgInfo (subdir, name);

        FileType::Enum fileType = FileType::NORMAL;
    
        if ( _strcmpi(ext, ".ARJ") == 0 )
            fileType = FileType::ARJ;
        else if ( _strcmpi(ext, ".BOT") == 0 )
            fileType = FileType::BOOTSECTOR;
    
        if (fileType == FileType::ARJ)
        {
            printf ("ARJ file detected\n");

            if (( buffer[0] != 0x60 ) || ( buffer[1] != 0xEA ))
            {
                printf ("Error: '%s' is not an ARJ file\n", filename);
                goto Error;
            }
            else
            {
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

                if (( p[0] != 0x60 ) || ( p[1] != 0xEA ))
                {   
                    printf ("Error: '%s' is not an ARJ file\n", filename);
                    goto Error;  
                }

                p += 2;

                if ( p[7] != 4 )
                {
                    printf ("demOS expect to have ARJ file compressed in mode 4 (current is %d)\n", p[7]);
                }

                u32 compressedSize = *(u32*)&p[14];
                _originalsize = *(u32*)&p[18];

                printf ("sizes of ARJ : compressed=%d - uncompressed=%d (ratio=%d%%)\n", compressedSize, _originalsize, compressedSize * 100 / _originalsize);

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

                printf ("image strip headers for %d bytes\n", shrinkSize);

                _filesize -= shrinkSize;
                memmove (&buffer[4], p, _filesize);

                *(u32*)buffer = STDswap32(_originalsize);
                {
                    sprintf (filename, "DATABIN\\%sX", _currentName);

                    FILE* data = fopen (filename, "wb");

                    if ( data == NULL )
                    {
                        printf ("ERROR: can not open file %s\n", filename);
                        goto Error;
                    }

                    u32 result = fwrite (buffer, 1, _filesize, data);
                    assert(result == _filesize);
                    fclose (data);
                }
            }
        }
        else if (fileType == FileType::BOOTSECTOR)
        {
            _filesize -= 28;
            memmove (buffer, buffer + 28, _filesize);
            _filesize -= 2;

            if (_filesize > 512)
            {
                printf ("bootsector size exceeds 512 bytes\n");
                goto Error;
            }
            
            if ( _executableBootSector )
            {
                u16 checksum = 0;

                for (u32 t = 0 ; t < (_filesize - 2) ; t += 2)
                {
                    checksum += STDswap16( *(u16*)&buffer[t] );
                }

                *(u16*)&buffer[_filesize - 2] = STDswap16(0x1234 - checksum);
            }
        }
    }


    return buffer;

Error:
    if ( buffer != NULL )
    {
        free (buffer);
    }

    return NULL;
}


bool writeST (char* _filename, u16 _nbSectorsPerTrack, u16 _nbsides, u16 _nbtracks, char** _filesList, u16 _preferedUnit, bool _executableBootSector)
{
	bool success = false;
	char filename[256];
    char fileid [256];


	u16 sides  = _nbsides - 1;
	u16 tracks = _nbtracks - 1;

	sprintf(filename, "%s.ST", _filename);
	FILE* fileImage = fopen (filename, "wb");

	makeId (_filename, fileid);

    HeaderFile  headerFile (_filename, fileid);
	BodyFile    bodyFile   (_filename, fileid);

	if ( ( fileImage != NULL ) && headerFile.isValid() && bodyFile.isValid() )
	{
		char temp [4096];
		s32	 currentsector = 0;

        std::vector<FileEntry>    fileEntries;
        std::vector<FileMetaData> fileMetaData;
	
		while (*_filesList)
		{
            FileEntry fileEntry;
			char* s1 = *_filesList++;
			char* s2 = *_filesList++;
			char* filesToProcess;
			char* p;
			bool  singleFile;
			u32   size = 0;
            u16   metadataIndexBak = fileMetaData.size();

			makeId (s1, filename);
            fileEntry.name = filename;

			singleFile = s2 == NULL;
			filesToProcess = singleFile ? s1 : s2;

			strcpy (temp, filesToProcess);
			p = strtok (temp, ";");

			while (p != NULL)
			{
                u32 fsize = 0;
                s32 originalsize = LOAD_NOTPACKED;

                {
                    void* buffer = loadDataFile(p, fsize, originalsize, _executableBootSector );

                    if (buffer == NULL)
                    {
                        return false;
                    }

                    printf ("write %s at %d\n", filename, ftell(fileImage));
                    u32 result = fwrite (buffer, 1, fsize, fileImage);
                    assert(result == fsize);

                    free(buffer);
                }

                sprintf (filename, "%s_%s", fileid, p);
                makeId (p, filename);

                {
                    FileMetaData metaData;

                    metaData.name         = filename;
                    metaData.filename     = p;
                    metaData.offset       = size;
                    metaData.size         = fsize;
                    metaData.originalsize = originalsize;

                    if (originalsize != LOAD_NOTPACKED )
                    {
                        metaData.filename += 'X';    
                        metaData.name     += 'X';

                        if (size == 0)
                        {
                            fileEntry.name += 'X';
                        }
                    }

                    fileMetaData.push_back(metaData);
                }

				size += fsize;
				p = strtok (NULL, ";");
			}

			u32 padding = (512 - (size & 0x1FF)) & 511;

			if ( padding > 0 )
			{
				memset(temp, 0, padding);
				fwrite (temp, 1, padding, fileImage);
			}
		
            u32 sector = currentsector % (_nbSectorsPerTrack * _nbsides);

            fileEntry.nbsectors     = (size + 511) / 512;
            fileEntry.track         = currentsector / (_nbSectorsPerTrack * _nbsides);
            fileEntry.side          = sector >= _nbSectorsPerTrack;
            fileEntry.startsector   = sector % _nbSectorsPerTrack;
            fileEntry.padding       = padding;
            fileEntry.metadataIndex = metadataIndexBak;

            fileEntries.push_back(fileEntry);

            currentsector += fileEntry.nbsectors;
			if ( currentsector > (_nbSectorsPerTrack * _nbtracks * _nbsides ))
			{
				printf ("ERROR: size exceeds support with file %s\n", *(_filesList - 2));
				goto end;
			}
		}

        u32 imagesize = ((u32)_nbSectorsPerTrack * (u32)_nbsides * (u32)_nbtracks) * 512;
        u32 pos = ftell(fileImage);

        if (pos <= imagesize)
        {
            u32 delta = imagesize - pos;

            void* buf = malloc (delta);
            assert (buf != NULL);
            memset (buf, 0, delta);
            fwrite (buf, delta, 1, fileImage);
            free(buf);

            headerFile.write(fileEntries, fileMetaData);
            bodyFile.write(fileEntries, fileMetaData, delta, _preferedUnit);

            success = true;
        }
	}

end:
	if ( fileImage != NULL )
	{
		fclose (fileImage);
	}

	return success;
}
