#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Internal variables for time simulation
static uint32_t system_ticks = 0;
static uint32_t start_time = 0;
static int sim_year = 2025;
static int sim_month = 5;
static int sim_day = 15;
static int sim_hour = 12;
static int sim_minute = 0;
static int sim_second = 0;

// Delay for a specified number of milliseconds
void delay(uint32_t milliseconds) {
    // This is a simple busy wait - in a real system we'd use timers
    volatile uint64_t i;
    for (i = 0; i < milliseconds * 100000; i++) {
        asm volatile("nop");
    }

    // Update system time
    system_ticks += milliseconds;
    if (system_ticks >= 1000) {
        sim_second += system_ticks / 1000;
        system_ticks %= 1000;
        
        // Update minutes
        if (sim_second >= 60) {
            sim_minute += sim_second / 60;
            sim_second %= 60;
            
            // Update hours
            if (sim_minute >= 60) {
                sim_hour += sim_minute / 60;
                sim_minute %= 60;
                
                // Update days
                if (sim_hour >= 24) {
                    sim_day += sim_hour / 24;
                    sim_hour %= 24;
                    
                    // Simplistic month handling
                    if (sim_day > 30) {
                        sim_month += sim_day / 30;
                        sim_day = (sim_day % 30) + 1;
                        
                        // Update year
                        if (sim_month > 12) {
                            sim_year += sim_month / 12;
                            sim_month = (sim_month % 12) + 1;
                        }
                    }
                }
            }
        }
    }
}

// Read from an I/O port (simplified simulation)
uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

// Write to an I/O port (simplified simulation)
void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "dN"(port));
}

// Simple pseudo-random number generator
static uint32_t rand_seed = 12345;

// Get random number
int rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (int)((rand_seed / 65536) % 32768);
}

// Set random seed
void srand(uint32_t seed) {
    rand_seed = seed;
}

// Get system ticks
uint32_t get_ticks(void) {
    return system_ticks;
}

// Get uptime in seconds 
uint32_t get_uptime(void) {
    return (get_ticks() - start_time) / 1000; // Assuming ticks increment in milliseconds
}

// Format time string (hh:mm:ss)
void format_time(char *buffer, uint32_t seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, secs);
}

// Format date string (yyyy-mm-dd)
void format_date(char *buffer, int year, int month, int day) {
    sprintf(buffer, "%04d-%02d-%02d", year, month, day);
}

// Parse hex string to integer
int hex_to_int(const char *hex) {
    int result = 0;
    
    while (*hex) {
        char c = *hex++;
        int value;
        
        if (c >= '0' && c <= '9')
            value = c - '0';
        else if (c >= 'A' && c <= 'F')
            value = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            value = c - 'a' + 10;
        else
            break; // Invalid hex character
            
        result = result * 16 + value;
    }
    
    return result;
}

// Check if character is digit
bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

// Check if character is letter
bool is_alpha(char c) {
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

// Check if character is alphanumeric
bool is_alnum(char c) {
    return (is_digit(c) || is_alpha(c));
}

// Check if character is whitespace
bool is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

// Hash function for strings
uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

// Encrypt string with simple XOR cipher
void encrypt_string(char *str, uint8_t key) {
    while (*str) {
        *str = *str ^ key;
        str++;
    }
}

// Decrypt string with simple XOR cipher
void decrypt_string(char *str, uint8_t key) {
    // XOR is symmetric, so encryption and decryption are the same operation
    encrypt_string(str, key);
}

// Find a substring in a string, case insensitive
char *strcasestr(const char *haystack, const char *needle) {
    size_t needleLen = strlen(needle);
    if (needleLen == 0) {
        return (char*)haystack;
    }

    char *p = (char*)haystack;
    while (*p) {
        if (strncasecmp(p, needle, needleLen) == 0) {
            return p;
        }
        p++;
    }
    return NULL;
}

// Helper function for strcasestr
int strncasecmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) {
        return 0;
    }

    do {
        char c1 = (*s1 <= 'Z' && *s1 >= 'A') ? *s1 + ('a' - 'A') : *s1;
        char c2 = (*s2 <= 'Z' && *s2 >= 'A') ? *s2 + ('a' - 'A') : *s2;
        if (c1 != c2) {
            return c1 - c2;
        }
        if (c1 == '\0') {
            break;
        }
        s1++;
        s2++;
    } while (--n);
    
    return 0;
}

// Trim whitespace from beginning and end of string
char *trim(char *str) {
    if (!str) return NULL;
    
    // Trim leading space
    char *start = str;
    while (is_space(*start)) start++;
    
    // If all spaces
    if (*start == 0) {
        *str = 0;
        return str;
    }
    
    // Trim trailing space
    char *end = start + strlen(start) - 1;
    while (end > start && is_space(*end)) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    // If there was leading space, move the trimmed string to the start
    if (start != str) {
        memmove(str, start, (end - start) + 2); // +2 for the null terminator
    }
    
    return str;
}

// Split string into tokens
int split_string(char *str, char delimiter, char **tokens, int max_tokens) {
    int count = 0;
    char *ptr = str;
    char *start = str;
    
    while (*ptr && count < max_tokens) {
        if (*ptr == delimiter) {
            *ptr = '\0';  // Replace delimiter with null terminator
            tokens[count++] = start;
            start = ptr + 1;
        }
        ptr++;
    }
    
    // Add the last token if it exists and we have space
    if (*start && count < max_tokens) {
        tokens[count++] = start;
    }
    
    return count;
}

// Convert string to lowercase
void to_lower(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + ('a' - 'A');
        }
    }
}

// Convert string to uppercase
void to_upper(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - ('a' - 'A');
        }
    }
}

// Gets current simulated year
int get_current_year(void) {
    return sim_year;
}

// Gets current simulated month (1-12)
int get_current_month(void) {
    return sim_month;
}

// Gets current simulated day (1-31)
int get_current_day(void) {
    return sim_day;
}

// Gets current simulated hour (0-23)
int get_current_hour(void) {
    return sim_hour;
}

// Gets current simulated minute (0-59)
int get_current_minute(void) {
    return sim_minute;
}

// Gets current simulated second (0-59)
int get_current_second(void) {
    return sim_second;
}