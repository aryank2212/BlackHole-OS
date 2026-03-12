/* ============================================================================
 * Simple Shell — BlackHole OS
 *
 * Provides a basic command-line interface.
 * Supported commands: help, echo, clear, alloc.
 * ========================================================================= */

#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../include/stdio.h"
#include "../include/string.h"
#include "../kernel/memory.h"
#include "../drivers/ata.h"

#define MAX_CMD_LEN 128

void shell_init(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    printf("\n========================================\n");
    printf("   Welcome to BlackHole OS Shell v0.1   \n");
    printf("========================================\n\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    printf("Type 'help' for a list of commands.\n\n");
}

/*
 * Parse and execute a command buffer.
 */
static void execute_command(char *cmd) {
    if (cmd[0] == '\0') {
        return; /* empty command */
    }

    if (strcmp(cmd, "help") == 0) {
        vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
        printf("Available commands:\n");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        printf("  help        - Show this help message\n");
        printf("  echo <text> - Print text back to the screen\n");
        printf("  clear       - Clear the screen\n");
        printf("  alloc       - Test memory allocator\n");
        printf("  uptime      - Show system uptime in centiseconds\n");
        printf("  sleep       - Sleep for 1 second\n");
        printf("  pagefault   - Trigger a deliberate Page Fault (Security Test)\n");
        printf("  diskread    - Read sector 10 from HDD\n");
        printf("  diskwrite   - Write 'Hello Disk!' to sector 10 of HDD\n");
    } 
    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } 
    else if (strncmp(cmd, "echo ", 5) == 0) {
        /* Print everything after 'echo ' */
        printf("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "alloc") == 0) {
        printf("Allocating 64 bytes... ");
        void *ptr = memory_alloc(64);
        if (ptr) {
            vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
            printf("SUCCESS\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
            /* Optional: test freeing it */
            memory_free(ptr);
            printf("Memory block freed.\n");
        } else {
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            printf("FAILED\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
        }
    }
    else if (strcmp(cmd, "uptime") == 0) {
        uint32_t ticks = timer_get_ticks();
        printf("System uptime: %u ticks (each tick = 10ms)\n", ticks);
    }
    else if (strcmp(cmd, "sleep") == 0) {
        printf("Sleeping for 1000ms... ");
        sleep(1000);
        printf("Done!\n");
    }
    else if (strcmp(cmd, "pagefault") == 0) {
        printf("Triggering Page Fault at 0xFFFFF000...\n");
        uint32_t *bad_ptr = (uint32_t *)0xFFFFF000;
        *bad_ptr = 0xDEADBEEF; /* This should crash the OS safely into our handler */
    }
    else if (strcmp(cmd, "diskwrite") == 0) {
        uint8_t buffer[512];
        memset(buffer, 0, 512);
        strcpy((char *)buffer, "Hello Disk! BlackHole OS was here.");
        printf("Writing to LBA 10...\n");
        ata_write_sector(10, buffer);
        printf("Write complete.\n");
    }
    else if (strcmp(cmd, "diskread") == 0) {
        uint8_t buffer[512];
        memset(buffer, 0, 512);
        printf("Reading from LBA 10...\n");
        ata_read_sector(10, buffer);
        printf("Data: %s\n", (char *)buffer);
    }
    else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        printf("Unknown command: %s\n", cmd);
        vga_set_color(VGA_WHITE, VGA_BLACK);
    }
}

void shell_loop(void) {
    char cmd_buffer[MAX_CMD_LEN];
    
    while (1) {
        vga_set_color(VGA_YELLOW, VGA_BLACK);
        printf("BH-OS> ");
        vga_set_color(VGA_WHITE, VGA_BLACK);

        /* Read one line of input */
        keyboard_readline(cmd_buffer, MAX_CMD_LEN);

        /* Execute it */
        execute_command(cmd_buffer);
    }
}
