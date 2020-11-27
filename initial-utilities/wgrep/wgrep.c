#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int BUFF_SIZE = 1024;

int
find_pattern(const char * text, const char *pattern)
{
    int n = strlen(text), m = strlen(pattern);
    for (int i = 0; i <= n - m; ++i) {
        if (strncmp(text+i, pattern, m) == 0) {
            return 1;
        }
    }
    return 0;
}

char * buff = NULL;
size_t buff_size = 0;

void
wgrep_main(const char * filename, const char * pattern)
{
    FILE * fp = stdin;
    if (filename) {
        fp = fopen(filename, "r");
    }
    if (fp == NULL) {
        puts("wgrep: cannot open file");
        exit(1);
    }

    while (1) {
        int rc = getline(&buff, &buff_size, fp);
        if (rc == -1 || rc == EOF) {
            break;
        }
        if (buff && find_pattern(buff, pattern)) {
            printf("%s", buff);
        }
    }
    fclose(fp);
}

int
main(int argc, char **argv)
{
    if (argc == 1) {
        puts("wgrep: searchterm [file ...]");
        return 1;
    } else if(argc == 2) {
        wgrep_main(NULL, argv[1]);
        return 0;
    }
    for (int i = 2; i < argc; ++i) {
        wgrep_main(argv[i], argv[1]);
    }
    return 0;
}
