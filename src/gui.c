#include <stdio.h>
#include <string.h>

#include "gui.h"
#include "utils.h"


size_t _copy_gui_text(char* to, char* from, size_t size) {
    size = (size >= GUI_ITEM_MAX_TEXT_SIZE)
        ? GUI_ITEM_MAX_TEXT_SIZE : size;
    memmove(to, from, size);

    return size;
}



void add_gui_button(struct gui_t* gui, 
        char* text,
        size_t text_size,
        int x, int y,
        void(*callback)(struct editor_t*), int visible) 
{
    if((gui->num_buttons+1) >= GUI_MAX_BUTTONS) {
        fprintf(stderr, "cant add more buttons, max reached.\n");
        return;
    }
  
    struct gui_button_t* bt = &gui->buttons[gui->num_buttons];

    *bt = (struct gui_button_t) {
        x, y, visible, callback
    };

    bt->text_size = _copy_gui_text(bt->text, text, text_size);
    gui->num_buttons++;
}


void add_gui_fslider(struct gui_t* gui,
        char* text,
        size_t text_size,
        int x, int y, int width,
        void(*callback)(struct editor_t*, float*), // <- can be NULL
        float* ptr,
        float min,
        float max,
        int visible)
{
    if((gui->num_fsliders+1) >= GUI_MAX_FSLIDERS) {
        fprintf(stderr, "cant add more float sliders, max reached.\n");
        return;
    }

    if(!ptr) {
        fprintf(stderr, "'add_gui_fslider': warning: slider float pointer is NULL\n");
    }

    struct gui_fslider_t* sldr = &gui->fsliders[gui->num_fsliders];

    *sldr = (struct gui_fslider_t) {
        x, y, width, ptr, min, max, visible, 0, callback
    };
   
    *sldr->ptr = fclamp(*sldr->ptr, sldr->min, sldr->max);

    sldr->text_size = _copy_gui_text(sldr->text, text, text_size);
    gui->num_fsliders++;
}




void set_glcolor(int i) {

    switch(i) {

        case COLOR_DEFAULT:
            glColor3f(ITEM_D_R, ITEM_D_G, ITEM_D_B);
            break;
        
        case COLOR_FOCUSED:
            glColor3f(ITEM_F_R, ITEM_F_G, ITEM_F_B);
            break;
        
        case COLOR_BG:
            glColor3f(ITEM_BG_R, ITEM_BG_G, ITEM_BG_B);
            break;
        
        case COLOR_DARK:
            glColor3f(ITEM_DRK_R, ITEM_DRK_G, ITEM_DRK_B);
            break;

    
        default: break;
    }
}

void update_gui_items(struct editor_t* ed, struct gui_t* gui) {


// -- UPDATE BUTTONS ---
//
    for(size_t i = 0; i < gui->num_buttons; i++) {
        struct gui_button_t* bt = &gui->buttons[i];
        if(!bt) {
            continue;
        }
        if(!bt->visible) {
            continue;
        }

        int box_w = (bt->text_size + 2) * ed->font.width;
        int m = mouse_on_area(ed, bt->x, bt->y, box_w, ed->font.height);
        
        if(m && ed->mouse_button && bt->callback) {
            bt->callback(ed);
        }

        set_glcolor(COLOR_BG);
        draw_framed_rect(ed, 
                bt->x, bt->y, 
                box_w,
                ed->font.r_height,
                ITEM_FRM_R, ITEM_FRM_G, ITEM_FRM_B, 0.5,
                MAP_XYWH);

        set_glcolor(COLOR_DARK);
        font_draw_char(ed, bt->x, bt->y, '>', 0);
        font_draw_char(ed, 
                bt->x + (bt->text_size+1) * ed->font.width,
                bt->y, '<', 0);


        set_glcolor(m);
        font_draw_data(ed, bt->text, bt->text_size, 
                bt->x + ed->font.width, bt->y, 0);

    }

// -- UPDATE FLOAT SLIDERS ---
//

    for(size_t i = 0; i < gui->num_fsliders; i++) {
        struct gui_fslider_t* s = &gui->fsliders[i];
        if(!s) {
            continue;
        }
        if(!s->visible) {
            continue;
        }
        if(!s->ptr) {
            continue;
        }
        
        int m = mouse_on_area(ed, s->x, s->y, s->width, ed->font.height);
        if(m && ed->mouse_button) {
            s->in_use = !s->in_use;
        }
        else if(ed->mouse_button && s->in_use) {
            s->in_use = 0;
        }


        if(s->in_use) {
            float tmp = fclamp(ed->mouse_x - s->x, 0, s->width);
            *s->ptr = map(tmp, 0, s->width, s->min, s->max);

            printf("%f\n", *s->ptr);
        }
        
        float rvalue = map(*s->ptr, s->min, s->max, 0, s->width);

        set_glcolor(COLOR_BG);
        draw_framed_rect(ed,
                s->x, s->y,
                s->width,
                ed->font.r_height,
                ITEM_FRM_R, ITEM_FRM_G, ITEM_FRM_B, 0.5,
                MAP_XYWH);

        set_glcolor(COLOR_DARK);
        draw_rect(ed, 
                s->x, s->y,
                rvalue, 
                ed->font.r_height,
                MAP_XYWH
                );

        set_glcolor(COLOR_FOCUSED);
        draw_line(ed, s->x+rvalue, s->y, s->x+rvalue, s->y+ed->font.height-2.0, 
                1.0, MAP_XYWH);
        
        draw_line(ed, s->x+rvalue-3, 
                s->y+5, s->x+rvalue-3,
                s->y+ed->font.height-7.0, 
                1.0, MAP_XYWH);

        if(s->in_use) {
            set_glcolor(COLOR_DEFAULT);
            font_draw_char(ed, s->x - ed->font.width-5, s->y, '>', 0);
        }



        set_glcolor(COLOR_DEFAULT);
        font_draw_data(ed, s->text, s->text_size, 
                s->x + ed->font.width, s->y, 0);
    }

}

int mouse_on_area(struct editor_t* ed, int x, int y, int w, int h) {
    return (ed->mouse_x > x) && (ed->mouse_x < x+w) &&
           (ed->mouse_y > y) && (ed->mouse_y < y+h);
}

