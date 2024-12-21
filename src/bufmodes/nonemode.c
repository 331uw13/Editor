#include "../editor.h"
#include "../config.h"

#include "nonemode.h"

/*
void bufmode_nonemode_keypress(
       struct editor_t* ed,
       struct buffer_t* buf,
       int key,
       int mods
       )
{
}
*/

void bufmode_nonemode_charinput(
       struct editor_t* ed,
       struct buffer_t* buf,
       unsigned char codepoint
       )
{

    if(codepoint == ed->keybinds[KB_QUIT_MODE]) {
        buffer_change_mode(ed, buf, buf->prev_mode);
    }
    else if(codepoint == ed->keybinds[KB_INSERTMODE]) {
        buffer_change_mode(ed, buf, BUFMODE_INSERT);
    }
    else if(codepoint == ed->keybinds[KB_REPLACEMODE]) {
        buffer_change_mode(ed, buf, BUFMODE_REPLACE);
    }
    else if(codepoint == ed->keybinds[KB_SELECTMODE]) {
        buffer_change_mode(ed, buf, BUFMODE_SELECT);
    }

}

