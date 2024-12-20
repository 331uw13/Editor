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

    editor_add_buffer(ed);
    read_file(ed, 0, "for-testing/another_file.txt\0", 0);
    
    editor_add_buffer(ed);
    read_file(ed, 1, "for-testing/test.txt\0", 0);

    editor_add_buffer(ed);
    read_file(ed, 2, "for-testing/bla.sh\0", 0);

    // TODO: read_file(ed, ADD_BUFFER_ONREAD, ...);

    editor_add_buffer(ed);
    read_file(ed, 3, "for-testing/verylongnameforthisfilefortestingreasonsidontknowhello\0", 0);
    
    editor_add_buffer(ed);
    read_file(ed, 4, "for-testing/readonly.txt\0", 0);
   

    while(!glfwWindowShouldClose(ed->win)) {
        glClearColor(0.06,0.06,0.06,1.0);
        glClear(GL_COLOR_BUFFER_BIT);


        draw_everything(ed);

        if(!is_safe_to_continue(ed)) {
            break;
        }

        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }
}


// TODO: argument parser

int main(int argc, char** argv) {
    struct editor_t ed;

    if(!init_editor(&ed, "Topaz-8.ttf", 1200, 750, INIT_FULLSCREEN)) {
        fprintf(stderr, "failed to initialize editor. return 1.\n");
        return 1;
    }

    run_loop(&ed);
    cleanup_editor(&ed);

    printf("Exit\n");
    return 0;
}

