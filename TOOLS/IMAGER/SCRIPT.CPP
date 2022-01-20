/*-----------------------------------------------------------------------------------------------
  The MIT License (MIT)

  Copyright (c) 2015-2022 J.Hubert

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
  and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies
  or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-------------------------------------------------------------------------------------------------*/

#include "DEMOSDK\BASTYPES.H"

#include "TOOLS\IMAGER\SCRIPT.H"

#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>


static int strTrim(char* _string, CONST char* _totrim)
{
    int  t, i;


    i = (int)strlen(_string);

    if (i > 0)
    {
        i--;

        while (i >= 0)
        {
            if (strchr(_totrim, _string[i]) == NULL)
                break;
            i--;
        }

        _string[i + 1] = 0;

        for (t = 0; (t <= i) && (strchr(_totrim, _string[t]) != NULL); t++) {}

        return t;
    }

    return 0;
}


bool FloppyImageScript::loadScript(const char* _filename)
{
    FILE* file = fopen(_filename, "rt");

    if (file != NULL)
    {
        char line[1024];
        int count = 0;

        while (fgets(line, sizeof(line), file))
        {
            if (line[0] != ';')
            {
                const char* totrim = " \t\r\n";

                if (count == 0)
                {
                    char* p;

                    p = strtok(line, totrim);
                    m_nbsectors = atoi(p);

                    p = strtok(NULL, totrim);
                    m_nbsides = atoi(p);

                    p = strtok(NULL, totrim);
                    m_nbtracks = atoi(p);

                    p = strtok(NULL, totrim);
                    m_preferedunit = atoi(p);

                    p = strtok(NULL, totrim);
                    m_executablebootsector = atoi(p);

                    count++;
                }
                else
                {
                    char* sep = strchr(line, '|');

                    if (sep != NULL)
                    {
                        *sep = 0;
                        sep++;
                        char* col1 = line + strTrim(line, totrim);
                        char* col2 = sep + strTrim(sep, totrim);

                        if (*col1 != 0)
                        {
                            m_strs.push_back(col1);

                            if (*col2 != 0)
                            {
                                m_strs.push_back(col2);
                            }
                            else
                            {
                                m_strs.push_back("");
                            }

                            count++;
                        }
                        else
                        {
                            if (*col2 != 0)
                            {
                                m_strs.back().append(col2);
                            }
                        }
                    }
                }
            }
        }

        for (const auto& s : m_strs)
        {
            if (s.empty())
                m_filelist.push_back(NULL);
            else
                m_filelist.push_back(s.c_str());
        }

        m_filelist.push_back(NULL);

        fclose(file);

        return m_filelist.size() > 1;
    }

    return false;
}
