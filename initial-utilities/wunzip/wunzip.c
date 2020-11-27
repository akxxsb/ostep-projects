#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define no_use(var) \

const int BUFF_SIZE = 1024;

struct RLENODE {
    unsigned char size[4];
    unsigned char c;
};

void set_size(unsigned char * size, int x) {
    for (int i = 0; i < 4; ++i) {
        size[i] = (x >> (i*8)) & 0xff;
    }
}

int get_size(unsigned char *size) {
    int cnt = 0;
    for (int i = 0; i < 4; ++i) {
        int tmp = size[i];
        cnt |= tmp << (i*8);
    }
    return cnt;
}


struct RLENODE node;
int size = 0;

int
wzip_main(const char * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        return 1;
    }

    char buff[BUFF_SIZE];
    while (fgets(buff, BUFF_SIZE, fp)) {
        for (int i = 0; buff[i]; ++i) {
            if (size == 0) {
                size = 1;
                node.c = buff[i];
            } else if (buff[i] == node.c) {
                ++size;
            } else {
                set_size(node.size, size);
                fwrite(&node, sizeof(node), 1, stdout);
                node.c = buff[i];
                size = 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

void
wunzip_main(const char * filename)
{

    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        exit(1);
    }

    struct RLENODE node;
    int rc = 0;
    while ((rc = fread(&node, sizeof(node), 1, fp))) {
        int size = get_size(node.size);
        for (int i = 0; i < size; ++i) {
            putchar(node.c);
        }
    }
    fclose(fp);
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        puts("wunzip: file1 [file2 ...]");
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        wunzip_main(argv[i]);
    }

    if(size) {
        set_size(node.size, size);
        fwrite(&node, sizeof(node), 1, stdout);
    }
    return 0;
}
