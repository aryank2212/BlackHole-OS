/* ============================================================================
 * ISR — C-side interrupt handler dispatcher
 * ========================================================================= */

#include "isr.h"
#include "idt.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/vga.h"
#include "../include/stdio.h"

/*
 * Exception 14 - Page Fault
 * The faulting virtual address is stored in CR2.
 */
void isr14_handler(void) {
    uint32_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
    
    vga_set_color(VGA_WHITE, VGA_RED);
    printf("\n*** KERNEL PANIC ***\n");
    printf("Page Fault (Exception 14) at Virtual Address: 0x%x\n", faulting_address);
    printf("System Halted.\n");
    
    while(1) {
        __asm__ __volatile__("hlt");
    }
}

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

void isr128_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2) {
    /* Syscall router */
    if (syscall_num == 1) {
        /* Syscall 1: Print String */
        printf("[Syscall] Ring 3 says: %s\n", (char *)arg1);
    } else {
        printf("[Syscall] Unknown syscall requested from User Space: %d\n", syscall_num);
    }
}
