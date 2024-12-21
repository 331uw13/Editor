#ifndef BUFFER_MODE_REPLACE_H
#define BUFFER_MODE_REPLACE_H

struct editor_t;
struct buffer_t;


void bufmode_replace_keypress(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        );

void bufmode_replace_charinput(
        struct editor_t* ed,
        struct buffer_t* buf,
        unsigned char codepoint
        );


#endif
