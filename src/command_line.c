#include <string.h>

#include "file_io.h"
#include "command_line.h"
#include "utils.h"


#define CMD_OPEN 6385555319 // "open <filename>"
#define CMD_WRITE 210732889424 // "write"
#define CMD_NB 5863637 // "nb" open empty buffer


// TODO: write(ed, bufid, filename, filenamesize);

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


    if(argc > 0) {

        switch(djb2_hash(args[0])) {
            
            case CMD_OPEN:
                if(argc < 2) {
                    write_message(ed, ERROR_MSG, "Usage: open <filename>\0");
                    goto done;
                }
                read_file(ed, ed->current_buffer, args[1], strlen(args[1]));
                break;

            case CMD_WRITE:
                write_file(ed, ed->current_buffer);
                break;

            case CMD_NB:
                if((ed->num_active_buffers+1) < MAX_BUFFERS) {
                    ed->num_active_buffers++;
                }

                break;

            default:
                write_message(ed, ERROR_MSG, "Command '%s' not found.\n", str->data);
                break;

        }
    }

done:


    ed->mode = MODE_NORMAL;

    string_clear_data(str);
    ed->cmd_cursor = 0;
}



