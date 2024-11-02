#ifndef STRING_H
#define STRING_H


// how many bytes the string data is incremented every time it needs more memory
// this can be changed to optimize memory usage
#define STRING_MEMORY_BLOCK_SIZE 32
#define STRING_MAX_SIZE 0xf4240


struct string_t {

    char*  data;
    size_t mem_size;  // allocated memory size
    size_t data_size; // actual data size

};



// 'init_size' can be zero, it will be allocated STRING_MEMORY_BLOCK_SIZE bytes.
struct string_t* create_string(size_t init_size);
void             delete_string(struct string_t** str); 

// returns 1 if the string is ready for data manipulation or reading, otherwise zero.
int     string_ready(struct string_t* str);

// check if string needs more memory.
//   returns 1 if no more memory is needed or memory is resized.
//   returns 0 if failed to resize memory.
int string_memcheck(struct string_t* str, size_t size);

int     string_add_char(struct string_t* str, char c, size_t index);
int     string_rem_char(struct string_t* str, size_t index);
int     string_append_char(struct string_t* str, char c);

// returns 0 if index is out of bounds.
char    string_get_char(struct string_t* str, size_t index);

// flags for 'string_move_data'
#define STRING_OVERWRITE_DATA 0x1
#define STRING_ZERO_SRC 0x2
//  'dst_offset'  where to move the data from source? (src_str)
//  'src_offset'  where to start moving 'size' bytes of data to 'dst_offset'
int     string_move_data(struct string_t* dst_str, struct string_t* src_str, 
                         size_t dst_offset, size_t src_offset,
                         size_t size, int flags);

// count whitespace characters until non-whitespace character is found
// or index of 'n' is reached.
size_t  string_count_ws_to(struct string_t* str, size_t n);

// NOTE: also returns 1 for empty string.
int     string_is_data_ws(struct string_t* str);

int     string_copy_all(struct string_t* dst_str, struct string_t* src_str);
int     string_cut_data(struct string_t* str, size_t offset, size_t size);
int     string_set_data(struct string_t* str, char* data, size_t size);
size_t  string_num_chars(struct string_t* str, size_t start, size_t end, char c);
int     string_clear_data(struct string_t* str);
size_t  string_count_begin_tabs(struct string_t* str);

// 'direction' for string_find_char.
#define STRFIND_NEXT 0
#define STRFIND_PREV 1
// if 'c' is found returns length from 'start_index' to found character.
// returns 0 if not found.
size_t string_find_char(struct string_t* str, size_t start_index, char c, int direction);

// useful in some cases when wanting to render text with line wrapping
// max_line_size is like the max column.
int count_data_linewraps(char* data, size_t size, int max_line_size);

#endif
