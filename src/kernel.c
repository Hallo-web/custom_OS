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
};

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
static uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

// Create a VGA entry (character + color)
static uint16_t vga_entry(unsigned char c, uint8_t color)
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

// Put a character at the current position and advance cursor
void terminal_putchar(char c)
{
    if (c == '\n')
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH)
    {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_row = 0;
    }
}

// Write a string to the terminal
void terminal_writestring(const char *data)
{
    for (int i = 0; data[i] != '\0'; i++)
        terminal_putchar(data[i]);
}

// Draw a large ASCII art logo for OSIRIS
void draw_logo()
{
    // Set a cyan color for the logo
    uint8_t logo_color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    uint8_t old_color = terminal_color;
    terminal_setcolor(logo_color);

    // Clear screen first
    terminal_initialize();

    // Position for the logo (starting from row 5)
    terminal_row = 5;

    // ASCII Logo
    terminal_column = 15;
    terminal_writestring(" ██████╗   ███████╗  ██╗  ██████╗   ██╗  ███████╗ ");
    terminal_row++;
    terminal_column = 15;
    terminal_writestring("██╔═══██╗  ██╔════╝  ██║  ██╔══██╗  ██║  ██╔════╝ ");
    terminal_row++;
    terminal_column = 15;
    terminal_writestring("██║   ██║  ███████╗  ██║  ██████╔╝  ██║  ███████╗ ");
    terminal_row++;
    terminal_column = 15;
    terminal_writestring("██║   ██║  ╚════██║  ██║  ██╔══██╗  ██║  ╚════██║ ");
    terminal_row++;
    terminal_column = 15;
    terminal_writestring(" ██████╔╝  ███████║  ██║  ██║  ██║  ██║  ███████║ ");
    terminal_row++;
    terminal_column = 15;
    terminal_writestring(" ╚═════╝   ╚══════╝  ╚═╝  ╚═╝  ╚═╝  ╚═╝  ╚══════╝ ");

    // Restore color
    terminal_setcolor(old_color);
}

// Delay function - simple busy wait
void delay(uint32_t count)
{
    volatile uint32_t i = 0;
    for (i = 0; i < count * 10000000; i++)
    {
        asm("nop");
    }
}

// Show boot sequence messages
void show_boot_sequence()
{
    terminal_initialize();
    terminal_writestring("O.S.I.R.I.S Boot Sequence v1.0\n");
    terminal_writestring("----------------------------\n\n");

    const char *messages[] = {
        "Initializing hardware...",
        "Loading kernel components...",
        "Setting up memory management...",
        "Configuring device drivers...",
        "Starting system services...",
        "Preparing terminal interface..."};

    uint8_t status_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint8_t old_color = terminal_color;

    for (int i = 0; i < 6; i++)
    {
        terminal_setcolor(text_color);
        terminal_writestring(messages[i]);

        // Delay to make it look like it's doing something
        delay(1);

        terminal_setcolor(status_color);
        terminal_column = 70;
        terminal_writestring("[OK]\n");
    }

    terminal_setcolor(old_color);
    terminal_writestring("\nBoot complete! Starting O.S.I.R.I.S...\n");
    delay(2);
}

// Initialize terminal interface
void init_terminal_interface()
{
    terminal_initialize();
    terminal_writestring("O.S.I.R.I.S Terminal v1.0\n");
    terminal_writestring("Copyright (c) 2025 OSIRIS OS Project\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    terminal_writestring("osiris> ");
}

// Main kernel function
void kernel_main(void)
{
    // First show boot sequence
    show_boot_sequence();

    // Then show logo
    draw_logo();

    // Wait to show the logo for a while
    delay(10);

    // Initialize terminal interface
    init_terminal_interface();

    // Halt CPU - in a real OS, we would handle input and implement a shell
    while (1)
    {
        // Simple cursor blinking effect
        terminal_column = 7; // End of "osiris> "
        terminal_putchar('_');
        delay(2);
        terminal_column = 7;
        terminal_putchar(' ');
        delay(1);
    }
}