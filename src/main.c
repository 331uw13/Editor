#include <stdio.h>
#include <unistd.h>
#include <GL/gl.h>
#include <math.h>

#include "editor.h"
#include "string.h"
#include "utils.h"
#include "gui.h"

#define LINE_NUM_STR_SIZE 28


void draw_buffer(struct editor_t* ed,
        struct buffer_t* buf,
        char* line_num_str,
        int x_drw_off,
        int y_drw_off)
{
    
    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i > (ed->max_row + buf->scroll)) {
            break;
        }
            

        glColor3f(0.9, 0.85, 0.7);
        font_draw_data_wrapped(ed, 
                buf->lines[i]->data, 
                buf->lines[i]->data_size, x_drw_off, i - y_drw_off,
                ed->max_column - x_drw_off);

        if(i == buf->cursor_y) {
            glColor3f(0.3, 0.55, 0.3);
        }
        else {
            glColor3f(0.3, 0.3, 0.3);
        }
        
        int l = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", i);
        font_draw_data(ed, line_num_str, LINE_NUM_STR_SIZE, x_drw_off-l-2, i - y_drw_off, 1);
        font_draw_char(ed, x_drw_off-2+1, i - y_drw_off, '|', 1);
    }

}

void draw_buffer_filenames(struct editor_t* ed) {
    
    int prev_x = 0;

    for(unsigned int j = 0; j < ed->num_active_buffers; j++) {
        struct buffer_t* b = &ed->buffers[j];
        if(!buffer_ready(b)) {
            continue;
        }

        if(b->filename_size >= BUFFER_MAX_FILENAME_SIZE) {
            fprintf(stderr, "warning: buffer may have too large filename, skipping..\n");
            continue;
        }
        
        const int fn_y = ed->window_height - ed->font.height;
        const int fn_x = prev_x+10;


        if(b->filename_size > 0) {
            glColor3f(1.0, 1.0, 1.0); 
            font_draw_data(ed, b->filename, b->filename_size, fn_x, fn_y, 0);
            prev_x = fn_x + (ed->font.width * b->filename_size);
        }
        else {
            glColor3f(0.6, 0.6, 0.6);
            font_draw_data(ed, "-EMPTY-", 7, fn_x, fn_y, 0);
            prev_x = fn_x + (ed->font.width * 7);
        }

    }
}


void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }

    float cursor_rgb[3]    = { 0.0, 1.0, 0.0 };
    float cursor_rgb_2[3]  = { 0.0, 0.35, 0.0 };
    char line_num_str[28];


    struct gui_t gui;
    gui.num_buttons = 0;
    gui.num_fsliders = 0;
    gui.num_isliders = 0;



    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        struct buffer_t* buf = &ed->buffers[ed->current_buffer];

        int x_drw_off = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", buf->num_used_lines)+2;
        int y_drw_off = buf->scroll;

        int cursor_draw_offset = x_drw_off;

        // find all tabs to the cursor x position so it can be offset correctly.

        size_t num_tabs =
            (FONT_TAB_WIDTH-1)*string_num_chars(buf->current, 0, buf->cursor_x, '\t');

        cursor_draw_offset += num_tabs;
    // DRAW CURSOR
    
        const float dcursor_x = column_to_location(ed, buf->cursor_x + cursor_draw_offset);
        const float dcursor_y = row_to_location(ed, buf->cursor_y - y_drw_off);

        /*
        // outside green frame
        //
        glColor3f(0.0, 0.0, 0.0);
        draw_framed_rect(ed, 
                1,
                1,

                ed->window_width-1,
                ed->window_height*2-2, 
                0.2, 0.4,0.1,
                0.5,
                MAP_XYWH
                );

        // cursor background
        glColor3f(cursor_rgb_2[0], cursor_rgb_2[1], cursor_rgb_2[2]);
        draw_rect(ed, 
                dcursor_x,
                dcursor_y,
                ed->font.width,
                ed->font.r_height, MAP_XYWH);

        // cursor right line
        glColor3f(cursor_rgb[0], cursor_rgb[1], cursor_rgb[2]);
        draw_rect(ed,
                dcursor_x + ed->font.width - 1,
                dcursor_y,
                1.5,
                ed->font.r_height, MAP_XYWH
                );

        // cursor bottom line
        draw_rect(ed,
                dcursor_x,
                dcursor_y + ed->font.height-4,
                ed->font.width,
                6, MAP_XYWH
                );

                */



        /*

        if(buffer_ready(buf)) {
            
            draw_buffer(ed, buf, line_num_str, x_drw_off, y_drw_off);
            draw_buffer_filenames(ed);




            update_gui_items(ed, &gui);

        }
        else {
            write_message(ed, ERROR_MSG, "The current buffer is not initialized properly?");
        }
            

        if(ed->mode == MODE_COMMAND_LINE) {
            
            glColor3f(0.21,0.126,0.1);
            draw_framed_rect(ed,

                    3, 3,
                    ed->window_width-5,
                    ed->font.r_height+5,
                    0.67, 0.45, 0.2,
                    0.3,
                    MAP_XYWH
                    );


            const int xoff = ed->font.width+10;
            const int cx = ed->cmd_cursor * ed->font.width + xoff;

            glColor3f(1.0, 0.4, 0.35);
            draw_line(ed, cx, 5, cx, 5+ed->font.height-5, 2.0, MAP_XYWH);
            
            glColor3f(0.55, 0.35, 0.15);
            font_draw_char(ed, 4, 4, '>', 0);

            glColor3f(0.7, 0.5, 0.35);
            font_draw_data(ed, ed->cmd_str->data, ed->cmd_str->data_size, xoff, 4, 0);
        }


        draw_error_buffer(ed);
        draw_info_buffer(ed);

        */
        do_safety_check(ed);

        ed->mouse_button = 0;
        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }
}



int main(int argc, char** argv) {
    struct editor_t* ed = 
        init_editor("Tamsyn8x16b.psf.gz",
                900, 750, !INIT_FULLSCREEN);
    
    if(ed) {
        run_loop(ed);
        cleanup_editor(&ed);
    }

    printf("Exit\n");
    return 0;
}




