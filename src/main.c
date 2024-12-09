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

    editor_add_buffer(ed);
    read_file(ed, 3, "for-testing/verylongnameforthisfilefortestingreasonsidontknowhello\0", 0);

    while(!glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        //glfwGetCursorPos(ed->win, &ed->mouse_x, &ed->mouse_y);

        draw_everything(ed);
        do_safety_check(ed); // <-- TODO: make this better

        //ed->mouse_button = 0;
        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }
}



int main(int argc, char** argv) {


    struct editor_t ed;

    if(!init_editor(&ed, "Topaz-8.ttf", 1200, 750, INIT_FULLSCREEN)) {
        fprintf(stderr, "failed to initialize editor. return 1.");
        return 1;
    }

    run_loop(&ed);
    cleanup_editor(&ed);

    printf("Exit\n");
    return 0;
}

