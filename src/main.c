#include <stdio.h>
#include <unistd.h>
#include <GL/glew.h>
#include <math.h>

#include "draw.h"
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

    for(size_t i = buf->scroll; i < buf->num_used_lines; i++) {
        if(i > (ed->max_row + buf->scroll)) {
            break;
        }

        str = buf->lines[i];
        if(!str) {
            continue;
        }

        const int y = (i+ydrw_off) - buf->scroll;
        
        font_set_color_hex(&ed->font, 0xFFEEAA);
        draw_data(ed, xdrw_off, y, str->data, str->data_size, DRW_ONGRID);

        
        const int lnum = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", i);
        

        font_set_color_hex(&ed->font, 0x504030);
        draw_data(ed, xdrw_off - lnum - 2, y, line_num_str, LINE_NUM_STR_SIZE, DRW_ONGRID);
        
        font_set_color_hex(&ed->font, 0x252525);
        draw_char(ed, xdrw_off - 2+1, y, '|', 1);
    
    }
}


void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }

    char line_num_str[LINE_NUM_STR_SIZE];

/*
    struct buffer_t* b = &ed->buffers[ed->current_buffer];
    for(int i = 0; i < 30; i++) {
        
        buffer_add_newline(b, 0, 0);
        struct string_t* str = b->lines[i];
        
        string_add_char(str, 0x20+i, 0);
    }
*/

    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        struct buffer_t* buf = &ed->buffers[ed->current_buffer];

        int xdrw_off = snprintf(line_num_str, LINE_NUM_STR_SIZE, "%li", buf->num_used_lines-1)+2;
        int ydrw_off = 1;

        int curdrw_off = xdrw_off;


        // find all tabs to the cursor x position so it can be offset correctly.
        size_t num_tabs =
            (FONT_TAB_WIDTH-1) * string_num_chars(buf->current, 0, buf->cursor_x, '\t');

        curdrw_off += num_tabs;
    
        const float dcur_x = column_to_location(ed, buf->cursor_x + curdrw_off);
        const float dcur_y = row_to_location(ed, (buf->cursor_y + ydrw_off) - buf->scroll);

        if(buffer_ready(buf)) {

            // cursor
            //
            set_color_hex(ed, 0x104410);
            draw_rect(ed, dcur_x, dcur_y+3, ed->font.char_w, ed->font.char_h*2+3);
            set_color_hex(ed, 0x10AA10);
            draw_rect(ed, dcur_x, dcur_y+3, ed->font.char_w, 3);
            draw_rect(ed, dcur_x+ed->font.char_w-1, dcur_y+3, 2, ed->font.char_h);

            // TODO draw multiple buffers
            //
            draw_buffer(ed, buf, line_num_str, xdrw_off, ydrw_off);
        }
        else {
            fprintf(stderr, "ERROR: Uninitialized buffer!\n");
        }


        if(ed->mode == MODE_COMMAND_LINE) {
            set_color_hex(ed, 0x112300);
            draw_rect(ed, 5, ed->font.char_h+9, ed->window_width-10, ed->font.char_h*2+9);
            
            font_set_color_hex(&ed->font, 0x337733);
            draw_char(ed, 10, ed->font.char_h+6, '>', DRW_NO_GRID);

            font_set_color_hex(&ed->font, 0x33EE33);
            draw_data(ed, ed->font.char_w+10, ed->font.char_h+6, 
                    ed->cmd_str->data, ed->cmd_str->data_size, DRW_NO_GRID);
        }


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




