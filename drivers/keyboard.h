#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Initialize the keyboard driver */
void keyboard_init(void);

/* Called by the IRQ1 handler — processes a raw scancode */
void keyboard_handle_scancode(unsigned char scancode);

/* Read a single character (blocking). Returns ASCII char. */
char keyboard_read(void);

/* Read a line into buf (max len chars). Echoes to VGA. Returns on Enter. */
void keyboard_readline(char *buf, int max);

#endif /* KEYBOARD_H */
