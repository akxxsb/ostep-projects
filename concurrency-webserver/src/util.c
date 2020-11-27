#include "util.h"
#include <pthread.h>
#include <unistd.h>

void
print_id()
{
    printf("pid %d || thread_id %lu = ", getpid(), pthread_self());
}

void *
Malloc(size_t size)
{
    void * ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "malloc falied, exit\n");
        exit(-1);
    }
    memset(ptr, 0, size);
    return ptr;
}
