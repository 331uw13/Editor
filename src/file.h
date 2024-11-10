#ifndef FILE_H
#define FILE_H



struct editor_t;

// returns the amount of bytes dealt with
// buffer 'file_opened' is set to 1
size_t read_file(struct editor_t* ed, unsigned int buf_id, char* filename, size_t filename_size);
size_t write_file(struct editor_t* ed, unsigned int buf_id, char* new_filename);


#endif
