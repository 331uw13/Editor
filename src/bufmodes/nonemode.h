#ifndef BUFFER_MODE_NONEMODE_H
#define BUFFER_MODE_NONEMODE_H

struct editor_t;
struct buffer_t;

/*
void bufmode_nonemode_keypress(
       struct editor_t* ed,
       struct buffer_t* buf,
       int key,
       int mods
       );
*/

void bufmode_nonemode_charinput(
       struct editor_t* ed,
       struct buffer_t* buf,
       unsigned char codepoint
       );

#endif

