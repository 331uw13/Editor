#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "string.h"



struct string_t* create_string() {
    struct string_t* s = NULL;

    size_t mem_size = sizeof *s;
    s = malloc(mem_size);
    if(!s) {
        fprintf(stderr, "failed to allocate memory for string_t.\n");
        goto error;
    }

    s->mem_size = 0;
    s->data_size = 0;
    s->data = NULL;

    s->data = malloc(STRING_MEMORY_BLOCK_SIZE);
    if(!s->data) {
        fprintf(stderr, "failed to allocate memory for string data.\n");
        free(s);
        s = NULL;
    }

    s->mem_size = STRING_MEMORY_BLOCK_SIZE;

error:
    return s;
}


void cleanup_string(struct string_t** str) {
    if((*str)) {
        if((*str)->data) {
            free((*str)->data);
            (*str)->data = NULL;
        }
        (*str)->data_size = 0;
        (*str)->mem_size = 0;
        free(*str);
        (*str) = NULL;
    }
}

int string_ready(struct string_t* str) {
    int ready = 0;
    if(str) {
        ready = (str->data && str->mem_size > 0);
    }
    return ready;
}

int string_memcheck(struct string_t* str, size_t size) {
    int ok = 0;

    if(string_ready(str)) {
        if(size <= str->mem_size) {
            
            // TODO: resize string to smaller size?
            ok = 1;
        }
        else {

            char* nptr = NULL;
            size_t new_size = size+STRING_MEMORY_BLOCK_SIZE;
            
            if(new_size >= STRING_MAX_SIZE) {
                fprintf(stderr, "at 'string_memcheck': new_size is too big.\n");
            }

            nptr = reallocarray(str->data, new_size, 1);
            if(nptr) {
                str->data = nptr;
                str->mem_size = new_size;
                ok = 1;
                //printf("resized string '%p' memory size to %li bytes\n", str, new_size);
            }
            else {
                fprintf(stderr, "failed to resize string data.\n");
            }
        }
    }

    return ok;
}

int string_add_char(struct string_t* str, char c, size_t index) {
    int ok = 0;
    if(string_ready(str)) {
        if(index > str->data_size) {
            goto error;
        }
        if(!string_memcheck(str, str->data_size+1)) {
            goto error;
        }
        
        //printf("string mem_size:%li\n", str->mem_size);
        
        if(index == str->data_size) {
            str->data[index] = c;
        }
        else {
            memmove(str->data+index+1, str->data+index, str->data_size-index);
            memset(str->data+index, c, 1);
        }
        str->data_size++;
        ok = 1;
    }

error:
    return ok;
}

int string_rem_char(struct string_t* str, size_t index) {
    int ok = 0;
    if(string_ready(str)) {
        if(index > str->data_size) {
            goto done;
        }
        
        memmove(str->data+index-1, str->data+index, str->data_size-index);
        str->data_size--;
        ok = 1;
    }

done:
    return ok;
}

int string_append_char(struct string_t* str, char c) {
    int ok = 0;
    
    if(string_ready(str)) {
        if(string_memcheck(str, str->data_size+1)) {
            str->data[str->data_size] = c;
            str->data_size++;
        }
    }

    return ok;
}


int string_move_data(struct string_t* dst_str, struct string_t* src_str, 
        size_t dst_offset, size_t src_offset,
        size_t size, int flags) {
 
    int ok = 0;
    if(string_ready(dst_str) && string_ready(src_str)) {

        if(size > src_str->data_size) {
            goto error;
        }
        if(dst_offset > dst_str->data_size) {
            goto error;
        }
        if(src_offset > src_str->data_size) {
            goto error;
        }

        const size_t new_dst_size = dst_str->data_size + size;


        if(!string_memcheck(dst_str, new_dst_size)) {
            goto error;
        }
        



        if(!(flags & STRING_OVERWRITE_DATA) 
                && dst_offset < dst_str->data_size) {
            // need to move the existing data away 
            // so it doesnt get overwritten by the next memmove
            memmove(
                    dst_str->data + dst_offset + size,
                    dst_str->data + dst_offset,
                    dst_str->data_size - dst_offset
                    );

        }

        memmove(dst_str->data + dst_offset,
                src_str->data + src_offset,
                size);


        dst_str->data_size = new_dst_size;

        if(flags & STRING_ZERO_SRC) {
            memset(src_str->data, 0, src_str->data_size);
            src_str->data_size = 0;
        }
    }

error:
    return ok;
}


int string_copy_all(struct string_t* dst_str, struct string_t* src_str) {
    int ok = 0;

    if(string_ready(dst_str) && string_ready(src_str)) {
        if(string_memcheck(dst_str, src_str->data_size)) {
            memmove(dst_str->data,
                    src_str->data, 
                    src_str->data_size);
            
            dst_str->data_size  = src_str->data_size;
        
            ok = 1;
        }

    }


    return ok;
}


int string_cut_data(struct string_t* str, size_t offset, size_t size) {
    int ok = 0;

    if(string_ready(str)) {
        const size_t cut_bytes = offset + size;
        if(cut_bytes > str->data_size) {
            goto error;
        }

        if(str->data_size < (offset+size)) {
            fprintf(stderr, "'string_cut_data': trying to cut too much data.\n");
            goto error;
        }


        memset(str->data + offset, 0, size);

        if(cut_bytes < str->data_size) {
            // remaining bytes
            size_t rbytes = str->data_size - cut_bytes;
            
            memmove(str->data + offset,
                    str->data + cut_bytes,
                    rbytes);

        }

        str->data_size -= size;
    }
error:
    return ok;
}

size_t string_num_chars(struct string_t* str, size_t start, size_t end, char c) {
    size_t num = 0;

    if(string_ready(str)) {
        if(start < str->data_size && end <= str->data_size) {
            for(size_t i = start; i < end; i++) {
                if(str->data[i] == c) {
                    num++;
                }
            }
        }
    }

    return num;
}

int string_set_data(struct string_t* str, char* data, size_t size) {
    int ok = 0;

    if(string_ready(str) && data && size > 0) {
        memmove(str->data, data, size);
    }


    return ok;
}

