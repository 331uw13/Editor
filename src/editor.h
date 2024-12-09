#ifndef EDITOR_H
#define EDITOR_H

/*

   https://github.com/331uw13/Editor

*/


#include <GLFW/glfw3.h>
#include "buffer.h"
#include "font.h"

#include "colors.h" // TODO: read colors from config file.
#include "config.h" // TODO


#define MAX_BUFFERS 16
#define ERROR_BUFFER_MAX_SIZE 256
#define INFO_BUFFER_MAX_SIZE 96
#define EDITOR_X_PADDING 8
#define EDITOR_Y_PADDING 8
#define EDITOR_TEXT_Y_SPACING 1.3 // how much space between lines?
                                  // ^ NOTE: do not set this to 0.
#define EDITOR_TEXT_X_SPACING 1.0 // how much space between characters?
#define COMMAND_LINE_MAX_SIZE 64

#define CLIPBOARD_INIT_SIZE 256

#define PRINTERR(str) \
    fprintf(stderr, "ERROR: %s '%s()' \033[31m" str "\033[0m\n", __FILE__, __func__)

#define CELLW (ed->font.char_w * EDITOR_TEXT_X_SPACING)
#define CELLH (ed->font.char_h * EDITOR_TEXT_Y_SPACING)

// Modes
// .. when MODE_NORMAL is used, the current buffer's mode is actually the one being used.
#define MODE_NORMAL 0
#define MODE_COMMAND_LINE 1
#define MODE_CONFIRM_CHOICE 2

#define LAYOUT_MAX_BUFFERS 4

struct editor_t {

    GLFWwindow* win;
    struct font_t font;

    struct buffer_t buffers[MAX_BUFFERS];
    unsigned int current_bufid;
    unsigned int num_buffers;

    struct buffer_t* layout[LAYOUT_MAX_BUFFERS];
    unsigned int num_vi_buffers; // visible buffers.

    int mode;

    int window_width;
    int window_height;
    int max_column;
    int max_row;

    int show_tabs;

    unsigned int vbo; // vertex buffer object
    unsigned int vao; // vertex array object
    unsigned int shader;
    unsigned int drw_color_hex; // set_color_hex() changes this value.
    int shader_color_uniloc; // uniform location.


    // the error which is written to the buffer must be null terminated.
    char    error_buf[ERROR_BUFFER_MAX_SIZE];
    size_t  error_buf_size; 

    // messages that are displayed for user. example:'x bytes written.'
    char    info_buf[INFO_BUFFER_MAX_SIZE];
    size_t  info_buf_size;


    struct string_t* cmd_str;    // for command line.
    long int         cmd_cursor; //


    struct string_t* clipbrd;


    unsigned int colors[NUM_COLORS];

    int glfwinitsuccess;
    int ready;
};

void do_safety_check(struct editor_t* ed); // TODO make this better <---

float  col_to_loc(struct editor_t* ed, long int col);
float  row_to_loc(struct editor_t* ed, long int row);
long int loc_to_col(struct editor_t* ed, float col);
long int loc_to_row(struct editor_t* ed, float row);


#define USER_ANSWER_YES 1
#define USER_ANSWER_NO  0
// NOTE: 'question' must be null character terminated.
//       if error happens returns 0
int confirm_user_choice(struct editor_t* ed, char* question);


// BUFFER CONTROL ----- 

int editor_add_buffer(struct editor_t* ed);
void set_buffer_layout(struct editor_t* ed, 
        struct buffer_t* buf, int lcol, int lrow);

// --------

// map X, Y, WIDTH, HEIGHT to -1.0 - +1.0
void map_xywh(struct editor_t* ed, float* x, float* y, float* w, float* h);


#define ERROR_MSG 0
#define INFO_MSG 1

void write_message(struct editor_t* ed, int type, char* err, ...);
void clear_error_buffer(struct editor_t* ed);
void clear_info_buffer(struct editor_t* ed);

void draw_error_buffer(struct editor_t* ed);
void draw_info_buffer(struct editor_t* ed);

//int setup_buffers(struct editor_t* ed);
int init_editor(
        struct editor_t* ed,
        const char* fontfile,
        int window_width, int window_height,
        int fullscreen);

void cleanup_editor(struct editor_t* e);

void editor_dump_buffer_layout(struct editor_t* ed);

#endif
