#ifndef FILE_IO_H
#define FILE_IO_H

#include "editor.h"

// doesnt return the file descriptor, 
// it can be accessed from buffer structure. buffer 'file_opened' is set to 1
size_t read_file(struct editor_t* ed, unsigned int buf_id, char* filename, size_t filename_size);
size_t write_file(struct editor_t* ed, unsigned int buf_id);




#endif
