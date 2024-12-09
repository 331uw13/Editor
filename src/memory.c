#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "memory.h"


static size_t find_next_size(size_t size, size_t newsize) {
    size_t res = size;

    // round up to next power of 2 for 64 bit integer.
    res |= (res >> 1);
    res |= (res >> 2);
    res |= (res >> 4);
    res |= (res >> 8);
    res |= (res >> 16);
    res |= (res >> 32);
    res++;

    
    while(res < newsize) {
        res *= EDITOR_MEM_MULT;
    }

    return res;
}


void* safe_resize_array(void* ptr, size_t esizeb, size_t ptrsize, size_t newsize, long int* nsize_ptr) {
    void* result = NULL;

    if((esizeb * newsize) <= 0) {
        goto error;
    }

    if((ptrsize < newsize) && (ptr != NULL)) {
        size_t amemsize = find_next_size(ptrsize, newsize);
        result = reallocarray(ptr, esizeb, amemsize);
        if(result == NULL) {
            fprintf(stderr, 
                    "\033[31m[ERROR]\033[0m %s | %s: 'reallocarray(ptr, esizeb, amemsize)' failed.\n"
                    "ptr = %p | esizeb = %li | amemsize = %li\n"
                    "(errno:%i) %s\n",
                    __FILE__, __func__, ptr, esizeb, amemsize, errno, strerror(errno));

            if(nsize_ptr) {
                *nsize_ptr = MEMRESIZE_ERROR;
            }

            result = ptr;
            goto error;
        }

        if(nsize_ptr) {
            *nsize_ptr = amemsize;
        }
    }
    else if(ptr == NULL) {
        if(!(result = malloc(newsize * esizeb))) {
            fprintf(stderr, "[ERROR] %s: 'malloc(newsize * ewsizeb)' failed.\n(errno:%d)%s\n",
                    __func__, errno, strerror(errno));
        
            if(nsize_ptr) {
                *nsize_ptr = MEMRESIZE_ERROR;
            }
        }
    }

error:
    return result;
}


void safe_free(void** ptr) {
    printf("safe_free is NOT IMPLEMENTED!\n");
}

/*
void init_mem_static_vars() {
}
*/


