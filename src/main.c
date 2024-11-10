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


    /*
    read_file(ed, 0, "for-testing/test.txt\0", 0);
    read_file(ed, 1, "for-testing/another_file.txt\0", 0);


    */
    
    ed->num_active_buffers = 1;
    set_buffer_dimensions(ed);

    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        //glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);


        draw_everything(ed);

        do_safety_check(ed);
        //ed->mouse_button = 0;
        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }
}



int main(int argc, char** argv) {


    struct editor_t* ed = 
        init_editor("Topaz-8.ttf", 1200, 750,! INIT_FULLSCREEN);
    
    if(ed) {
        run_loop(ed);
        cleanup_editor(&ed);
    }

    printf("Exit\n");
    return 0;
}

