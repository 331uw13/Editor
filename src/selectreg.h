#ifndef EDITOR_SELECT_REGION_H
#define EDITOR_SELECT_REGION_H

#include <stddef.h>

struct buffer_t;
struct string_t;

struct select_t {
    
    long int x0; // begin position
    long int y0; //

    long int x1; // end position
    long int y1; //

    int inverted;
    size_t scroll_point; // the point where select mode was enabled.
};



// callbacks

int copy_selected_reg_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr);

int remove_selected_reg_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr);

int inc_selected_reg_indent_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr);

int dec_selected_reg_indent_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr);

#endif
