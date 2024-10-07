#ifndef EDITOR_GUI_H
#define EDITOR_GUI_H

#include "editor.h"



// background
//
#define ITEM_BG_R 0.1
#define ITEM_BG_G 0.12
#define ITEM_BG_B 0.12

// default.
//
#define ITEM_D_R 0.2
#define ITEM_D_G 0.33
#define ITEM_D_B 0.33

// focused
//
#define ITEM_F_R 0.35
#define ITEM_F_G 0.63
#define ITEM_F_B 0.63

// dark.
//
#define ITEM_DRK_R 0.11
#define ITEM_DRK_G 0.19
#define ITEM_DRK_B 0.19

// framed rect frame color.
//
#define ITEM_FRM_R 0.1
#define ITEM_FRM_G 0.3
#define ITEM_FRM_B 0.3

#define COLOR_DEFAULT 0
#define COLOR_FOCUSED 1
#define COLOR_DARK    2
#define COLOR_BG      3



#define GUI_ITEM_MAX_TEXT_SIZE 64

struct gui_button_t {
    int x;
    int y;
    int visible;
    void(*callback)(struct editor_t*);
    
    char text[GUI_ITEM_MAX_TEXT_SIZE];
    size_t text_size;
};


struct gui_fslider_t {
    int x;
    int y;
    int width;
    float* ptr;
    float min;
    float max;
    int visible;
    int in_use;
    void(*callback)(struct editor_t*, float*);

    char text[GUI_ITEM_MAX_TEXT_SIZE];
    size_t text_size;
};

struct gui_islider_t {
};

struct gui_text_input_t {
};


#define GUI_MAX_BUTTONS      32
#define GUI_MAX_FSLIDERS     32
#define GUI_MAX_ISLIDERS     32
#define GUI_MAX_TEXT_INPUTS  32


#define GUI_BUTTON        0
#define GUI_FSLIDER       1


struct gui_t {
    struct gui_button_t buttons[GUI_MAX_BUTTONS];
    size_t num_buttons;

    struct gui_fslider_t fsliders[GUI_MAX_FSLIDERS];
    size_t num_fsliders;
    
    struct gui_islider_t isliders[GUI_MAX_ISLIDERS];
    size_t num_isliders;

};

// TODO: shift non visible gui items to end of the array,
//       makes updating faster: break loop when first non visible is found.

void add_gui_button(struct gui_t* gui,
        char* text,
        size_t text_size,
        int x, int y,
        void(*callback)(struct editor_t*), int visible);


void add_gui_fslider(struct gui_t* gui,
        char* text,
        size_t text_size,
        int x, int y, int width,
        void(*callback)(struct editor_t*, float*), // <- can be NULL
        float* ptr,
        float min,
        float max,
        int visible);


void set_glcolor(int i);
void update_gui_items(struct editor_t* ed, struct gui_t* gui);
int mouse_on_area(struct editor_t* ed, int x, int y, int w, int h);










#endif
