/* ============================================================================
 * Standard I/O Library — BlackHole OS
 *
 * Implements printf formatting and writing to VGA driver.
 * Supported format specifiers: %s, %c, %d, %i, %x, %X, %u, %%
 * ========================================================================= */

#include "../include/stdio.h"
#include "../include/string.h"
#include "../drivers/vga.h"

int putchar(int c) {
    vga_putchar((char)c);
    return c;
}

int puts(const char *str) {
    vga_print(str);
    vga_putchar('\n');
    return 0;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int chars_written = 0;
    
    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++; /* Skip '%' */
            switch (format[i]) {
                case 's': {
                    const char *str = va_arg(args, const char *);
                    if (!str) str = "(null)";
                    vga_print(str);
                    chars_written += strlen(str);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_putchar(c);
                    chars_written++;
                    break;
                }
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    char buf[32];
                    itoa(val, buf, 10);
                    vga_print(buf);
                    chars_written += strlen(buf);
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    char buf[32];
                    uitoa(val, buf, 10);
                    vga_print(buf);
                    chars_written += strlen(buf);
                    break;
                }
                case 'x':
                case 'X': {
                    uint32_t val = va_arg(args, uint32_t);
                    char buf[32];
                    uitoa(val, buf, 16);
                    /* Optional: uppercase for X could be added,
                       but for now just passing through to uitoa base 16 */
                    vga_print("0x");
                    vga_print(buf);
                    chars_written += 2 + strlen(buf);
                    break;
                }
                case '%': {
                    vga_putchar('%');
                    chars_written++;
                    break;
                }
                default: {
                    /* Unknown format specifier, print it literally */
                    vga_putchar('%');
                    vga_putchar(format[i]);
                    chars_written += 2;
                    break;
                }
            }
        } else {
            vga_putchar(format[i]);
            chars_written++;
        }
    }
    
    va_end(args);
    return chars_written;
}
