#include <GLFW/glfw3.h>
#include <stdio.h>

#include "editor.h"
#include "command_line.h"
#include "file.h"
#include "utils.h"



static void _handle_enter_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y)) {
        if(buf->cursor_y > (buf->scroll + ed->max_row)) {
            buffer_scroll(buf, -1);
        }
    }
}

static void _handle_backspace_key_on_buffer(struct editor_t* ed, struct buffer_t* buf) {
    if(buf->cursor_x > 0) {
        if(string_rem_char(buf->current, buf->cursor_x)) {
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


static void _key_mod_input_CTRL(struct editor_t* ed, struct buffer_t* buf, int key) {

    switch(key) {
            // goto end of the line.
        case GLFW_KEY_D: 
            if(ed->mode != MODE_NORMAL) { return; }
            move_cursor_to(buf, buf->current->data_size, buf->cursor_y);
            break;
        
            // goto middle of the line.
        case GLFW_KEY_S:
            if(ed->mode != MODE_NORMAL) { return; }
            if(buf->current->data_size > 1) {
                move_cursor_to(buf, buf->current->data_size/2, buf->cursor_y);
            }
            break;

            // goto start of the line.
        case GLFW_KEY_A: 
            if(ed->mode != MODE_NORMAL) { return; }
            move_cursor_to(buf, 0, buf->cursor_y);
            break;

        case GLFW_KEY_W:
            if(ed->mode != MODE_NORMAL) { return; }
            write_file(ed, buf->id);
            break;

        case GLFW_KEY_P:
            ed->mode = (ed->mode == MODE_NORMAL) ? MODE_COMMAND_LINE : MODE_NORMAL;
            break;

        case GLFW_KEY_MINUS:
            font_set_scale(&ed->font, ed->font.scale + 0.1);
            break;

        case GLFW_KEY_0:
            font_set_scale(&ed->font, ed->font.scale - 0.1);
            break;

        case GLFW_KEY_X:
            clear_error_buffer(ed);
            break;


        case GLFW_KEY_E:
            if(buffer_remove_line(buf, buf->cursor_y)) {
                buf->cursor_x = 0;
            }
            break;

        case GLFW_KEY_RIGHT:
            move_cursor(buf, string_find_char(buf->current,
                        buf->cursor_x, 0x20, STRFIND_NEXT) + 1, 0);
            break;

        case GLFW_KEY_LEFT:
            move_cursor(buf, -string_find_char(buf->current,
                        buf->cursor_x, 0x20, STRFIND_PREV) - 1, 0);
            break;


        default:break;
    }
}


void key_input_handler(GLFWwindow* win, int key, int scancode, int action, int mods) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    
    if(action == GLFW_RELEASE) { return; }
    if(!ed) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_buffer];
    clear_info_buffer(ed);


    if(mods) {

        if(mods == GLFW_MOD_CONTROL) {
            _key_mod_input_CTRL(ed, buf, key);
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
                        _handle_enter_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_BACKSPACE:
                        _handle_backspace_key_on_buffer(ed, buf);
                        break;

                    case GLFW_KEY_TAB:
                        if(string_add_char(buf->current, '\t', buf->cursor_x)) {
                            move_cursor(buf, 1, 0);
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

                    case GLFW_KEY_ESCAPE:
                        ed->mode = MODE_NORMAL;
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
                struct buffer_t* buf = &ed->buffers[ed->current_buffer];
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

    struct buffer_t* buf = &ed->buffers[ed->current_buffer];
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



