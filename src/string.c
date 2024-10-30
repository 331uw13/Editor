#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "string.h"



struct string_t* create_string(size_t init_size) {
    struct string_t* s = NULL;

    s = malloc(sizeof *s);
    if(!s) {
        fprintf(stderr, "failed to allocate memory for string_t.\n");
        goto error;
    }

    s->mem_size = 0;
    s->data_size = 0;
    s->data = NULL;

    size_t mem_size = (init_size == 0) ? STRING_MEMORY_BLOCK_SIZE : init_size; 

    s->data = malloc(mem_size);
    if(!s->data) {
        fprintf(stderr, "failed to allocate memory for string data.\n");
        free(s);
        s = NULL;
    }

    s->mem_size = mem_size;

error:
    return s;
}


void delete_string(struct string_t** str) {
    if(*str) {
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
        ready = (   str->data 
                && str->mem_size > 0
                && str->data_size <= str->mem_size
                );
    }
    return ready;
}

int string_memcheck(struct string_t* str, size_t size) {
    int ok = 0;

    if(string_ready(str)) {
        if(size <= str->mem_size) {
            // TODO: resize string to smaller size?
            //   
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
        if(str->data_size == 0) {
            goto done;
        }
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
            ok = 1;
        }
    }

    return ok;
}

char string_get_char(struct string_t* str, size_t index) {
    char c = 0;

    if(string_ready(str)) {
        if(index < str->data_size && str->data_size <= str->mem_size) {
            c = str->data[index];
        }
    }

    return c;
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
        ok = 1;
    }

error:
    return ok;
}

size_t string_count_ws_to(struct string_t* str, size_t n) {
    size_t res = 0;

    if(string_ready(str)) {
        if(n > str->data_size) {
            n = str->data_size;
        }

        int found = 0;

        size_t i = 0;
        for(; i < n; i++) {
            if(str->data[i] != 0x20) {
                res = i;
                found = 1;
                break;
            }
        }

        if(!found && (i == n)) {
            res = i;
        }
    }

    return res;
}

int string_is_data_ws(struct string_t* str) {
    int res = 1;

    if(string_ready(str)) {
        for(size_t i = 0; i < str->data_size; i++) {
            if(str->data[i] != 0x20) {
                res = 0;
                break;
            }
        }
    }

    return res;
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
        ok = 1;
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

int string_clear_data(struct string_t* str) {
    int ok = 0;
    if(string_ready(str)) {
        memset(str->data, 0, str->data_size);
        str->data_size = 0;
        ok = 1;
    }
    return ok;
}


int string_set_data(struct string_t* str, char* data, size_t size) {
    int ok = 0;

    if(string_ready(str) && data && (size > 0)) {
        memmove(str->data, data, size);
        ok = 1;
    }

    return ok;
}

size_t string_find_char(struct string_t* str, size_t start_index, char c, int direction) {
    size_t len = 0;
    size_t i = start_index;

    size_t end = 0;
    int inc = 0;

    switch(direction) {
        case STRFIND_NEXT:
            end = str->data_size;
            inc = 1;
            break;

        case STRFIND_PREV:
            end = 0;
            inc = -1;
            break;

        default: 
            return 0;
    }

    while((str->data[i] != c)) {
        long int x = i+inc;
        if(x == end) {
            break;
        }
        i = x;
        len++;
    }

    return len;
}

int count_data_linewraps(char* data, size_t size, int max_line_size) {
    int count = 0;
    int x = 0;

    for(size_t i = 0; i < size; i++) {
        if(data[i] == 0xA) {
            count++;
        }
        x++;

        if(max_line_size > 0 && (x >= max_line_size)) {
            count++;
            x = 0;
        }
    }

    return count;
}


