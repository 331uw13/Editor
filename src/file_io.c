#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>



#include "file_io.h"




size_t read_file(struct editor_t* ed, unsigned int buf_id, char* filename, size_t filename_size) {
    size_t total_bytes_read = 0;

    if(ed) {

        if(!filename) {
            fprintf(stderr, "'read_file': filename is NULL!\n");
            goto error;
        }

        if(filename_size == 0) {
            fprintf(stderr, "'read_file': filename_size cant be 0\n");
            goto error;
        }


        //
        // this is here until filename array is made dynamic.
        //
        if(filename_size >= BUFFER_MAX_FILENAME_SIZE) {
            write_message(ed, ERROR_MSG, "Filename size is too big.\0");
            goto error;
        }


        if(buf_id > MAX_BUFFERS) {
            fprintf(stderr, "'read_file': invalid buffer id.\n");
            goto error;
        }

        struct buffer_t* buf = &ed->buffers[buf_id];

        if(!buf) {
            fprintf(stderr, "'read_file': failed to get pointer to buffer.\n");
            goto error;
        }

        if(!buffer_ready(buf)) {
            goto error;
        }

        if(buf->file_opened > 0) {
            write_message(ed, ERROR_MSG, "Some file is already opened for buffer '%i'.\0", buf_id);
            goto error;
        }

        if(access(filename, F_OK)) {
            write_message(ed, ERROR_MSG, "File doesnt exist '%s'.\0", filename);
            goto error;
        }

        memmove(buf->filename, filename, filename_size);
        buf->filename_size = filename_size;

        int fd = open(filename, O_RDONLY);
        if(fd == EACCES) {
            write_message(ed, ERROR_MSG, "Read access denied. filename:'%s'\0", filename);
            goto error;
        }
        if(fd < 0) {
            perror("open");
            goto error;
        }

        printf("file descriptor: %i\n", fd);

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
            fprintf(stderr, "failed to create new mapping.\n");
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
                    fprintf(stderr, "'open_file': cant resize buffer?\n");
                    goto error_and_close;
                }

                bytes = 0;
                offset = i+1;
                y++;
                
                if(y >= buf->num_alloc_lines) {
                    fprintf(stderr, "'open_file': y is going out of bounds.\n");
                    goto error_and_close;
                }
                
                str = buf->lines[y];
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
        
        buf->file_opened = 1;
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
        fprintf(stderr, "'write_file': failed to get pointer to buffer.\n");
        goto error;
    }


    if(buf->filename_size == 0) {
        // TODO: ask to create the file.
        write_message(ed, ERROR_MSG, "no file to write into.\0");
        goto error;
    }

    // TODO: save a backup file before opening the file with O_TRUNC

    printf("opening '%s'\n", buf->filename);

    int fd = open(buf->filename, O_WRONLY | O_APPEND | O_TRUNC);
    if(fd == EACCES) {
        write_message(ed, ERROR_MSG, "Write access denied. filename:'%s'\0", buf->filename);
        goto error;
    }

    if(fd < 0) {
        perror("open");
        goto error;
    }


    struct string_t* str = NULL;


    for(size_t i = 0; i < buf->num_used_lines; i++) {
        str = buf->lines[i];
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
    
    write_message(ed, INFO_MSG, "written %l bytes.\0", bytes_written);

    return bytes_written;
}





