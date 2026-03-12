/* ============================================================================
 * IDT — Interrupt Descriptor Table + PIC remapping
 * ========================================================================= */

#include "idt.h"
#include "isr.h"

/* IDT table and pointer */
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idtp;

void idt_set_gate(int n, unsigned int handler, unsigned short sel, unsigned char flags) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].selector    = sel;
    idt[n].zero        = 0;
    idt[n].type_attr   = flags;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

/*
 * Remap the 8259 PIC so IRQ 0-15 map to IDT entries 32-47
 * (by default they overlap with CPU exceptions 0-15).
 */
static void pic_remap(void) {
    /* ICW1: begin initialization, expect ICW4 */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /* ICW2: IRQ base offsets */
    outb(0x21, 0x20);   /* Master PIC: IRQ 0-7  → INT 32-39 */
    outb(0xA1, 0x28);   /* Slave  PIC: IRQ 8-15 → INT 40-47 */

    /* ICW3: tell master about slave on IRQ2, tell slave its cascade identity */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    /* ICW4: 8086 mode */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* Mask all IRQs except IRQ0 (timer) and IRQ1 (keyboard) */
    outb(0x21, 0xFC);   /* Master: 1111 1100 — IRQ0, IRQ1 unmasked */
    outb(0xA1, 0xFF);   /* Slave:  all masked */
}

void idt_init(void) {
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (unsigned int)&idt;

    /* Zero out IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Remap PIC */
    pic_remap();

    idt_set_gate(14, (unsigned int)isr_14, 0x08, 0x8E);

    /* Install IRQ0 (timer) handler — IDT entry 32 */
    idt_set_gate(32, (unsigned int)isr_irq0, 0x08, 0x8E);

    /* Install IRQ1 (keyboard) handler — IDT entry 33 */
    /* isr_irq1 is defined in isr.asm */
    idt_set_gate(33, (unsigned int)isr_irq1, 0x08, 0x8E);

    /* Load IDT */
    __asm__ __volatile__("lidt (%0)" : : "r"(&idtp));

    /* Enable interrupts */
    __asm__ __volatile__("sti");
}
