#include "utils.h"


float normalize(float t, float min, float max) {
    return (t-min)/(max-min);
}

float lerp(float t, float min, float max) {
    return (max-min)*t+min;
}


float map(float t, float s_min, float s_max, float d_min, float d_max) {
    return lerp(normalize(t, s_min, s_max), d_min, d_max);
}

