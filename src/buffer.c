#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "memory.h"
#include "utils.h"
#include "editor.h"

int create_buffer(struct editor_t* ed, struct buffer_t* buf, int id) {
    int ok = 0;

    if(!buf) {
        goto error;
    }

    if(buf->lines != NULL) {
        fprintf(stderr, "[ERROR] %s | trying to initialize buffer again.\n",
                __func__);
        goto error;
    }

    size_t allocated_lines = BUFFER_INIT_SIZE;
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

    buf->width = ed->window_width; 
    buf->height = ed->window_height - CELLH;
    buf->max_col = buf->width / CELLW;
    buf->max_row = buf->height / CELLH;

    buf->select = (struct select_t) { 0, 0, 0, 0, 0, 0 };
    buf->file = (struct buffer_file_t) { {0}, 0, 0, 0};
    
    buf->prev_mode = BUFMODE_INSERT;
    buf->mode = BUFMODE_INSERT;
    buffer_change_mode(ed, buf, BUFMODE_INSERT);

    buf->lines = malloc(mem_size);
    if(!buf->lines) {
        fprintf(stderr, "[ERROR] %s | failed to allocate memory for buffer.\n",
                __func__);
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

void buffer_init_select(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        buf->select = (struct select_t) {
            .x0 = buf->cursor_x,
            .y0 = buf->cursor_y,
            .x1 = buf->cursor_x,
            .y1 = buf->cursor_y,
            .inverted = 0,
            .scroll_point = buf->scroll
        };
    }
}

void buffer_change_mode(struct editor_t* ed, struct buffer_t* buf, unsigned int bufmode) {
    if(bufmode >= BUFMODE_INVALID) {
        fprintf(stderr, "invalid mode(%i) for buffer %i\n", bufmode, buf->id);
        return;
    }

    if(buf->mode != BUFMODE_NONE) {
        buf->prev_mode = buf->mode;
    }
    buf->mode = bufmode;
    memmove(buf->mode_indicstr,
            BUFFER_MODE_INDICATORS[bufmode],
            BUFFER_MODE_INDICSIZE
            );

    switch(buf->mode) {

        case BUFMODE_SELECT:
            buffer_init_select(buf);
            buf->cursor_color = ed->colors[SELECT_CURSOR_COLOR];
            buf->cursor_charcolor = ed->colors[SELECT_CURSORCHAR_COLOR];
            break;
     
        case BUFMODE_BLOCKSLCT:
            buffer_init_select(buf);
            buf->cursor_color = ed->colors[BLOCKSLCT_CURSOR_COLOR];
            buf->cursor_charcolor = ed->colors[BLOCKSLCT_CURSORCHAR_COLOR];
            break;
        
        case BUFMODE_INSERT:
            buf->cursor_color = ed->colors[INSERT_CURSOR_COLOR];
            buf->cursor_charcolor = ed->colors[INSERT_CURSORCHAR_COLOR];
            break;
        
        case BUFMODE_REPLACE:
            buf->cursor_color = ed->colors[REPLACE_CURSOR_COLOR];
            buf->cursor_charcolor = ed->colors[REPLACE_CURSORCHAR_COLOR];
            break;
        
        case BUFMODE_NONE:
            buf->cursor_color = ed->colors[NONEMODE_CURSOR_COLOR];
            buf->cursor_charcolor = ed->colors[NONEMODE_CURSORCHAR_COLOR];
            break;

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

        if(buf->lines == NULL) {
            fprintf(stderr, "[ERROR] %s | buf->lines == NULL\n",
                    __func__);
            goto giveup;
        }

        if(buf->num_alloc_lines == 0) {
            fprintf(stderr, "[ERROR] %s | buf->num_alloc_lines == 0\n",
                    __func__);
            goto giveup;
        }

        if(buf->ready == 0) {
            fprintf(stderr, "[ERROR] %s | buf->ready == 0\n",
                    __func__);
            goto giveup;
        }

        if(buf->num_used_lines > buf->num_alloc_lines) {
            fprintf(stderr, "[ERROR] %s | buf->num_used_lines > buf->num_alloc_lines\n",
                    __func__);
            goto giveup;
        }

        res = 1;

        /*
        res = (    buf->lines != NULL 
                && buf->num_alloc_lines > 0 
                && buf->ready
                && buf->num_used_lines <= buf->num_alloc_lines
                );
        buf->ready = res;
        */
giveup:
        buf->ready = res;
    }


    return res;
}


int buffer_reset_data(struct buffer_t* buf) {
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
        buf->num_alloc_lines = BUFFER_INIT_SIZE;
        
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
    }

error:
    return res;
}

int buffer_memcheck(struct buffer_t* buf, size_t n) {
    int res = 0;

    if(buffer_ready(buf)) {
        if(n > buf->num_alloc_lines) {

            long int nsize = 0;

            if(!(buf->lines = (struct string_t**)safe_resize_array(
                    buf->lines, sizeof **buf->lines,
                    buf->num_alloc_lines, n,
                    &nsize
                    ))) {
                goto error;
            }
            if(nsize == MEMRESIZE_ERROR) {
                goto error;
            }

            for(size_t i = buf->num_alloc_lines; i < nsize; i++) {
                buf->lines[i] = create_string(0);
            }

            printf("\033[32m resized buffer(%i) '%p' to hold %li lines\033[0m\n", 
                    buf->id, buf, nsize);

            buf->num_alloc_lines = nsize;

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
        buf->scroll = liclamp(y, 0, buf->num_used_lines);
        buf->prev_scroll = buf->scroll;
    }
}

void buffer_scroll(struct buffer_t* buf, int offset) {
    buffer_scroll_to(buf, buf->scroll + offset);
}

void buffer_update_selected(struct buffer_t* buf) {
    if(buffer_ready(buf)) {

        if(!buf->select.inverted) {
            buf->select.x1 = buf->cursor_x;
            buf->select.y1 = buf->cursor_y;
        }
        else {
            buf->select.x0 = buf->cursor_x;
            buf->select.y0 = buf->cursor_y;
        }
    }
}

void buffer_swap_selected(struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        long int tmp = 0;

        tmp = buf->select.y0;
        buf->select.y0 = buf->select.y1;
        buf->select.y1 = tmp;

        tmp = buf->select.x0;
        buf->select.x0 = buf->select.x1;
        buf->select.x1 = tmp;

        buf->select.inverted =! buf->select.inverted;
    }
}

int buffer_remove_selected(struct buffer_t* buf) {
    int ok = 0;
    if(buffer_ready(buf)) {
        
        buffer_proc_selected_reg(buf, NULL, remove_selected_reg_callback);

        const int gap = buf->select.y1 - buf->select.y0;

        struct string_t* startstr = buffer_get_string(buf, buf->select.y0);
        struct string_t* endstr   = buffer_get_string(buf, buf->select.y1);

        if(!startstr || !endstr) { 
            goto error;
        }

        if(gap > 0) {
            if(!string_move_data(
                        startstr,
                        endstr,
                        startstr->data_size, // dst_offset
                        0, // src_offset
                        endstr->data_size,
                        STRING_ZERO_SRC)) {
                goto error;
            }

            buffer_remove_lines(buf, buf->select.y0+1, gap);
        
            if(!buf->select.inverted) {
                buffer_scroll_to(buf, buf->select.scroll_point);
            }

        }
        
        move_cursor_to(buf, buf->select.x0, buf->select.y0);
        ok = 1;
    }
error:
    return ok;
}

void buffer_proc_selected_reg(
        struct buffer_t* buf, void* userptr,
        int(*callback)(struct buffer_t*, struct string_t*, size_t, int, void*))
{

    if(!buffer_ready(buf) || (callback == NULL)) {
        return;
    }


    if((buf->select.y1 < buf->select.y0)
    || ((buf->select.x1 < buf->select.x0) && (buf->select.y1 == buf->select.y0))) {
        buffer_swap_selected(buf);
    }

    
    long int a = buf->select.y0;
    long int b = buf->select.y1;

    long int gap = b - a;
    long int end = a + gap;

    for(long int i = a; i <= end; i++) {
        struct string_t* line = buffer_get_string(buf, i);
        if(!line) {
            continue;
        }

        int flag = 0;
        if(i == a) {
            flag |= PROCSELECTED_BEGIN;
        }
        if(i == end) {
            flag |= PROCSELECTED_END;
        }

        if(!callback(buf, line, i, flag, userptr)) {
            break;
        }
    }
    

}

void buffer_copy_selected(struct editor_t* ed, struct buffer_t* buf) {
    if(buffer_ready(buf)) {
        string_clear_data(ed->clipbrd);
        buffer_proc_selected_reg(buf, ed, copy_selected_reg_callback);

        string_app_char(ed->clipbrd, '\0');
        buffer_change_mode(ed, buf, BUFMODE_INSERT);
    }
}

void buffer_paste_clipboard(struct editor_t* ed, struct buffer_t* buf) {
    if(buffer_ready(buf) && ed->clipbrd->data_size > 0) {
        
        struct string_t* target = buf->current;
        long int x = buf->cursor_x;
        long int y = buf->cursor_y;
        //const long int x_origin = x;

        for(size_t i = 0; i < ed->clipbrd->data_size; i++) {
            char c = ed->clipbrd->data[i];

            if(c == '\n') {
                buffer_add_newline(buf, x, y, BUFADDNL_NO_INDENT);
                target = buffer_get_string(buf, y+1);
                if(!target) {
                    break;
                }

                y++;
                x = 0;
                continue;
            }

            if(!char_ok(c)) {
                continue;
            }

            if(!string_add_char(target, c, x)) {
                fprintf(stderr, "[ERROR] %s | %x/'%c' to %p failed.",
                        __func__, c, c, target);
                break;
            }

            x++;
        }
    }
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

    if(buf->mode == BUFMODE_SELECT) {
        buffer_update_selected(buf);
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

int buffer_add_newline(struct buffer_t* buf, size_t col, size_t row, int option) {
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


    struct string_t* current = buffer_get_string(buf, row);
    struct string_t* below = buffer_get_string(buf, buf->cursor_y+1);

    if(!current || !below) {
        buf->ready = 0;
        goto error;
    }
    

    buffer_shift_data(buf, row, BUFFER_SHIFT_DOWN);

    size_t nm = 0;
    if(current->data_size > 0) {
        
        string_cut_data(current, col, current->data_size - col);
        string_cut_data(below, 0, col);


        if(option == BUFADDNL_USE_INDENT) {
            for(size_t i = 0; i < current->data_size; i++) {
                char c = current->data[i];
                if(c != 0x20) {
                    break;
                }

                nm++;
                string_add_char(below, 0x20, 0);
            }
        }
    }
    
    buf->cursor_px = 0;

    buffer_update_content_xoff(buf);
    move_cursor_to(buf, nm, buf->cursor_y+1);
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

int buffer_remove_lines(struct buffer_t* buf, size_t row, size_t n) {
    int ok = 0;

    for(size_t i = 0; i < n; i++) {
        buffer_remove_line(buf, row);
    }

    return ok;
}

int buffer_backspace(struct buffer_t* buf) {
    int ok = 0;

    if(buf->cursor_x > 0) {

        size_t idx = (buf->cursor_x <= FONT_TAB_WIDTH) ? 0 : (buf->cursor_x - FONT_TAB_WIDTH);
        size_t num_spaces = string_num_chars(buf->current, idx, buf->cursor_x, 0x20);

        buf->cursor_px = 0;

        if(num_spaces == FONT_TAB_WIDTH && is_on_end_of_tab(buf->cursor_x)) {
            string_cut_data(buf->current, idx, num_spaces);
            move_cursor(buf, -FONT_TAB_WIDTH, 0);
        }
        else {
            string_rem_char(buf->current, buf->cursor_x);
            move_cursor(buf, -1, 0);
        }
    }
    else if(buf->cursor_y > 0) {
        struct string_t* ln = buffer_get_string(buf, buf->cursor_y-1);
        if(!ln) {
            goto error;
        }
        
        size_t lnsize = ln->data_size;

        buffer_shift_data(buf, buf->cursor_y+1, BUFFER_SHIFT_UP);
        buffer_dec_size(buf, 1);
        move_cursor_to(buf, lnsize, buf->cursor_y - 1);
    }

    buffer_update_content_xoff(buf);

    ok = 1;
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



