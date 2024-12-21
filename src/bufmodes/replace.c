#include "../editor.h"
#include "../utils.h"

#include "replace.h"
#include "common.h"


void bufmode_replace_keypress(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        )
{ 
    bufmode_common_cursormov(ed, buf, key, mods);
}


void bufmode_replace_charinput(
        struct editor_t* ed,
        struct buffer_t* buf,
        unsigned char codepoint
        )
{
    if(string_set_char(buf->current, codepoint, buf->cursor_x)) {
        move_cursor(buf, 1, 0);
    }
}


