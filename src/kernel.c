#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "vga.h"      // VGA display functions
#include "string.h"   // String utilities
#include "user.h"     // User management
#include "fs.h"       // File system operations
#include "terminal.h" // Terminal handling
#include "editor.h"   // Text editor
#include "utils.h"    // Utility functions
#include "system.h"   // System information and control

// VGA text buffer address
static uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

// System state flags
#define SYSTEM_RUNNING 0
#define SYSTEM_HALTED 1
#define SYSTEM_REBOOT 2
#define SYSTEM_SHUTDOWN 3

// Terminal state
static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;
static int system_state = SYSTEM_RUNNING;

// Current command buffer
static char command_buffer[256];
static int command_length = 0;

// Command history
#define HISTORY_SIZE 10
static char command_history[HISTORY_SIZE][256];
static int history_count = 0;
static int history_position = -1;

// PS/2 keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Keyboard state
static bool shift_pressed = false;
static bool caps_lock = false;
static bool ctrl_pressed = false;

// Time tracking (simulated)
static uint32_t system_ticks = 0;
static uint32_t uptime_seconds = 0;

// Hidden admin password
#define HIDDEN_ADMIN_PASSWORD "osiris1371" // Reference to Egyptian mythology, 1371 BCE is when Osiris temple was built

// The secret message revealing the hidden admin password
const char *SECRET_MESSAGE = "The key to enlightenment is found in the year the temple was built: osiris1371";

// Function declarations
void init_terminal_interface(void);
void print_fancy_header(const char *title);
void print_centered(const char *str, int row, uint8_t color);
void draw_box(int x1, int y1, int x2, int y2, uint8_t color);
void terminal_writestring_colored(const char *data, uint8_t color);
void printf(const char *format, ...);

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

    // Update system time
    system_ticks += milliseconds;
    if (system_ticks >= 1000)
    {
        uptime_seconds += system_ticks / 1000;
        system_ticks %= 1000;
    }
}

// Show boot sequence messages with slower, more human-readable timing
void show_boot_sequence()
{
    terminal_initialize();
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint8_t status_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    uint8_t old_color = terminal_color;

    // Title with fancy effects
    print_fancy_header("O.S.I.R.I.S Boot Sequence v2.0");

    terminal_row++;

    const char *messages[] = {
        "Initializing hardware detection...",
        "Loading kernel components...",
        "Setting up memory management...",
        "Configuring virtual device drivers...",
        "Starting system services...",
        "Initializing virtual file system...",
        "Loading user interface components...",
        "Preparing terminal interface...",
        "Setting up command interpreter...",
        "Performing security checks..."};

    // Initialize progress counter
    int total_steps = sizeof(messages) / sizeof(messages[0]);

    for (int i = 0; i < total_steps; i++)
    {
        terminal_setcolor(text_color);

        // Calculate progress percentage
        int progress = (i * 100) / total_steps;

        // Write the message character by character for effect
        const char *msg = messages[i];
        terminal_column = 2;
        for (int j = 0; msg[j] != '\0'; j++)
        {
            terminal_putchar(msg[j]);
            delay(10); // 10ms delay between characters
        }

        // Pause before showing status
        delay(100); // 100ms pause

        // Show progress bar
        terminal_column = 40;
        display_progress_bar(i + 1, total_steps, 20);

        // Show status
        terminal_column = 70;
        terminal_setcolor(status_color);
        terminal_writestring("[OK]");
        terminal_putchar('\n');

        // Pause between lines
        delay(150); // 150ms between lines
    }

    terminal_setcolor(old_color);
    terminal_row++;
    print_centered("Boot sequence complete! Starting O.S.I.R.I.S...", terminal_row,
                   vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_row++;
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

// Keyboard mapping for US QWERTY layout
// Normal keys (unshifted)
const char keyboard_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Shifted keys
const char keyboard_map_shifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Special key codes
#define KEY_SHIFT 42
#define KEY_SHIFT_R 54
#define KEY_CTRL 29
#define KEY_ALT 56
#define KEY_CAPS_LOCK 58
#define KEY_F1 59
#define KEY_F2 60
#define KEY_F3 61
#define KEY_F4 62
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_HOME 71
#define KEY_END 79
#define KEY_PGUP 73
#define KEY_PGDN 81
#define KEY_DELETE 83
#define KEY_BACKSPACE 14
#define KEY_ENTER 28
#define KEY_ESC 1

// Process keyboard input with improved key handling
char get_keyboard_input()
{
    if ((inb(KEYBOARD_STATUS_PORT) & 1) != 0)
    {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        // Check for key release
        if (scancode & 0x80)
        {
            scancode &= 0x7F; // Clear the top bit

            // Handle modifier key releases
            if (scancode == KEY_SHIFT || scancode == KEY_SHIFT_R)
            {
                shift_pressed = false;
            }

            return 0; // We don't process key releases further
        }

        // Handle special keys
        switch (scancode)
        {
        case KEY_SHIFT:
        case KEY_SHIFT_R:
            shift_pressed = true;
            return 0;

        case KEY_CAPS_LOCK:
            caps_lock = !caps_lock;
            return 0;

        case KEY_F1:
            return 0; // Special handling for function keys

        case KEY_ESC:
            return 27; // ESC character