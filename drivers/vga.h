#ifndef VGA_H
#define VGA_H

/* ---- VGA Text Mode Constants ---- */
#define VGA_ADDRESS  0xB8000
#define VGA_WIDTH    80
#define VGA_HEIGHT   25

/* ---- Color Codes ---- */
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GREY    0x7
#define VGA_DARK_GREY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

/* ---- Public API ---- */

/* Initialize VGA driver: clear screen, reset cursor to (0,0) */
void vga_init(void);

/* Clear the entire screen */
void vga_clear(void);

/* Print a null-terminated string at the current cursor position */
void vga_print(const char *str);

/* Print a string at a specific (row, col) position */
void vga_print_at(const char *str, int row, int col);

/* Print a single character at the current cursor position */
void vga_putchar(char c);

/* Set foreground and background colors for subsequent prints */
void vga_set_color(unsigned char fg, unsigned char bg);

/* Move the cursor to a specific position */
void vga_set_cursor(int row, int col);

/* Print a newline (advance cursor to next line) */
void vga_newline(void);

#endif /* VGA_H */
