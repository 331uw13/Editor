#include <stdio.h>
#include <unistd.h>
#include <GL/glew.h>
#include <math.h>

#include "draw.h"
#include "editor.h"
#include "string.h"
#include "utils.h"
#include "file.h"


/*
#include "memory.h"
#include <stdlib.h>

int main() {
    

    size_t size = 2;
    size_t newsize = 52;


    int* array = malloc(size * sizeof *array);
    const size_t b = sizeof *array;
    
    printf("before: %p\n", array);

    size_t returned_size = 0;

    array = safe_resize_array(array,
            sizeof *array,
            size,
            newsize,
            &returned_size
            );

    printf(" after: %p\n", array);

    printf("wanted new size: %li\n", newsize);
    printf("actual new size: %li\n", returned_size);

    free(array);

    return 0;
}
*/






void run_loop(struct editor_t* ed) {
    if(!ed) { return; }
    if(!ed->ready) { return; }


    read_file(ed, 0, "for-testing/test.txt\0", 0);
    read_file(ed, 1, "for-testing/another_file.txt\0", 0);


    
    ed->num_active_buffers = 2;
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

