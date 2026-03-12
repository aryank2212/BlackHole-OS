#ifndef STRING_H
#define STRING_H

#include "stdint.h"

void *memset(void *dst, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

/* Utility: converts integer to string. `base` should be 10 or 16. */
char *itoa(int value, char *str, int base);
char *uitoa(uint32_t value, char *str, int base);

#endif /* STRING_H */
