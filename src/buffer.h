#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include "string.h"


#define BUFFER_MEMORY_BLOCK_SIZE 64 // TODO: rename these in string.h and buffer.h
#define LINENUM_BUF_SIZE 28
#define BUFFER_MAX_SIZE 0xf4240
#define BUFFER_MAX_FILENAME_SIZE 256


struct select_t { // TODO.
    
    long int x0; // begin position
    long int y0; //

    long int x1; // end position
    long int y1; //

    struct string_t* begin;
    struct string_t* end;
};

struct file_t {
    int    opened;
    char   name[BUFFER_MAX_FILENAME_SIZE+1];
    size_t name_size;
    int    readonly;
};

struct buffer_t {
    struct string_t** lines;
    size_t num_alloc_lines; // number of allocated lines.
    size_t num_used_lines;  // actual number of used lines in the buffer.
 
    int content_xoff; // used to offset the buffer content 
                      // so line numbers and content dont overlap

    struct file_t file;
    
    long int cursor_x;
    long int cursor_y;
    long int cursor_px;
    long int cursor_py;

    int max_row;
    int max_col;
    
    int x;
    int y;
    int width;
    int height;
    
    struct select_t select;
    struct string_t* current; // &lines[buffer->cursor_y],
                              // set everytime move_cursor_to is called.
    size_t scroll;
    size_t prev_scroll;

    /*
    int     file_opened;
    char    filename[BUFFER_MAX_FILENAME_SIZE+1];
    size_t  filename_size;
    */

    int id; // NOTE: buffer can be accessed from 'editor->buffers[id]'
    int ready;
};

int    create_buffer(struct buffer_t* buf, int id);
void   delete_buffer(struct buffer_t* buf);
void   buffer_update_content_xoff(struct buffer_t* buf);
void   buffer_update_selected(struct buffer_t* buf);
int    buffer_ready(struct buffer_t* buf);
void   buffer_reset(struct buffer_t* buf);
int    buffer_clear_all(struct buffer_t* buf);
// TODO: 'buffer_clear(buf, start_y, end_y);

// check if buffer needs more memory.
//   returns 1 if no more memory is needed or memory is resized.
//   returns 0 if failed to resize memory.
// size_t n is the "new size"
int    buffer_memcheck(struct buffer_t* buf, size_t n);

// this calls 'buffer_memcheck' and
// increments/decrements the 'num_used_lines'
//   returns 0 on failure.
//   returns 1 on success or no need for change.
int buffer_inc_size(struct buffer_t* buf, size_t n);
int buffer_dec_size(struct buffer_t* buf, size_t n);

void  buffer_scroll_to(struct buffer_t* buf, size_t y);
void  buffer_scroll   (struct buffer_t* buf, int offset);

// NOTE: 'move_cursor_to' sets the scroll if row is offscreen
void  move_cursor_to(struct buffer_t* buf, long int col, long int row);
void  move_cursor   (struct buffer_t* buf, int xoff, int yoff);

#define BUFFER_SHIFT_DOWN  0
#define BUFFER_SHIFT_UP    1
void    buffer_shift_data(struct buffer_t* buf, size_t row, int direction);

// find the last line that has some data in it.
size_t  buffer_find_last_line(struct buffer_t* buf);
int     buffer_add_newline(struct buffer_t* buf, size_t col, size_t row);
int     buffer_remove_line(struct buffer_t* buf, size_t row);

// returns the string pointer from buf->lines[index] but
// if 'index' is out of bounds returns NULL.
struct string_t* buffer_get_string(struct buffer_t* buf, size_t index);


#endif
