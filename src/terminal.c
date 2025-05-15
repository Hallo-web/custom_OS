#include "terminal.h"
#include "vga.h"
#include "string.h"
#include "system.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Global variables
extern int terminal_row;
extern int terminal_column;
extern uint8_t terminal_color;
extern uint16_t *terminal_buffer;
extern char command_buffer[256];
extern int command_length;
extern char command_history[COMMAND_HISTORY_SIZE][256];
extern int history_count;
extern int history_position;
extern bool shift_pressed;
extern bool caps_lock;
extern bool ctrl_pressed;
extern int system_state;

static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

// I/O functions
extern uint8_t inb(uint16_t port);
extern void outb(uint16_t port, uint8_t val);

// Terminal functions
extern void terminal_initialize(void);
extern void terminal_setcolor(uint8_t color);
extern void terminal_putchar(char c);
extern void terminal_writestring(const char *data);
extern void terminal_writestring_colored(const char *data, uint8_t color);
extern void terminal_clear_region(int x1, int y1, int x2, int y2);
extern void delay(uint32_t milliseconds);
extern void print_centered(const char *str, int row, uint8_t color);
extern void draw_box(int x1, int y1, int x2, int y2, uint8_t color);
extern void display_progress_bar(int progress, int total, int width);
extern void draw_logo(void);

// Keyboard definitions
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEY_SHIFT 42
#define KEY_SHIFT_R 54
#define KEY_CTRL 29
#define KEY_ALT 56
#define KEY_CAPS_LOCK 58
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

// External keyboard maps
extern const char keyboard_map[128];
extern const char keyboard_map_shifted[128];

// Command processing
void execute_command(const char *command)
{
    // Basic command parser
    if (strcmp(command, "") == 0)
    {
        return; // Empty command
    }
    else if (strcmp(command, "help") == 0)
    {
        display_help();
    }
    else if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0)
    {
        clear_screen();
    }
    else if (strcmp(command, "about") == 0)
    {
        display_about();
    }
    else if (strcmp(command, "info") == 0 || strcmp(command, "sysinfo") == 0)
    {
        display_system_info();
    }
    else if (strcmp(command, "reboot") == 0)
    {
        if (confirm_action("Are you sure you want to reboot the system? (y/n): "))
        {
            simulate_reboot();
        }
    }
    else if (strcmp(command, "shutdown") == 0 || strcmp(command, "halt") == 0)
    {
        if (confirm_action("Are you sure you want to shut down the system? (y/n): "))
        {
            perform_shutdown();
        }
    }
    else if (strcmp(command, "calendar") == 0)
    {
        show_calendar();
    }
    else if (strcmp(command, "time") == 0 || strcmp(command, "clock") == 0)
    {
        show_clock();
    }
    else if (strcmp(command, "ascii") == 0)
    {
        show_ascii_table();
    }
    else if (strcmp(command, "calc") == 0)
    {
        run_calculator();
    }
    else if (strncmp(command, "echo ", 5) == 0)
    {
        terminal_writestring(command + 5);
        terminal_putchar('\n');
    }
    else if (strncmp(command, "manual ", 7) == 0 || strncmp(command, "man ", 4) == 0)
    {
        const char *cmd = strncmp(command, "manual ", 7) == 0 ? command + 7 : command + 4;
        display_manual(cmd);
    }
    else if (strcmp(command, "disk") == 0)
    {
        display_disk_usage();
    }
    else if (strcmp(command, "screensaver") == 0)
    {
        run_screensaver();
    }
    else if (strncmp(command, "title ", 6) == 0)
    {
        set_terminal_title(command + 6);
    }
    else if (strcmp(command, "secret") == 0)
    {
        extern const char *SECRET_MESSAGE;
        terminal_writestring_colored("Secret message: ", vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring(SECRET_MESSAGE);
        terminal_putchar('\n');
    }
    else
    {
        terminal_writestring_colored("Unknown command: ", vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring(command);
        terminal_putchar('\n');
        terminal_writestring("Type 'help' for a list of commands.\n");
    }
}

void add_command_history(const char *command)
{
    // Don't add empty commands or duplicates of the last command
    if (strlen(command) == 0 || (history_count > 0 && strcmp(command_history[history_count - 1], command) == 0))
    {
        return;
    }

    // Add command to history
    if (history_count < COMMAND_HISTORY_SIZE)
    {
        strcpy(command_history[history_count], command);
        history_count++;
    }
    else
    {
        // Shift history and add new command at the end
        for (int i = 0; i < COMMAND_HISTORY_SIZE - 1; i++)
        {
            strcpy(command_history[i], command_history[i + 1]);
        }
        strcpy(command_history[COMMAND_HISTORY_SIZE - 1], command);
    }

    // Reset history position
    history_position = history_count;
}

const char *get_previous_command(void)
{
    if (history_count == 0 || history_position <= 0)
    {
        return NULL;
    }

    history_position--;
    return command_history[history_position];
}

const char *get_next_command(void)
{
    if (history_count == 0 || history_position >= history_count)
    {
        return NULL;
    }

    history_position++;
    if (history_position == history_count)
    {
        return ""; // Return empty string at the end of history
    }
    return command_history[history_position];
}

void process_command(void)
{
    // Null-terminate the command
    command_buffer[command_length] = '\0';

    // Add to history if not empty
    if (command_length > 0)
    {
        add_command_history(command_buffer);
    }

    // Execute the command
    execute_command(command_buffer);

    // Reset command buffer
    command_length = 0;

    // Display new prompt
    display_command_prompt();
}

void display_command_prompt(void)
{
    terminal_writestring_colored("OSIRIS> ", vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
}

void handle_keyboard(void)
{
    char c = get_keyboard_input();

    if (c == 0)
    {
        return; // No character to process
    }

    // Handle special keys
    if (c == '\n' || c == '\r')
    {
        terminal_putchar('\n');
        process_command();
    }
    else if (c == '\b')
    {
        if (command_length > 0)
        {
            command_length--;
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
        }
    }
    else if (c == 27)
    { // ESC key
        // Clear current command
        while (command_length > 0)
        {
            terminal_putchar('\b');
            terminal_putchar(' ');
            terminal_putchar('\b');
            command_length--;
        }
    }
    else if (c >= ' ' && c <= '~')
    { // Printable ASCII
        if (command_length < 255)
        { // Leave room for null terminator
            command_buffer[command_length++] = c;
            terminal_putchar(c);
        }
    }
}

void init_terminal_interface(void)
{
    // Initialize terminal
    terminal_initialize();

    // Display logo and welcome message
    draw_logo();

    // Set initial cursor position
    terminal_row = 16;
    terminal_column = 0;

    // Display welcome message
    display_welcome_message();

    // Show command prompt
    display_command_prompt();

    // Reset command buffer and history
    command_length = 0;
    history_count = 0;
    history_position = 0;
}

void run_terminal(void)
{
    // Initialize terminal interface
    init_terminal_interface();

    // Main terminal loop
    while (system_state == SYSTEM_RUNNING)
    {
        // Process keyboard input
        handle_keyboard();

        // Small delay to avoid consuming too much CPU
        delay(10);
    }
}

void clear_screen(void)
{
    terminal_initialize();
    display_command_prompt();
}

void display_help(void)
{
    uint8_t header_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t cmd_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint8_t desc_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    terminal_writestring_colored("Available commands:\n", header_color);

    terminal_writestring_colored("  help        ", cmd_color);
    terminal_writestring_colored("- Display this help information\n", desc_color);

    terminal_writestring_colored("  clear, cls  ", cmd_color);
    terminal_writestring_colored("- Clear the screen\n", desc_color);

    terminal_writestring_colored("  about       ", cmd_color);
    terminal_writestring_colored("- Display information about OSIRIS OS\n", desc_color);

    terminal_writestring_colored("  info        ", cmd_color);
    terminal_writestring_colored("- Display system information\n", desc_color);

    terminal_writestring_colored("  reboot      ", cmd_color);
    terminal_writestring_colored("- Reboot the system\n", desc_color);

    terminal_writestring_colored("  shutdown    ", cmd_color);
    terminal_writestring_colored("- Shut down the system\n", desc_color);

    terminal_writestring_colored("  calendar    ", cmd_color);
    terminal_writestring_colored("- Display a calendar\n", desc_color);

    terminal_writestring_colored("  time, clock ", cmd_color);
    terminal_writestring_colored("- Display the current time\n", desc_color);

    terminal_writestring_colored("  ascii       ", cmd_color);
    terminal_writestring_colored("- Display ASCII table\n", desc_color);

    terminal_writestring_colored("  calc        ", cmd_color);
    terminal_writestring_colored("- Run a simple calculator\n", desc_color);

    terminal_writestring_colored("  echo [text] ", cmd_color);
    terminal_writestring_colored("- Display text\n", desc_color);

    terminal_writestring_colored("  manual [cmd]", cmd_color);
    terminal_writestring_colored("- Display manual for a command\n", desc_color);

    terminal_writestring_colored("  disk        ", cmd_color);
    terminal_writestring_colored("- Display disk usage\n", desc_color);

    terminal_writestring_colored("  screensaver ", cmd_color);
    terminal_writestring_colored("- Run a simple screensaver\n", desc_color);

    terminal_writestring_colored("  title [text]", cmd_color);
    terminal_writestring_colored("- Set terminal title\n", desc_color);
}

void display_welcome_message(void)
{
    terminal_writestring_colored("\nWelcome to O.S.I.R.I.S - Operating System Interface v2.0\n",
                                 vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Type 'help' for a list of available commands.\n\n");
}

void display_about(void)
{
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    terminal_writestring_colored("About OSIRIS OS\n", title_color);
    terminal_writestring_colored("---------------\n", title_color);
    terminal_writestring_colored("OSIRIS (Operating System Interface Research Integration & Security)\n", text_color);
    terminal_writestring_colored("Version: 2.0\n", text_color);
    terminal_writestring_colored("Build Date: May 15, 2025\n", text_color);
    terminal_writestring_colored("\nOSIRIS is a lightweight, terminal-based operating system designed\n", text_color);
    terminal_writestring_colored("for research, education, and specialized applications. It provides\n", text_color);
    terminal_writestring_colored("a simple but powerful command interface for system operations.\n", text_color);
    terminal_writestring_colored("\nFeatures:\n", text_color);
    terminal_writestring_colored("- Minimal resource footprint\n", text_color);
    terminal_writestring_colored("- Text-mode interface\n", text_color);
    terminal_writestring_colored("- Basic file system operations\n", text_color);
    terminal_writestring_colored("- System monitoring tools\n", text_color);
    terminal_writestring_colored("- Integrated text editor\n", text_color);
}

int read_line(char *buffer, int max_length, int input_type)
{
    int length = 0;
    char c;

    while (1)
    {
        c = get_keyboard_input();

        if (c == 0)
        {
            // No input, try again
            delay(10);
            continue;
        }

        if (c == '\n' || c == '\r')
        {
            terminal_putchar('\n');
            buffer[length] = '\0';
            return length;
        }
        else if (c == '\b')
        {
            if (length > 0)
            {
                length--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        }
        else if (c == 27)
        { // ESC
            // Clear current line
            while (length > 0)
            {
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
                length--;
            }
        }
        else if (c >= ' ' && c <= '~' && length < max_length - 1)
        {
            buffer[length++] = c;

            if (input_type == TERM_INPUT_NORMAL)
            {
                terminal_putchar(c);
            }
            else if (input_type == TERM_INPUT_PASSWORD)
            {
                terminal_putchar('*');
            }
            // For TERM_INPUT_HIDDEN, we don't display anything
        }
    }
}

char *get_input(const char *prompt, int input_type)
{
    static char buffer[256];

    terminal_writestring(prompt);
    read_line(buffer, sizeof(buffer), input_type);

    return buffer;
}

bool confirm_action(const char *prompt)
{
    char *input = get_input(prompt, TERM_INPUT_NORMAL);
    return (input[0] == 'y' || input[0] == 'Y');
}

void press_any_key(const char *message)
{
    if (message)
    {
        terminal_writestring(message);
    }

    // Wait for any key press
    while (1)
    {
        char c = get_keyboard_input();
        if (c != 0)
        {
            break;
        }
        delay(10);
    }
}

void display_system_info(void)
{
    system_info_t info = get_system_info();
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t label_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint8_t value_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);

    terminal_writestring_colored("System Information\n", title_color);
    terminal_writestring_colored("------------------\n", title_color);

    terminal_writestring_colored("OS Name: ", label_color);
    terminal_writestring_colored(info.os_name, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Version: ", label_color);
    terminal_writestring_colored(info.os_version, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Build Date: ", label_color);
    terminal_writestring_colored(info.build_date, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Kernel Version: ", label_color);
    terminal_writestring_colored(info.kernel_version, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Uptime: ", label_color);
    char uptime_str[32];
    int hours = info.uptime_seconds / 3600;
    int minutes = (info.uptime_seconds % 3600) / 60;
    int seconds = info.uptime_seconds % 60;
    sprintf("%02d:%02d:%02d", hours, minutes, seconds);
    terminal_writestring_colored(uptime_str, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Memory Total: ", label_color);
    char mem_total[16];
    itoa(info.memory_total / 1024, mem_total, 10);
    terminal_writestring_colored(mem_total, value_color);
    terminal_writestring_colored(" KB", value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Memory Used: ", label_color);
    char mem_used[16];
    itoa(info.memory_used / 1024, mem_used, 10);
    terminal_writestring_colored(mem_used, value_color);
    terminal_writestring_colored(" KB", value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Current User: ", label_color);
    terminal_writestring_colored(info.current_user, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Active Processes: ", label_color);
    char proc_count[8];
    itoa(info.num_processes, proc_count, 10);
    terminal_writestring_colored(proc_count, value_color);
    terminal_putchar('\n');

    terminal_writestring_colored("Files: ", label_color);
    char file_count[8];
    itoa(info.num_files, file_count, 10);
    terminal_writestring_colored(file_count, value_color);
    terminal_putchar('\n');
}

void simulate_reboot(void)
{
    // Clear screen
    terminal_initialize();

    terminal_writestring_colored("\n\n SYSTEM REBOOT\n\n", vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    terminal_writestring("Stopping processes...\n");
    delay(500);

    terminal_writestring("Saving system state...\n");
    delay(300);

    terminal_writestring("Preparing to reboot...\n");
    delay(500);

    terminal_writestring("Rebooting...\n\n");
    delay(1000);

    // Simulate reboot by showing boot sequence again
    show_boot_sequence();

    // Re-initialize terminal interface
    init_terminal_interface();
}

void perform_shutdown(void)
{
    // Set system state to shutdown
    set_system_state(SYSTEM_SHUTDOWN);

    // Clear screen
    terminal_initialize();

    terminal_writestring_colored("\n\n SYSTEM SHUTDOWN\n\n", vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE));
    terminal_writestring("Stopping all processes...\n");
    delay(500);

    terminal_writestring("Saving user data...\n");
    delay(300);

    terminal_writestring("Unmounting filesystems...\n");
    delay(200);

    terminal_writestring("Syncing disks...\n");
    delay(300);

    terminal_writestring("Powering off...\n\n");
    delay(500);

    // Display final message
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_clear_region(0, 0, VGA_WIDTH - 1, VGA_HEIGHT - 1);
    print_centered("It is now safe to turn off your computer.", 12, vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    // System halted - in a real OS this would trigger actual shutdown
    while (1)
    {
        asm volatile("hlt");
    }
}

void show_calendar(void)
{
    // Simple calendar display (simulated for May 2025)
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t header_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    uint8_t day_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    uint8_t current_color = vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_CYAN);

    terminal_writestring_colored("      May 2025      \n", title_color);
    terminal_writestring_colored(" Su Mo Tu We Th Fr Sa\n", header_color);

    // First day of May 2025 is Thursday (offset 4)
    terminal_writestring_colored("             1  2  3\n", day_color);
    terminal_writestring_colored("  4  5  6  7  8  9 10\n", day_color);
    terminal_writestring_colored(" 11 12 13 14 ", day_color);

    // Highlight current day (15)
    terminal_writestring_colored("15", current_color);

    terminal_writestring_colored(" 16 17\n", day_color);
    terminal_writestring_colored(" 18 19 20 21 22 23 24\n", day_color);
    terminal_writestring_colored(" 25 26 27 28 29 30 31\n", day_color);
}

void show_clock(void)
{
    // Simulated clock - in a real OS this would get the actual time
    // For our demo we'll just show a fixed time
    uint8_t clock_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    terminal_writestring_colored("Current Time: 10:45:22\n", clock_color);
    terminal_writestring_colored("Date: May 15, 2025\n", clock_color);
}

void show_ascii_table(void)
{
    // Display an ASCII table
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    terminal_writestring_colored("ASCII Table (32-127)\n", title_color);
    terminal_writestring_colored("------------------\n", title_color);

    for (int row = 0; row < 12; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            int ascii = 32 + row * 8 + col;
            if (ascii <= 127)
            {
                char buffer[16];
                itoa(ascii, buffer, 10);
                terminal_writestring(buffer);
                terminal_writestring(": ");
                if (ascii == 32)
                {
                    terminal_writestring("SP");
                }
                else
                {
                    terminal_putchar((char)ascii);
                }
                terminal_writestring("  ");
            }
        }
        terminal_putchar('\n');
    }
}

void run_calculator(void)
{
    terminal_writestring_colored("Simple Calculator\n", vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Enter expression (e.g., 5+3, 10-2, 4*3, 8/2): ");

    char buffer[64];
    read_line(buffer, sizeof(buffer), TERM_INPUT_NORMAL);

    // Simple calculator that handles basic operations
    int result = 0;
    int num1 = 0, num2 = 0;
    char op = 0;
    int pos = 0;

    // Parse first number
    while (buffer[pos] >= '0' && buffer[pos] <= '9')
    {
        num1 = num1 * 10 + (buffer[pos] - '0');
        pos++;
    }

    // Parse operator
    if (buffer[pos] == '+' || buffer[pos] == '-' || buffer[pos] == '*' || buffer[pos] == '/')
    {
        op = buffer[pos];
        pos++;
    }
    else
    {
        terminal_writestring_colored("Invalid operator\n", vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        return;
    }

    // Parse second number
    while (buffer[pos] >= '0' && buffer[pos] <= '9')
    {
        num2 = num2 * 10 + (buffer[pos] - '0');
        pos++;
    }

    // Calculate result
    switch (op)
    {
    case '+':
        result = num1 + num2;
        break;
    case '-':
        result = num1 - num2;
        break;
    case '*':
        result = num1 * num2;
        break;
    case '/':
        if (num2 == 0)
        {
            terminal_writestring_colored("Error: Division by zero\n", vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            return;
        }
        result = num1 / num2;
        break;
    }

    // Display result
    terminal_writestring("Result: ");
    char result_str[16];
    itoa(result, result_str, 10);
    terminal_writestring_colored(result_str, vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_putchar('\n');
}

void display_manual(const char *command)
{
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

    if (strcmp(command, "help") == 0)
    {
        terminal_writestring_colored("MANUAL: help\n", title_color);
        terminal_writestring_colored("-------------\n", title_color);
        terminal_writestring_colored("Displays a list of available commands with brief descriptions.\n", text_color);
        terminal_writestring_colored("Usage: help\n", text_color);
    }
    else if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0)
    {
        terminal_writestring_colored("MANUAL: clear/cls\n", title_color);
        terminal_writestring_colored("-----------------\n", title_color);
        terminal_writestring_colored("Clears the terminal screen and resets cursor position.\n", text_color);
        terminal_writestring_colored("Usage: clear\n", text_color);
        terminal_writestring_colored("   or: cls\n", text_color);
    }
    else if (strcmp(command, "about") == 0)
    {
        terminal_writestring_colored("MANUAL: about\n", title_color);
        terminal_writestring_colored("-------------\n", title_color);
        terminal_writestring_colored("Displays information about the operating system.\n", text_color);
        terminal_writestring_colored("Usage: about\n", text_color);
    }
    else if (strcmp(command, "info") == 0 || strcmp(command, "sysinfo") == 0)
    {
        terminal_writestring_colored("MANUAL: info/sysinfo\n", title_color);
        terminal_writestring_colored("-------------------\n", title_color);
        terminal_writestring_colored("Displays detailed system information including memory usage,\n", text_color);
        terminal_writestring_colored("uptime, and other system statistics.\n", text_color);
        terminal_writestring_colored("Usage: info\n", text_color);
        terminal_writestring_colored("   or: sysinfo\n", text_color);
    }
    else
    {
        terminal_writestring_colored("No manual entry for '", text_color);
        terminal_writestring(command);
        terminal_writestring_colored("'\n", text_color);
    }
}

void display_disk_usage(void)
{
    // Simulated disk usage
    uint8_t title_color = vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    uint8_t text_color = vga_entry_