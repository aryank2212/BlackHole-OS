/* ============================================================================
 * Keyboard Driver — BlackHole OS
 *
 * Handles PS/2 keyboard via IRQ1.
 * Translates US QWERTY scancodes (set 1) to ASCII.
 * Uses a 256-byte ring buffer for key input.
 * ========================================================================= */

#include "keyboard.h"
#include "../drivers/vga.h"

/* ---- Scancode → ASCII table (US QWERTY, set 1, lowercase) ---- */
static const char scancode_to_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* F1-F10 */
    0, 0,  /* Num Lock, Scroll Lock */
    0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Shift scancode table */
static const char scancode_to_ascii_shift[128] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* ---- Ring buffer for key input ---- */
#define KEY_BUF_SIZE 256
static char key_buffer[KEY_BUF_SIZE];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;

/* Modifier state */
static int shift_held = 0;

static void buf_push(char c) {
    int next = (buf_head + 1) % KEY_BUF_SIZE;
    if (next != buf_tail) {    /* drop key if buffer full */
        key_buffer[buf_head] = c;
        buf_head = next;
    }
}

static char buf_pop(void) {
    if (buf_tail == buf_head) return 0;  /* empty */
    char c = key_buffer[buf_tail];
    buf_tail = (buf_tail + 1) % KEY_BUF_SIZE;
    return c;
}

static int buf_empty(void) {
    return buf_tail == buf_head;
}

/* ---- Public API ---- */

void keyboard_init(void) {
    buf_head = 0;
    buf_tail = 0;
    shift_held = 0;
}

/*
 * Called from IRQ1 handler (interrupt context).
 * Translates scancode → ASCII and pushes to ring buffer.
 */
void keyboard_handle_scancode(unsigned char scancode) {
    /* Key release (bit 7 set) */
    if (scancode & 0x80) {
        unsigned char released = scancode & 0x7F;
        /* Left/Right Shift released */
        if (released == 0x2A || released == 0x36) {
            shift_held = 0;
        }
        return;
    }

    /* Left/Right Shift pressed */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_held = 1;
        return;
    }

    /* Translate to ASCII */
    char c;
    if (shift_held) {
        c = scancode_to_ascii_shift[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    if (c != 0) {
        buf_push(c);
    }
}

/*
 * Blocking read — waits until a key is available.
 */
char keyboard_read(void) {
    while (buf_empty()) {
        __asm__ __volatile__("hlt");   /* sleep until next interrupt */
    }
    return buf_pop();
}

/*
 * Read a line of input into buf (max `max-1` chars + null terminator).
 * Echoes characters to VGA. Returns on Enter.
 */
void keyboard_readline(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = keyboard_read();

        if (c == '\n') {
            vga_putchar('\n');
            break;
        }

        if (c == '\b') {
            if (i > 0) {
                i--;
                /* Erase character on screen: move back, print space, move back */
                vga_print("\b \b");
            }
            continue;
        }

        buf[i++] = c;
        vga_putchar(c);
    }
    buf[i] = '\0';
}
