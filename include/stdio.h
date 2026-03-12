#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

/* Print unformatted string */
int puts(const char *str);

/* Put single character */
int putchar(int c);

/* Print formatted string to screen */
int printf(const char *format, ...);

#endif /* STDIO_H */
