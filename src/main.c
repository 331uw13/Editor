#include <stdio.h>
#include <unistd.h>
#include <GL/glew.h>
#include <math.h>

#include "editor.h"
#include "string.h"
#include "utils.h"

#define LINE_NUM_STR_SIZE 28


void draw_buffer(struct editor_t* ed,
        struct buffer_t* buf,
        char* line_num_str,
        int xdrw_off,
        int ydrw_off)
{
    
    struct string_t* str = NULL;

    for(size_t i = 0; i < buf->num_used_lines; i++) {
        if(i > (ed->max_row + buf->scroll)) {
            break;
        }

        str = buf->lines[i];
        if(!str) {
            continue;
        }
        
        draw_data(ed, xdrw_off, i+ydrw_off, str->data, str->data_size, DRW_ONGRID);

        
        const int lnum = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", i);
        draw_data(ed, xdrw_off - lnum - 2, i + ydrw_off, line_num_str, LINE_NUM_STR_SIZE, DRW_ONGRID);
        draw_char(ed, xdrw_off - 2+1, i + ydrw_off, '|', 1);
    
    }
}


void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }

    //float cursor_rgb[3]    = { 0.0, 1.0, 0.0 };
    //float cursor_rgb_2[3]  = { 0.0, 0.35, 0.0 };
    char line_num_str[LINE_NUM_STR_SIZE];

    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        struct buffer_t* buf = &ed->buffers[ed->current_buffer];

        int xdrw_off = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", buf->num_used_lines)+2;
        int ydrw_off = 1;

        int curdrw_off = xdrw_off;


        // find all tabs to the cursor x position so it can be offset correctly.
        size_t num_tabs =
            (FONT_TAB_WIDTH-1) * string_num_chars(buf->current, 0, buf->cursor_x, '\t');

        curdrw_off += num_tabs;
    
        const float dcursor_x = column_to_location(ed, buf->cursor_x + curdrw_off);
        const float dcursor_y = row_to_location(ed, buf->cursor_y - ydrw_off);


        /*
        int x = 0;
        int y = 2;
        for(int i = 0x20; i < 0x7F; i++) {
            
            draw_char(ed, x, y, i, DRW_ONGRID);
            x++;

            if(x > ed->max_column) {
                x = 0;
                y++;
            }


        }
        */


        
        if(buffer_ready(buf)) {
        
            draw_buffer(ed, buf, line_num_str, xdrw_off, ydrw_off);
        
        }
        else {
            fprintf(stderr, "ERROR: Uninitialized buffer!\n");
        }


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
        init_editor("Topaz-8.ttf",
                900, 750, !INIT_FULLSCREEN);
    
    if(ed) {
        run_loop(ed);
        cleanup_editor(&ed);
    }

    printf("Exit\n");
    return 0;
}




