#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "editor.h"
#include "file.h"
#include "command_line.h"
#include "utils.h"


#define CMD_OPEN 6385555319     // "open <filename>"
#define CMD_WRITE 210732889424  // "write"
#define CMD_QDOT 5863684        // "q." close the editor without asking anything.
#define CMD_OPEN_NEW 7572764151204206 // "open-new" open file to new buffer

#define CMD_TEST1 210728875318  // "test1"
#define CMD_TEST2 210728875319  // "test2"

void execute_cmd(struct editor_t* ed, struct string_t* str) {
    if(!str) { return; }

    if(str->data_size > COMMAND_LINE_MAX_SIZE) {
        str->data_size = COMMAND_LINE_MAX_SIZE;
    }

    clear_error_buffer(ed);

    char args[COMMAND_LINE_MAX_SIZE][COMMAND_LINE_MAX_SIZE];
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

    struct buffer_t* buf = &ed->buffers[ed->current_buf_id];

    if(argc > 0) {

        switch(djb2_hash(args[0])) {
           
            case CMD_TEST1:
                read_file(ed, ed->current_buf_id, "for-testing/another_file.txt\0", 0);
                break;

            case CMD_TEST2:
                read_file(ed, ed->current_buf_id, "for-testing/test.txt\0", 0);
                break;

            case CMD_QDOT:
                glfwSetWindowShouldClose(ed->win, GLFW_TRUE);
                break;

            case CMD_OPEN_NEW:
                if(argc < 2) {
                    write_message(ed, ERROR_MSG, "Usage: open-new <filename>\0");
                    goto done;
                }
                if(ed->num_active_buffers+1 >= MAX_BUFFERS) {
                    write_message(ed, ERROR_MSG, "Oops, max buffers reached.\0");
                    goto done;
                }
                read_file(ed, ed->num_active_buffers++, args[1], strlen(args[1]));
                set_buffer_dimensions(ed);
                break;

            case CMD_OPEN:
                if(argc < 2) {
                    write_message(ed, ERROR_MSG, "Usage: open <filename>\0");
                    goto done;
                }
                read_file(ed, ed->current_buf_id, args[1], strlen(args[1]));
                break;

            case CMD_WRITE:
                if(argc < 2) {
                    write_file(ed, ed->current_buf_id, NULL);
                }
                else if (argc == 2) {
                    if(access(args[1], F_OK) != 0) {
                        if(confirm_user_choice(ed, "Create new file?") == USER_ANSWER_NO) {
                            goto done;
                        }
                    }
                    write_file(ed, ed->current_buf_id, args[1]);
                }
                break;



            default:
                write_message(ed, ERROR_MSG, "Command '%s' not found.\n", args[0]);
                break;

        }
    }

done:


    ed->mode = MODE_NORMAL;

    string_clear_data(str);
    ed->cmd_cursor = 0;
}



