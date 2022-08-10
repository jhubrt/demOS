#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <TOS.H>


int main(int argc, char** argv)
{
    char commandline[512] = "_";
    char filename[256] = "";
    FILE* file;


    file = fopen ("D:\\PROJECTS\\DEMOS\\TOOLS\\EMUEXEC\\EMUEXEC.CFG", "rb");

    if (file == NULL)
    {
        printf ("ERROR: cannot open config file\n");
        return 1;
    }
    else
    {
        fgets(filename, (int)sizeof(filename), file);
        strtok (filename, "\r\n");

/*      strcpy (commandline, filename);
        strcat (commandline, " ");*/

        fgets(&commandline[strlen(commandline)], (int)sizeof(commandline), file);
        strtok (commandline, "\r\n");

        fclose(file);

        printf ("Execute: '%s'\n", filename);
        printf ("CommandLine: '%s'\n", commandline);

        Pexec( 0, filename, commandline, NULL);
    }

    return 0;
}
