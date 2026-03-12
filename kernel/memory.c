/* ============================================================================
 * Memory Allocator — BlackHole OS
 *
 * A simple linked-list (free-list) memory allocator.
 * Manages a fixed heap starting at 1MB (0x100000).
 * ========================================================================= */

#include "memory.h"
#include "../drivers/vga.h"

/* We start the heap at 1MB (well above our kernel and BIOS areas) */
#define HEAP_START 0x100000
#define HEAP_SIZE  0x100000  /* 1MB heap size for now */

/* Block header representing an allocated or free block */
typedef struct block_header {
    unsigned int size;           /* doesn't include the size of the header itself */
    int is_free;                 /* 1 if free, 0 if allocated */
    struct block_header *next;
} block_header_t;

static block_header_t *free_list = (block_header_t *)HEAP_START;

void memory_init(void) {
    /* Initialize the free list with a single huge block */
    free_list->size = HEAP_SIZE - sizeof(block_header_t);
    free_list->is_free = 1;
    free_list->next = 0; /* 0 represents NULL */
}

void *memory_alloc(unsigned int size) {
    if (size == 0) return 0;
    
    /* Align size to 4 bytes */
    unsigned int aligned_size = (size + 3) & ~3;
    
    block_header_t *current = free_list;
    while (current != 0) {
        if (current->is_free && current->size >= aligned_size) {
            /* Can we split the block? */
            if (current->size >= aligned_size + sizeof(block_header_t) + 4) {
                block_header_t *new_block = (block_header_t *)((char *)current + sizeof(block_header_t) + aligned_size);
                new_block->size = current->size - aligned_size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = aligned_size;
                current->next = new_block;
            }
            current->is_free = 0;
            /* Return pointer to the data area right after the header */
            return (void *)((char *)current + sizeof(block_header_t));
        }
        current = current->next;
    }
    
    /* Out of memory */
    return 0;
}

void memory_free(void *ptr) {
    if (ptr == 0) return;
    
    /* Get the header right before the data pointer */
    block_header_t *header = (block_header_t *)((char *)ptr - sizeof(block_header_t));
    header->is_free = 1;
    
    /* Coalesce consecutive free blocks */
    block_header_t *current = free_list;
    while (current != 0) {
        if (current->is_free && current->next != 0 && current->next->is_free) {
            current->size += sizeof(block_header_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}
