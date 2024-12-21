#include "common.h"
#include "../editor.h"
#include "../file.h"

void keypress_ctrlmod(struct editor_t* ed, struct buffer_t* buf, int key);
void keypress_shiftmod(struct editor_t* ed, struct buffer_t* buf, int key);


void bufmode_common_cursormov(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        )
{

    if(mods) {
        switch(mods) {
            case GLFW_MOD_SHIFT:
                keypress_shiftmod(ed, buf, key);
                break;
            
            case GLFW_MOD_CONTROL:
                keypress_ctrlmod(ed, buf, key);
                break;
    
        }

        return;
    }

    switch(key) {
        
        case GLFW_KEY_LEFT:
            move_cursor(buf, -1, 0);
            break;
        
        case GLFW_KEY_RIGHT:
            move_cursor(buf,  1, 0);
            break;
        
        case GLFW_KEY_DOWN:
            move_cursor(buf,  0, 1);
            break;
        
        case GLFW_KEY_UP:
            move_cursor(buf,  0, -1);
            break;

    }

}

void keypress_ctrlmod(struct editor_t* ed, struct buffer_t* buf, int key) {

    switch(key) {
            // goto end of the line.
        case GLFW_KEY_D: 
            move_cursor_to(buf, buf->current->data_size, buf->cursor_y);
            break;
        
            // goto middle of the line.
        case GLFW_KEY_S:
            if(buf->current->data_size > 1) {
                move_cursor_to(buf, buf->current->data_size/2, buf->cursor_y);
            }
            break;

            // goto start of the line.
        case GLFW_KEY_A: 
            move_cursor_to(buf, 0, buf->cursor_y);
            break;

        case GLFW_KEY_RIGHT:
            move_cursor(buf, 4, 0);
            break;
        
        case GLFW_KEY_LEFT:
            move_cursor(buf, -4, 0);
            break;

        case GLFW_KEY_UP:
            move_cursor(buf, 0, -4);
            break;
        
        case GLFW_KEY_DOWN:
            move_cursor(buf, 0, 4);
            break;

        default:break;
    }

    // -- CONTROL
}

void keypress_shiftmod(struct editor_t* ed, struct buffer_t* buf, int key) {
    

    switch(key) {

        // skip by 1 word (right)
        //
        case GLFW_KEY_RIGHT:
            {
                size_t where = buf->cursor_x;
                size_t imax = buf->current->data_size;
                size_t jump_to = imax;
                int err = 0;

                for(size_t i = where; i < imax; i++) {
                    char c = string_get_char(buf->current, i);
                    if(c == 0) {
                        err = 1;
                        break;     
                    }
                    if(c == 0x20) {
                        char next_c = string_get_char(buf->current, i+1);
                        if(next_c == 0) {
                            err = 1;
                            break;
                        }

                        if(next_c == 0x20) {
                            continue;
                        }

                        jump_to = i + 1;
                        break;
                    }

                }
                if(!err) {
                    move_cursor_to(buf, jump_to, buf->cursor_y);
                }
           }
            break;

        // skip by 1 word (left)
        //
        case GLFW_KEY_LEFT:
            {

                size_t min_x = string_count_ws_to(buf->current, buf->current->data_size);
                size_t where = buf->cursor_x == buf->current->data_size ? (buf->cursor_x - 1) : buf->cursor_x;
                size_t jump_to = min_x;
                int err = 0;

                for(size_t i = where; i > 0; i--) {
                    char c = string_get_char(buf->current, i);
                    if(c == 0) {
                        err = 1;
                        break;
                    }
                    if(c == 0x20) {
                        char prev_c = string_get_char(buf->current, i-1);
                        if(prev_c == 0) {
                            err = 1;
                            break;
                        }

                        if(prev_c == 0x20) {
                            continue;
                        }

                        jump_to = i - 1;
                        break;
                    }
                }

                if(!err) {
                    move_cursor_to(buf, jump_to, buf->cursor_y);
                }
            }
            break;
           
        // find non white space. up/down
        //
        case GLFW_KEY_UP:
            {
                if(buf->cursor_y > 0) {
                    struct string_t* line = NULL;
                    long int y = buf->cursor_y - 1;
                    int err = 0;

                    for(; y > 0; y--) {
                        line = buffer_get_string(buf, y);
                        if(!line) {
                            err = 1;
                            break;
                        }

                        if(!string_is_data_ws(line)) {
                            break;
                        }
                    }

                    if(!err) {
                        move_cursor_to(buf, buf->cursor_x, y);
                    }
                }
            }
            break;

        case GLFW_KEY_DOWN:
            {
                struct string_t* line = NULL;
                long int y = buf->cursor_y + 1;
                int err = 0;

                for(; y < buf->num_used_lines; y++) {
                    line = buffer_get_string(buf, y);
                    if(!line) {
                        err = 1;
                        break;
                    }

                    if(!string_is_data_ws(line)) {
                        break;
                    }
                }

                if(!err) {
                    move_cursor_to(buf, buf->cursor_x, y);
                }
            }
            break;

    }
 
    // -- SHIFT
}
