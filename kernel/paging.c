/* ============================================================================
 * Virtual Memory (Paging) — BlackHole OS
 *
 * Implements x86 32-bit paging.
 * Identity maps the first 4MB of RAM so that physical == virtual addresses
 * for the kernel code, VGA buffer, and early allocations.
 * ========================================================================= */

#include "paging.h"
#include "memory.h"
#include "../drivers/vga.h"
#include <stdint.h>
#include <stdio.h>

/* Allocate enough raw memory to ensure we can find a 4KB aligned chunk */
static uint8_t page_directory_raw[4096 * 2];
static uint8_t first_page_table_raw[4096 * 2];

static uint32_t *page_directory;
static uint32_t *first_page_table;

/*
 * Load the Page Directory base address into CR3
 */
static void load_page_directory(uint32_t *pd) {
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(pd));
}

/*
 * Enable Paging by setting the PG (Paging Enable) bit in CR0
 */
static void enable_paging(void) {
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; /* Set bit 31 */
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(cr0));
}

void paging_init(void) {
    /* Manually align pointers to 4KB (4096 bytes) boundaries */
    page_directory = (uint32_t *)((((uint32_t)page_directory_raw) + 4095) & ~4095);
    first_page_table = (uint32_t *)((((uint32_t)first_page_table_raw) + 4095) & ~4095);

    /*
     * 1. Initialize all 1024 Page Directory Entries to:
     *    - Not present (bit 0 = 0)
     *    - Read/Write (bit 1 = 1)
     */
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    /*
     * 2. Identity map the first 4MB of memory (0x00000000 -> 0x003FFFFF)
     *    This covers the kernel at 0x1000, VGA at 0xB8000, and initial heap.
     *    Each PDE maps 4MB. The first PDE maps the very first 4MB using `first_page_table`.
     */
    for (int i = 0; i < 1024; i++) {
        /*
         * Physical address starts at 0, incrementing by 4096 (4KB page size).
         * Set Present (bit 0) and Read/Write (bit 1) -> 0x3
         */
        first_page_table[i] = (i * 4096) | 3;
    }

    /*
     * 3. Attach the first page table to the first entry of the Page Directory.
     *    The address must be cast to uint32_t and flags applied: Present(1) | Read/Write(2) = 3
     */
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    /*
     * 4. Load the structural base into the CPU and active Paging mode.
     */
    load_page_directory(page_directory);
    enable_paging();
}
