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
#define EDITOR_X_PADDING 4
#define EDITOR_Y_PADDING 1
#define EDITOR_TEXT_Y_SPACING 1.3 // how much space between lines?
                                  // ^ NOTE: do not set this to 0.
#define EDITOR_TEXT_X_SPACING 1.0 // how much space between characters?
#define CMDSTR_MAX_SIZE 64

#define CLIPBOARD_INIT_SIZE 256

#define PRINTERR(str) \
    fprintf(stderr, "ERROR: %s '%s()' \033[31m" str "\033[0m\n", __FILE__, __func__)

#define CELLW (ed->font.char_w * EDITOR_TEXT_X_SPACING)
#define CELLH (ed->font.char_h * EDITOR_TEXT_Y_SPACING)

// Modes
// .. when MODE_NORMAL is used, the current buffer's mode is actually the one being used.
#define MODE_NORMAL 0 
#define MODE_CMDL 1 // command line
#define MODE_CONFIRM_CHOICE 2

#define LAYOUT_MAX_BUFFERS 4

struct editor_t {

    GLFWwindow* win;
    struct font_t font;

    struct buffer_t buffers[MAX_BUFFERS];
    unsigned int current_bufid;
    unsigned int num_buffers;

    int mode;

    int window_width;
    int window_height;
    int max_column;
    int max_row;

    int tabs_visible;

    unsigned int vbo; // vertex buffer object
    unsigned int vao; // vertex array object
    unsigned int shader;
    unsigned int drw_color_hex; // set_color_hex() changes this value.
    int shader_color_uniloc; // uniform location.


    // the error which is written to the buffer must be null terminated.
    char    error_buf[ERROR_BUFFER_MAX_SIZE];
    size_t  error_buf_size; 

    // line wraps happened on last draw
    // used to offset info buffer below error buffer
    // if they happen to be drawn at the same time.
    size_t  error_buf_lw; 

    // messages that are displayed for user. example:'x bytes written.'
    char    info_buf[INFO_BUFFER_MAX_SIZE];
    size_t  info_buf_size;

    struct string_t* cmdstr;    // for command line.
    long int         cmd_cursor; //

    struct string_t* clipbrd;
    unsigned int colors[NUM_COLORS];

    unsigned int keybinds[NUM_KEYBINDS]; // see config.c/h
    int cfg[NUM_CONFIG_VARS];

    // TODO: "extra rows reserved" because other things to offset data drawing and max cursor row

    int glfwinitsuccess;
    int ready;
};

int is_safe_to_continue(struct editor_t* ed);

float  col_to_loc(struct editor_t* ed, long int col);
float  row_to_loc(struct editor_t* ed, long int row);
long int loc_to_col(struct editor_t* ed, float col);
long int loc_to_row(struct editor_t* ed, float row);


#define USER_ANSWER_YES 1
#define USER_ANSWER_NO  0
#define PRESELECT_NO 1
#define PRESELECT_YES 0
// NOTE: 'question' must be null character terminated.
//       if error happens returns 0
int confirm_user_choice(struct editor_t* ed, char* question, int pre_select);


// BUFFER CONTROL ----- 

int editor_add_buffer(struct editor_t* ed);

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

void showtabs(struct editor_t* ed, int visible);

//int setup_buffers(struct editor_t* ed);
int init_editor(
        struct editor_t* ed,
        const char* fontfile,
        int window_width, int window_height
        );

void cleanup_editor(struct editor_t* e);

void editor_dump_buffer_layout(struct editor_t* ed);

#endif
