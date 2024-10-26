#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#include "editor.h"
#include "command_line.h"
#include "file.h"
#include "utils.h"

#include "config.h"


static void handle_enter_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y)) {
        if(buf->cursor_y > (buf->scroll + ed->max_row)) {
            buffer_scroll(buf, -1);
        }
    }
}

static void handle_backspace_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buf->cursor_x > 0) {


        size_t idx = (buf->cursor_x <= FONT_TAB_WIDTH) ? 0 : (buf->cursor_x - FONT_TAB_WIDTH);
        size_t num_spaces = string_num_chars(buf->current, idx, buf->cursor_x, 0x20);

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
            write_file(ed, buf->id);
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

        case GLFW_KEY_UP:
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, 0);
            break;
        
        case GLFW_KEY_DOWN: 
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, buf->num_used_lines-1);
            break;

        case GLFW_KEY_S:
            {
                if(!string_ready(buf->current)) {
                    return;
                }
                buf->select = (struct select_t) {
                    buf->cursor_x, buf->cursor_y, // begin position
                    buf->cursor_x, buf->cursor_y, // end position
                    buf->current, // begin string
                    buf->current, // end string
                };
                
                ed->mode = (ed->mode == MODE_SELECT) ? MODE_NORMAL : MODE_SELECT;
            }
            break;

        default:break;
    }
    // -- ALT
}

static void _key_mod_input_SHIFT(struct editor_t* ed, struct buffer_t* buf, int key) {
    
    ONLY_NORMAL_MODE;

    switch(key) {

        /*
        // skip by 1 word (right)
        //
        case GLFW_KEY_RIGHT:
            move_cursor(buf, string_find_char(buf->current,
                        buf->cursor_x, 0x20, STRFIND_NEXT) + 1, 0);
            break;

            
        // skip by 1 word (left)
        //
        case GLFW_KEY_LEFT:
            { // dont want to go to 0 X position 
              // if the start of the line only contains spaces.
              //
                long int dist = string_find_char(
                        buf->current, buf->cursor_x, 0x20, STRFIND_PREV) + 1;
                long int pos = buf->cursor_x - dist;

                if(pos == 0) {
                    for(size_t i = 0; i < buf->current->data_size; i++) {
                        if(buf->current->data[i] != 0x20) {
                            pos = i;
                            break;
                        }
                    }
                }

                move_cursor_to(buf, pos, MOVCUR_KEEP_Y);
            }
            break;
            */

        /*
        // find non white space. up/down
        //
        case GLFW_KEY_UP:
            {
                if(buf->cursor_y <= 0) { return; }
                struct string_t* str = NULL;
                for(size_t i = buf->cursor_y-1; i > 0; i--) {
                    str = buffer_get_string(buf, i);
                    if(!str) {
                        break;
                    }
                    if(!string_is_whitespace(str) || (i-1 == 0)) {
                        move_cursor_to(buf, MOVCUR_KEEP_X, i);
                        break;
                    }
                }
            }
            break;

        case GLFW_KEY_DOWN:
            {
                if(buf->cursor_y >= buf->num_used_lines) { return; }
                struct string_t* str = NULL;
                for(size_t i = buf->cursor_y+1; i < buf->num_used_lines; i++) {
                    str = buffer_get_string(buf, i);
                    if(!str) {
                        break;
                    }
                    if(!string_is_whitespace(str) || (i+1 == buf->num_used_lines)) {
                        move_cursor_to(buf, MOVCUR_KEEP_X, i);
                        break;
                    }
                }
            }
            break;

            */
    }
    // -- SHIFT
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
        }

        return;
    }

    switch(ed->mode) {

        case MODE_NORMAL: // text edit mode.
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
                        /*
                        if(string_add_char(buf->current, '\t', buf->cursor_x)) {
                            move_cursor(buf, 1, 0);
                        }
                        */
                        break;

                    case GLFW_KEY_DELETE:
                        string_rem_char(buf->current, buf->cursor_x+1);
                        break;

                }
            }
            break;
   
        case MODE_SELECT:
            {
                switch(key) {
                    
                    case GLFW_KEY_ESCAPE:
                        ed->mode = MODE_NORMAL;
                        break;


                    case GLFW_KEY_LEFT:
                        move_cursor(buf, -1, 0);
                        buffer_update_selected(buf);
                        break;

                    case GLFW_KEY_RIGHT:
                        move_cursor(buf, 1, 0);
                        buffer_update_selected(buf);
                        break;

                    case GLFW_KEY_DOWN:
                        move_cursor(buf, 0, 1);
                        buffer_update_selected(buf);
                        break;

                    case GLFW_KEY_UP:
                        move_cursor(buf, 0, -1);
                        buffer_update_selected(buf);
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

        default:
            fprintf(stderr, "warning: current mode is invalid.\n");
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
                if(string_add_char(buf->current, codepoint, buf->cursor_x)) {
                    move_cursor(buf, 1, 0);
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

void mouse_bttn_input_handler(GLFWwindow* win, int button, int action, int mods) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    if(action == GLFW_PRESS) {
        ed->mouse_button = 1;
    }
}



