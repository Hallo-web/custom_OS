__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,   // magic
    0x00,         // flags
    -(0x1BADB002) // checksum = -(magic + flags)
};

/* Hardware text mode color constants */
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

// Global variables for terminal
static int terminal_row = 0;
static int terminal_column = 0;
static char *const VGA_MEMORY = (char *)0xb8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;
static int cursor_enabled = 0;

// Cursor position for terminal
static int cursor_x = 0;
static int cursor_y = 0;

// Function to create a VGA entry color
static inline unsigned char vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

// Function to create a VGA entry
static inline unsigned short vga_entry(unsigned char c, unsigned char color)
{
    return (unsigned short)c | (unsigned short)color << 8;
}

// Function to clear the screen with specific color
void clear_screen(unsigned char color)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        VGA_MEMORY[i * 2] = ' ';
        VGA_MEMORY[i * 2 + 1] = color;
    }
    terminal_row = 0;
    terminal_column = 0;
}

// Function to write a character at a specific position with a specific color
void putchar_at(char c, unsigned char color, int x, int y)
{
    const int index = y * VGA_WIDTH + x;
    VGA_MEMORY[index * 2] = c;
    VGA_MEMORY[index * 2 + 1] = color;
}

// Function to print a string at a specific position with a specific color
void print_at(const char *str, unsigned char color, int x, int y)
{
    int i = 0;
    while (str[i] != '\0')
    {
        putchar_at(str[i], color, x + i, y);
        i++;
    }
}

// Function to print a centered string on a specific row with a specific color
void print_centered(const char *str, unsigned char color, int row)
{
    int len = 0;
    while (str[len])
    {
        len++;
    }

    int col = (VGA_WIDTH - len) / 2;
    print_at(str, color, col, row);
}

// Sleep function to create delays (simple busy wait)
void sleep(unsigned int ticks)
{
    for (unsigned int i = 0; i < ticks * 10000000; i++)
    {
        __asm__ volatile("nop");
    }
}

// Function to display a large O.S.I.R.I.S text (ASCII art style)
void display_large_title()
{
    const char *logo[] = {
        " ██████╗    ███████╗   ██╗  ██████╗   ██╗  ███████╗ ",
        "██╔═══██╗  ██╔════╝   ██║  ██╔══██╗  ██║  ██╔════╝ ",
        "██║   ██║  ███████╗   ██║  ██████╔╝  ██║  ███████╗ ",
        "██║   ██║  ╚════██║   ██║  ██╔══██╗  ██║  ╚════██║ ",
        " ██████╔╝  ███████║   ██║  ██║  ██║  ██║  ███████║ ",
        " ╚═════╝   ╚══════╝   ╚═╝  ╚═╝  ╚═╝  ╚═╝  ╚══════╝ "};

    unsigned char logo_color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);

    int start_row = (VGA_HEIGHT - 6) / 2 - 3; // Center vertically with offset
    for (int i = 0; i < 6; i++)
    {
        print_centered(logo[i], logo_color, start_row + i);
    }
}

// Function to show a bootup sequence
void show_boot_sequence()
{
    // Initial screen
    clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    print_centered("O.S.I.R.I.S Bootloader v1.0", vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), 2);

    // Display boot messages
    const char *boot_messages[] = {
        "Initializing hardware...",
        "Loading kernel...",
        "Setting up memory management...",
        "Configuring device drivers...",
        "Starting system services...",
        "Preparing terminal interface..."};

    for (int i = 0; i < 6; i++)
    {
        print_at(boot_messages[i], vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK), 2, 5 + i);
        sleep(1); // Delay between messages
        print_at("[OK]", vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK), 70, 5 + i);
    }

    // Complete boot sequence
    sleep(1);
    print_centered("Boot complete! Starting O.S.I.R.I.S...", vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK), 13);
    sleep(2);
}

// Draw the cursor
void draw_cursor()
{
    if (cursor_enabled)
    {
        // Draw a blinking cursor (underscore)
        putchar_at('_', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), cursor_x, cursor_y);
    }
}

// Hide the cursor
void hide_cursor()
{
    if (cursor_enabled)
    {
        putchar_at(' ', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), cursor_x, cursor_y);
    }
}

// Terminal print function
void terminal_print(const char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        // Handle newline
        if (str[i] == '\n')
        {
            terminal_column = 0;
            terminal_row++;
            if (terminal_row >= VGA_HEIGHT)
            {
                // Simple scrolling - just clear for now
                // In a real OS, you would scroll the content up
                clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                terminal_row = 0;
            }
        }
        else
        {
            putchar_at(str[i], vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), terminal_column, terminal_row);
            terminal_column++;
            if (terminal_column >= VGA_WIDTH)
            {
                terminal_column = 0;
                terminal_row++;
                if (terminal_row >= VGA_HEIGHT)
                {
                    // Simple scrolling
                    clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                    terminal_row = 0;
                }
            }
        }
        i++;
    }

    // Update cursor position
    cursor_x = terminal_column;
    cursor_y = terminal_row;
}

// Initialize terminal
void init_terminal()
{
    clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_row = 0;
    terminal_column = 0;
    cursor_x = 0;
    cursor_y = 0;
    cursor_enabled = 1;

    // Print welcome message and prompt
    terminal_print("O.S.I.R.I.S Terminal v1.0\n");
    terminal_print("Copyright (c) 2025 OSIRIS OS Project\n");
    terminal_print("Type 'help' for available commands\n\n");
    terminal_print("osiris> ");
}

// Simple keyboard handler (placeholder)
void keyboard_handler()
{
    // This would normally read from the keyboard port
    // For now, this is just a placeholder
}

// Main kernel function
void kernel_main()
{
    // Show boot sequence
    show_boot_sequence();

    // Display the large OSIRIS title
    clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    display_large_title();

    // Wait and then transition to terminal
    sleep(3);

    // Initialize the terminal
    init_terminal();

    // Main loop for terminal blinking cursor
    while (1)
    {
        draw_cursor();
        sleep(1);
        hide_cursor();
        sleep(1);
        // In a real OS, you would handle keyboard input here
    }
}