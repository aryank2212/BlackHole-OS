/* ============================================================================
 * VGA Text Mode Driver — BlackHole OS
 *
 * Manages the 80x25 VGA text buffer at 0xB8000.
 * Supports cursor tracking, color attributes, newlines, and scrolling.
 * ========================================================================= */

#include "vga.h"

/* ---- Internal State ---- */
static volatile char *vga_buffer = (volatile char *)VGA_ADDRESS;
static int cursor_row = 0;
static int cursor_col = 0;
static unsigned char current_color = (VGA_BLACK << 4) | VGA_LIGHT_GREEN;

/* ---- Helpers ---- */

static int vga_offset(int row, int col) {
    return (row * VGA_WIDTH + col) * 2;
}

/*
 * Scroll the screen up by one line:
 *  - Copy rows 1..24 to rows 0..23
 *  - Clear the last row
 */
static void vga_scroll(void) {
    /* Move each row up */
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            int src = vga_offset(row, col);
            int dst = vga_offset(row - 1, col);
            vga_buffer[dst]     = vga_buffer[src];
            vga_buffer[dst + 1] = vga_buffer[src + 1];
        }
    }
    /* Clear the last row */
    for (int col = 0; col < VGA_WIDTH; col++) {
        int off = vga_offset(VGA_HEIGHT - 1, col);
        vga_buffer[off]     = ' ';
        vga_buffer[off + 1] = current_color;
    }
}

/* ---- Public API ---- */

void vga_init(void) {
    current_color = (VGA_BLACK << 4) | VGA_LIGHT_GREEN;
    cursor_row = 0;
    cursor_col = 0;
    vga_clear();
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga_buffer[i]     = ' ';
        vga_buffer[i + 1] = current_color;
    }
    cursor_row = 0;
    cursor_col = 0;
}

void vga_set_color(unsigned char fg, unsigned char bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

void vga_set_cursor(int row, int col) {
    if (row >= 0 && row < VGA_HEIGHT) cursor_row = row;
    if (col >= 0 && col < VGA_WIDTH)  cursor_col = col;
}

void vga_newline(void) {
    cursor_col = 0;
    cursor_row++;
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
        cursor_row = VGA_HEIGHT - 1;
    }
}

void vga_putchar(char c) {
    if (c == '\b') {
        /* Backspace: move cursor back one position */
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_WIDTH - 1;
        }
        return;
    }
    if (c == '\n') {
        vga_newline();
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        return;
    }
    if (c == '\t') {
        /* Tab: advance to next 4-column boundary */
        int next = (cursor_col + 4) & ~3;
        while (cursor_col < next && cursor_col < VGA_WIDTH) {
            int off = vga_offset(cursor_row, cursor_col);
            vga_buffer[off]     = ' ';
            vga_buffer[off + 1] = current_color;
            cursor_col++;
        }
        if (cursor_col >= VGA_WIDTH) {
            vga_newline();
        }
        return;
    }

    int off = vga_offset(cursor_row, cursor_col);
    vga_buffer[off]     = c;
    vga_buffer[off + 1] = current_color;

    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
        vga_newline();
    }
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

void vga_print_at(const char *str, int row, int col) {
    vga_set_cursor(row, col);
    vga_print(str);
}
