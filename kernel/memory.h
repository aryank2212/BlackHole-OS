#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"

/* Initialize the memory allocator */
void memory_init(void);

/* Allocate a block of memory of the given size. Returns 0 (NULL) on failure. */
void *memory_alloc(unsigned int size);

/* Free a previously allocated block. */
void memory_free(void *ptr);

#endif /* MEMORY_H */
