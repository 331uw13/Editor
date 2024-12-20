#ifndef BUFMODE_COMMON_H
#define BUFMODE_COMMON_H

struct editor_t;
struct buffer_t;


void bufmode_common_cursormov(
        struct editor_t* ed,
        struct buffer_t* buf,
        int key,
        int mods
        );



#endif
