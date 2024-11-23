#include <stdio.h>

#include "selectreg.h"
#include "editor.h"


int copy_selected_reg_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr)
{
    int res = 1;

    struct editor_t* ed = (struct editor_t*) userptr;
    if(!ed) {
        res = 0;
        goto error;
    }
    
    const int isbegin = (flag & PROCSELECTED_BEGIN);
    const int isend   = (flag & PROCSELECTED_END);


    size_t srcoffs = 0;
    size_t size = line->data_size;

    if(isbegin) {
        srcoffs = buf->select.x0;
        size = line->data_size - srcoffs;
    }
    if(isend) {
        size = buf->select.x1 - srcoffs;
    }



    if(!(res = string_move_data(
            ed->clipbrd,
            line,
            ed->clipbrd->data_size, // dst offset
            srcoffs,
            size,
            0
            ))) {
        printf("[ERROR] %s, %s(): string_move_data failed.\n  srcoffs = %li, size = %li\n",
                __FILE__, __func__, srcoffs, size);
        goto error;
    }

    if(!isend) {
        res = string_app_char(ed->clipbrd, '\n');
    }

error:
    return res;
}


int remove_selected_reg_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr)
{
    int res = 1;

    const int isbegin = (flag & PROCSELECTED_BEGIN);
    const int isend   = (flag & PROCSELECTED_END);

    // at buffer.c 'buffer_remove_selected' handles the rest.
    //              -----------------------------------------
    
    if(isbegin && isend) {
        
        long int len = buf->select.x1 - buf->select.x0;
        if(len > 0) {
            res = string_cut_data(line, buf->select.x0, len);
        }
    }
    else {

        if(isbegin) {
            res = string_cut_data(line, buf->select.x0,  (line->data_size - buf->select.x0));
        }
        else if(isend) {
            res = string_cut_data(line, 0, buf->select.x1);
        }
        else {
            res = string_clear_data(line);
        }
    }
    

done:
    return res;
}

int inc_selected_reg_indent_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr)
{
    int res = 1;
  
    for(size_t i = 0; i < FONT_TAB_WIDTH; i++) {
        string_add_char(line, 0x20, 0);
    }


    return res;
}

int dec_selected_reg_indent_callback
    (struct buffer_t* buf, struct string_t* line, size_t y, int flag, void* userptr)
{
    int res = 1;

    size_t count = string_count_ws_to(line, line->data_size);

    if(count < FONT_TAB_WIDTH) {
        res = 0;
        goto done;
    }


    string_cut_data(line, 0, FONT_TAB_WIDTH);

done:
    return res;
}



