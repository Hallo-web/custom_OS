#include "string.h"
#include <stddef.h>
#include <stdarg.h>
#include "vga.h" // For terminal output functions

// String comparison
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// String comparison (ignoring case)
int strcasecmp(const char *s1, const char *s2)
{
    unsigned char c1, c2;
    do
    {
        c1 = *s1++;
        c2 = *s2++;
        // Convert to lowercase
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 'a' - 'A';
    } while (c1 && c1 == c2);

    return c1 - c2;
}

// String copy
void strcpy(char *dest, const char *src)
{
    while ((*dest++ = *src++))
        ;
}

// String length
size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// String concatenation
char *strcat(char *dest, const char *src)
{
    char *ptr = dest + strlen(dest);
    while (*src)
    {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

// String containing
char *strstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
        return (char *)haystack;

    while (*haystack)
    {
        if (strncmp(haystack, needle, needle_len) == 0)
        {
            return (char *)haystack;
        }
        haystack++;
    }
    return NULL;
}

// String comparison for n characters
int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// Reverse a string
void reverse(char *str, int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end)
    {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Integer to string conversion
char *itoa(int num, char *str, int base)
{
    int i = 0;
    bool is_negative = false;

    // Handle 0 explicitly
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers only for decimal
    if (num < 0 && base == 10)
    {
        is_negative = true;
        num = -num;
    }

    // Convert number to string
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // Add negative sign if needed
    if (is_negative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    reverse(str, i);

    return str;
}

// String to integer conversion
int atoi(const char *str)
{
    int result = 0;
    int sign = 1;

    // Skip leading whitespace
    while (*str == ' ' || *str == '\t')
    {
        str++;
    }

    // Handle sign
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert string to integer
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}

// Simple printf-like function
void printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[64];

    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%' && format[i + 1] != '\0')
        {
            i++;
            switch (format[i])
            {
            case 'd':
            {
                // Integer
                int val = va_arg(args, int);
                itoa(val, buffer, 10);
                terminal_writestring(buffer);
                break;
            }
            case 'x':
            {
                // Hexadecimal
                int val = va_arg(args, int);
                itoa(val, buffer, 16);
                terminal_writestring(buffer);
                break;
            }
            case 'c':
            {
                // Character
                int c = va_arg(args, int);
                terminal_putchar((char)c);
                break;
            }
            case 's':
            {
                // String
                char *s = va_arg(args, char *);
                if (s == NULL)
                {
                    terminal_writestring("(null)");
                }
                else
                {
                    terminal_writestring(s);
                }
                break;
            }
            case '%':
            {
                // Literal %
                terminal_putchar('%');
                break;
            }
            default:
                terminal_putchar('%');
                terminal_putchar(format[i]);
                break;
            }
        }
        else
        {
            terminal_putchar(format[i]);
        }
    }

    va_end(args);
}