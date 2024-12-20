#ifndef BUFFER_MODE_SELECT_H
#define BUFFER_MODE_SELECT_H

struct editor_t;
struct buffer_t;


void bufmode_select_keypress(
       struct editor_t* ed,
       struct buffer_t* buf,
       int key,
       int mods
       );

void bufmode_select_charinput(
       struct editor_t* ed,
       struct buffer_t* buf,
       unsigned char codepoint
       );

#endif

