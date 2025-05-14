#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

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

// Initialize terminal interface
void init_terminal_interface(void);

// Terminal state
static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;

// Current command buffer
static char command_buffer[256];
static int command_length = 0;

// PS/2 keyboard port
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

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

// Draw a simpler ASCII art logo for OSIRIS
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

    // Simpler ASCII Logo
    terminal_column = 20;
    terminal_writestring("    ____   _____  _____  _____  _____  _____   ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("   / __ \\ / ____||_   _||  __ \\|_   _|/ ____|  ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("  | |  | | (___    | |  | |__) | | | | (___    ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("  | |  | |\\___ \\   | |  |  _  /  | |  \\___ \\   ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("  | |__| |____) | _| |_ | | \\ \\ _| |_ ____) |  ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("   \\____/|_____/ |_____||_|  \\_\\_____|\\_____/  ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("                                               ");
    terminal_row++;
    terminal_column = 20;
    terminal_writestring("             Operating System v1.0             ");

    // Restore color
    terminal_setcolor(old_color);
}

// Delay function - improved to be more consistent
void delay(uint32_t milliseconds)
{
    // This is a simple busy wait - in a real system we'd use timers
    // We'll simulate milliseconds with busy looping, which is not precise
    // but works for demonstration purposes
    volatile uint64_t i;
    for (i = 0; i < milliseconds * 100000; i++)
    {
        asm volatile("nop");
    }
}

// Show boot sequence messages with slower, more human-readable timing
void show_boot_sequence()
{
    terminal_initialize();
    terminal_writestring("O.S.I.R.I.S Boot Sequence v1.0\n");
    terminal_writestring("------------------------------\n\n");

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

        // Write the message character by character for effect
        const char *msg = messages[i];
        for (int j = 0; msg[j] != '\0'; j++)
        {
            terminal_putchar(msg[j]);
            delay(30); // 30ms delay between characters
        }

        // Pause before showing status
        delay(500); // 500ms pause

        terminal_setcolor(status_color);
        terminal_column = 70;
        terminal_writestring("[OK]\n");

        // Pause between lines
        delay(300); // 300ms between lines
    }

    terminal_setcolor(old_color);
    terminal_writestring("\nBoot complete! Starting O.S.I.R.I.S...\n");
    delay(1000); // 1 second pause before continuing
}

// Read from an I/O port
uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

// Write to an I/O port
void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "dN"(port));
}

// Simple US QWERTY keyboard mapping
const char keyboard_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Process keyboard input
char get_keyboard_input()
{
    if ((inb(KEYBOARD_STATUS_PORT) & 1) != 0)
    {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        // Only process key presses (not releases)
        if (scancode < 128)
        {
            return keyboard_map[scancode];
        }
    }
    return 0; // No input
}

// Process the current command
void process_command()
{
    if (command_length == 0)
    {
        return;
    }

    // Add null terminator
    command_buffer[command_length] = '\0';

    // Process commands
    if (strcmp(command_buffer, "help") == 0)
    {
        terminal_writestring("\nAvailable commands:\n");
        terminal_writestring("  help     - Show this help message\n");
        terminal_writestring("  clear    - Clear the screen\n");
        terminal_writestring("  logo     - Display the OSIRIS logo\n");
        terminal_writestring("  about    - Show information about OSIRIS\n");
        terminal_writestring("  reboot   - Simulate system reboot\n");
    }
    else if (strcmp(command_buffer, "clear") == 0)
    {
        terminal_initialize();
    }
    else if (strcmp(command_buffer, "logo") == 0)
    {
        draw_logo();
        delay(2000);
        terminal_initialize();
    }
    else if (strcmp(command_buffer, "about") == 0)
    {
        terminal_writestring("\nO.S.I.R.I.S - Operating System Interface Research & Integration System\n");
        terminal_writestring("Version 1.0 - 2025\n");
        terminal_writestring("A minimal, educational operating system for x86 architecture.\n");
    }
    else if (strcmp(command_buffer, "reboot") == 0)
    {
        terminal_writestring("\nRebooting system...\n");
        delay(1000);
        show_boot_sequence();
        draw_logo();
        delay(2000);
        init_terminal_interface();
        return;
    }
    else
    {
        terminal_writestring("\nUnknown command: ");
        terminal_writestring(command_buffer);
        terminal_writestring("\nType 'help' for available commands.\n");
    }

    // Reset command buffer
    command_length = 0;

    // Print prompt
    terminal_writestring("\n>> ");
}

// Handle keyboard input
void handle_keyboard()
{
    char c = get_keyboard_input();
    if (c != 0)
    {
        // Handle backspace
        if (c == '\b' && command_length > 0)
        {
            command_length--;
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        // Handle enter
        else if (c == '\n')
        {
            terminal_putchar('\n');
            process_command();
        }
        // Handle regular character
        else if (c >= ' ' && command_length < 255)
        {
            command_buffer[command_length++] = c;
            terminal_putchar(c);
        }
    }
}

// Display blinking cursor at current position
void display_cursor(bool visible)
{
    if (visible)
    {
        terminal_putentryat('_', terminal_color, terminal_column, terminal_row);
    }
    else
    {
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
}

// Initialize terminal interface
void init_terminal_interface(void)
{
    terminal_initialize();
    terminal_writestring("O.S.I.R.I.S Terminal v1.0\n");
    terminal_writestring("Copyright (c) 2025 OSIRIS OS Project\n");
    terminal_writestring("Type 'help' for available commands.\n\n");
    terminal_writestring("===============O.S.I.R.I.S===============\n");
    terminal_writestring(">> ");

    // Reset command buffer
    command_length = 0;
}

// Main kernel function
void kernel_main(void)
{
    // First show boot sequence
    show_boot_sequence();

    // Then show logo
    draw_logo();

    // Wait to show the logo for a while
    delay(2000); // Show logo for 2 seconds

    // Initialize terminal interface
    init_terminal_interface();

    bool cursor_visible = true;
    uint32_t cursor_timer = 0;
    uint32_t input_timer = 0;

    // Main loop - handle keyboard input and cursor blinking
    while (1)
    {
        // Check for keyboard input every 10ms
        if (input_timer >= 10)
        {
            handle_keyboard();
            input_timer = 0;
        }

        // Blink cursor every 500ms
        if (cursor_timer >= 500)
        {
            cursor_visible = !cursor_visible;
            display_cursor(cursor_visible);
            cursor_timer = 0;
        }

        // Wait 10ms to avoid excessive CPU usage
        delay(10);
        cursor_timer += 10;
        input_timer += 10;
    }
}