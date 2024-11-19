#ifndef EDITOR_MEMORY_H
#define EDITOR_MEMORY_H

#include <stddef.h>



#define EDITOR_MEM_MULT 8 // how many times newsizeb should be multiplied?
                          // note: newsizeb * EDMEM_MULT may not always be the final size.

// returns the same pointer that is passed as
// argument(void* ptr) if memory resizing fails.
// but if the size is 0, then NULL is returned.
// 'esizeb' is the size of the element (in bytes).
// 'ptrsize' is the current number of elements.
// 'nsize_ptr' actual size that the memory is resized to is set here if its not NULL.
void* safe_resize_array
      (void* ptr, size_t esizeb, size_t ptrsize, size_t newsize, size_t* nsize_ptr);


// TODO: make double free protection.
// ----
//
// ptr is set to NULL after calling this function.
void safe_free(void** ptr); // --- NOT IMPLEMENTED YET ---

void init_mem_static_vars();

#endif
