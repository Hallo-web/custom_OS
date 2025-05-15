#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h> // Added for bool type used in some functions

// String comparison
int strcmp(const char *s1, const char *s2);

// String comparison (ignoring case)
int strcasecmp(const char *s1, const char *s2);

// String copy
void strcpy(char *dest, const char *src);

// String length
size_t strlen(const char *str);

// String concatenation
char *strcat(char *dest, const char *src);

// String containing
char *strstr(const char *haystack, const char *needle);

// String comparison for n characters
int strncmp(const char *s1, const char *s2, size_t n);

// Reverse a string
void reverse(char *str, int length);

// Integer to string conversion
char *itoa(int num, char *str, int base);

// String to integer conversion
int atoi(const char *str);

#endif // STRING_H