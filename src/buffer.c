#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"


int setup_buffer(struct buffer_t* buf, int id) {
    int ok = 0;

    if(!buf) {
        goto error;
    }

    if(buf->lines != NULL) {
        printf("warning: trying to initialize buffer again.\n");
        goto error;
    }

    size_t num_lines = 16;
    size_t mem_size = num_lines * sizeof **buf->lines;

    buf->ready = 0;
    buf->id = id;
    buf->lines = NULL;
    buf->mem_size = 0;
    buf->num_lines = 0;
    buf->num_used_lines = 0;
    buf->cursor_x = 0;
    buf->cursor_y = 0;
    buf->file_opened = 0;
    buf->filename_size = 0;
    buf->current = NULL;

    memset(buf->filename, 0, BUFFER_MAX_FILENAME_SIZE);

    buf->lines = malloc(mem_size);
    if(!buf->lines) {
        fprintf(stderr, "failed to allocate memory for buffer.\n");
        goto error;
    }

    for(size_t i = 0; i < num_lines; i++) {
        if(!(buf->lines[i] = create_string())) {
            fprintf(stderr, "buffer line '%li' is not created going to return 0 at 'setup_buffer'\n", i);
            goto error;
        }
    }

    buf->mem_size = mem_size;
    buf->num_lines = num_lines;
    buf->num_used_lines = 1;

    buf->cursor_x = 0;
    buf->cursor_y = 0;

    buf->current = buf->lines[0];
    buf->ready = 1;

    ok = 1;

error:
    return ok;
}     



void cleanup_buffer(struct buffer_t* buf) {
    if(buf) {
        if(buf->lines) {
            for(size_t i = 0; i < buf->num_lines; i++) {
                cleanup_string(&buf->lines[i]);
            }
            free(buf->lines);
            buf->lines = NULL;
            printf("freed buffer\n");
        }

        buf->ready = 0;
        buf->cursor_x = 0;
        buf->cursor_y = 0;
        buf->num_lines = 0;
        buf->mem_size = 0;
        buf->current = NULL;
    }
}

int buffer_ready(struct buffer_t* buf) {
    int res = 0;
    if(buf) {
        res = (buf->lines && buf->mem_size > 0 && buf->ready);
        buf->ready = res;
    }

    return res;
}


void buffer_clear_all(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        if(!buf->lines) {
            return;
        }

        int id = buf->id;

        cleanup_buffer(buf);
        setup_buffer(buf, id);

    }
}



int buffer_memcheck(struct buffer_t* buf, size_t n) {
    int res = 0;

    if(buffer_ready(buf)) {
        if(n <= buf->num_lines) {
            // TODO resize to smaller size.
            res = 1;
        }
        else {
            struct string_t** nptr = NULL;
            size_t new_size = n + BUFFER_MEMORY_BLOCK_SIZE;

            if(!buf->lines) {
                fprintf(stderr, "FATAL! buf->lines is NULL!\n");
                goto error;
            }

            if(new_size > BUFFER_MAX_SIZE) {
                fprintf(stderr, "at 'buffer_memcheck': new_size is too big, failing.\n");
                goto error;
            }

            nptr = reallocarray(buf->lines, new_size, sizeof **buf->lines);

            if(nptr) {
                buf->lines = nptr;

                for(size_t i = buf->num_lines; i < new_size; i++) {
                    buf->lines[i] = create_string();
                }

                buf->num_lines = new_size;
                buf->mem_size = new_size * sizeof **buf->lines;

                res = 1;
#ifdef BUFFER_PRINT_MEMORY_RESIZE
                printf("resized buffer '%p' memory size to hold %li lines.\n", buf, n);
#endif
            }
            else {
                fprintf(stderr, "failed to resize buffer.\n");
            }
        }
    }

error:
    return res;
}

int buffer_inc_size(struct buffer_t* buf, size_t n) {
    int res = 0;
    if(buffer_ready(buf)) {
        if(buffer_memcheck(buf, buf->num_used_lines + n)) {
            buf->num_used_lines += n;
            res = 1;
        }
    }
    return res;
}


int buffer_dec_size(struct buffer_t* buf, size_t n) {
    int res = 0;
    if(buffer_ready(buf)) {

        if(n > buf->num_used_lines) {
            goto error;
        }

        if(buf->num_used_lines > 1) {
            buf->num_used_lines -= n;
            res = 1;
        }

    }
error:
    return res;
}

void move_cursor_to(struct buffer_t* buf, size_t col, size_t row) {
    if(!buffer_ready(buf)) { return; }

    if(buf->num_used_lines > row) {
        buf->cursor_y = row;
        buf->current = buf->lines[row];

        buf->cursor_x = (buf->current->data_size > col) 
            ? col : buf->current->data_size;
    }

}

void move_cursor(struct buffer_t* buf, int xoff, int yoff) {
    if(!buffer_ready(buf)) { return; }

    if(xoff < 0 && buf->cursor_x == 0) {
        xoff = 0;
    }
    if(yoff < 0 && buf->cursor_y == 0) {
        yoff = 0;
    }
    move_cursor_to(buf, buf->cursor_x+xoff, buf->cursor_y+yoff);
}


size_t buffer_find_last_line(struct buffer_t* buf) {
    size_t res = 0;
    if(!buffer_ready(buf)) {
        goto error;
    }

    // TODO should only go from buf->num_used_lines.

    size_t num_used = buf->num_used_lines;

    if(num_used > 0) {
        for(size_t i = num_used-1; i > 0; i--) {
            const struct string_t* s = buf->lines[i];
            if(!s) {
                fprintf(stderr, "warning null string at '%li', buffer '%i'\n",
                        i, buf->id);
                continue;
            }
            if(s->data_size > 0 && s->data) {
                res = i;
                break;
            }
        }
    }
    
error:
    return res;
}


void buffer_shift_data(struct buffer_t* buf, size_t row, int direction) {
    if(!buffer_ready(buf)) { return; }

    size_t max = buffer_find_last_line(buf)+1;
    struct string_t* astr = NULL;
    struct string_t* bstr = NULL;

    if(max >= buf->num_lines) {
        // TODO  umm. ok? try resizing?
        return;
    }

    if(direction == BUFFER_SHIFT_DOWN) {

        for(size_t i = max; i > row; i--) {
            astr = buf->lines[i];
            bstr = buf->lines[i-1];
            if(!astr || !bstr) { 
                continue;
            }
            string_copy_all(astr, bstr);
        }

    }
    else if(direction == BUFFER_SHIFT_UP && row > 0) {
        
        for(size_t i = row-1; i < max; i++) {

            astr = buf->lines[i-1];
            bstr = buf->lines[i];
      
            if(astr->data_size > 0) {
                string_move_data(astr, bstr, astr->data_size, 0, 
                        bstr->data_size, STRING_ZERO_SRC);
            }
            else {
                string_copy_all(astr, bstr);
            }
            string_cut_data(bstr, 0, bstr->data_size);
        }

    }
}

int buffer_add_newline(struct buffer_t* buf, size_t col, size_t row) {
    int ok = 0;
    if(!buffer_ready(buf)) {
        goto error;
    }

    if(row >= buf->num_lines) {
        goto error;
    }

    if(!buffer_inc_size(buf, 1)) {
        goto error;
    }

    struct string_t* current = buf->lines[row];
    struct string_t* below = buffer_get_string(buf, buf->cursor_y+1);

    if(!current || !below) {
        buf->ready = 0;
        goto error;
    }

    if(current == below) {
        goto error;
    }
    

    buffer_shift_data(buf, row, BUFFER_SHIFT_DOWN);

    size_t num_tabs = 0;

    if(current->data_size > 0) {
        
        string_cut_data(current, col, current->data_size - col);
        string_cut_data(below, 0, col);

        num_tabs = string_num_chars(current, 0, current->data_size, '\t');
        for(size_t i = 0; i < num_tabs; i++) {
            string_add_char(below, '\t', 0);
        }
    }

    
    move_cursor_to(buf, num_tabs, buf->cursor_y+1);
    ok = 1;

error:
    return ok;
}

struct string_t* buffer_get_string(struct buffer_t* buf, size_t index) {
    struct string_t* str = buf->current;

    if(buffer_ready(buf)) {
        if((index < buf->num_used_lines) && (buf->num_lines >= buf->num_used_lines)) {
            str = buf->lines[index];
        }
    }

    return str;
}






