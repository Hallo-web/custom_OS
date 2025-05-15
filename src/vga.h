#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// Constants for VGA text mode
enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
    VGA_COLOR_YELLOW = 16,
};

// External variables (exposed for kernel.c)
extern int terminal_row;
extern int terminal_column;
extern uint8_t terminal_color;
extern uint16_t *terminal_buffer;

// Create a VGA entry color
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);

// Create a VGA entry (character + color)
uint16_t vga_entry(unsigned char c, uint8_t color);

// Initialize the terminal
void terminal_initialize(void);

// Set the terminal color
void terminal_setcolor(uint8_t color);

// Put a character at a specific position
void terminal_putentryat(char c, uint8_t color, int x, int y);

// Scroll the terminal up one line
void terminal_scroll(void);

// Put a character at the current position and advance cursor
void terminal_putchar(char c);

// Write a string to the terminal
void terminal_writestring(const char *data);

// Write a string with a specific color
void terminal_writestring_colored(const char *data, uint8_t color);

// Clear a specific line
void clear_line(int line);

// Clear a specific region
void terminal_clear_region(int x1, int y1, int x2, int y2);

// Display a progress bar
void display_progress_bar(int progress, int total, int width);

// Draw a box border
void draw_box(int x1, int y1, int x2, int y2, uint8_t color);

// Print centered text
void print_centered(const char *str, int row, uint8_t color);

// Print a fancy header
void print_fancy_header(const char *title);

// Draw the OSIRIS logo
void draw_logo(void);

#endif // VGA_H