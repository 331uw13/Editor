#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "utils.h"

int create_buffer(struct buffer_t* buf, int id) {
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
    buf->current = NULL;
    buf->max_col = 0;
    buf->max_row = 0;
    buf->x = 0;
    buf->y = 0;

    buf->content_xoff = 0;
    buf->num_alloc_lines = allocated_lines;
    buf->num_used_lines = 1;
    buf->cursor_x = 0;
    buf->cursor_y = 0;
    buf->cursor_px = 0;
    buf->cursor_py = 0;
    buf->scroll = 0;
    buf->prev_scroll = 0;
    buf->ready = 1;

    buf->max_row = 1;  // these are set in 'editor.c' set_buffer_dimensions()
    buf->max_col = 1;  // ^
    buf->width = 64;   // ^
    buf->height = 64;  // ^

    buf->select = (struct select_t) { 0, 0, 0, 0, NULL, NULL };
    buf->file = (struct buffer_file_t) { 0, {0}, 0, 0};
    
    buffer_change_mode(buf, BUFMODE_INSERT);

    buf->lines = malloc(mem_size);
    if(!buf->lines) {
        fprintf(stderr, "failed to allocate memory for buffer.\n");
        goto error;
    }

    for(size_t i = 0; i < allocated_lines; i++) {
        if(!(buf->lines[i] = create_string(0))) {
            fprintf(stderr, "buffer line '%li' is not created going to return 0 at 'setup_buffer'\n", i);
            goto error;
        }
    }
    
    buf->current = buf->lines[0];
    buffer_update_content_xoff(buf);

    ok = 1;
    printf("buffer %i ready. %p\n", buf->id, buf->lines);

error:
    return ok;
}     

void delete_buffer(struct buffer_t* buf) {
    if(buf) {
        if(buf->lines) {
            for(size_t i = 0; i < buf->num_alloc_lines; i++) {
                delete_string(&buf->lines[i]);
            }
            free(buf->lines);
            buf->lines = NULL;
            printf(" deleted buffer %i data.\n", buf->id);
        }

        buf->scroll = 0;
        buf->prev_scroll = 0;
        buf->file.opened = 0;
        buf->ready = 0;
        buf->cursor_x = 0;
        buf->cursor_y = 0;
        buf->num_used_lines = 0;
        buf->num_alloc_lines = 0;
        buf->id = 0;
        buf->current = NULL;
    }
}

void buffer_change_mode(struct buffer_t* buf, unsigned int bufmode) {
    if(bufmode >= BUFMODE_INVALID) {
        fprintf(stderr, "invalid mode(%i) for buffer %i\n", bufmode, buf->id);
        return;
    }

    buf->mode = bufmode;
    memmove(buf->mode_indicstr,
            BUFFER_MODE_INDICATORS[bufmode],
            BUFFER_MODE_INDICSIZE
            );
}


void buffer_update_content_xoff(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        char tmpbuf[LINENUM_BUF_SIZE] = {0};
        buf->content_xoff = snprintf(tmpbuf, LINENUM_BUF_SIZE, 
                "%li", buf->num_used_lines-1)+2;
    }
}


void buffer_update_selected(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        buf->select.x1 = buf->cursor_x;
        buf->select.y1 = buf->cursor_y;
    }
}


int buffer_ready(struct buffer_t* buf) {
    int res = 0;
    if(buf) {
        res = (    buf->lines != NULL 
                && buf->num_alloc_lines > 0 
                && buf->ready
                && buf->num_used_lines <= buf->num_alloc_lines
                );
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

        delete_buffer(buf);
        create_buffer(buf, id);

    }
}

int buffer_clear_all(struct buffer_t* buf) {
    int res = 0;
    if(buffer_ready(buf)) {
        // TODO: no need to free everything. 
        //       BUFFER_MEMORY_BLOCK amount of lines can stay just make them zero

        for(size_t i = 0; i < buf->num_alloc_lines; i++) {
            delete_string(&buf->lines[i]);
        }
        
        free(buf->lines);
        
        buf->lines = NULL;
        buf->num_used_lines = 0;
        buf->num_alloc_lines = BUFFER_MEMORY_BLOCK_SIZE;
        
        buf->scroll = 0;
        buf->prev_scroll = 0;
        buf->cursor_x = 0;
        buf->cursor_y = 0;

        buf->lines = malloc(buf->num_alloc_lines * sizeof **buf->lines);
        if(!buf->lines) {
            buf->num_alloc_lines = 0;
            fprintf(stderr, "'%s': failed to allocate memory for buffer after free.\n",
                    __func__);
            goto error;
        }

        for(size_t i = 0; i < buf->num_alloc_lines; i++) {
            if(!(buf->lines[i] = create_string(0))) {
                fprintf(stderr, "'%s': failed to create string.\n",
                        __func__);
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

            if(n > BUFFER_MAX_SIZE) {
                fprintf(stderr, "buffer %i reached max size.\n", buf->id);
                goto error;
            }

            ptr = reallocarray(buf->lines, nbsize, sizeof **buf->lines);
            if(!ptr) {
                fprintf(stderr, "failed to allocate more memory for buffer.\n");
                goto error;
            }
            
            buf->lines = ptr;

            for(size_t i = buf->num_alloc_lines; i < nbsize; i++) {
                buf->lines[i] = create_string(0);
            }

            buf->num_alloc_lines = nbsize;

            //printf("\033[32m + resized buffer to hold %li lines.\033[0m\n", buf->num_alloc_lines);
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

        buf->prev_scroll = buf->scroll;
    }
}

void buffer_scroll(struct buffer_t* buf, int offset) {
    buffer_scroll_to(buf, buf->scroll + offset);
}

void move_cursor_to(struct buffer_t* buf, long int col, long int row) {
    if(!buffer_ready(buf)) { return; }

    row = liclamp(row, 0, buf->num_used_lines);

    long int moved_up = (row < buf->cursor_y);
    long int moved_down = (row > buf->cursor_y);


    if(row >= (buf->scroll + buf->max_row) && moved_down) {
        long int scr = row - buf->max_row + 1;

        if(scr <= buf->prev_scroll) {
            scr = buf->prev_scroll+1;
        }

        buffer_scroll_to(buf, scr);
    }
    else if(row < buf->scroll && moved_up) {
        buffer_scroll_to(buf, row);
    }

    if(row < buf->num_used_lines) {
        const long int prev_row = buf->cursor_y;

        buf->cursor_y = row;
        buf->current = buf->lines[row];

        if((col == buf->cursor_x)
                && (prev_row != row) 
                && (buf->cursor_px != 0)) {

            col = buf->cursor_px;
        }
        
        buf->cursor_x = liclamp(col, 0, buf->current->data_size);

        const long int len = buf->current->data_size
            - string_count_ws_to(buf->current, buf->current->data_size);

        if((len >= 2) 
                && (string_count_ws_to(buf->current, buf->cursor_x) != buf->cursor_x)) {
            
            buf->cursor_px = buf->cursor_x;
        }
       
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
        if(!buffer_memcheck(buf, max)) {
            return;
        }
    }

    switch(direction) {
        case BUFFER_SHIFT_DOWN:
            {
                for(size_t i = max; i > row; i--) {
                    astr = buffer_get_string(buf, i);
                    bstr = buffer_get_string(buf, i-1);

                    if(!astr || !bstr) {
                        break;
                    }

                    // copy from bstr to astr.
                    string_copy_all(astr, bstr);
                }
            }
            break;

        case BUFFER_SHIFT_UP:
            {
                if(row == 0) {
                    return;
                }
                for(size_t i = row-1; i < max; i++) {
                    astr = buffer_get_string(buf, i-1);
                    bstr = buffer_get_string(buf, i);

                    if(!astr || !bstr)  {
                        break;
                    }

                    if(astr->data_size > 0) {
                        if(!string_move_data(astr, bstr, astr->data_size, 
                                    0, bstr->data_size, STRING_ZERO_SRC))
                        {
                            fprintf(stderr, "WARNING! '%s' string_move_data failed\n",
                                    __func__);
                            return;
                        }
                    }
                    else {
                        string_copy_all(astr, bstr);
                    }
                    string_cut_data(bstr, 0, bstr->data_size);
                }
            }
            break;
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


    struct string_t* current = buffer_get_string(buf, row);//buf->lines[row];
    struct string_t* below = buffer_get_string(buf, buf->cursor_y+1);

    if(!current || !below) {
        buf->ready = 0;
        goto error;
    }
    

    buffer_shift_data(buf, row, BUFFER_SHIFT_DOWN);

    size_t num_tabs = 0;
    if(current->data_size > 0) {
        
        string_cut_data(current, col, current->data_size - col);
        string_cut_data(below, 0, col);


        for(size_t i = 0; i < current->data_size; i++) {
            char c = current->data[i];
            if(c != 0x20) {
                break;
            }

            num_tabs++;
            string_add_char(below, 0x20, 0);
        }
    }
    
    buf->cursor_px = 0;

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
    struct string_t* str = NULL;

    if(buffer_ready(buf)) {
        if((row < buf->num_used_lines) 
                && (buf->num_alloc_lines >= buf->num_used_lines)) {
            
            str = buf->lines[row];
        }
    }

    return str;
}



