#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H


struct editor_t;

void execute_cmd(struct editor_t* ed, struct string_t* str);

void commandline_keypress(struct editor_t* ed, int key, int mods);
void commandline_charinput(struct editor_t* ed, unsigned char codepoint);


#endif
