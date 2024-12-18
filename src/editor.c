#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <zlib.h>
#include <GL/glew.h>
#include <math.h>

#include "editor.h"
#include "input_handler.h"
#include "utils.h"
#include "shader.h"
#include "draw.h"
#include "memory.h"

void _framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);

    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(ed) {
        ed->window_width = width;
        ed->window_height = height;

        if(ed->font.ready) {
            ed->max_column = width / CELLW;
            ed->max_row = height / CELLH;
        }

        printf("resized window:%ix%i\n", ed->window_width, ed->window_height);
   
        for(int i = 0; i < ed->num_buffers; i++) {
            struct buffer_t* buf = &ed->buffers[i];
            buf->height = height;
            buf->width = width;
            buf->max_col = width / CELLW;
            buf->max_row = height / CELLH;
        }
    }

}

void do_safety_check(struct editor_t* ed) {
    if(!ed) {
        fprintf(stderr, "oh no... where is my pointer.\n");
        return;
    }

    if(ed->current_bufid > MAX_BUFFERS) {
        ed->current_bufid = MAX_BUFFERS;  
    }

    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    

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

float col_to_loc(struct editor_t* ed, long int col) {
    return (col * ed->font.char_w + EDITOR_X_PADDING)
        * EDITOR_TEXT_X_SPACING;
}

float row_to_loc(struct editor_t* ed, long int row) {
    return (row * ed->font.char_h + EDITOR_Y_PADDING)
        * EDITOR_TEXT_Y_SPACING;
}

long int loc_to_col(struct editor_t* ed, float col) {
    long int c = 0;
    if(col > 0.0f) {
        c = floorf((col / ed->font.char_w) / EDITOR_TEXT_X_SPACING);
    }
    return c;
}

long int loc_to_row(struct editor_t* ed, float row) {
    long int r = 0;
    if(row > 0.0f) {
        r = floorf((row / ed->font.char_h) / EDITOR_TEXT_Y_SPACING);
    }
    return r;
}

int confirm_user_choice(struct editor_t* ed, char* question) {
    int res = 0;
    int wait_for_answer = 1;

    if(!question) {
        goto error;
    }

    const size_t q_len = strlen(question);
    if(q_len == 0) {
        fprintf(stderr, "question is empty.\n");
        goto error;
    }


    const int max_column = q_len + 4;
    const float w = max_column * CELLW;
    
    int num_lines = count_data_linewraps(question, q_len, max_column);
    const float h = CELLH * (num_lines + 4);

    if(h * w <= 0) {
        fprintf(stderr, "question box is too small.\n");
        goto error;
    }

    const float x = ed->window_width / 2 - (w / 2);
    const float y = ed->window_height / 2 - (h / 2);

    int old_mode = ed->mode;
    ed->mode = MODE_CONFIRM_CHOICE;

    while(wait_for_answer && !glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);
        draw_everything(ed);

        set_color_hex(ed, 0x242010);
        draw_framed_rect(ed,
                x, y, w, h,
                0x8E8E50,
                2.0,
                MAP_XYWH, DRW_NO_GRID);


        font_set_color_hex(&ed->font, 0x8E8E4F);
        int num_nl = draw_data_w(ed, 
                x + CELLW,
                y + CELLH,
                question,
                q_len,
                x + w,
                DRW_NO_GRID
                );

        draw_data(ed,
                x + CELLW,
                y + (CELLH * (num_nl + 2)),
                "Y(es) / N(o)\0", -1,
                DRW_NO_GRID);

        if(glfwGetKey(ed->win, GLFW_KEY_Y) == GLFW_PRESS) {
            res = USER_ANSWER_YES;
            wait_for_answer = 0;
        }
        else if(glfwGetKey(ed->win, GLFW_KEY_N) == GLFW_PRESS) {
            res = USER_ANSWER_NO;
            wait_for_answer = 0;
        }


        glfwSwapBuffers(ed->win);
        glfwWaitEvents();
    }

    ed->mode = old_mode;

error:
    return res;
}

/*
void _buffer_layout(struct editor_t* ed) {
    
    struct buffer_t* buf = NULL;
    int num_bufs = iclamp(ed->num_buffers, 0, MAX_BUFFERS);

    if(num_bufs == 0) {
        fprintf(stderr, "number of active buffers is zero.\n");
        return;
    }

    const int cw = CELLW;
    const int ch = CELLH;

    int max_width = ed->window_width / num_bufs;
    int max_height = ed->window_height - ch;

    for(int i = 0; i < num_bufs; i++) {
        buf = &ed->buffers[i];
        if(!buf) {
            continue;
        }

        buf->x = i * (max_width);
        buf->y = 0;
        buf->width = max_width;
        buf->height = max_height;


        buf->max_col = buf->width / cw;
        buf->max_row = buf->height / ch;

    }
}
*/


int editor_add_buffer(struct editor_t* ed) {
    int res = 0;

    unsigned int nextindex = ed->num_buffers + 1;
    if(nextindex >= MAX_BUFFERS) {
        write_message(ed, ERROR_MSG, "Max buffers reached.\0");
        goto error;
    }

    struct buffer_t* buf = &ed->buffers[ed->num_buffers];

    if(buf->ready) {
        write_message(ed, ERROR_MSG, "Buffer with id %i seems to be already created.\0", 
                buf->id);
        goto error;
    }

    if(!create_buffer(ed, buf, ed->num_buffers)) {
        goto error;
    }

    //set_buffer_layout(ed, buf, 0, 0);

    ed->num_buffers = nextindex;

    // TODO: dont change this if used has set it to not visible
    if(ed->num_buffers > 1) {
        ed->show_tabs = 1;
    }

    res = 1;

error:
    return res;
}
/*
static void buffer_layout_R(struct editor_t* ed, struct buffer_t* buf, int x, int y, int w, int h) {
    buf->x = x;
    buf->y = y;
    buf->width = w;
    buf->height = h;
    buf->max_col = buf->width / CELLW;
    buf->max_row = buf->height / CELLH;
}

void editor_dump_buffer_layout(struct editor_t* ed) {
    printf("---- Editor Buffer Layout ---\n");

    for(int i = 0; i < LAYOUT_MAX_BUFFERS; i++) {
        struct buffer_t* lb = ed->layout[i];
        if(lb) {
            printf(" \033[32m%i\033[0m: \033[34mID: %i\033[0m, X: %i, Y: %i,"
                    " W: %i, H: %i, MAXCOL: %i, MAXROW: %i\n",
                    i, lb->id, lb->x, lb->y, lb->width, lb->height,
                    lb->max_col, lb->max_row);
        }
        else {
            printf(" \033[90m%i: <NULL>\033[0m\n", i);
        }
    }

    printf("-----------------------------\n");
}

void set_buffer_layout(struct editor_t* ed, struct buffer_t* buf, int lcol, int lrow) {
   
    if(!buffer_ready(buf)) {
        return;
    }


    const int cw = CELLW;
    const int ch = CELLH;

    int max_h = ed->window_height - ch;
    int max_w = ed->window_width;


    int lwidth = ed->window_width;
    int lheight = ed->window_height;
    int lx = 0;
    int ly = 0;

    int layout_index = (lrow * 2 + lcol);
    struct buffer_t* lb = ed->layout[layout_index];

    if(lrow == 0) {
        if(lb) {
            if(lb->id == buf->id)  {
                fprintf(stderr, "[WARNING] %s | nothing to do the id's match.\n",
                        __func__);
                return;
            }

            // different buffer already exists at 'layout_index' move it to 'next_index'.
            // horizontal.
            
            int next_index = (layout_index+1)%2;
            printf("buffer %p(%i) at %i, move existing to %i\n",
                    lb, lb->id, layout_index, next_index);

            ed->layout[next_index] = lb;
            ed->layout[layout_index] = buf;

            lwidth /= 2;

            buffer_layout_R(ed, lb, lwidth, ly, lwidth, lheight);
        }
        else {
            ed->layout[layout_index] = buf;
        }
       
        buffer_layout_R(ed, buf, lx, ly, lwidth, lheight);
    }

}
*/


void map_xywh(struct editor_t* ed, float* x, float* y, float* w, float* h) {
    // uhh.
    if(x) {
        *x = map(*x, 0.0, ed->window_width, -1.0, 1.0);
    }
    if(y) {
        *y = map(*y, 0.0, ed->window_height, 1.0, -1.0);
    }
    if(w) {
        *w = (*w > 0) ? (*w / ed->window_width) : *w;
        *w *= 2;
    }
    if(h) {
        *h = (*h > 0) ? (*h / ed->window_height) : *h;
        *h *= 2;
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
            clear_info_buffer(ed);
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

    if(type == ERROR_MSG) {
        fprintf(stderr, "%s\n", buffer);
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

void draw_error_buffer(struct editor_t* ed) {
    if(ed->error_buf_size == 0) {
        return;
    }

    const int chars_shown = 32;
    const int bx = ed->max_column - chars_shown - 2;
    const int by = 1;

    set_color_hex(ed, 0x201310);


    int lines_shown = count_data_linewraps(
            ed->error_buf, ed->error_buf_size, chars_shown) + 2;


    draw_framed_rect(ed, 
            bx, by,
            chars_shown, lines_shown,
            0x351010, 2.0, MAP_XYWH, DRW_ONGRID);

    font_set_color_hex(&ed->font, 0x651800);
    draw_data(ed, bx+1, by, "-- ERROR --\0", -1, DRW_ONGRID);
    
    font_set_color_hex(&ed->font, 0x403030);
    draw_data(ed, bx+13, by, "ctrl+o to close\0", -1, DRW_ONGRID);

    font_set_color_hex(&ed->font, 0xA75030);
    draw_data_w(ed, 
            bx, by+1,
            ed->error_buf, ed->error_buf_size,
            bx + chars_shown,
            DRW_ONGRID);
}

void draw_info_buffer(struct editor_t* ed) {
    if(ed->info_buf_size == 0) {
        return;
    }

    const float x = 0;
    const float y = ed->window_height - ed->font.char_h-2;
    const float w = ed->window_width;
    const float h = ed->font.char_h;

    set_color_hex(ed, 0x051212);
    draw_framed_rect(ed,
            x, y,
            w, h,
            0x052030, 2.0, 
            MAP_XYWH, DRW_NO_GRID);


    font_set_color_hex(&ed->font, 0x104050);
    draw_char(ed, x+10, y, '>', DRW_NO_GRID);
    
    font_set_color_hex(&ed->font, 0x109090);
    draw_data(ed, x+ed->font.char_w+20, y, ed->info_buf, ed->info_buf_size, DRW_NO_GRID);
}

int init_editor(struct editor_t* ed, const char* fontfile, 
        int window_width, int window_height, int fullscreen) {

    int result = 0;

    if(!fontfile) {
        fprintf(stderr, "the font file cant be NULL.\n");
        goto giveup;
    }

    
    /*
    struct editor_t* ed = NULL;
    ed = malloc(sizeof *ed);

    if(!ed) {
        fprintf(stderr, "failed to allocate memory for editor!\n");
        goto giveup;
    }
    */

    ed->ready = 0;
    ed->win = NULL;
    ed->window_width = 0;
    ed->window_height = 0;
    ed->current_bufid = 0;
    ed->max_row = 0;
    ed->max_column = 0;
    ed->num_buffers = 0;
    ed->vbo = 0;
    ed->vao = 0;
    ed->shader = 0;
    ed->shader_color_uniloc = -1;
    ed->cmd_cursor = 0;
    ed->mode = -1;
    ed->glfwinitsuccess = 0;
    ed->error_buf_size = 0;
    ed->info_buf_size = 0;
    ed->num_vi_buffers = 0;
    ed->show_tabs = 0;

    for(unsigned int i = 0; i < LAYOUT_MAX_BUFFERS; i++) {
        ed->layout[i] = NULL;
    }

    for(unsigned int i = 0; i < MAX_BUFFERS; i++) {
        struct buffer_t* buf = &ed->buffers[i];
        buf->x = 0;
        buf->y = 0;
        buf->width = 0;
        buf->height = 0;
        buf->max_col = 0;
        buf->max_row = 0;
        buf->id = -1;
        buf->ready = 0;
        buf->lines = NULL;
    }


    init_colors(ed);
    //init_mem_static_vars();

    if(!glfwInit()) { 
        // TODO  handle glfw errors better!
        fprintf(stderr, "glfw failed to initialize.\n");
        goto giveup;
    }
  
    ed->glfwinitsuccess = 1;

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
    glfwSetWindowUserPointer(ed->win, ed);
    glfwSetFramebufferSizeCallback(ed->win, _framebuffer_size_callback);
    glfwSetKeyCallback    (ed->win, key_input_handler);
    glfwSetCharCallback   (ed->win, char_input_handler);
    glfwSetScrollCallback (ed->win, scroll_input_handler);
    //glfwSetMouseButtonCallback (ed->win, mouse_bttn_input_handler);
    glfwGetWindowSize(ed->win, &ed->window_width, &ed->window_height);

    //glfwSetInputMode(ed->win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    if(!load_font(fontfile, &ed->font, 
                FONT_VERTEX_SHADER_SRC,
                FONT_FRAGMENT_SHADER_SRC)) {
        goto giveup;
    }

    printf("+ font loaded '%s'\n", fontfile);


    ed->max_column = ed->window_width / CELLW;
    ed->max_row = (ed->window_height / CELLH) - 2;

   
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
   
    ed->shader_color_uniloc = glGetUniformLocation(ed->shader, "v_color");
    if(ed->shader_color_uniloc < 0) {
        fprintf(stderr, "[ERROR] %s | failed to get shader uniform location\n",
                __func__);
        goto giveup;
    }

    memset(ed->error_buf, 0, ERROR_BUFFER_MAX_SIZE);
    memset(ed->info_buf, 0, INFO_BUFFER_MAX_SIZE);
    /*
    if(!create_buffers(ed)) {
        goto giveup;
    }
    */

    ed->cmd_str = create_string(COMMAND_LINE_MAX_SIZE);
    ed->clipbrd = create_string(CLIPBOARD_INIT_SIZE);


    //  -- ready.

    ed->mode = MODE_NORMAL;
    ed->ready = 1;

    result = 1;

    printf("\033[32meditor is initialized.\033[0m\n");

giveup:
    return result;
}


void cleanup_editor(struct editor_t* e) {
    
    for(int i = 0; i < e->num_buffers; i++) {
        delete_buffer(&e->buffers[i]);
    }

    if(e->glfwinitsuccess) {
        if(e->win) {
            glfwDestroyWindow(e->win);
            e->win = NULL;
            printf(" window deleted.\n");
        }

        glfwTerminate();

        printf(" glfw terminated.\n");
    }

    unload_font(&e->font);

    if(e->vbo) {
        glDeleteBuffers(1, &e->vbo);
        printf(" vbo deleted.\n");
    }
    if(e->vao) {
        glDeleteVertexArrays(1, &e->vao);
        printf(" vao deleted\n");
    }
}

