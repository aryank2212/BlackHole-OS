#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "idt.h"
#include "paging.h"
#include "memory.h"
#include "../shell/shell.h"

/*
 * Kernel entry point — called from kernel_entry.asm
 */
void kernel_main(void) {
    /* Initialize VGA */
    vga_init();

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("BlackHole OS v0.0.3\n");

    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_print("Kernel loaded successfully.\n");

    vga_print("Initializing Virtual Memory (Paging)...\n");
    paging_init();

    vga_print("Initializing IDT & PIC...\n");
    idt_init();
    
    vga_print("Initializing Timer...\n");
    timer_init(100); /* 100 Hz = 10ms per tick */
    
    keyboard_init();

    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("> IDT initialized.\n");
    vga_print("> Timer running at 100Hz.\n");
    vga_print("> Keyboard driver ready.\n");

    /* Initialize memory allocator */
    memory_init();
    vga_print("> Memory allocator ready.\n");

    /* Test memory allocation */
    void *ptr1 = memory_alloc(128);
    void *ptr2 = memory_alloc(256);
    if (ptr1 && ptr2) {
        vga_print("> Memory allocation test PASSED.\n\n");
    } else {
        vga_print("> Memory allocation test FAILED.\n\n");
    }

    /* Start the shell */
    shell_init();
    shell_loop();
}
