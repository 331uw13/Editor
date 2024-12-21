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
#include "config.h"


void _framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);

    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(ed) {
        ed->window_width = width;
        ed->window_height = height;

        const int cols = width / CELLW;
        const int rows = height / CELLH;

        if(ed->font.ready) {
            ed->max_column = cols;
            ed->max_row = rows;
        }


        printf("resized window:%ix%i\n", ed->window_width, ed->window_height);
   
        for(int i = 0; i < ed->num_buffers; i++) {
            struct buffer_t* buf = &ed->buffers[i];
            buf->height = height;
            buf->width = width;
            buf->max_col = cols;
            buf->max_row = rows - 1;
        }
    }

}

int is_safe_to_continue(struct editor_t* ed) {
    int result = 0;

    if(!ed) {
        fprintf(stderr, "[ERROR] %s | the main pointer is NULL!\n",
                __func__);
        goto done;
    }

    if(ed->current_bufid >= MAX_BUFFERS) {
        ed->current_bufid = MAX_BUFFERS-1;
    }


    struct buffer_t* buf = &ed->buffers[ed->current_bufid];
    if(!buf->lines) {
        fprintf(stderr, "[ERROR] %s | buffer doesnt seem to be initialized.\n",
                __func__);
        goto done;
    }

    buf->cursor_x = liclamp(buf->cursor_x, 0, buf->current->data_size);
    buf->cursor_y = liclamp(buf->cursor_y, 0, buf->num_used_lines);


    result = 1;

done:
    return result;
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

int confirm_user_choice(struct editor_t* ed, char* question, int pre_select) {
    int result = 0;

    if(!question) {
        goto error;
    }

    const size_t q_len = strlen(question);
    if(q_len == 0) {
        fprintf(stderr, "question is empty.\n");
        goto error;
    }

    int q_maxcol = ed->cfg[CFG_QUESTION_MAXCOL];
    int rows = 1+count_data_linewraps(question, q_len, q_maxcol);
    int wait_for_answer = 1;

    ed->mode = MODE_CONFIRM_CHOICE;

    double p_nowtime = glfwGetTime();
    double time = 0.0;

    while(wait_for_answer && !glfwWindowShouldClose(ed->win)) {
        glClear(GL_COLOR_BUFFER_BIT);

        if(glfwGetKey(ed->win, GLFW_KEY_Y)) {
            result = USER_ANSWER_YES;
            break;
        }
        else if(glfwGetKey(ed->win, GLFW_KEY_N)) {
            result = USER_ANSWER_NO;
            break;
        }
        else if(glfwGetKey(ed->win, GLFW_KEY_LEFT)) {
            pre_select = PRESELECT_YES;
        }
        else if(glfwGetKey(ed->win, GLFW_KEY_RIGHT)) {
            pre_select = PRESELECT_NO;
        }
        else if(glfwGetKey(ed->win, GLFW_KEY_ENTER)) {
            if(time > 0.1) { 
                // need to add time delay here 
                // otherwise it can go through after executing a command.
                result = !pre_select;
                break;
            }
        }


        draw_everything(ed);
    
        int x = ed->max_column / 2 - (q_maxcol / 2);
        int y = ed->max_row / 2 - (rows / 2);

        set_color(ed, 0x2a241b);
        draw_rect(ed, x-1, y-1, q_maxcol+2, rows+3, DRW_ONGRID, 0.0, 2.5);


        font_set_color_hex(&ed->font, 0xd08d2a);
        draw_data_wrp(ed, x, y, question, q_len, q_maxcol);

        draw_char(ed, x-1, y-1, '"');

        font_set_color_hex(&ed->font, 0x946a2b);
        draw_data(ed, x, y+rows+1, "(y)es / (n)o\0", -1);


        set_color(ed, 0xd08d2a);
        draw_rect(ed, 
                (x + pre_select * 8) * CELLW+10,
                (y+rows+2) * CELLH + 3,

                CELLW*4, 1, DRW_NO_GRID, DRW_NOADJAFTER);


        
        glfwSwapBuffers(ed->win);
        glfwPollEvents();

        const double nowtime = glfwGetTime();
        time += (nowtime - p_nowtime);
        p_nowtime = nowtime;
    }

    ed->mode = MODE_NORMAL;

error:
    return result;
}


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
        ed->tabs_visible = 1;
    }

    res = 1;

error:
    return res;
}

void map_xywh(struct editor_t* ed, float* x, float* y, float* w, float* h) {
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

    const int errbuf_maxcol = ed->cfg[CFG_ERRORBUF_MAXCOL];
    const size_t rows = 1+count_data_linewraps(
            ed->error_buf, ed->error_buf_size, errbuf_maxcol
            );

    int x = ed->max_column - errbuf_maxcol;
    int y = 0;


    set_color(ed, 0x241917);
    draw_rect(ed, x-2, y, errbuf_maxcol+2, rows, DRW_ONGRID, DRW_NOADJAFTER);

    draw_rect(ed, x-9, y, 7, 1, DRW_ONGRID, DRW_NOADJAFTER);
    font_set_color_hex(&ed->font,0xe74c37);
    draw_data(ed, x-9, y, "(error)", 7);
    
    font_set_color_hex(&ed->font, 0xc66352);
    draw_data_wrp(ed, x-1, y, ed->error_buf, ed->error_buf_size, errbuf_maxcol);

    ed->error_buf_lw = rows;
}

void draw_info_buffer(struct editor_t* ed) {
    if(ed->info_buf_size == 0) {
        return;
    }


    const int infobuf_maxcol = ed->cfg[CFG_INFOBUF_MAXCOL];
    const size_t rows = 1+count_data_linewraps(
            ed->info_buf, ed->info_buf_size, infobuf_maxcol
            );


    int x = ed->max_column - infobuf_maxcol;
    int y = 0;
    if(ed->error_buf_size > 0) {
        y += ed->error_buf_lw;
    }

    set_color(ed, 0x222242);

    draw_rect(ed, x-4, y, 2, 1, DRW_ONGRID, DRW_NOADJAFTER);
    font_set_color_hex(&ed->font, 0x008080);
    draw_data(ed, x-4, y, "->", 2);

    draw_rect(ed, x-2, y, infobuf_maxcol+2, rows, DRW_ONGRID, DRW_NOADJAFTER);

    font_set_color_hex(&ed->font, 0x00D2D2);
    draw_data_wrp(ed, x-1, y, 
            ed->info_buf, ed->info_buf_size, infobuf_maxcol);

}


int init_editor(struct editor_t* ed, const char* fontfile, 
        int window_width, int window_height) {

    int result = 0;

    if(!fontfile) {
        fprintf(stderr, "the font file cant be NULL.\n");
        goto giveup;
    }

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
    ed->tabs_visible = 0;
    ed->error_buf_lw = 0;

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

    for(int i = 0; i < NUM_CONFIG_VARS; i++) {
        ed->cfg[i] = 0;
    }

    for(int i = 0; i < NUM_KEYBINDS; i++) {
        ed->keybinds[i] = 0;
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

    /*
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, fullscreen);
    glfwWindowHint(GLFW_FLOATING, !fullscreen);
    glfwWindowHint(GLFW_RESIZABLE, fullscreen);
    glfwWindowHint(GLFW_DECORATED, 1);
    */

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

    ed->cmdstr = create_string(CMDSTR_MAX_SIZE);
    ed->clipbrd = create_string(CLIPBOARD_INIT_SIZE);

    config_setup(ed);

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

    delete_string(&e->cmdstr);
    delete_string(&e->clipbrd);

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
    }
    if(e->vao) {
        glDeleteVertexArrays(1, &e->vao);
    }
}

