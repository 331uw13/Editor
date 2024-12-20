
#include "insert.h"
#include "common.h"
#include "../editor.h"
#include "../utils.h"


void bufmode_insert_keypress(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        )
{ 
    bufmode_common_cursormov(ed, buf, key, mods);


    switch(key) {
    
        case GLFW_KEY_ENTER:
            {
                if(buffer_add_newline(buf, buf->cursor_x, buf->cursor_y, BUFADDNL_USE_INDENT)) {
                    
                    // adjust scroll if cursor is at bottom of screen.
                    if(buf->cursor_y > (buf->scroll + ed->max_row)) {
                        buffer_scroll(buf, -1);
                    }
                }
            }
            break;

        case GLFW_KEY_BACKSPACE:
            {
                buffer_backspace(buf);
            }
            break;
    

        case GLFW_KEY_TAB:
            if(buf->mode == BUFMODE_INSERT) {
                for(int i = 0; i < FONT_TAB_WIDTH; i++) {
                    string_add_char(buf->current, 0x20, buf->cursor_x);
                }
                move_cursor(buf, FONT_TAB_WIDTH, 0);
            }
            break;
    }
}


void bufmode_insert_charinput(
        struct editor_t* ed,
        struct buffer_t* buf,
        unsigned char codepoint
        )
{

    if(string_add_char(buf->current, codepoint, buf->cursor_x)) {
        move_cursor(buf, 1, 0);
    }
}



