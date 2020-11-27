#include <stdio.h>
#include <stdlib.h>

const int BUFF_SIZE = 1024;

void
wcat_main(const char * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        puts("wcat: cannot open file");
        exit(1);
    }
    char buff[BUFF_SIZE];
    while (fgets(buff, BUFF_SIZE, fp)) {
        printf("%s",buff);
    }
    fclose(fp);
}

int
main(int argc, char **argv)
{
    if (argc == 1) {
        return 0;
    }
    for (int i = 1; i < argc; ++i) {
        wcat_main(argv[i]);
    }
    return 0;
}
