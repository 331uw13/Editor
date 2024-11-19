#ifndef EDITOR_COLORS_H
#define EDITOR_COLORS_H

// ..._A brighter color
// ..._B dimmer color


// TODO read from config file
// ----


#define BACKGROUND_COLOR 0
#define FOREGROUND_COLOR 1

#define FILENAME_COLOR_A 2
#define FILENAME_COLOR_B 3

#define BAR_COLOR_A 4
#define BAR_COLOR_B 5

#define LINENUM_COLOR_A 6
#define LINENUM_COLOR_B 7

#define READONLY_COLOR 8

#define INSERT_CURSOR_COLOR_A 9
#define INSERT_CURSOR_COLOR_B 10
#define INSERT_CURSORCHAR_COLOR 11

#define REPLACE_CURSOR_COLOR_A 12
#define REPLACE_CURSOR_COLOR_B 13
#define REPLACE_CURSORCHAR_COLOR 14

#define SELECT_CURSOR_COLOR_A 15
#define SELECT_CURSOR_COLOR_B 16
#define SELECT_REGION_COLOR 17
#define SELECT_CURSORCHAR_COLOR 18

#define NONEMODE_CURSOR_COLOR_A 19
#define NONEMODE_CURSOR_COLOR_B 20
#define NONEMODE_CURSORCHAR_COLOR 21

#define COMMENT_COLOR 22
#define NUMBER_COLOR 23

#define NUM_COLORS 24



struct editor_t;

void init_colors(struct editor_t* ed);


#endif
