#include <stdio.h>
#include <unistd.h>
#include <GL/gl.h>
#include <math.h>

#include "editor.h"
#include "string.h"




void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }

    float cursor_rgb[3]    = { 0.0, 1.0, 0.0 };
    float cursor_rgb_2[3]  = { 0.0, 0.35, 0.0 };

    char line_num_str[28];


    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        struct buffer_t* buf = &ed->buffers[ed->current_buffer];

        int draw_offset = snprintf(line_num_str, 28, "%li", buf->num_lines)+2;
        int cursor_draw_offset = draw_offset;

        // find all tabs to the cursor x position so it can be offset correctly.

        size_t num_tabs =
            (FONT_TAB_WIDTH-1)*string_num_chars(buf->current, 0, buf->cursor_x, '\t');

        cursor_draw_offset += num_tabs;
    // DRAW CURSOR
    
        const float dcursor_x = column_to_location(ed, buf->cursor_x + cursor_draw_offset);
        const float dcursor_y = row_to_location(ed, buf->cursor_y);

        // cursor background
        glColor3f(cursor_rgb_2[0], cursor_rgb_2[1], cursor_rgb_2[2]);
        draw_rect(ed, 
                dcursor_x,
                dcursor_y,
                ed->font.r_width,
                ed->font.r_height, MAP_XYWH);

        // cursor right line
        glColor3f(cursor_rgb[0], cursor_rgb[1], cursor_rgb[2]);
        draw_rect(ed,
                dcursor_x + ed->font.width - 1,
                dcursor_y,
                3,
                ed->font.r_height, MAP_XYWH
                );

        // cursor bottom line
        draw_rect(ed,
                dcursor_x,
                dcursor_y + ed->font.height-4,
                ed->font.r_width,
                6, MAP_XYWH
                );

        if(buf->ready) {

            int max_rows = ed->window_height / ed->font.height;

            for(size_t i = 0; i < buf->num_used_lines; i++) {

                if(i > max_rows) {
                    break;
                }

                glColor3f(0.9, 0.85, 0.7);
                font_draw_str(ed, 
                        buf->lines[i]->data, 
                        buf->lines[i]->data_size, draw_offset, i);

                if(i == buf->cursor_y) {
                    glColor3f(0.3, 0.55, 0.3);
                }
                else {
                    glColor3f(0.3, 0.3, 0.3);
                }
            
                int l = snprintf(line_num_str, 28, "%li", i);
                font_draw_str(ed, line_num_str, 28, draw_offset-l-2, i);
                font_draw_char(ed, draw_offset-2+1, i, '|');
            }
        }
        else {
            write_error(ed, "The current buffer is not initialized properly?");
        }


        draw_errors(ed);


        do_safety_check(ed);
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




