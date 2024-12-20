#ifndef BUFFER_MODE_INSERT_H
#define BUFFER_MODE_INSERT_H

struct editor_t;
struct buffer_t;


void bufmode_insert_keypress(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        );

void bufmode_insert_charinput(
        struct editor_t* ed,
        struct buffer_t* buf,
        unsigned char codepoint
        );


#endif
