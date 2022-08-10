#include <stdio.h>


int main(int argc, char** argv)
{
    int t;

    for (t = 0 ; t < argc ; t++)
        printf("%d: '%s'\n", t, argv[t]);

    while(1);

    return 0;
}
