#include <math.h>

#include "utils.h"
#include "editor.h"


float normalize(float t, float min, float max) {
    return (t-min)/(max-min);
}

float lerp(float t, float min, float max) {
    return (max-min)*t+min;
}

float map(float t, float s_min, float s_max, float d_min, float d_max) {
    return lerp(normalize(t, s_min, s_max), d_min, d_max);
}

int iclamp(int t, int min, int max) {
    return (t < min) ? min : (t > max) ? max : t;
}

long int liclamp(long int t, long int min, long int max) {
    return (t < min) ? min : (t > max) ? max : t;
}

float fclamp(float t, float min, float max) {
    return (t < min) ? min : (t > max) ? max : t;
}

int inbounds(long int t, long int min, long int max) {
    return (t < min) ? 0 : (t > max) ? 0 : 1;
}

unsigned long djb2_hash(char* str) {
    unsigned long hash = 5381;
    int c;
    while((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int char_ok(char c) {
    return (c < 0x1F) ? 0 : (c > 0x7F) ? 0 : 1;
}

int is_on_end_of_tab(long int x) {
    int res = 0;

    /*
       (1-4) = "tab width".
       0123456789
       .    .....  <- x=5  return 1
       .     ....  <- x=6  return 0

       */

    if(x > 0) {
        res = (x == round(x / FONT_TAB_WIDTH) * FONT_TAB_WIDTH);
    }

    return res;
}

