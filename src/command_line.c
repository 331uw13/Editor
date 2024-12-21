#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

#include "editor.h"
#include "file.h"
#include "command_line.h"
#include "utils.h"


#define CMD_LN 5863583
#define CMD_QUIT 6385632776
#define CMD_OPENF 210723325629


void execute_cmd(struct editor_t* ed, struct string_t* str) {
    if(!str) { return; }

    if(str->data_size > CMDSTR_MAX_SIZE) {
        str->data_size = CMDSTR_MAX_SIZE;
    }

    clear_error_buffer(ed);

    char args[CMDSTR_MAX_SIZE][CMDSTR_MAX_SIZE];
    size_t argc = 0;
    size_t j = 0;

    for(size_t i = 0; i < str->data_size; i++) {
        char c = str->data[i];
       
        if(c == 0) {
            goto done;
        }

        if(c != 0x20) {
            args[argc][j] = c;
            j++;
        }

        if((c == 0x20 || (i+1 == str->data_size)) && (j > 0)) {
            args[argc][j] = 0;
            argc++;
            j = 0;
            continue;
        }
    }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];

    if(argc > 0) {
        switch(djb2_hash(args[0])) {

            case CMD_OPENF:
                {
                    if(argc != 2) {
                        write_message(ed, ERROR_MSG, "openf <filepath>");
                        goto done;
                    }

                    printf("%li\n", read_file(ed, buf->id, args[1], strlen(args[1])));
                }
                break;

            case CMD_QUIT:
                glfwSetWindowShouldClose(ed->win, 1);
                break;

            case CMD_LN:
                {
                    if(argc != 2) {
                        write_message(ed, ERROR_MSG, "ln <line number to go>");
                        goto done;
                    }

                    long int ln = atoi(args[1]);
                    ln = liclamp(ln, 0, buf->num_used_lines-1);

                    printf("%li\n", ln);

                    move_cursor_to(buf, buf->cursor_x, ln);
                }
                break;


            default:
                write_message(ed, ERROR_MSG, "Command '%s'\n Not found.\n", args[0]);
                break;

        }
    }

done:


    ed->mode = MODE_NORMAL;

    string_clear_data(str);
    ed->cmd_cursor = 0;
}


void commandline_keypress(struct editor_t* ed, int key, int mods) {
    
    if(((mods & GLFW_MOD_CONTROL) && (key == GLFW_KEY_Q))
    || (key == GLFW_KEY_ESCAPE)) {
        ed->mode = MODE_NORMAL;
        return;
    }

    if((mods & GLFW_MOD_CONTROL) && (key == GLFW_KEY_RIGHT)) {
        ed->cmd_cursor = ed->cmdstr->data_size;
        return;
    }
    if((mods & GLFW_MOD_CONTROL) && (key == GLFW_KEY_LEFT)) {
        ed->cmd_cursor = 0;
        return;
    }
    if((mods & GLFW_MOD_CONTROL) && (key == GLFW_KEY_L)) {
        string_clear_data(ed->cmdstr);
        ed->cmd_cursor = 0;
        return;
    }

    switch(key) {
    
        case GLFW_KEY_LEFT:
            if(ed->cmd_cursor > 0) {
                ed->cmd_cursor--;
            }
            break;

        case GLFW_KEY_RIGHT:
            if(ed->cmd_cursor+1 <= ed->cmdstr->data_size) {
                ed->cmd_cursor++;
            }
            break;


        case GLFW_KEY_BACKSPACE:
            if(ed->cmd_cursor > 0) {
                if(string_rem_char(ed->cmdstr, ed->cmd_cursor)) {
                    ed->cmd_cursor--;
                }
            }
            break;

        case GLFW_KEY_ENTER:
            execute_cmd(ed, ed->cmdstr);
            ed->mode = MODE_NORMAL;
            break;
    }


}

void commandline_charinput(struct editor_t* ed, unsigned char codepoint) {
    if(!char_ok(codepoint)) {
        return;
    }


    if(ed->cmdstr->data_size+1 < CMDSTR_MAX_SIZE) {
        if(string_add_char(ed->cmdstr, codepoint, ed->cmd_cursor)) {
            ed->cmd_cursor++;
        }
    }


}






