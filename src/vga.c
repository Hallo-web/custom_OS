#include "vga.h"
#include "string.h"
#include <stdbool.h>

// VGA text buffer address
static uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

// Terminal state
static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;

// Create a VGA entry color
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

// Create a VGA entry (character + color)
uint16_t vga_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | (uint16_t)color << 8;
}

// Initialize the terminal
void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;

    // Clear the screen
    for (int y = 0; y < VGA_HEIGHT; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            const int index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

// Set the terminal color
void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

// Put a character at a specific position
void terminal_putentryat(char c, uint8_t color, int x, int y)
{
    const int index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

// Scroll the terminal up one line
void terminal_scroll()
{
    for (int y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (int x = 0; x < VGA_WIDTH; x++)
        {
            const int dst_index = y * VGA_WIDTH + x;
            const int src_index = (y + 1) * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }

    // Clear the last line
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        const int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

// Put a character at the current position and advance cursor
void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
        return;
    }
    else if (c == '\r')
    {
        terminal_column = 0;
        return;
    }
    else if (c == '\t')
    {
        // Tab character - advance to next 4-column boundary
        terminal_column = (terminal_column + 4) & ~3;
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT)
                terminal_scroll();
        }
        return;
    }
    else if (c == '\b')
    {
        if (terminal_column > 0)
        {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH)
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
    }
}

// Write a string to the terminal
void terminal_writestring(const char *data)
{
    for (int i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

// Write a string with a specific color
void terminal_writestring_colored(const char *data, uint8_t color)
{
    uint8_t old_color = terminal_color;
    terminal_setcolor(color);
    terminal_writestring(data);
    terminal_setcolor(old_color);
}

// Clear a specific line
void clear_line(int line)
{
    for (int x = 0; x < VGA_WIDTH; x++)
    {
        terminal_putentryat(' ', terminal_color, x, line);
    }
}

// Clear a specific region
void terminal_clear_region(int x1, int y1, int x2, int y2)
{
    for (int y = y1; y <= y2; y++)
    {
        for (int x = x1; x <= x2; x++)
        {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
}

// Display a progress bar
void display_progress_bar(int progress, int total, int width)
{
    int filled = width * progress / total;

    terminal_putchar('[');
    for (int i = 0; i < width; i++)
    {
        if (i < filled)
        {
            terminal_putchar('=');
        }
        else
        {
            terminal_putchar(' ');
        }
    }
    terminal_putchar(']');

    // Print percentage
    char buffer[5];
    itoa(progress * 100 / total, buffer, 10);
    terminal_putchar(' ');
    terminal_writestring(buffer);
    terminal_writestring("%");
}

// Draw a box border
void draw_box(int x1, int y1, int x2, int y2, uint8_t color)
{
    uint8_t old_color = terminal_color;
    terminal_setcolor(color);

    // Draw horizontal borders
    for (int x = x1; x <= x2; x++)
    {
        terminal_putentryat('-', color, x, y1);
        terminal_putentryat('-', color, x, y2);
    }

    // Draw vertical borders
    for (int y = y1 + 1; y < y2; y++)
    {
        terminal_putentryat('|', color, x1, y);
        terminal_putentryat('|', color, x2, y);
    }

    // Draw corners
    terminal_putentryat('+', color, x1, y1);
    terminal_putentryat('+', color, x2, y1);
    terminal_putentryat('+', color, x1, y2);
    terminal_putentryat('+', color, x2, y2);

    terminal_setcolor(old_color);
}

// Print centered text
void print_centered(const char *str, int row, uint8_t color)
{
    uint8_t old_color = terminal_color;
    terminal_setcolor(color);

    int len = strlen(str);
    terminal_column = (VGA_WIDTH - len) / 2;
    terminal_row = row;

    terminal_writestring(str);

    terminal_setcolor(old_color);
}

// Print a fancy header
void print_fancy_header(const char *title)
{
    int title_len = strlen(title);
    int padding = (VGA_WIDTH - title_len - 4) / 2;
    uint8_t header_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);

    uint8_t old_color = terminal_color;
    terminal_setcolor(header_color);

    // Top border
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putentryat('=', header_color, i, terminal_row);
    }
    terminal_row++;

    // Title row
    for (int i = 0; i < padding; i++)
    {
        terminal_putentryat(' ', header_color, i, terminal_row);
    }

    terminal_column = padding;
    terminal_writestring("[ ");
    terminal_writestring(title);
    terminal_writestring(" ]");

    for (int i = padding + title_len + 4; i < VGA_WIDTH; i++)
    {
        terminal_putentryat(' ', header_color, i, terminal_row);
    }
    terminal_row++;

    // Bottom border
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putentryat('=', header_color, i, terminal_row);
    }
    terminal_row++;

    terminal_column = 0;
    terminal_setcolor(old_color);
}

// Draw a simpler ASCII art logo for OSIRIS
void draw_logo()
{
    // Set a cyan color for the logo
    uint8_t logo_color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    uint8_t highlight_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t old_color = terminal_color;

    // Clear screen first
    terminal_initialize();

    // Draw a fancy border around the logo
    draw_box(15, 4, 65, 14, vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));

    // Position for the logo (starting from row 5)
    terminal_row = 5;

    // Simpler ASCII Logo with some highlights
    terminal_setcolor(logo_color);
    terminal_column = 20;
    terminal_writestring("    ____   _____  _____  _____  _____  _____   ");
    terminal_row++;

    terminal_column = 20;
    terminal_setcolor(highlight_color);
    terminal_writestring("   / __ \\ ");
    terminal_setcolor(logo_color);
    terminal_writestring("/ ____||_   _||  __ \\|_   _|/ ____|  ");
    terminal_row++;

    terminal_column = 20;
    terminal_setcolor(highlight_color);
    terminal_writestring("  | |  | |");
    terminal_setcolor(logo_color);
    terminal_writestring(" (___    | |  | |__) | | | | (___    ");
    terminal_row++;

    terminal_column = 20;
    terminal_setcolor(highlight_color);
    terminal_writestring("  | |  | |");
    terminal_setcolor(logo_color);
    terminal_writestring("\\___ \\   | |  |  _  /  | |  \\___ \\   ");
    terminal_row++;

    terminal_column = 20;
    terminal_setcolor(highlight_color);
    terminal_writestring("  | |__| |");
    terminal_setcolor(logo_color);
    terminal_writestring("____) | _| |_ | | \\ \\ _| |_ ____) |  ");
    terminal_row++;

    terminal_column = 20;
    terminal_setcolor(highlight_color);
    terminal_writestring("   \\____/");
    terminal_setcolor(logo_color);
    terminal_writestring("|_____/ |_____||_|  \\_\\_____|\\_____/  ");
    terminal_row++;

    terminal_column = 20;
    terminal_writestring("                                               ");
    terminal_row++;

    // Version with gradient effect
    uint8_t grad_colors[] = {
        vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK),
        vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK),
        vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)};

    terminal_column = 20;
    const char *version_text = "    Operating System Interface v2.0    ";
    for (int i = 0; version_text[i] != '\0'; i++)
    {
        terminal_setcolor(grad_colors[i % 3]);
        terminal_putchar(version_text[i]);
    }

    terminal_row += 2;
    terminal_column = 17;
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Research, Integration & Security Information System");

    // Restore color
    terminal_setcolor(old_color);
}