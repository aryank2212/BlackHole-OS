#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/ata.h"
#include "gdt.h"
#include "idt.h"
#include "paging.h"
#include "memory.h"
#include "../include/stdio.h"
#include "../shell/shell.h"
#include "user_mode.h"
#include "task.h"

/*
 * Kernel entry point — called from kernel_entry.asm
 */
void kernel_main(void) {
    /* Initialize VGA */
    vga_init();

    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    printf("BlackHole OS v0.0.3\n");

    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    printf("Kernel loaded successfully.\n");

    printf("Initializing Global Descriptor Table (GDT)...\n");
    gdt_init();

    printf("Initializing Virtual Memory (Paging)...\n");
    paging_init();

    printf("Initializing IDT & PIC...\n");
    idt_init();
    
    printf("Initializing Timer...\n");
    timer_init(100); /* 100 Hz = 10ms per tick */
    
    keyboard_init();
    
    ata_init();

    vga_set_color(VGA_WHITE, VGA_BLACK);
    printf("> IDT initialized.\n");
    printf("> Timer running at 100Hz.\n");
    printf("> Keyboard driver ready.\n");

    /* Initialize memory allocator */
    memory_init();
    printf("> Memory allocator ready.\n");

    /* Test memory allocation */
    void *ptr1 = memory_alloc(128);
    void *ptr2 = memory_alloc(256);
    if (ptr1 && ptr2) {
        printf("> Memory allocation test PASSED.\n\n");
    } else {
        printf("> Memory allocation test FAILED.\n\n");
    }

    printf("Initializing Multitasking Scheduler...\n");
    tasking_init();
    printf("> Task zero active.\n\n");

    printf("Dropping to User Mode (Ring 3)...\n");
    switch_to_user_mode();
    
    /* We should never reach here because user_main() takes over */
    /* Start the shell */
    shell_init();
    shell_loop();
}
