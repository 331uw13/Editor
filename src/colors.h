#ifndef EDITOR_COLORS_H
#define EDITOR_COLORS_H

// ..._A brighter color
// ..._B dimmer color


// TODO read from config file
// ----


#define BACKGROUND_COLOR 0
#define FOREGROUND_COLOR 1

#define FILENAME_COLOR 2
#define BAR_COLOR 4

#define LINENUM_COLOR_A 6
#define LINENUM_COLOR_B 7

#define READONLY_COLOR 8

#define INSERT_CURSOR_COLOR 9
#define INSERT_CURSORCHAR_COLOR 11

#define REPLACE_CURSOR_COLOR 12
#define REPLACE_CURSORCHAR_COLOR 14

#define SELECT_CURSOR_COLOR 15
#define SELECT_REGION_COLOR 17
#define SELECT_CURSORCHAR_COLOR 18

#define NONEMODE_CURSOR_COLOR 19
#define NONEMODE_CURSORCHAR_COLOR 21

#define BLOCKSLCT_CURSOR_COLOR 22
#define BLOCKSLCT_CURSORCHAR_COLOR 23


#define NUM_COLORS 28



struct editor_t;

void init_colors(struct editor_t* ed);


#endif
