#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void fail(char *why)
{
    fputs(why, stderr);
    fputs("\n", stderr);
    exit(1);
}

void *pmalloc (size_t size)
{
    void *result = malloc(size);

    if (result == NULL) {
        fprintf(stderr, "Out of memory in pmalloc");
        exit(1);
    }

    return result;
}

    
void *pzmalloc (size_t size)
{
    void *result = pmalloc(size);

    return memset(result, 0, size);
}

                
