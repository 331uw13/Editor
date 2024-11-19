#include "colors.h"
#include "editor.h"

void init_colors(struct editor_t* ed) {


    ed->colors[BACKGROUND_COLOR] = 0x101111;
    ed->colors[FOREGROUND_COLOR] = 0xdbbb88;
    ed->colors[FILENAME_COLOR_A] = 0x90a0e0;
    ed->colors[FILENAME_COLOR_B] = 0x203030;
    ed->colors[BAR_COLOR_A] = 0x041624;
    ed->colors[BAR_COLOR_B] = 0x03101a;
    ed->colors[LINENUM_COLOR_A] = 0x406040;
    ed->colors[LINENUM_COLOR_B] = 0x252525;
    ed->colors[READONLY_COLOR] = 0xa34514;
    
    ed->colors[INSERT_CURSOR_COLOR_A] = 0x10ee10;
    ed->colors[INSERT_CURSOR_COLOR_B] = 0x0d400d;
    ed->colors[INSERT_CURSORCHAR_COLOR] = 0xefffaa;

    ed->colors[REPLACE_CURSOR_COLOR_A] = 0xf07118;
    ed->colors[REPLACE_CURSOR_COLOR_B] = 0x69320c;
    ed->colors[REPLACE_CURSORCHAR_COLOR] = 0xffefaa;
    
    ed->colors[SELECT_CURSOR_COLOR_A] = 0xfc7ec1;
    ed->colors[SELECT_CURSOR_COLOR_B] = 0xa33b72;
    ed->colors[SELECT_REGION_COLOR] = 0x661d43;
    ed->colors[SELECT_CURSORCHAR_COLOR] = 0xdbbb88;
    
    ed->colors[NONEMODE_CURSOR_COLOR_A] = 0x067075;
    ed->colors[NONEMODE_CURSOR_COLOR_B] = 0x023638;
    ed->colors[NONEMODE_CURSORCHAR_COLOR] = 0xdbbb88;

    ed->colors[COMMENT_COLOR] = 0x505050;
    ed->colors[NUMBER_COLOR] = 0x68d166;
}



