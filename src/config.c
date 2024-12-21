#include <GLFW/glfw3.h>

#include "editor.h"
#include "config.h"




void config_setup(struct editor_t* ed) {


    // LEFT CONTROL +
    ed->keybinds[KB_NONEMODE] = GLFW_KEY_X;
    ed->keybinds[KB_PASTE_CLIPBOARD] = GLFW_KEY_V; // only in "insert mode"
    ed->keybinds[KB_CMDL] = GLFW_KEY_L;

    // in "none mode"
    ed->keybinds[KB_QUIT_MODE] = 'q';
    ed->keybinds[KB_INSERTMODE] = 'c';
    ed->keybinds[KB_REPLACEMODE] = 'a';
    ed->keybinds[KB_SELECTMODE] = 's';
    ed->keybinds[KB_BLOCKSLCTMODE] = 'd';

    // in "select mode"
    ed->keybinds[KB_COPY_SELECTED] = 'c';
    ed->keybinds[KB_COPYNREMOVE_SELECTED] = 'f';




    ed->cfg[CFG_INFOBUF_MAXCOL] = 32;
    ed->cfg[CFG_ERRORBUF_MAXCOL] = 32;
    ed->cfg[CFG_QUESTION_MAXCOL] = 42;

}


