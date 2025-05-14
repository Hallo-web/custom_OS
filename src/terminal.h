#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>

// Terminal flags
#define TERM_INPUT_NORMAL 0
#define TERM_INPUT_PASSWORD 1
#define TERM_INPUT_HIDDEN 2

// Command history size
#define COMMAND_HISTORY_SIZE 20

// Commands
void init_terminal_interface(void);
void handle_keyboard(void);
void process_command(void);
char get_keyboard_input(void);
void display_cursor(bool visible);
void show_boot_sequence(void);
void run_terminal(void);
void execute_command(const char *command);
void save_command_history(const char *command);
const char *get_previous_command(void);
const char *get_next_command(void);
void display_command_prompt(void);
void add_command_history(const char *command);

// Input handling
int read_line(char *buffer, int max_length, int input_type);
char *get_input(const char *prompt, int input_type);
bool confirm_action(const char *prompt);
void press_any_key(const char *message);

// Terminal display
void clear_screen(void);
void display_help(void);
void display_welcome_message(void);
void display_login_prompt(void);
void display_system_info(void);
void show_calendar(void);
void show_clock(void);
void show_ascii_table(void);
void run_calculator(void);
void simulate_reboot(void);
void perform_shutdown(void);
void display_about(void);
void display_manual(const char *command);
void display_disk_usage(void);
void run_screensaver(void);
void set_terminal_title(const char *title);

#endif // TERMINAL_H