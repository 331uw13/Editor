#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <zlib.h>

#include "editor.h"
#include "input_handler.h"
#include "utils.h"


void _framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);

    struct editor_t* ed = glfwGetWindowUserPointer(win);
    if(ed) {
        ed->window_width = width;
        ed->window_height = height;
        printf("resized:%ix%i\n", ed->window_width, ed->window_height);
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

    if(buf->cursor_y > buf->num_lines) {
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
    return (float)(col * (ed->font.header.width * ed->font.scale) + EDITOR_X_PADDING);
}

float row_to_location(struct editor_t* ed, size_t row) {
    return (float)(row * (ed->font.header.height * ed->font.scale) + EDITOR_Y_PADDING);
}

void set_font_scale(struct editor_t* ed, float scale) {
    ed->font.scale = scale;
    ed->font.width = (ed->font.header.width) * (scale);
    ed->font.height = (ed->font.header.height) * (scale);

    ed->font.r_width = ed->font.width * 2;
    ed->font.r_height = ed->font.height * 1.8;

}

int load_font_from_file(const char* fontfile, struct psf2_font* font) {
    int res = 0;
    if(font == NULL || fontfile == NULL) {
        fprintf(stderr, "no fontfile or pointer to font at 'load_font_from_file'\n");
        goto error;
    }

    if(access(fontfile, F_OK) != 0) {
        fprintf(stderr, "the font file doesnt exist.\n");
        goto error;
    }

    gzFile file = gzopen(fontfile, "r");
    if(file == NULL) {
        perror("gzopen");
        fprintf(stderr, "failed to open the font file.\n");
        goto error;
    }

    size_t length = gzfread(&font->header, sizeof font->header, 1, file);
    if(length == 0) {
        fprintf(stderr, "%s\n", gzerror(file, NULL));
        goto error_and_close;
    }

    if( (font->header.magic[0] != 0x72) ||
        (font->header.magic[1] != 0xb5) ||
        (font->header.magic[2] != 0x4A) ||
        (font->header.magic[3] != 0x86)) {
        fprintf(stderr, "'%s' header magic bytes dont match.\n", fontfile);
        goto error_and_close;
    }


    if(gzeof(file)) {
        fprintf(stderr, "file '%s' is missing header.\n", fontfile);
        goto error_and_close;
    }

    font->data_size = 256*font->header.height;
    font->data = malloc(font->data_size);
    if(!font->data) {
        fprintf(stderr, "failed to allocate memory for font data.\n");
        goto error_and_close;
    }

    font->scale = 1.5;
    font->spacing = 0;

    if((res = gzfread(font->data, font->data_size, 1, file)) == 0) {
        fprintf(stderr, "%s\n", gzerror(file, NULL));
    }

error_and_close:
    gzclose(file);

error:
    return res;
}



void unload_font(struct psf2_font* font) {
    if(font) {
        if(font->data) {
            free(font->data);
            font->data = NULL;
            printf("\033[32m unloaded font.\033[0m\n");
        }
    }
}


void font_draw_char(struct editor_t* ed, int col, int row, char c) {
    if(!ed || c <= 0x1F || c >= 0x7F) { return; }
    if(!ed->font.data) { return; }

    float x = column_to_location(ed, col);
    float y = row_to_location(ed, row);

    glPointSize(ed->font.scale);
    glBegin(GL_POINTS);

    int origin_x = x;


    for(unsigned char i = 0; i < ed->font.header.height; i++) {
        unsigned char g = ed->font.data[c*ed->font.header.height+i];
        for(int j = 0; j < 8; j++) {
            if(g & 0x80) {
                float mx = map(x, 0.0, ed->window_width, -1.0, 1.0);
                float my = map(y, 0.0, ed->window_height, 1.0, -1.0);
            
                glVertex2f(mx, my);

            }
            g = g << 1;
            x += ed->font.scale;
        }
        y += ed->font.scale;
        x = origin_x;
    }

    glEnd();
}


void font_draw_str(struct editor_t* ed, char* str, size_t size, int col, int row) {
    if(!str || size == 0) { return; }

    for(size_t i = 0; i < size; i++) {
        char c = str[i];

        switch(c) {
            case '\t':
                col += FONT_TAB_WIDTH;
                continue;

            case 0:
                return;
        }

        font_draw_char(ed, col, row, c);
        col++;
    }
}

void font_draw_str_wrapped(struct editor_t* ed, char* str, 
        size_t size, int col, int row, int max_column) {
    if(!str || size == 0) { return; }

    const int col_origin = col;

    size_t word_size = 0;
    char word_buf[LINE_WRAP_WORD_BUFFER_SIZE] = {0};

    for(size_t i = 0; i < size; i++) {
        char c = str[i];


        if(word_size >= LINE_WRAP_WORD_BUFFER_SIZE) {

            word_size = LINE_WRAP_WORD_BUFFER_SIZE;

            // no need to check for tabs or spaces because it would have been wrapped earlier,
            // this "word" doesnt have them so just draw it and go to next line when max column is reached.

            for(size_t j = 0; j < word_size; j++) {
                char b = word_buf[j];
                if(b == 0) {
                    return;
                }
                font_draw_char(ed, col, row, b);
                col++;
                if(col > max_column) {
                    col = col_origin;
                    row++;
                }
            }

            //memset(word_buf, 0, word_size);
            word_size = 0;

        }

        if(c == '\t') {
            col += FONT_TAB_WIDTH;
            continue;
        }

        if(c == 0) {
            return;
        }

        word_buf[word_size] = c;
        word_size++;


        if(c == 0x20 || (i+1 == size)) {

            if((col + word_size) > max_column) {
                row++;
                col = col_origin;
            }

            font_draw_str(ed, word_buf, word_size, col, row);
            col += word_size;
            word_size = 0;

        }
    }
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
    }
    if(h) {
        *h = (*h > 0) ? (*h / ed->window_height) : *h;
    }
}


void draw_rect(struct editor_t* ed, float x, float y, float w, float h, int flag) {
    if(flag == MAP_XYWH) {
        map_xywh(ed, &x, &y, &w, &h);
    }
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+w, y);
    glVertex2f(x+w, y-h);
    glVertex2f(x, y-h);
    glEnd();
}

void draw_framed_rect(struct editor_t* ed, 
        float x, float y, float w, float h, 
        float fr, float fg, float fb,  float fthickness,
        int flag) {


    if(flag == MAP_XYWH) {
        map_xywh(ed, &x, &y, &w, &h);
        flag = XYWH_ALREADY_MAPPED;
    }

    draw_rect(ed, x, y, w, h, flag);

    glColor3f(fr, fg, fb);
    glLineWidth(fthickness);
    glBegin(GL_LINES);

    glVertex2f(x, y);
    glVertex2f(x+w, y);

    glVertex2f(x, y-h);
    glVertex2f(x+w, y-h);

    glVertex2f(x, y);
    glVertex2f(x, y-h);

    glVertex2f(x+w, y);
    glVertex2f(x+w, y-h);

    glEnd();


}

void draw_line(struct editor_t* ed, float x0, float y0, float x1, float y1, 
        float thickness, int flag) {
    if(flag == MAP_XYWH) {
        map_xywh(ed, &x0, &y0, NULL, NULL);
        map_xywh(ed, &x1, &y1, NULL, NULL);
    }
    glLineWidth(thickness);
    glBegin(GL_LINES);

    glVertex2f(x0, y0);
    glVertex2f(x1, y1);

    glEnd();
}

void write_error(struct editor_t* ed, char* err, ...) {
    if(!err) { return; }

    va_list ptr;
    va_start(ptr, err);

    size_t i = 0;

    if(ed->error_buf_size >= ERROR_BUFFER_MAX_SIZE) {
        ed->error_buf_size = 0;
        memset(ed->error_buf, 0, ERROR_BUFFER_MAX_SIZE);
    }


    // build the buffer here and then copy it to the real 'editor->error_buf' 
    // with 0 byte at the end.
    char buffer[ERROR_BUFFER_MAX_SIZE] = {0};
    size_t buf_size = 0;

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
                            ERROR_BUFFER_MAX_SIZE, "%i", va_arg(ptr, int));
                    break;

                case 's':
                    buf_size += snprintf(buffer + buf_size,
                            ERROR_BUFFER_MAX_SIZE, "%s", va_arg(ptr, char*));
                    break;
           
                case 'X':
                case 'x':
                    buf_size += snprintf(buffer + buf_size,
                            ERROR_BUFFER_MAX_SIZE, "%x", va_arg(ptr, unsigned int));
                    break;

            }

            if(buf_size >= ERROR_BUFFER_MAX_SIZE) {
                buf_size = ERROR_BUFFER_MAX_SIZE-1;
                break;
            }

            i++;
        }
        else {
            buffer[buf_size] = c;
            buf_size++;
        }

  
        i++;

        if(i >= ERROR_BUFFER_MAX_SIZE) {
            break;
        }
    }
   
    va_end(ptr);

    if((ed->error_buf_size + buf_size) >= ERROR_BUFFER_MAX_SIZE) {
        clear_error_buffer(ed);
    }

    memmove(ed->error_buf + ed->error_buf_size, buffer, buf_size);
    ed->error_buf_size += buf_size;

}

void clear_error_buffer(struct editor_t* ed) {
    if(ed->error_buf_size >= ERROR_BUFFER_MAX_SIZE) {
        ed->error_buf_size = ERROR_BUFFER_MAX_SIZE;
    }
    memset(ed->error_buf, 0, ed->error_buf_size);
    ed->error_buf_size = 0;
}


void draw_errors(struct editor_t* ed) {
    if(ed->error_buf_size > 0) {
        
        const int bytes_shown = 64;

        float width = ed->font.r_width * bytes_shown;
        float height = ed->font.r_height * 8;

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
        font_draw_str(ed, "--> ERROR <--", 13, x, y);
        
        glColor3f(0.5, 0.4, 0.4);
        font_draw_str(ed, "Press ALT+C to close this box", 29, x+15, y);


        glColor3f(0.9, 0.85, 0.85);
        font_draw_str_wrapped(ed, ed->error_buf, ed->error_buf_size, 
                x, y+1, x+60);
        
    }
}


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

    if(!load_font_from_file(fontfile, &ed->font)) {
        free(ed);
        ed = NULL;
        goto giveup;
    }

    ed->ready = 0;
    ed->win = NULL;
    ed->window_width = 0;
    ed->window_height = 0;
    ed->current_buffer = 0;

    memset(ed->error_buf, 0, ERROR_BUFFER_MAX_SIZE);

    if(!setup_buffers(ed)) {
        goto giveup;
    }


    set_font_scale(ed, 1.5);

    if(!glfwInit()) { 
        // TODO  handle glfw errors better!
        fprintf(stderr, "glfw failed to initialize.\n");
        goto giveup;
    }
    

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, fullscreen);
    glfwWindowHint(GLFW_FLOATING, !fullscreen);
    glfwWindowHint(GLFW_RESIZABLE, fullscreen);
    
    glfwWindowHint(GLFW_DECORATED, 1);
    
    ed->win = glfwCreateWindow(window_width, window_height, "Editor", NULL, NULL);
    if(!ed->win) {
        fprintf(stderr, "failed to create window.\n");
        goto giveup;
    }

    glfwMakeContextCurrent(ed->win);

    glfwSetWindowSizeLimits(ed->win, 800, 700, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwGetWindowSize(ed->win, &ed->window_width, &ed->window_height);
    glfwSetWindowUserPointer(ed->win, ed); // set user pointer for use in callbacks
   
    glfwSetFramebufferSizeCallback(ed->win, _framebuffer_size_callback);
    glfwSetKeyCallback(ed->win, key_input_handler);
    glfwSetCharCallback(ed->win, char_input_handler);

    ed->ready = 1;

giveup:
    return ed;
}


void cleanup_editor(struct editor_t** e) {
    if((*e)) {
        

        for(int i = 0; i < MAX_BUFFERS; i++) {
            cleanup_buffer(&(*e)->buffers[i]);
        }
        
        if((*e)->win) {
            glfwDestroyWindow((*e)->win);
            (*e)->win = NULL;
            printf("\033[32m destroyed window.\033[0m\n");
        }
        
        if((*e)->ready) {
            glfwTerminate();
            printf("\033[32m terminated glfw.\033[0m\n");
        }
        
        unload_font(&(*e)->font);
        

        free(*e);
        *e = NULL;
        
        printf("\033[32m freed editor, bye.\033[0m\n");
    }
}


