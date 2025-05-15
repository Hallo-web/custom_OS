#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>
#include "vga.h"    // VGA display functions
#include "string.h" // String utilities
// These headers are included but files don't exist yet
// #include "user.h"     // User management
// #include "fs.h"       // File system operations
// #include "terminal.h" // Terminal handling
// #include "editor.h"   // Text editor
// #include "utils.h"    // Utility functions
// #include "system.h"   // System information and control

// System state flags
#define SYSTEM_RUNNING 0
#define SYSTEM_HALTED 1
#define SYSTEM_REBOOT 2
#define SYSTEM_SHUTDOWN 3

// Terminal state - Use externals from vga.c instead of redefining
extern int terminal_row;
extern int terminal_column;
extern uint8_t terminal_color;
extern uint16_t *terminal_buffer;

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

// Simple printf-like function
void printf(const char *format, ...);

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

// The kernel main function, called from boot.asm
void kernel_main(void)
{
    // Initialize terminal
    terminal_initialize();

    // Display boot sequence
    show_boot_sequence();

    // Draw the OSIRIS logo
    draw_logo();

    // Welcome message
    terminal_row = 16;
    terminal_column = 0;

    uint8_t welcome_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    terminal_setcolor(welcome_color);

    print_centered("Welcome to OSIRIS OS!", terminal_row, welcome_color);
    terminal_row += 2;

    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_setcolor(text_color);

    print_centered("System loaded successfully.", terminal_row, text_color);
    terminal_row += 1;

    print_centered("Press any key to continue...", terminal_row, text_color);

    // Infinite loop - in a real OS, we would handle keyboard input here
    while (1)
    {
        // Halt the CPU until the next interrupt
        asm volatile("hlt");
    }
}