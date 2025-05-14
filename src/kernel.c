// Cursor position for terminal
static int cursor_x = 0;
static int cursor_y = 0;
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

// Command buffer for terminal input
static char command_buffer[256];
static int command_pos = 0;

// Cursor position for terminal
#define MAX_FILES 16
#define MAX_FILENAME_LENGTH 32
#define MAX_FILE_CONTENT_LENGTH 1024

// Very simple in-memory filesystem structure
typedef struct
{
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
    int content_length;
    int created; // 0 = empty slot, 1 = file exists
} File;

// Global filesystem
static File filesystem[MAX_FILES];
static int fs_initialized = 0;

// Initialize filesystem
void init_filesystem()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        filesystem[i].created = 0;
        filesystem[i].content_length = 0;
        filesystem[i].filename[0] = '\0';
    }

    // Create a README file
    int readme_index = 0;
    for (int i = 0; i < MAX_FILENAME_LENGTH - 1 && "README.TXT"[i] != '\0'; i++)
    {
        filesystem[0].filename[i] = "README.TXT"[i];
    }
    filesystem[0].filename[8] = '\0';

    const char *readme_content = "Welcome to O.S.I.R.I.S!\n\n"
                                 "This is a simple operating system with basic terminal functionality.\n"
                                 "Use 'help' command to see available commands.\n";

    for (int i = 0; i < MAX_FILE_CONTENT_LENGTH - 1 && readme_content[i] != '\0'; i++)
    {
        filesystem[0].content[i] = readme_content[i];
        filesystem[0].content_length++;
    }
    filesystem[0].content[filesystem[0].content_length] = '\0';
    filesystem[0].created = 1;

    fs_initialized = 1;
}

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
    // Much slower sleep function - increased by 5x
    for (unsigned int i = 0; i < ticks * 50000000; i++)
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
    command_pos = 0;
    command_buffer[0] = '\0';

    // Print welcome message and prompt
    terminal_print("O.S.I.R.I.S Terminal v1.0\n");
    terminal_print("Copyright (c) 2025 OSIRIS OS Project\n");
    terminal_print("Type 'help' for available commands\n\n");
    terminal_print("osiris> ");
}

// Keyboard port definitions
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Read a byte from a port
unsigned char inb(unsigned short port)
{
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

// Write a byte to a port
void outb(unsigned short port, unsigned char data)
{
    __asm__("out %%al, %%dx" : : "a"(data), "d"(port));
}

// US keyboard layout scancode to ASCII mapping
const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0};

// Check if keyboard has data available
int keyboard_has_key()
{
    return inb(KEYBOARD_STATUS_PORT) & 1;
}

// Get a key from keyboard (blocking)
char get_key()
{
    // Wait for key press
    while (!keyboard_has_key())
        ;

    // Read scancode
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    // Only process key press events (not key release)
    if (scancode < 0x80)
    {
        // Convert scancode to ASCII
        if (scancode < sizeof(scancode_to_ascii))
        {
            return scancode_to_ascii[scancode];
        }
    }

    return 0; // Return 0 for unsupported keys
}

// String comparison helper
int str_equals(const char *s1, const char *s2)
{
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0')
    {
        if (s1[i] != s2[i])
        {
            return 0;
        }
        i++;
    }
    return s1[i] == s2[i]; // Both ended at the same time
}

// String starts with helper
int str_starts_with(const char *str, const char *prefix)
{
    int i = 0;
    while (prefix[i] != '\0')
    {
        if (str[i] != prefix[i])
        {
            return 0;
        }
        i++;
    }
    return 1;
}

// Process a command
void process_command(const char *cmd)
{
    if (cmd[0] == 0)
    {
        // Empty command, do nothing
        terminal_print("\nosiris> ");
        return;
    }

    // Check if filesystem is initialized
    if (!fs_initialized)
    {
        init_filesystem();
    }

    // Process different commands
    if (str_equals(cmd, "help"))
    {
        terminal_print("\nAvailable commands:\n");
        terminal_print("  help     - Show this help message\n");
        terminal_print("  clear    - Clear the screen\n");
        terminal_print("  version  - Show OS version\n");
        terminal_print("  splash   - Show splash screen\n");
        terminal_print("  ls       - List files\n");
        terminal_print("  cat      - Display file contents (usage: cat filename)\n");
        terminal_print("  write    - Create a new file (usage: write filename)\n");
        terminal_print("\nosiris> ");
    }
    else if (str_equals(cmd, "clear"))
    {
        clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_row = 0;
        terminal_column = 0;
        terminal_print("osiris> ");
    }
    else if (str_equals(cmd, "version"))
    {
        terminal_print("\nO.S.I.R.I.S Operating System v1.0\n");
        terminal_print("Copyright (c) 2025 OSIRIS OS Project\n");
        terminal_print("\nosiris> ");
    }
    else if (str_equals(cmd, "splash"))
    {
        // Show splash screen
        clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        display_large_title();
        sleep(8);

        // Return to terminal
        clear_screen(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        terminal_row = 0;
        terminal_column = 0;
        terminal_print("O.S.I.R.I.S Terminal v1.0\n");
        terminal_print("osiris> ");
    }
    else if (str_equals(cmd, "ls"))
    {
        terminal_print("\nFiles:\n");
        int files_found = 0;

        for (int i = 0; i < MAX_FILES; i++)
        {
            if (filesystem[i].created)
            {
                terminal_print("  ");
                terminal_print(filesystem[i].filename);
                terminal_print("\n");
                files_found++;
            }
        }

        if (files_found == 0)
        {
            terminal_print("  No files found\n");
        }

        terminal_print("\nosiris> ");
    }
    else if (str_starts_with(cmd, "cat "))
    {
        // Extract filename (skip "cat " prefix)
        const char *filename = cmd + 4;
        int found = 0;

        for (int i = 0; i < MAX_FILES; i++)
        {
            if (filesystem[i].created && str_equals(filesystem[i].filename, filename))
            {
                terminal_print("\n");
                terminal_print(filesystem[i].content);
                terminal_print("\n\nosiris> ");
                found = 1;
                break;
            }
        }

        if (!found)
        {
            terminal_print("\nFile not found: ");
            terminal_print(filename);
            terminal_print("\n\nosiris> ");
        }
    }
    else if (str_starts_with(cmd, "write "))
    {
        // Extract filename (skip "write " prefix)
        const char *filename = cmd + 6;
        int file_idx = -1;

        // Find an empty slot or overwrite existing file
        for (int i = 0; i < MAX_FILES; i++)
        {
            if (!filesystem[i].created)
            {
                file_idx = i;
                break;
            }
            else if (str_equals(filesystem[i].filename, filename))
            {
                file_idx = i;
                break;
            }
        }

        if (file_idx == -1)
        {
            terminal_print("\nFilesystem full\n\nosiris> ");
        }
        else
        {
            // Copy filename
            int j = 0;
            while (filename[j] != '\0' && j < MAX_FILENAME_LENGTH - 1)
            {
                filesystem[file_idx].filename[j] = filename[j];
                j++;
            }
            filesystem[file_idx].filename[j] = '\0';

            // Enter content mode
            terminal_print("\nWriting to file. Enter content (end with EOF on a new line):\n");

            // Reset content
            filesystem[file_idx].content_length = 0;
            filesystem[file_idx].content[0] = '\0';
            filesystem[file_idx].created = 1;

            // Let main loop handle content entry (not implemented here for simplicity)
            // In a real OS, you would switch to a file content mode

            terminal_print("\nFile created!\n\nosiris> ");
        }
    }
    else
    {
        terminal_print("\nUnknown command: ");
        terminal_print(cmd);
        terminal_print("\nType 'help' for available commands\n");
        terminal_print("osiris> ");
    }
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
    sleep(8);

    // Initialize filesystem
    init_filesystem();

    // Initialize the terminal
    init_terminal();

    // Main terminal loop with keyboard input
    while (1)
    {
        // Show the cursor
        draw_cursor();

        // Check for keyboard input (with timeout)
        int timeout_counter = 0;
        while (!keyboard_has_key() && timeout_counter < 20000000)
        {
            timeout_counter++;
        }

        if (keyboard_has_key())
        {
            // Hide cursor during typing
            hide_cursor();

            // Get the key
            char key = get_key();

            // Process the key
            if (key == '\n')
            {
                // Process the command
                command_buffer[command_pos] = '\0';
                process_command(command_buffer);
                command_pos = 0;
            }
            else if (key == '\b' && command_pos > 0)
            {
                // Handle backspace
                command_pos--;
                cursor_x--;
                terminal_column--;
                putchar_at(' ', vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), cursor_x, cursor_y);
            }
            else if (key >= ' ' && key <= '~' && command_pos < 255)
            {
                // Regular character
                command_buffer[command_pos++] = key;
                putchar_at(key, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK), cursor_x, cursor_y);
                cursor_x++;
                terminal_column++;
            }
        }
        else
        {
            // No key pressed, blink the cursor
            hide_cursor();
            sleep(1);
        }
    }
}