#ifndef EDITOR_H
#define EDITOR_H

#include <GLFW/glfw3.h>
#include "buffer.h"
#include "config.h" // TODO

#define FONT_TAB_WIDTH 4
#define MAX_BUFFERS 8
#define ERROR_BUFFER_MAX_SIZE 256
#define LINE_WRAP_WORD_BUFFER_SIZE 512

#define EDITOR_X_PADDING 10
#define EDITOR_Y_PADDING 10

struct psf2header {
    unsigned char magic[4];
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int length;
    unsigned int charsize;
    unsigned int height;
    unsigned int width;
};

struct psf2_font {
    struct psf2header header;
    unsigned char* data;
    unsigned int data_size;
    float scale;
    int spacing;

    int width;  // in scale
    int height; //

    int r_width;   // rendering font width or height size anything is kind of fucked.
    int r_height;  // so this should help a little bit until its fixed.

};

struct editor_t {
    GLFWwindow* win;
    struct psf2_font font;


    struct buffer_t buffers[MAX_BUFFERS];
    unsigned int current_buffer;

    int window_width;
    int window_height;


    // NOTE: the error which is written to the buffer must be null terminated
    char error_buf[ERROR_BUFFER_MAX_SIZE];
    size_t error_buf_size; 

    double mouse_x;
    double mouse_y;

    // set to 1 if init_editor() returns with pointer,
    //                           GLFW is initialized and everything should be fine.
    int ready; 
};

void do_safety_check(struct editor_t* ed); // TODO make this better <---

float column_to_location(struct editor_t* ed, size_t col);
float row_to_location(struct editor_t* ed, size_t row);

void set_font_scale(struct editor_t* ed, float scale);
int  load_font_from_file(const char* fontfile, struct psf2_font* font);
void unload_font(struct psf2_font* font);

void font_draw_char(struct editor_t* ed, int col, int row, char c);
void font_draw_str(struct editor_t* ed, char* str, size_t size, int col, int row);

void font_draw_str_wrapped(struct editor_t* ed, char* str, 
        size_t size, int col, int row, int max_column);

// map X, Y, WIDTH, HEIGHT to usable coordinates.
// if pointer is set to NULL, continues to operate and leaves the null pointer alone
void map_xywh(struct editor_t* ed, 
        float* x, float* y, float* w, float* h);


// for drawing functions.
#define MAP_XYWH 0               // map coordinates to coordinates for opengl.
#define XYWH_ALREADY_MAPPED 1    // 

void draw_rect(struct editor_t* ed, float x, float y, float w, float h, int flag);

void draw_framed_rect(struct editor_t* ed, 
        float x, float y, float w, float h, 
        float frame_r, float frame_g, float frame_b, // rgb 0.0 - 1.0
        float fthickness, // frame thickness
        int flag);

void draw_line(struct editor_t* ed, float x0, float y0, float x1, float y1, 
        float thickness, int flag);

void write_error(struct editor_t* ed, char* err, ...);
void clear_error_buffer(struct editor_t* ed);
void draw_errors(struct editor_t* ed);


int setup_buffers(struct editor_t* ed);

struct editor_t* init_editor(
        const char* fontfile,
        int window_width, int window_height,
        int fullscreen);

void cleanup_editor(struct editor_t** e);


#endif



