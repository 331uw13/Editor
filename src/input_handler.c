#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#include "editor.h"
#include "command_line.h"
#include "file.h"
#include "utils.h"

#include "config.h"


static void handle_enter_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buf->mode != BUFMODE_INSERT
    && buf->mode != BUFMODE_REPLACE) {
        return;
    }
    if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y)) {
        if(buf->cursor_y > (buf->scroll + ed->max_row)) {
            buffer_scroll(buf, -1);
        }
    }
}

static void handle_backspace_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buf->mode != BUFMODE_INSERT
    && buf->mode != BUFMODE_REPLACE) {
        return;
    }
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
            return;
        }
        
        size_t lnsize = ln->data_size;

        buffer_shift_data(buf, buf->cursor_y+1, BUFFER_SHIFT_UP);
        buffer_dec_size(buf, 1);
        move_cursor_to(buf, lnsize, buf->cursor_y - 1);
    }
}


#define ONLY_NORMAL_MODE if(ed->mode != MODE_NORMAL) { return; }

static void _key_mod_input_CONTROL(struct editor_t* ed, struct buffer_t* buf, int key) {

    ONLY_NORMAL_MODE;

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


        case GLFW_KEY_W:
            write_file(ed, buf->id, NULL);
            break;

        case GLFW_KEY_P:
            ed->mode = MODE_COMMAND_LINE;
            break;

        case GLFW_KEY_X:
            clear_error_buffer(ed);
            break;

        case GLFW_KEY_E:
            if(buffer_remove_line(buf, buf->cursor_y)) {
                buf->cursor_x = 0;
            }
            break;

        case GLFW_KEY_TAB:
            if(ed->current_buf_id+1 >= ed->num_active_buffers) {
                ed->current_buf_id = 0;
            }
            else {
                ed->current_buf_id++;
            }
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


static void _key_mod_input_ALT(struct editor_t* ed, struct buffer_t* buf, int key) {
    switch(key) {

        /*
        case GLFW_KEY_UP:
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, 0);
            break;
        
        case GLFW_KEY_DOWN: 
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, buf->num_used_lines-1);
            break;

TODO:   CTRL+SHIFT  up/down

            */

        case GLFW_KEY_V:
            buffer_change_mode(buf, BUFMODE_SELECT);
            buf->select = (struct select_t) {
                .x0 = buf->cursor_x,
                .y0 = buf->cursor_y,
                .x1 = buf->cursor_x,
                .y1 = buf->cursor_y
            };
            break;
        
        case GLFW_KEY_B:
            buffer_change_mode(buf, BUFMODE_INSERT);
            break;
        
        case GLFW_KEY_C:
            buffer_change_mode(buf, BUFMODE_REPLACE);
            break;

        default:break;
    }
    // -- ALT
}

static void _key_mod_input_SHIFT(struct editor_t* ed, struct buffer_t* buf, int key) {
    
    ONLY_NORMAL_MODE;

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


static void _key_mod_input_SHIFTCTRL(struct editor_t* ed, struct buffer_t* buf, int key) {
    
    switch(key) {
    

    }

}

void key_input_handler(GLFWwindow* win, int key, int scancode, int action, int mods) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    
    if(action == GLFW_RELEASE) { return; }
    if(!ed) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_buf_id];
    clear_info_buffer(ed);


    if(mods) {


        switch(mods) {
            case GLFW_MOD_SHIFT:
                _key_mod_input_SHIFT(ed, buf, key);
                break;
            
            case GLFW_MOD_ALT:
                _key_mod_input_ALT(ed, buf, key);
                break;
            
            case GLFW_MOD_CONTROL:
                _key_mod_input_CONTROL(ed, buf, key);
                break;

            default:

                if((mods & GLFW_MOD_SHIFT) && (mods & GLFW_MOD_CONTROL)) {
                _key_mod_input_SHIFTCTRL(ed, buf, key);
            }
                break;
        }
        return;  // TODO:  check multiple mod.
    }


    switch(ed->mode) {

        case MODE_NORMAL:
            {
                
                switch(key) {

                    case GLFW_KEY_LEFT:
                        move_cursor(buf, -1, 0);
                        break;

                    case GLFW_KEY_RIGHT:
                        move_cursor(buf, 1, 0);
                        break;

                    case GLFW_KEY_DOWN:
                        move_cursor(buf, 0, 1);
                        break;

                    case GLFW_KEY_UP:
                        move_cursor(buf, 0, -1);
                        break;

                    case GLFW_KEY_ESCAPE:
                        buffer_change_mode(buf, BUFMODE_INSERT);
                        break;
                        


                    case GLFW_KEY_ENTER:
                        handle_enter_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_BACKSPACE:
                        handle_backspace_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_TAB:
                        for(int i = 0; i < FONT_TAB_WIDTH; i++) {
                            string_add_char(buf->current, 0x20, buf->cursor_x);
                        }
                        move_cursor(buf, FONT_TAB_WIDTH, 0);
                        break;

                    case GLFW_KEY_DELETE:
                        string_rem_char(buf->current, buf->cursor_x+1);
                        break;

                }
            }
            break;
   
        case MODE_COMMAND_LINE:
            {
                switch(key) {
 
                    case GLFW_KEY_ESCAPE:
                        ed->mode = MODE_NORMAL;
                        break;
  
                    case GLFW_KEY_LEFT:
                        if(ed->cmd_cursor > 0) {
                            ed->cmd_cursor--;
                        }
                        break;

                    case GLFW_KEY_RIGHT:
                        if((ed->cmd_cursor+1) <= ed->cmd_str->data_size) {
                            ed->cmd_cursor++;
                        }
                        break;

                    case GLFW_KEY_ENTER:
                        execute_cmd(ed, ed->cmd_str);
                        break;

                    case GLFW_KEY_BACKSPACE:
                        if(ed->cmd_cursor > 0) {
                            if(string_rem_char(ed->cmd_str, ed->cmd_cursor)) {
                                ed->cmd_cursor--;
                            }
                        }
                        break;
                }
            }
            break;
    }
}


static int procselected_delete_CALLBACK
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr)
{
    int res = 1;

    const int isbegin = (flag & PROCSELECTED_BEGIN);
    const int isend   = (flag & PROCSELECTED_END);

    if(isbegin && isend) {
        
        long int len = buf->select.x1 - buf->select.x0;
        if(len > 0) {
            res = string_cut_data(line, buf->select.x0, len);
        }
    }
    else {

        if(isbegin) {
            res = string_cut_data(line, buf->select.x0,  (line->data_size - buf->select.x0));
        }
        else if(isend) {
            res = string_cut_data(line, 0, buf->select.x1);
        }
        else {
            res = string_clear_data(line);
        }
    }

done:
    return res;
}

static void select_mode_keypress(struct editor_t* ed, struct buffer_t* buf, int codepoint) {

    switch(codepoint) {
    
        case 'd':
            {
                buffer_proc_selected_reg(buf, NULL, procselected_delete_CALLBACK);
                buffer_change_mode(buf, BUFMODE_INSERT);

                // a bit hacky but will do for now...
                
                const long int deleted = buf->select.y1 - buf->select.y0;
                if(deleted > 0) {

                    struct string_t* startstr = buffer_get_string(buf, buf->select.y0);
                    struct string_t* endstr   = buffer_get_string(buf, buf->select.y1);
                    


                    if(!startstr || !endstr) {
                        return;
                    }
                    
                    string_move_data(
                            startstr,
                            endstr,
                            startstr->data_size, // dst offset
                            0, // src offset
                            endstr->data_size,
                            STRING_ZERO_SRC
                            );

                    buffer_remove_lines(buf, buf->select.y0+1, deleted);
                    /*
                    for(size_t i = 0; i < deleted; i++) {
                        buffer_remove_line(buf, buf->select.y0+1);  // TODO function for this
                    }
                    */

                }

                move_cursor_to(buf, buf->select.x0, buf->select.y0);

            }
            break;

    }
}

void char_input_handler(GLFWwindow* win, unsigned int codepoint) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    if(!char_ok(codepoint)) {
        return;
    }

    switch(ed->mode) {

        case MODE_NORMAL:
            {
                struct buffer_t* buf = &ed->buffers[ed->current_buf_id];
                if(!buffer_ready(buf)) {
                    return;
                }

                switch(buf->mode) {
                    case BUFMODE_SELECT:
                        {
                            select_mode_keypress(ed, buf, codepoint);
                        }
                        break;

                    case BUFMODE_INSERT:
                        {
                            if(string_add_char(buf->current, codepoint, buf->cursor_x)) {
                                move_cursor(buf, 1, 0);
                            }
                        }
                        break;

                    case BUFMODE_REPLACE:
                        {
                            if(string_set_char(buf->current, codepoint, buf->cursor_x)) {
                                move_cursor(buf, 1, 0);
                            }
                        }
                        break;
                }
            }
            break;

        case MODE_COMMAND_LINE:
            {
                if(ed->cmd_str->data_size+1 >= COMMAND_LINE_MAX_SIZE) {
                    return;
                }
                if(string_add_char(ed->cmd_str, codepoint, ed->cmd_cursor)) {
                    ed->cmd_cursor++;
                }
            }
            break;
    }
}

void scroll_input_handler(GLFWwindow* win, double xoff, double yoff) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_buf_id];
    int iyoff = (int)-yoff;
    
    buffer_scroll(buf, iyoff);
    move_cursor(buf, 0, iyoff);
}

/*
void mouse_bttn_input_handler(GLFWwindow* win, int button, int action, int mods) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    if(action == GLFW_PRESS) {
        ed->mouse_button = 1;
    }
}
*/



