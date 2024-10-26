#include <stdio.h>
#include <unistd.h>
#include <GL/glew.h>
#include <math.h>

#include "draw.h"
#include "editor.h"
#include "string.h"
#include "utils.h"
#include "file.h"


void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }


    read_file(ed, 0, "testf.txt", 9);
    read_file(ed, 1, "another_testf.txt", 17);

    ed->num_active_buffers = 2;
    set_buffer_dimensions(ed);


    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        //struct buffer_t* buf = &ed->buffers[ed->current_buf_id];


        draw_buffers(ed);

        if(ed->mode == MODE_COMMAND_LINE) {
            set_color_hex(ed, 0x231100);
            draw_framed_rect(ed, 
                    5, 3,
                    ed->window_width-10, ed->font.char_h+8,
                    0x554510,
                    1.0,
                    MAP_XYWH, DRW_NO_GRID);
            
            font_set_color_hex(&ed->font, 0x773310);
            draw_char(ed, 10, 6, '>', DRW_NO_GRID);


            // cmd_cursor
            //
            set_color_hex(ed, 0x225522);
            draw_rect(ed, 
                    10 + (ed->cmd_cursor+1) * ed->font.char_w,
                    6,
                    ed->font.char_w,
                    ed->font.char_h,
                    MAP_XYWH, DRW_NO_GRID);

            font_set_color_hex(&ed->font, 0xAA6633);
            draw_data(ed, ed->font.char_w+10, 6, 
                    ed->cmd_str->data, ed->cmd_str->data_size, DRW_NO_GRID);
        }


        draw_error_buffer(ed);
        draw_info_buffer(ed);
        do_safety_check(ed);

        ed->mouse_button = 0;
        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }
}



int main(int argc, char** argv) {
    struct editor_t* ed = 
        init_editor("Topaz-8.ttf",
                1200, 750, !INIT_FULLSCREEN);
    
    if(ed) {
        run_loop(ed);
        cleanup_editor(&ed);
    }

    printf("Exit\n");
    return 0;
}


