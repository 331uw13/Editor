#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

// temporary!
#include <time.h>



#include "file_io.h"




int open_file(struct editor_t* ed, unsigned int buf_id, char* filename) {
    int ok = 0;

    if(ed) {

        if(!filename) {
            fprintf(stderr, "'create_fd': filename is NULL!\n");
            goto error;
        }

        if(buf_id > MAX_BUFFERS) {
            fprintf(stderr, "'create_fd': invalid buffer id.\n");
            goto error;
        }

        struct buffer_t* buf = &ed->buffers[buf_id];

        if(!buf) {
            fprintf(stderr, "'create_fd': failed to get pointer to buffer.\n");
            goto error;
        }

        if(!buffer_ready(buf)) {
            goto error;
        }

        int fd = open(filename, O_RDWR | O_CREAT);
        if(fd == EACCES) {
            write_error(ed, "Access denied. filename:'%s'", filename);
            goto error;
        }

        printf("file descriptor: %i\n", fd);

        struct stat sb;
        if(fstat(fd, &sb) != 0) {
            write_error(ed, "Failed to get information about file '%s'", filename);
            close(fd);

        }

        // -------- TEMPORARY! ----------
        struct timespec ts;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        size_t start_ns = ts.tv_nsec;
        // -----------

        
        char* ptr = mmap(NULL, 
                sb.st_size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE, fd, 0);




        struct string_t* str = buf->lines[0];

        if(!string_ready(str)) {
            goto error_and_munmap;
        }

        size_t y = 0;
        size_t bytes = 0;
        size_t offset = 0;

        buffer_memcheck(buf, BUFFER_MEMORY_BLOCK_SIZE);

        for(size_t i = 0; i < sb.st_size; i++) {
            char c = ptr[i];
            if(c == '\n') {
                
                if(string_memcheck(str, bytes)) {
                    memmove(str->data,
                            ptr + offset,
                            bytes);

                    str->data_size += bytes;
                }
                
                bytes = 0;
                offset = i+1;

                if(buffer_inc_size(buf, 1)) {
                    y++;
                    if(y >= buf->num_lines) {
                        fprintf(stderr, "'open_file': 'y' is out of bounds\n");
                        goto error_and_munmap;
                    }

                    str = buf->lines[y];
                }

                continue;
            }

            bytes++;

        }


error_and_munmap:

        munmap(ptr, sb.st_size);
        
        // -------- TEMPORARY! ----------
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
        size_t time_read = ts.tv_nsec - start_ns;
        // -----------

        printf("\033[94m --> Read file in %lims. \033[90m(%li nanosecods).\033[0m\n", 
                time_read/1000000, time_read);
        buf->fd = fd;
    }

error:
    return ok;
}

void close_file(struct editor_t* ed, unsigned int buf_id) {

    if(buf_id >= MAX_BUFFERS) {
        return;
    }

    struct buffer_t* buf = &ed->buffers[buf_id];

    if(buf->fd > 0) {
        close(buf->fd);
        buf->fd = -1;
        printf("file closed.\n");
    }
    
}




