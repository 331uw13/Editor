#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

float normalize(float t, float min, float max);
float lerp(float t, float min, float max);
float map(float t, float s_min, float s_max, float d_min, float d_max);

int       iclamp(int t, int min, int max);
long int  liclamp(long int t, long int min, long int max);
float     fclamp(float t, float min, float max);

int inbounds(long int t, long int min, long int max);

unsigned long djb2_hash(char* str);

#endif
