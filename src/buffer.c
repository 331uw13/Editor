#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "utils.h"

int setup_buffer(struct buffer_t* buf, int id) {
    int ok = 0;


    if(!buf) {
        goto error;
    }

    if(buf->lines != NULL) {
        fprintf(stderr, "trying to initialize buffer again.\n");
        goto error;
    }

    size_t allocated_lines = BUFFER_MEMORY_BLOCK_SIZE;
    size_t mem_size = allocated_lines * sizeof **buf->lines;

    buf->ready = 0;
    buf->id = id;
    buf->lines = NULL;
    buf->num_alloc_lines = 0;
    buf->num_used_lines = 0;
    buf->cursor_x = 0;
    buf->cursor_y = 0;
    buf->file_opened = 0;
    buf->filename_size = 0;
    buf->current = NULL;
    buf->max_col = 0;
    buf->max_row = 0;
    buf->x = 0;
    buf->y = 0;
    buf->content_xoff = 0;

    memset(buf->filename, 0, BUFFER_MAX_FILENAME_SIZE);

    buf->lines = malloc(mem_size);
    if(!buf->lines) {
        fprintf(stderr, "failed to allocate memory for buffer.\n");
        goto error;
    }

    for(size_t i = 0; i < allocated_lines; i++) {
        if(!(buf->lines[i] = create_string())) {
            fprintf(stderr, "buffer line '%li' is not created going to return 0 at 'setup_buffer'\n", i);
            goto error;
        }
    }

    buf->num_alloc_lines = allocated_lines;
    buf->num_used_lines = 1;

    buf->cursor_x = 0;
    buf->cursor_y = 0;
    buf->scroll = 0;

    buf->current = buf->lines[0];
    buf->ready = 1;

    buf->max_row = 20;
    buf->max_col = 32;

    buffer_update_content_xoff(buf);

    ok = 1;
    printf("buffer %i ready. %p\n", buf->id, buf->lines);

error:
    return ok;
}     



void cleanup_buffer(struct buffer_t* buf) {
    if(buf) {
        if(buf->lines) {
            for(size_t i = 0; i < buf->num_alloc_lines; i++) {
                cleanup_string(&buf->lines[i]);
            }
            free(buf->lines);
            buf->lines = NULL;
            printf(" freed buffer %i data.\n", buf->id);
        }

        buf->scroll = 0;
        buf->file_opened = 0;
        buf->ready = 0;
        buf->cursor_x = 0;
        buf->cursor_y = 0;
        buf->num_used_lines = 0;
        buf->num_alloc_lines = 0;
        buf->id = 0;
        buf->current = NULL;
    }
}

void buffer_update_content_xoff(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        char tmpbuf[LINENUM_BUF_SIZE] = {0};
        buf->content_xoff = snprintf(tmpbuf, LINENUM_BUF_SIZE, 
                "%li", buf->num_used_lines-1)+2;
    }
}

int buffer_ready(struct buffer_t* buf) {
    int res = 0;
    if(buf) {
        res = (buf->lines && buf->num_alloc_lines > 0 && buf->ready);
        buf->ready = res;
    }

    return res;
}

void buffer_reset(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        if(!buf->lines) {
            return;
        }

        int id = buf->id;

        cleanup_buffer(buf);
        setup_buffer(buf, id);

    }
}

int buffer_clear_all(struct buffer_t* buf) {
    int res = 0;
    if(buffer_ready(buf)) {
        // TODO: no need to free everything. 
        //       BUFFER_MEMORY_BLOCK amount of lines can stay just make them zero

        for(size_t i = 0; i < buf->num_alloc_lines; i++) {
            cleanup_string(&buf->lines[i]);
        }
        
        free(buf->lines);
        
        buf->lines = NULL;
        buf->num_used_lines = 0;
        buf->num_alloc_lines = BUFFER_MEMORY_BLOCK_SIZE;
        
        buf->scroll = 0;
        buf->cursor_x = 0;
        buf->cursor_y = 0;

        buf->lines = malloc(buf->num_alloc_lines * sizeof **buf->lines);
        if(!buf->lines) {
            buf->num_alloc_lines = 0;
            fprintf(stderr, "'buffer_clear_all': failed to allocate memory for buffer after free.\n");
            goto error;
        }

        for(size_t i = 0; i < buf->num_alloc_lines; i++) {
            if(!(buf->lines[i] = create_string())) {
                fprintf(stderr, "'buffer_clear_all': failed to create string.\n");
                goto error;
            }
        }

        buf->num_used_lines = 1;
        printf("cleared buffer %i.\n", buf->id);
    }

error:
    return res;
}

int buffer_memcheck(struct buffer_t* buf, size_t n) {
    int res = 0;

    if(buffer_ready(buf)) {
        if(n > buf->num_alloc_lines) {

            size_t nbsize = n + BUFFER_MEMORY_BLOCK_SIZE;
            struct string_t** ptr = NULL;


            ptr = reallocarray(buf->lines, nbsize, sizeof **buf->lines);
            if(!ptr) {
                fprintf(stderr, "failed to allocate more memory for buffer.\n");
                goto error;
            }
            
            buf->lines = ptr;

            for(size_t i = buf->num_alloc_lines; i < nbsize; i++) {
                buf->lines[i] = create_string();
            }

            buf->num_alloc_lines = nbsize;

        
#ifdef BUFFER_PRINT_MEMORY_RESIZE
            printf("\033[32m + resized buffer to hold %li lines.\033[0m\n", buf->num_alloc_lines);
#endif
        }
        res = 1;
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

void buffer_scroll_to(struct buffer_t* buf, size_t y) {
    if(buffer_ready(buf)) {
        if(buf->num_used_lines < buf->max_row) {
            return;
        }
        buf->scroll = liclamp(y, 
                0, buf->num_used_lines - buf->max_row+5);
    }
}

void buffer_scroll(struct buffer_t* buf, int offset) {
    buffer_scroll_to(buf, buf->scroll + offset);
}


void move_cursor_to(struct buffer_t* buf, size_t col, size_t row) {
    if(!buffer_ready(buf)) { return; }

    if(row > (buf->scroll + buf->max_row)-1) {
        buffer_scroll(buf, 1);
    }
    else if(row < (buf->scroll)) {
        buffer_scroll(buf, -1);
    }

    if(row < buf->num_used_lines) {
        buf->cursor_y = row;
        buf->current = buf->lines[row];
        
        buf->cursor_x = 
            (col < buf->current->data_size) 
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

    size_t num_used = buf->num_used_lines;

    if(num_used > 0) {
        for(size_t i = num_used-1; i > 0; i--) {
            const struct string_t* s = buf->lines[i];
            if(!s) {
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

    if(max >= buf->num_alloc_lines) {
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

    if(row >= buf->num_alloc_lines) {
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

    buffer_update_content_xoff(buf);
    move_cursor_to(buf, num_tabs, buf->cursor_y+1);
    ok = 1;

error:
    return ok;
}

int buffer_remove_line(struct buffer_t* buf, size_t row) {
    int ok = 0;

    if(buffer_ready(buf)) {
        if(row >= buf->num_used_lines) {
            goto error;
        }

        struct string_t* str = buffer_get_string(buf, row);
        if(!str) {
            goto error;
        }

        if(string_clear_data(str)) {
            buffer_shift_data(buf, row+1, BUFFER_SHIFT_UP);
            buffer_dec_size(buf, 1);
        
            ok = 1;
        }
    }

error:
    return ok;
}


struct string_t* buffer_get_string(struct buffer_t* buf, size_t row) {
    struct string_t* str = buf->current;

    if(buffer_ready(buf)) {
        if((row < buf->num_used_lines) 
                && (buf->num_alloc_lines >= buf->num_used_lines)) {
            
            str = buf->lines[row];
        }
    }

    return str;
}



