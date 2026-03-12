/* ============================================================================
 * ISR — C-side interrupt handler dispatcher
 * ========================================================================= */

#include "isr.h"
#include "idt.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"

/*
 * IRQ0 handler — called from isr.asm
 * Informs the timer driver and sends EOI.
 */
void irq0_handler(void) {
    timer_callback();
    outb(0x20, 0x20); /* EOI */
}

/*
 * IRQ1 handler — called from isr.asm after saving registers.
 * Reads the scancode and passes it to the keyboard driver.
 */
void irq1_handler(void) {
    unsigned char scancode = inb(0x60);
    keyboard_handle_scancode(scancode);

    /* Send End-Of-Interrupt to master PIC */
    outb(0x20, 0x20);
}
