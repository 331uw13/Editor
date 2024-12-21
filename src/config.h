#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H


struct editor_t;



#define KB_NONEMODE 0
#define KB_QUIT_MODE 1
#define KB_INSERTMODE 2
#define KB_REPLACEMODE 3
#define KB_SELECTMODE 4
#define KB_BLOCKSLCTMODE 5
#define KB_COPY_SELECTED 6
#define KB_COPYNREMOVE_SELECTED 7
#define KB_PASTE_CLIPBOARD 8
#define KB_CMDL 9

#define NUM_KEYBINDS 16


#define CFG_INFOBUF_MAXCOL 0
#define CFG_ERRORBUF_MAXCOL 1
#define CFG_QUESTION_MAXCOL 2

#define NUM_CONFIG_VARS 16



void config_setup(struct editor_t* ed);





#endif
