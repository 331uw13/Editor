
#include <stdio.h> // NOTE: remove after use <--
#include <GLFW/glfw3.h>


#include "editor.h"
#include "file_io.h"


void _handle_enter_key(struct buffer_t* buf) {
    if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y)) {
        //move_cursor_to(buf, 0, buf->cursor_y+1);
    }
}

void _handle_backspace_key(struct buffer_t* buf) {
    if(buf->cursor_x > 0) {
        if(string_rem_char(buf->current, buf->cursor_x)) {
            move_cursor(buf, -1, 0);
        }
    }
    else if(buf->cursor_y > 0) {
        size_t size = buf->lines[buf->cursor_y-1]->data_size;
        buffer_shift_data(buf, buf->cursor_y+1, BUFFER_SHIFT_UP);
        move_cursor_to(buf, size, buf->cursor_y-1);
        buffer_dec_size(buf, 1);
    }

}


void _key_input_CTRL(struct editor_t* ed, struct buffer_t* buf, int key) {

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


    }
}

void _key_input_ALT(struct editor_t* ed, struct buffer_t* buf, int key) {
    switch(key) {
        case GLFW_KEY_C:
            clear_error_buffer(ed);
            break;
    }
}

void key_input_handler(GLFWwindow* win, int key, int scancode, int action, int mods) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    
    if(!ed) { return; }
    if(action == GLFW_RELEASE) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_buffer];


    struct string_t* tmp = buf->current;

    if(mods == 0) {
        switch(key) {

            case GLFW_KEY_HOME:
                buffer_clear_all(buf);
                open_file(ed, ed->current_buffer, "testfile");
                close_file(ed, ed->current_buffer);
                break;

            case GLFW_KEY_D:
                if(mods == GLFW_MOD_CONTROL) {
                    move_cursor_to(buf, buf->current->data_size, buf->cursor_y);
                }
                break;

            case GLFW_KEY_A:
                if(mods == GLFW_MOD_CONTROL) {
                    move_cursor_to(buf, 0, buf->cursor_y);
                }
                break;

            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(win, 1);
                break;

            case GLFW_KEY_BACKSPACE:
                _handle_backspace_key(buf);
                break;

            case GLFW_KEY_ENTER:
                _handle_enter_key(buf);
                break;
     

            case GLFW_KEY_TAB:
                string_add_char(buf->current, '\t', buf->cursor_x);
                move_cursor(buf, 1, 0);
                break;

     // cursor movement
     
            case GLFW_KEY_LEFT:
                move_cursor(buf, -1, 0);
                break;
            
            case GLFW_KEY_RIGHT:
                move_cursor(buf, 1, 0);
                break;
            
            case GLFW_KEY_UP:
                move_cursor(buf, 0, -1);
                break;
            

            case GLFW_KEY_DOWN:
                move_cursor(buf, 0, 1);
                break;


            default: break;
        }
    }
    else if(mods > 0) {
        if(mods == GLFW_MOD_CONTROL) {
            _key_input_CTRL(ed, buf, key);
        }
        else if(mods == GLFW_MOD_ALT) {
            _key_input_ALT(ed, buf, key);
        }
    }


}


void char_input_handler(GLFWwindow* win, unsigned int codepoint) {
    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(!ed) { return; }

    struct buffer_t* buf = &ed->buffers[ed->current_buffer];

    string_add_char(buf->current, codepoint, buf->cursor_x);
    move_cursor(buf, 1, 0);


    //printf("  '%c' | 0x%x\n", codepoint, codepoint);

}

