#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include "string.h"


#define BUFFER_MEMORY_BLOCK_SIZE 64 // TODO: rename these in string.h and buffer.h
#define BUFFER_MAX_SIZE 0xf4240
#define BUFFER_MAX_FILENAME_SIZE 256

#define BUFFER_SHIFT_DOWN  0
#define BUFFER_SHIFT_UP    1


//#define BUFFER_PRINT_MEMORY_RESIZE


struct buffer_t {
    struct string_t** lines;
    size_t mem_size;  // allocated memory size.
    size_t num_lines; // 
    size_t num_used_lines; // actual number of used lines in the buffer.


    size_t cursor_x;
    size_t cursor_y;

    struct string_t* current;

    char filename[BUFFER_MAX_FILENAME_SIZE];
    size_t filename_size;
    int file_opened;

    int id; // buffer can be accessed from 'editor->buffers[id]'
    int ready; // is the buffer initialized?

};

int    setup_buffer(struct buffer_t* buf, int id);
void   cleanup_buffer(struct buffer_t* buf);
int    buffer_ready(struct buffer_t* buf);

void   buffer_clear_all(struct buffer_t* buf);

// check if buffer needs more memory.
//   returns 1 if no more memory is needed or memory is resized.
//   returns 0 if failed to resize memory.
// size_t n is the "new size"
int    buffer_memcheck(struct buffer_t* buf, size_t n);

// this calls 'buffer_memcheck' and
// increments or decrements the 'num_used_lines'
// returns 0 on failure.
// returns 1 on success or no need to change size.
int   buffer_inc_size(struct buffer_t* buf, size_t n);
int   buffer_dec_size(struct buffer_t* buf, size_t n);


void  move_cursor_to(struct buffer_t* buf, size_t col, size_t row);
void  move_cursor(struct buffer_t* buf, int xoff, int yoff);

// also moves the result to 'num_used_lines'
size_t  buffer_find_last_line(struct buffer_t* buf);
void    buffer_shift_data(struct buffer_t* buf, size_t row, int direction);
int     buffer_add_newline(struct buffer_t* buf, size_t col, size_t row);

// returns the string pointer safely from 'index'
// if 'index' is out of bounds returns 'buffer->current'.
struct string_t* buffer_get_string(struct buffer_t* buf, size_t index);



#endif
