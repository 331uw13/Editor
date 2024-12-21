#include "../editor.h"

#include "select.h"
#include "common.h"

void bufmode_select_keypress(
       struct editor_t* ed,
       struct buffer_t* buf,
       int key,
       int mods
       )
{
    bufmode_common_cursormov(ed, buf, key, mods);
}

void bufmode_select_charinput(
       struct editor_t* ed,
       struct buffer_t* buf,
       unsigned char codepoint
       )
{

    if(codepoint == ed->keybinds[KB_QUIT_MODE]) {
        buffer_change_mode(ed, buf, buf->prev_mode);
    }
    else if(codepoint == ed->keybinds[KB_COPY_SELECTED]) {
        buffer_copy_selected(ed, buf);
    }
    else if(codepoint == ed->keybinds[KB_COPYNREMOVE_SELECTED]) {
        buffer_copy_selected(ed, buf);
        buffer_remove_selected(buf);
    }

}
