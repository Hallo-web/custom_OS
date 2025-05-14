#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Delay for a specified number of milliseconds
void delay(uint32_t milliseconds);

// Read from an I/O port
uint8_t inb(uint16_t port);

// Write to an I/O port
void outb(uint16_t port, uint8_t val);

// Get random number
int rand(void);

// Set random seed
void srand(uint32_t seed);

// Get system ticks
uint32_t get_ticks(void);

// Get uptime in seconds
uint32_t get_uptime(void);

// Format time string (hh:mm:ss)
void format_time(char *buffer, uint32_t seconds);

// Format date string (yyyy-mm-dd)
void format_date(char *buffer, int year, int month, int day);

// Parse hex string to integer
int hex_to_int(const char *hex);

// Check if character is digit
bool is_digit(char c);

// Check if character is letter
bool is_alpha(char c);

// Check if character is alphanumeric
bool is_alnum(char c);

// Check if character is whitespace
bool is_space(char c);

// Hash function for strings
uint32_t hash_string(const char *str);

// Encrypt string with simple XOR cipher
void encrypt_string(char *str, uint8_t key);

// Decrypt string with simple XOR cipher
void decrypt_string(char *str, uint8_t key);

// Find a substring in a string, case insensitive
char *strcasestr(const char *haystack, const char *needle);

// Trim whitespace from beginning and end of string
char *trim(char *str);

// Split string into tokens
int split_string(char *str, char delimiter, char **tokens, int max_tokens);

// Convert string to lowercase
void to_lower(char *str);

// Convert string to uppercase
void to_upper(char *str);

// Gets current simulated year
int get_current_year(void);

// Gets current simulated month (1-12)
int get_current_month(void);

// Gets current simulated day (1-31)
int get_current_day(void);

// Gets current simulated hour (0-23)
int get_current_hour(void);

// Gets current simulated minute (0-59)
int get_current_minute(void);

// Gets current simulated second (0-59)
int get_current_second(void);

#endif // UTILS_H