#ifndef EDITOR_H
#define EDITOR_H

/*


   https://github.com/331uw13/Editor


*/


#include <GLFW/glfw3.h>
#include "buffer.h"
#include "font.h"


#include "config.h" // TODO


#define MAX_BUFFERS 8
//#define LINE_WRAP_WORD_BUFFER_SIZE 64//512

#define ERROR_BUFFER_MAX_SIZE 256
#define INFO_BUFFER_MAX_SIZE 64

#define EDITOR_X_PADDING 10
#define EDITOR_Y_PADDING 10

/*
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
*/

#define MODE_NORMAL 0
#define MODE_COMMAND_LINE 1

#define COMMAND_LINE_MAX_SIZE 32

struct editor_t {
    GLFWwindow* win;//struct psf2_font font;


    struct font_t font;

    struct buffer_t buffers[MAX_BUFFERS];
    unsigned int current_buffer;
    unsigned int num_active_buffers;

    int mode;

    int window_width;
    int window_height;

    int max_column;
    int max_row;


    unsigned int vbo; // vertex buffer object.
    unsigned int vao; // vertex array object.
    unsigned int shader;


    // the error which is written to the buffer must be null terminated.
    char    error_buf[ERROR_BUFFER_MAX_SIZE];
    size_t  error_buf_size; 

    // messages that are displayed for user. example:'x bytes written.'
    char    info_buf[INFO_BUFFER_MAX_SIZE];
    size_t  info_buf_size;

    struct string_t* cmd_str;    // for command line.
    long int         cmd_cursor; //

    double mouse_x;
    double mouse_y;
    int    mouse_button; // 1 if left mouse button was pressed.

    // set to 1 if init_editor() returns with pointer,
    //                           GLFW is initialized and everything should be fine.
    int ready;
};

void do_safety_check(struct editor_t* ed); // TODO make this better <---

float column_to_location(struct editor_t* ed, size_t col);
float row_to_location(struct editor_t* ed, size_t row);
/*
void set_font_scale(struct editor_t* ed, float scale);
int  load_font_from_file(const char* fontfile, struct psf2_font* font);
void unload_font(struct psf2_font* font);
void font_draw_char(struct editor_t* ed, int col, int row, char c, int on_grid);
*/
// if on_grid, is set to 1, then x and y are treated as column and row.
/*
void font_draw_data(struct editor_t* ed,
        char* str, size_t size,
        int x,
        int y,
        int on_grid);

// TODO:
void font_draw_data_wrapped(struct editor_t* ed, char* str, 
        size_t size, int col, int row, int max_column);
*/

// for drawing functions.
//
#define MAP_XYWH 0               // map coordinates to coordinates for opengl ?
#define XYWH_ALREADY_MAPPED 1    // 

// map X, Y, WIDTH, HEIGHT to usable coordinates.
// if pointer is set to NULL, continues to operate and leaves the null pointer alone
void map_xywh(struct editor_t* ed, 
        float* x, float* y, float* w, float* h);

/*
void draw_rect(struct editor_t* ed, float x, float y, float w, float h, int flag);
void draw_framed_rect(struct editor_t* ed, 
        float x, float y, float w, float h, 
        float frame_r, float frame_g, float frame_b, // rgb 0.0 - 1.0
        float fthickness, // frame thickness
        int flag);
void draw_line(struct editor_t* ed, float x0, float y0, float x1, float y1, 
        float thickness, int flag);
*/

// message types
//
#define ERROR_MSG 0
#define INFO_MSG 1

void write_message(struct editor_t* ed, int type, char* err, ...);
void clear_error_buffer(struct editor_t* ed);
void clear_info_buffer(struct editor_t* ed);

/*
void draw_error_buffer(struct editor_t* ed);
void draw_info_buffer(struct editor_t* ed);
*/
int setup_buffers(struct editor_t* ed);

struct editor_t* init_editor(
        const char* fontfile,
        int window_width, int window_height,
        int fullscreen);

void cleanup_editor(struct editor_t** e);


#endif



