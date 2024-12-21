#include "colors.h"
#include "editor.h"

void init_colors(struct editor_t* ed) {


    ed->colors[BACKGROUND_COLOR] = 0x101111;
    ed->colors[FOREGROUND_COLOR] = 0xdbbb88;
    ed->colors[FILENAME_COLOR] = 0x90a0e0;
    ed->colors[BAR_COLOR] = 0x041624;
    ed->colors[LINENUM_COLOR_A] = 0x406040;
    ed->colors[LINENUM_COLOR_B] = 0x252525;
    ed->colors[READONLY_COLOR] = 0xa34514;
    
    ed->colors[INSERT_CURSOR_COLOR] = 0x178a84;
    ed->colors[INSERT_CURSORCHAR_COLOR] = 0x051205;

    ed->colors[REPLACE_CURSOR_COLOR] = 0xf07118;
    ed->colors[REPLACE_CURSORCHAR_COLOR] = 0xffefaa;
    
    ed->colors[SELECT_CURSOR_COLOR] = 0xfc7ec1;
    ed->colors[SELECT_REGION_COLOR] = 0x661d43;
    ed->colors[SELECT_CURSORCHAR_COLOR] = 0xa1ace9;
    
    ed->colors[NONEMODE_CURSOR_COLOR] = 0x6c6c6c;
    ed->colors[NONEMODE_CURSORCHAR_COLOR] = 0x222222;
    
    ed->colors[BLOCKSLCT_CURSOR_COLOR] = 0x1da55e;
    ed->colors[BLOCKSLCT_CURSORCHAR_COLOR] = 0x222222;

    ed->colors[CMDLBG_COLOR] = 0x261231;
    ed->colors[CMDLFG_COLOR] = 0xcf35d5;
    ed->colors[CMDLCURSOR_COLOR] = 0x52286b;
    ed->colors[CMDLCURSORCHAR_COLOR] = 0x25c1d3;
    
}



