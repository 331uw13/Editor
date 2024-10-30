#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#include "file.h"
#include "editor.h"
#include "utils.h"



size_t read_file(struct editor_t* ed, unsigned int buf_id, char* filename, size_t filename_size) {
    size_t total_bytes_read = 0;

    if(ed) {

        if(!filename) {
            PRINTERR("filename is NULL!");
            goto error;
        }

        if(filename_size == 0) {
            filename_size = strlen(filename);
        }

        if(filename_size >= BUFFER_MAX_FILENAME_SIZE) {
            PRINTERR("filename size is too big");
            goto error;
        }

        if(buf_id >= MAX_BUFFERS) {
            PRINTERR("invalid buffer id");
            goto error;
        }

        struct buffer_t* buf = &ed->buffers[buf_id];

        if(!buf) {
            PRINTERR("buffer is NULL");
            goto error;
        }

        if(!buffer_ready(buf)) {
            goto error;
        }

        if(access(filename, F_OK)) {
            write_message(ed, ERROR_MSG, "File doesnt exist '%s'.\0", filename);
            PRINTERR("failed to access file");
            goto error;
        }


        if(buf->file.opened) {
            
            if(confirm_user_choice(ed, "The current buffer is not empty, data will may be lost. continue?")
                    == USER_ANSWER_NO) {
                goto error;
            }
        }

        // copy the filename for buffer
        //
        memmove(buf->file.name, filename, filename_size);
        buf->file.name_size = filename_size;
        buf->file.name[buf->file.name_size] = 0;


        int fd = open(filename, O_RDONLY);
        if(fd == EACCES) {
            write_message(ed, ERROR_MSG, "Read access denied. filename:'%s'\0", filename);
            goto error;
        }
        if(fd < 0) {
            perror("open");
            goto error;
        }

        // check if the file is read only.
        //
        if((buf->file.readonly = access(filename, W_OK) != F_OK)) {
            printf("warning: file is read only.\n");
        }

        printf("file descriptor: %i\n", fd);

        // get the information about the file.
        //
        struct stat sb;
        if(fstat(fd, &sb) != 0) {
            write_message(ed, ERROR_MSG, "Failed to get information about file '%s'\0", filename);
            goto error_and_close;
        }

        if(sb.st_size <= 0) {
            write_message(ed, ERROR_MSG, "File size seems to be zero, nothing to do.\0");
            goto error_and_close;
        }


        struct timespec ts;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        size_t start_ns = ts.tv_nsec;
        
        char* ptr = NULL;
        ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        if(!ptr) {
            write_message(ed, ERROR_MSG, "'%s' failed to create new mapping.\0", __func__);
            perror("mmap");
            goto error_and_close;
        }


        buffer_clear_all(buf);

        if(!buffer_ready(buf)) {
            write_message(ed, ERROR_MSG, "buffer %i after clear is not ready.\0", buf->id);
            goto error_and_close;
        }


        size_t offset = 0;
        size_t bytes = 0;
        size_t y = 0; // current string.

        struct string_t* str = buf->lines[0];


        // loop through all characters.
        // count bytes until newline character is found, 
        // then copy from ptr + offset to current string.


        for(size_t i = 0; i < sb.st_size; i++) {
            if(!str) {
                break;
            }
            char c = ptr[i];
            
            int eof = (i+1 == sb.st_size);
            total_bytes_read++;
            bytes++;

            if(c == '\n' || eof) {
                if(!string_memcheck(str, bytes - 1)) {
                    fprintf(stderr, "warning: failed to copy data to string.\n");
                    continue;
                }
                
                memmove(str->data, ptr + offset, bytes - 1);
                str->data_size = bytes - 1;

                if(eof) {
                    break;
                }

                if(!buffer_inc_size(buf, 1)) {
                    PRINTERR("cant resize buffer?");
                    goto error_and_close;
                }

                bytes = 0;
                offset = i+1;
                y++;
                
                if(y >= buf->num_alloc_lines) {
                    PRINTERR("y is going out of bounds");
                    goto error_and_close;
                }
                
                str = buffer_get_string(buf, y);//buf->lines[y];
                if(!str) {
                    PRINTERR("buffer line is NULL");
                    goto error_and_close;
                }
            }
        }

        // replace tab characters with spaces.
        // TODO: this is probably not the best idea for everything..

        for(size_t i = 0; i < buf->num_used_lines; i++) {
            str = buffer_get_string(buf, i);
            if(!str) {
                continue;
            }

            for(size_t j = 0; j < str->data_size; j++) {
                char c = str->data[j];
                if(c == '\t') {
                    str->data[j] = 0x20;

                    for(int k = 0; k < FONT_TAB_WIDTH-1; k++) {
                        string_add_char(str, 0x20, j);
                    }
                }
            }
        }


        write_message(ed, INFO_MSG, "bytes read: %l\0", total_bytes_read);

error_and_close:

        close(fd);
        
        if(ptr) {
            munmap(ptr, sb.st_size);
        }
        
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        size_t time_read = ts.tv_nsec - start_ns;


        printf("\033[94m --> Read file in %lims. \033[90m(%li nanosecods).\033[0m\n", 
                time_read/1000000, time_read);

        buffer_update_content_xoff(buf);
        
        buf->current = buf->lines[0];
        buf->file.opened = 1;
    }

error:
    return total_bytes_read;
}

size_t write_file(struct editor_t* ed, unsigned int buf_id) {
    size_t bytes_written = 0;

    if(buf_id >= MAX_BUFFERS) {
        goto error;
    }

    struct buffer_t* buf = &ed->buffers[buf_id];
    if(!buf) {
        fprintf(stderr, "'%s': failed to get pointer to buffer.\n",
                __func__);
        goto error;
    }


    if(buf->file.name_size == 0) {
        // TODO: ask to create the file.
        write_message(ed, ERROR_MSG, "no file to write into.\0");
        goto error;
    }

    // TODO: save a backup file before opening the file with O_TRUNC

    int fd = open(buf->file.name, O_WRONLY | O_APPEND | O_TRUNC);
    if(fd == EACCES) {
        write_message(ed, ERROR_MSG, "Write access denied. filename:'%s'\0", buf->file.name);
        goto error;
    }

    if(fd < 0) {
        perror("open");
        goto error;
    }


    struct string_t* str = NULL;

    for(size_t i = 0; i < buf->num_used_lines; i++) {
        str = buffer_get_string(buf, i);
        if(!str) {
            continue;
        }
        if(string_ready(str)) {
            bytes_written += write(fd, str->data, str->data_size);
            bytes_written += write(fd, "\n", 1);
        }
    }

    close(fd);

error:
    
    if(bytes_written > 0) {
        write_message(ed, INFO_MSG, "written %l bytes.\0", bytes_written);
    }
    else if(buf->file.readonly) {
        write_message(ed, INFO_MSG, "Permission denied! read only file.\0");
    }

    return bytes_written;
}





