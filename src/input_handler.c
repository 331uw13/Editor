#include <GLFW/glfw3.h>
#include <stdio.h>
//#include <math.h>

#include "editor.h"
//#include "file.h"
//#include "utils.h"
//#include "selectreg.h"
//#include "config.h"


#include "bufmodes/nonemode.h"
#include "bufmodes/insert.h"
#include "bufmodes/replace.h"
#include "bufmodes/select.h"
#include "command_line.h"

/*   
--->
     Everything from here redirects commands
     to src/bufmodes/something  OR  src/editormodes/something
    
     from there the functions for the corresponding mode are done.

*/



void char_input_handler(GLFWwindow* win, unsigned int codepoint) {
    struct editor_t* ed = NULL;
    ed = glfwGetWindowUserPointer(win);

    if(!ed) {
        fprintf(stderr, "[ERROR] %s | pointer is missing.\n",
                __func__);
        return;
    }
    
    clear_info_buffer(ed);
    
    if(ed->mode == MODE_CMDL) {
        commandline_charinput(ed, codepoint);
        return;
    }

    if(ed->mode != MODE_NORMAL) {
        return;
    }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    if(!buffer_ready(buf)) {
        // error message is printed from the function above.
        return;
    }

    switch(buf->mode) {

        case BUFMODE_NONE:
            bufmode_nonemode_charinput(ed, buf, codepoint);
            break;

        case BUFMODE_INSERT:
            bufmode_insert_charinput(ed, buf, codepoint);
            break;

        case BUFMODE_REPLACE:
            bufmode_replace_charinput(ed, buf, codepoint);
            break;
    
        case BUFMODE_SELECT:
            bufmode_select_charinput(ed, buf, codepoint);
            break;
    }

}



void key_input_handler(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if(action == GLFW_RELEASE) {
        return;
    }

    struct editor_t* ed = NULL;
    ed = glfwGetWindowUserPointer(win);
    if(!ed) {
        fprintf(stderr, "[ERROR] %s | pointer is missing.\n",
                __func__);
        return;
    }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    if(!buffer_ready(buf)) {
        // error message is printed from the function above.
        return;
    }


    if(ed->mode == MODE_CMDL) {
        commandline_keypress(ed, key, mods);
        return;
    }

    if(ed->mode != MODE_NORMAL) {
        return;
    }
    
    // FOR TESTING:
    if((mods & GLFW_MOD_CONTROL) && (key == GLFW_KEY_M)) {
        printf("%i\n", confirm_user_choice(ed, "Data may be lost. Continue?\0", PRESELECT_NO));
    }

    if((mods & GLFW_MOD_CONTROL) && (key == ed->keybinds[KB_CMDL])) {
        ed->mode = MODE_CMDL;
        return;
    }

    if(mods == GLFW_MOD_ALT) {
        switch(key) {

            case GLFW_KEY_O:
                clear_error_buffer(ed);
                break;

            case GLFW_KEY_TAB:
                ed->tabs_visible = !ed->tabs_visible;
                break;

            case GLFW_KEY_LEFT:
                if(ed->current_bufid > 0) {
                    ed->current_bufid--;
                }
                break;


            case GLFW_KEY_RIGHT:
                if(ed->current_bufid+1 < ed->num_buffers) {
                    ed->current_bufid++;
                }
                break;
        }
        return;
    }

    if((mods == GLFW_MOD_CONTROL) 
            && (key == ed->keybinds[KB_NONEMODE])) {
        buffer_change_mode(ed, buf, BUFMODE_NONE);
        return;
    }

    switch(buf->mode) {

        case BUFMODE_INSERT:
            bufmode_insert_keypress(ed, buf, key, mods);
            break;

        case BUFMODE_REPLACE:
            bufmode_replace_keypress(ed, buf, key, mods);
            break;

        case BUFMODE_SELECT:
            bufmode_select_keypress(ed, buf, key, mods);
            break;
    }


}










































    /*
static void handle_enter_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buf->mode != BUFMODE_INSERT
    && buf->mode != BUFMODE_REPLACE) {
        return;
    }
    if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y, BUFADDNL_USE_INDENT)) {
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

    buffer_update_content_xoff(buf);
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

        case GLFW_KEY_O:
            clear_error_buffer(ed);
            break;

        case GLFW_KEY_E:
            if(buffer_remove_line(buf, buf->cursor_y)) {
                buf->cursor_x = 0;
            }
            break;

        case GLFW_KEY_TAB:
            if(ed->current_bufid+1 >= ed->num_buffers) {
                ed->current_bufid = 0;
            }
            else {
                ed->current_bufid++;
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

            // sets the mode to none so the user can select new mode.
        case GLFW_KEY_X: 
            buffer_change_mode(ed, buf, BUFMODE_NONE);
            break;

        case GLFW_KEY_V:
            buffer_paste_clipboard(ed, buf);
            break;

        default:break;
    }
    // -- CONTROL
}


static void _key_mod_input_ALT(struct editor_t* ed, struct buffer_t* buf, int key) {
    switch(key) {

        case GLFW_KEY_TAB:
            ed->tabs_visible = !ed->tabs_visible;
            break;

        case GLFW_KEY_LEFT:
            if(ed->current_bufid > 0) {
                ed->current_bufid--;
            }
            break;


        case GLFW_KEY_RIGHT:
            if(ed->current_bufid+1 < ed->num_buffers) {
                ed->current_bufid++;
            }
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
    
        case GLFW_KEY_UP:
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, 0);
            break;
        
        case GLFW_KEY_DOWN: 
            ONLY_NORMAL_MODE;
            move_cursor_to(buf, 0, buf->num_used_lines-1);
            break;


        default:break;
    }
}

void key_input_handler(GLFWwindow* win, int key, int scancode, int action, int mods) {
    struct editor_t* ed = NULL;
    ed = glfwGetWindowUserPointer(win);
    
    if(action == GLFW_RELEASE) { 
        return;
    }

    if(ed == NULL) { 
        fprintf(stderr, "[ERROR] %s | glfwGetWindowUserPointer failed\n",
                __func__);
        return;
    }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    clear_info_buffer(ed);

    if(buf->mode == BUFMODE_NONE) {
        return;
    }


    if(buf->mode == BUFMODE_BLOCKSLCT) {
    }


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
                        buffer_change_mode(ed, buf, BUFMODE_INSERT);
                        break;
                        


                    case GLFW_KEY_ENTER:
                        handle_enter_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_BACKSPACE:
                        handle_backspace_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_TAB:
                        if(buf->mode == BUFMODE_INSERT) {
                            for(int i = 0; i < FONT_TAB_WIDTH; i++) {
                                string_add_char(buf->current, 0x20, buf->cursor_x);
                            }
                            move_cursor(buf, FONT_TAB_WIDTH, 0);
                        }
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


static void select_mode_keypress(struct editor_t* ed, struct buffer_t* buf,  unsigned int codepoint) {

    switch(codepoint) {

        case 'c':
            {
                buffer_copy_selected(ed, buf);
            }
            break;

        case 'd':
            {
                buffer_copy_selected(ed, buf);
                buffer_remove_selected(buf);
            }
            break;

        case 'q':
            buffer_change_mode(ed, buf, buf->prev_mode);
            break;


        case 'r':
            {
                buffer_remove_selected(buf);

                buffer_update_content_xoff(buf);
                move_cursor_to(buf, buf->select.x0, buf->select.y0);

                if(!buf->select.inverted) {
                    buffer_scroll_to(buf, buf->select.scroll_point);
                }

                buffer_change_mode(ed, buf, BUFMODE_INSERT);
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
                struct buffer_t* buf = &ed->buffers[ed->current_bufid];
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

                    case BUFMODE_NONE:
                        {
                            // NONEMODE_COMMANDS

                            switch(codepoint) {

                                case 'd':
                                    buffer_change_mode(ed, buf, BUFMODE_BLOCKSLCT);
                                    break;

                                case 's':
                                    buffer_change_mode(ed, buf, BUFMODE_SELECT);
                                    break;
                                
                                case 'c':
                                    buffer_change_mode(ed, buf, BUFMODE_INSERT);
                                    break;
                                
                                case 'a':
                                    buffer_change_mode(ed, buf, BUFMODE_REPLACE);
                                    break;


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
*/

void scroll_input_handler(GLFWwindow* win, double xoff, double yoff) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    int iyoff = (int)-yoff;
    
    buffer_scroll(buf, iyoff);
    move_cursor(buf, 0, iyoff);
}



