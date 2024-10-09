#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <zlib.h>
#include <GL/glew.h>

#include "editor.h"
#include "input_handler.h"
#include "utils.h"
#include "shader.h"
#include "draw.h"

void _framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);

    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(ed) {
        ed->window_width = width;
        ed->window_height = height;

        if(ed->font.ready) {

            ed->max_column = width / ed->font.char_w;
            ed->max_row = height / ed->font.char_h - 2;

        }
        /*
        if(ed->font.data) {
            ed->max_column = width / ed->font.width;
            ed->max_row = (height / ed->font.height) - 2;
        }
        */

        printf("resized window:%ix%i\n", ed->window_width, ed->window_height);
    }

}

void do_safety_check(struct editor_t* ed) {
    if(!ed) {
        fprintf(stderr, "oh no... where is my pointer.\n");
        return;
    }

    if(ed->current_buffer > MAX_BUFFERS) {
        ed->current_buffer = MAX_BUFFERS;  
    }

    struct buffer_t* buf = &ed->buffers[ed->current_buffer];
    

    // TODO: halt program and draw fatal error messages on screen

    if(!buf->lines) {
        fprintf(stderr, "buffer is not initialized?\n");
        // TODO: try to initialize buffer?
        return;
    }


    // these things should not happen but just make sure.

    if(buf->cursor_y >= buf->num_used_lines) {
        buf->cursor_y = buf->num_used_lines;
    }

    if(!buf->current) {
        buf->current = buf->lines[buf->cursor_y];
    }

    if(buf->cursor_x > buf->current->data_size) {
        buf->cursor_x = buf->current->data_size;
    }

}

float column_to_location(struct editor_t* ed, size_t col) {
    return (col * ed->font.char_w + EDITOR_X_PADDING)
        * EDITOR_TEXT_X_SPACING;
}

float row_to_location(struct editor_t* ed, size_t row) {
    return (row * ed->font.char_h + EDITOR_Y_PADDING)
        * EDITOR_TEXT_Y_SPACING;
}


void map_xywh(struct editor_t* ed, float* x, float* y, float* w, float* h) {
    if(x) {
        *x = map(*x, 0.0, ed->window_width, -1.0, 1.0);
    }
    if(y) {
        *y = map(*y, 0.0, ed->window_height, 1.0, -1.0);
    }
    if(w) {
        *w = map(*w, 0.0, ed->window_width, 0.0, 2.0);
        //*w = (*w > 0) ? (*w / ed->window_width) : *w;
    }
    if(h) {
        *h = (*h > 0) ? (*h / ed->window_height) : *h;
    }
}

void write_message(struct editor_t* ed, int type, char* err, ...) {
    if(!err) { return; }

    va_list ptr;
    va_start(ptr, err);

    size_t max_size = 0;

    char* buf_ptr = NULL;
    size_t* size_ptr = NULL;

    switch(type) {
        
        case ERROR_MSG:
            max_size = ERROR_BUFFER_MAX_SIZE;
            buf_ptr = ed->error_buf;
            size_ptr = &ed->error_buf_size;

            clear_error_buffer(ed);

            break;

        case INFO_MSG:
            max_size = INFO_BUFFER_MAX_SIZE;
            buf_ptr = ed->info_buf;
            size_ptr = &ed->info_buf_size;
            break;

        // ...

        default:
            fprintf(stderr, "invalid message type!\n");
            return;
    }

    // these two checks should never happen but just in case.
    //
    if(!buf_ptr) {
        fprintf(stderr, "'write_message': failed to get pointer to buffer.\n");
        return;
    }
    if(!size_ptr) {
        fprintf(stderr, "'write_message': failed to get pointer to buffer size\n");
        return;
    }

    // build the buffer here and then copy it to the buffer(buf_ptr)
    // with null character at the end.
    char buffer[max_size];
    size_t buf_size = 0;
    size_t i = 0;

    memset(buffer, 0, max_size);

    while(1) {
        
        char c = err[i];
        if(c == 0) {
            break;
        }

        if(c == '%') {
            char t = err[i+1];

            switch(t) {
                
                case 'u':
                case 'd':
                case 'i':
                    buf_size += snprintf(buffer + buf_size,
                            max_size, "%i", va_arg(ptr, int));
                    break;

                case 'l':
                    buf_size += snprintf(buffer + buf_size,
                            max_size , "%li", va_arg(ptr, size_t));
                    break;

                case 's':
                    buf_size += snprintf(buffer + buf_size,
                            max_size, "%s", va_arg(ptr, char*));
                    break;
           
                case 'X':
                case 'x':
                    buf_size += snprintf(buffer + buf_size,
                            max_size, "%x", va_arg(ptr, unsigned int));
                    break;

            }

            if(buf_size >= max_size) {
                buf_size = max_size - 1;
                break;
            }

            i++;
        }
        else {
            buffer[buf_size] = c;
            buf_size++;
        }

  
        i++;

        if(i >= max_size) {
            break;
        }
    }
  

    va_end(ptr);

    if((*size_ptr + buf_size) >= max_size) {
        memset(buf_ptr, 0, max_size);
        *size_ptr = 0;
    }

    memmove(buf_ptr + *size_ptr, buffer, buf_size);
    *size_ptr += buf_size;

}

void clear_error_buffer(struct editor_t* ed) {
    memset(ed->error_buf, 0, ERROR_BUFFER_MAX_SIZE);
    ed->error_buf_size = 0;
}

void clear_info_buffer(struct editor_t* ed) {
    memset(ed->info_buf, 0, INFO_BUFFER_MAX_SIZE);
    ed->info_buf_size = 0;
}


/*
void draw_error_buffer(struct editor_t* ed) {
    if(ed->error_buf_size > 0) {
        
        const int bytes_shown = 64;

        float width = ed->font.r_width * bytes_shown;
        float height = ed->font.r_height * 8; // TODO: not always 8 if max size is changed.

        float x = ed->window_width / ed->font.width - (bytes_shown+2);
        float y = 1;

        // background
        glColor3f(0.15, 0.1, 0.1);

        float xloc = column_to_location(ed, x) - 5;
        float yloc = row_to_location(ed, y) - 5;

        draw_framed_rect(ed, 
                xloc, 
                yloc,
                width, height, 0.8, 0.2, 0.2, 3.0, MAP_XYWH);


        glColor3f(0.7, 0.4, 0.4);
        font_draw_data(ed, "--> ERROR <--", 13, x, y, 1);
        
        glColor3f(0.5, 0.4, 0.4);
        font_draw_data(ed, "Press ALT+C to close this box", 29, x+15, y, 1);


        glColor3f(0.9, 0.85, 0.85);
        font_draw_data_wrapped(ed, ed->error_buf, ed->error_buf_size, 
                x, y+1, x+60);
    }
}

void draw_info_buffer(struct editor_t* ed) {
    if(ed->info_buf_size > 0) {
        
        float x = 1;
        float y = ed->window_height - 2*ed->font.height-3;

        glColor3f(0.05, 0.05, 0.05);

        draw_framed_rect(ed, x, y, ed->window_width-1, ed->font.r_height,
                0.1, 0.3, 0.3, 
                0.3, MAP_XYWH);

        
        glColor3f(0.2, 0.4, 0.4);
        font_draw_char(ed, x+10, y, '>', 0);

        glColor3f(0.5, 0.75, 0.75);
        font_draw_char(ed, x + ed->font.width+10, y, '>', 0);

        glColor3f(0.4, 0.5, 0.5);
        font_draw_data(ed, ed->info_buf, ed->info_buf_size, 
                x + ed->font.width*2 + 20, y, 0);
    }
}
*/


int setup_buffers(struct editor_t* ed) {
    int ok = 0;

    for(int i = 0; i < MAX_BUFFERS; i++) {
        struct buffer_t* buf = &ed->buffers[i];
        if(buf) {
            if(!setup_buffer(buf, i)) {
                fprintf(stderr, "buffer '%i' failed to initialize.\n", i);
                goto error;
            }
        }
    }
    ok = 1;

error:
    return ok;
}

struct editor_t* init_editor(const char* fontfile, 
        int window_width, int window_height, int fullscreen) {

    if(!fontfile) {
        fprintf(stderr, "the font file cant be NULL.\n");
        goto giveup;
    }

    struct editor_t* ed = NULL;
    ed = malloc(sizeof *ed);

    if(!ed) {
        fprintf(stderr, "failed to allocate memory for editor!\n");
        goto giveup;
    }

    ed->ready = 0;
    ed->mode = -1;
    ed->win = NULL;
    ed->window_width = 0;
    ed->window_height = 0;
    ed->current_buffer = 0;
    ed->max_row = 0;
    ed->max_column = 0;
    ed->num_active_buffers = 1;
    ed->vbo = 0;
    ed->vao = 0;
    ed->shader = 0;
    ed->shader_color_uniloc = -1;
    ed->cmd_cursor = 0;

    if(!glfwInit()) { 
        // TODO  handle glfw errors better!
        fprintf(stderr, "glfw failed to initialize.\n");
        goto giveup;
    }
   
    printf("+ initialized glfw.\n");

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, fullscreen);
    glfwWindowHint(GLFW_FLOATING, !fullscreen);
    glfwWindowHint(GLFW_RESIZABLE, fullscreen);
    glfwWindowHint(GLFW_DECORATED, 1);
    
    ed->win = glfwCreateWindow(window_width, window_height, "Editor", NULL, NULL);
    if(!ed->win) {
        fprintf(stderr, "failed to create window.\n");
        goto giveup;
    }
    printf("+ window created.\n");

    glfwMakeContextCurrent(ed->win);


    GLenum glew_err = glewInit();
    if(glew_err != GLEW_OK) {
        fprintf(stderr, "error when intializing glew: %s\n", glewGetErrorString(glew_err));
        goto giveup;
    }

    printf("  glew version: %s\n", glewGetString(GLEW_VERSION));
    printf("  opengl version: %s\n", glGetString(GL_VERSION));

    glfwSetWindowSizeLimits(ed->win, 800, 700, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(ed->win, ed); // set user pointer for use in callbacks
                                           //
    glfwSetFramebufferSizeCallback(ed->win, _framebuffer_size_callback);
    glfwSetKeyCallback    (ed->win, key_input_handler);
    glfwSetCharCallback   (ed->win, char_input_handler);
    glfwSetScrollCallback (ed->win, scroll_input_handler);
    glfwSetMouseButtonCallback (ed->win, mouse_bttn_input_handler);

    glfwGetWindowSize(ed->win, &ed->window_width, &ed->window_height);

    

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    if(!load_font(fontfile, &ed->font, 
                FONT_VERTEX_SHADER_SRC,
                FONT_FRAGMENT_SHADER_SRC)) {
        goto giveup;
    }
    

    printf("font loaded '%s'\n", fontfile);

    
    ed->max_column = (ed->window_width / ed->font.char_w);
    ed->max_row = (ed->window_height / ed->font.char_h) - 2;


   
    ed->shader = create_shader_program(
            DRW_VERTEX_SHADER_SRC,  
            DRW_FRAGMENT_SHADER_SRC
            );

    if(!ed->shader) {
        goto giveup;
    }



    glGenVertexArrays(1, &ed->vao);
    glBindVertexArray(ed->vao);

    glGenBuffers(1, &ed->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ed->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, NULL, GL_DYNAMIC_DRAW);

    // positions
    //
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
   
    ed->shader_color_uniloc = glGetUniformLocation(ed->shader, "v_color");


    memset(ed->error_buf, 0, ERROR_BUFFER_MAX_SIZE);
    memset(ed->info_buf, 0, INFO_BUFFER_MAX_SIZE);
    if(!setup_buffers(ed)) {
        goto giveup;
    }
    ed->cmd_str = create_string();

    //  -- ready.
    ed->mode = MODE_NORMAL;
    ed->ready = 1;

giveup:
    return ed;
}


void cleanup_editor(struct editor_t** e) {
    if((*e)) {
        printf("cleanup.\n");
        for(int i = 0; i < MAX_BUFFERS; i++) {
            cleanup_buffer(&(*e)->buffers[i]);
        }
        
        if((*e)->win) {
            glfwDestroyWindow((*e)->win);
            (*e)->win = NULL;
            printf(" destroyed window.\n");
        }
     
        if((*e)->ready) {
            glfwTerminate();
            printf(" terminated glfw.\n");
        }

        delete_shader_program((*e)->shader);
        if((*e)->vbo) {
            glDeleteBuffers(1, &(*e)->vbo);
        }
        if((*e)->vao) {
            glDeleteVertexArrays(1, &(*e)->vao);
        }

        unload_font(&(*e)->font);
        cleanup_string(&(*e)->cmd_str);

        free(*e);
        *e = NULL;
        
        printf("\033[32m freed editor, bye.\033[0m\n");
    }
}


