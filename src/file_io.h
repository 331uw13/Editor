#ifndef FILE_IO_H
#define FILE_IO_H

#include "editor.h"

// doesnt return the file descriptor, 
// it can be accessed from buffer structure. buffer 'file_opened' is set to 1
int open_file(struct editor_t* ed, unsigned int buf_id, char* filename);
void close_file(struct editor_t* ed, unsigned int buf_id);


#endif
