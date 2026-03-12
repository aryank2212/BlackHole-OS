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
#include "../kernel/task.h"
#include "../drivers/ata.h"
#include "../fs/fat16.h"

#define MAX_CMD_LEN 128

/* Function executed in Ring 3 User Space */
void user_main(void) {
    const char *msg = "Hello from Ring 3 (User Space)!";
    
    /* Trigger Syscall 1 (Print String) via Software Interrupt 0x80 */
    __asm__ __volatile__(
        "mov $1, %%eax\n"
        "mov %0, %%ebx\n"
        "int $0x80"
        : 
        : "r" (msg)
        : "eax", "ebx"
    );

    /* Spin infinitely in Ring 3 since we cannot 'return' to Ring 0 normally */
    while(1);
}

/* Background thread demonstrating preemptive multitasking parallel to the shell */
void background_task(void) {
    uint32_t count = 0;
    while(1) {
        /* Hacky way to print to the top right corner directly using VGA memory exclusively for the background task */
        uint16_t *vga = (uint16_t *)0xB8000;
        char buffer[16];
        uitoa(count++, buffer, 10);
        
        int offset = 70; /* Towards top right limit */
        for(int i = 0; buffer[i] != '\0'; i++) {
            vga[offset + i] = (vga[offset + i] & 0xFF00) | buffer[i];
        }
        
        /* Yield heavily to let the primary shell execute fluidly */
        sleep(1000); 
    }
}

void shell_init(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    printf("\n========================================\n");
    printf("   Welcome to BlackHole OS Shell v0.1   \n");
    printf("========================================\n\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    printf("Type 'help' for a list of commands.\n\n");
}

/* Forward declarations for filesystem commands */
static void cmd_ls(void);
static void cmd_cat(const char *filename);
static void cmd_write(const char *args);
static void cmd_rm(const char *filename);

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
        printf("  ls          - List files in root directory\n");
        printf("  cat <file>  - Read and display file contents\n");
        printf("  write <f> <text> - Create/overwrite a file\n");
        printf("  rm <file>   - Delete a file\n");
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
    else if (strcmp(cmd, "ls") == 0) {
        cmd_ls();
    }
    else if (strncmp(cmd, "cat ", 4) == 0) {
        cmd_cat(cmd + 4);
    }
    else if (strncmp(cmd, "write ", 6) == 0) {
        cmd_write(cmd + 6);
    }
    else if (strncmp(cmd, "rm ", 3) == 0) {
        cmd_rm(cmd + 3);
    }
    else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        printf("Unknown command: %s\n", cmd);
        vga_set_color(VGA_WHITE, VGA_BLACK);
    }
}

/* ---- FAT16 shell command helpers ---- */
static void cmd_ls(void) {
    printf("Root directory:\n");
    fat16_list_root();
}

static void cmd_cat(const char *filename) {
    uint8_t buf[2048];
    memset(buf, 0, sizeof(buf));
    int bytes = fat16_read_file(filename, buf, sizeof(buf) - 1);
    if (bytes < 0) {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        printf("File not found: %s\n", filename);
        vga_set_color(VGA_WHITE, VGA_BLACK);
    } else {
        buf[bytes] = '\0';
        printf("%s\n", (char *)buf);
    }
}

static void cmd_write(const char *args) {
    /* args = "filename text..." */
    char filename[32];
    int i = 0;
    while (args[i] && args[i] != ' ' && i < 31) {
        filename[i] = args[i];
        i++;
    }
    filename[i] = '\0';
    if (args[i] == ' ') i++;
    const char *data = args + i;
    uint32_t len = strlen(data);
    if (fat16_write_file(filename, (const uint8_t *)data, len) == 0) {
        vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
        printf("Wrote %u bytes to '%s'.\n", len, filename);
        vga_set_color(VGA_WHITE, VGA_BLACK);
    }
}

static void cmd_rm(const char *filename) {
    if (fat16_delete_file(filename) == 0) {
        printf("Deleted '%s'.\n", filename);
    } else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        printf("File not found: %s\n", filename);
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

void shell_start_multitasking_demo(void) {
    shell_init();
    
    /* Spawn our background task which will write autonomously via IRQ0 context switching */
    create_kernel_thread(background_task);

    shell_loop();
}
