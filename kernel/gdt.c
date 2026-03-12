#include "gdt.h"
#include "../include/string.h"

/* Lets us access our ASM functions from our C code. */
extern void gdt_flush(uint32_t);
extern void tss_flush(void);

/* Internal function prototypes. */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0);

gdt_entry_t gdt_entries[6];
gdt_ptr_t   gdt_ptr;
tss_entry_t tss_entry;

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                /* Null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Code segment - Ring 0 */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Data segment - Ring 0 */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* Code segment - Ring 3 */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* Data segment - Ring 3 */
    
    write_tss(5, 0x10, 0x0);                    /* TSS Segment */

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}

/* Set the value of one GDT entry. */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    /* Compute the base and limit of our TSS entry */
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = sizeof(tss_entry) - 1;

    /* Add the TSS descriptor's address to the GDT. */
    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    /* Ensure the descriptor is initially zero. */
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0  = ss0;  /* Set the kernel stack segment. */
    tss_entry.esp0 = esp0; /* Set the kernel stack pointer. */
    
    /* Here we set the cs, ss, ds, es, fs, and gs entries in the TSS.
     * These specify what segments should be loaded when the processor switches to kernel mode.
     * We specify 0x08 for the CS and 0x10 for the DS (Ring 0 segments). */
    tss_entry.cs   = 0x0b;    /* Wait, Ring 0 CS is 0x08. 0x0B is RPL3, wait. We set CS and SS below properly. */
    tss_entry.cs   = 0x08 | 0x3;
    tss_entry.cs   = 0x08; /* Actual ring 0 CS */
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x10;
}

void tss_set_stack(uint32_t esp) {
    tss_entry.esp0 = esp;
}
