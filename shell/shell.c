/* ============================================================================
 * Simple Shell — BlackHole OS
 *
 * Provides a basic command-line interface.
 * Supported commands: help, echo, clear, alloc.
 * ========================================================================= */

#include "shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../kernel/memory.h"

#define MAX_CMD_LEN 128

/*
 * Compare two strings up to n characters.
 * Returns 0 if equal, non-zero otherwise.
 */
static int strcmp_n(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

/*
 * Compare two null-terminated strings for exact equality.
 */
static int strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return s1[i] - s2[i];
        i++;
    }
    return s1[i] - s2[i];
}

void shell_init(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("\n========================================\n");
    vga_print("   Welcome to BlackHole OS Shell v0.1   \n");
    vga_print("========================================\n\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_print("Type 'help' for a list of commands.\n\n");
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
        vga_print("Available commands:\n");
        vga_set_color(VGA_WHITE, VGA_BLACK);
        vga_print("  help        - Show this help message\n");
        vga_print("  echo <text> - Print text back to the screen\n");
        vga_print("  clear       - Clear the screen\n");
        vga_print("  alloc       - Test memory allocator\n");
    } 
    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } 
    else if (strcmp_n(cmd, "echo ", 5) == 0) {
        /* Print everything after 'echo ' */
        vga_print(cmd + 5);
        vga_print("\n");
    }
    else if (strcmp(cmd, "alloc") == 0) {
        vga_print("Allocating 64 bytes... ");
        void *ptr = memory_alloc(64);
        if (ptr) {
            vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
            vga_print("SUCCESS\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
            /* Optional: test freeing it */
            memory_free(ptr);
            vga_print("Memory block freed.\n");
        } else {
            vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
            vga_print("FAILED\n");
            vga_set_color(VGA_WHITE, VGA_BLACK);
        }
    }
    else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_print("Unknown command: ");
        vga_print(cmd);
        vga_print("\n");
        vga_set_color(VGA_WHITE, VGA_BLACK);
    }
}

void shell_loop(void) {
    char cmd_buffer[MAX_CMD_LEN];
    
    while (1) {
        vga_set_color(VGA_YELLOW, VGA_BLACK);
        vga_print("BH-OS> ");
        vga_set_color(VGA_WHITE, VGA_BLACK);

        /* Read one line of input */
        keyboard_readline(cmd_buffer, MAX_CMD_LEN);

        /* Execute it */
        execute_command(cmd_buffer);
    }
}
