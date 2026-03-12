#ifndef KERNEL_H
#define KERNEL_H

/* VGA text mode buffer address */
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

/* VGA color attributes */
#define VGA_COLOR_BLACK       0x0
#define VGA_COLOR_WHITE       0xF
#define VGA_COLOR_LIGHT_GREEN 0xA

/* Default: light green on black */
#define VGA_DEFAULT_COLOR ((VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREEN)

void kernel_main(void);

#endif /* KERNEL_H */
