#include "editor.h"
#include "vga.h"
#include "terminal.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Variables to track editor state
static char editor_content[2048];
static int cursor_pos = 0;
static int editor_length = 0;
static char current_filename[64] = "";
static bool editor_modified = false;
static int editor_start_row = 2;
static int editor_visible_rows = 20;
static int editor_scroll_offset = 0;
static int editor_mode = 0; // 0 = normal, 1 = insert

// Function to initialize the editor
void editor_init(void)
{
    editor_content[0] = '\0';
    cursor_pos = 0;
    editor_length = 0;
    current_filename[0] = '\0';
    editor_modified = false;
    editor_scroll_offset = 0;
    editor_mode = 0;
}

// Function to load file content into editor
bool editor_load_file(const char *filename)
{
    // Check if file exists
    if (!fs_file_exists(filename))
    {
        return false;
    }

    // Read file content
    const char *content = fs_read_file(filename);
    if (!content)
    {
        return false;
    }

    // Copy content to editor buffer
    int i = 0;
    while (content[i] != '\0' && i < 2047)
    {
        editor_content[i] = content[i];
        i++;
    }
    editor_content[i] = '\0';
    editor_length = i;
    cursor_pos = 0;

    // Update filename
    int j = 0;
    while (filename[j] != '\0' && j < 63)
    {
        current_filename[j] = filename[j];
        j++;
    }
    current_filename[j] = '\0';

    editor_modified = false;
    editor_scroll_offset = 0;

    return true;
}

// Function to save editor content to file
bool editor_save_file(void)
{
    if (current_filename[0] == '\0')
    {
        return false;
    }

    // Create file if it doesn't exist
    if (!fs_file_exists(current_filename))
    {
        if (!fs_create_file(current_filename, get_current_username()))
        {
            return false;
        }
    }

    // Write content to file
    if (!fs_write_file(current_filename, editor_content))
    {
        return false;
    }

    editor_modified = false;
    return true;
}

// Function to save editor content to a new file
bool editor_save_as(const char *filename)
{
    // Update filename
    int j = 0;
    while (filename[j] != '\0' && j < 63)
    {
        current_filename[j] = filename[j];
        j++;
    }
    current_filename[j] = '\0';

    return editor_save_file();
}

// Function to insert character at cursor position
void editor_insert_char(char c)
{
    if (editor_length >= 2047)
    {
        return; // Buffer full
    }

    // Make room for the new character
    for (int i = editor_length; i > cursor_pos; i--)
    {
        editor_content[i] = editor_content[i - 1];
    }

    // Insert character
    editor_content[cursor_pos] = c;
    cursor_pos++;
    editor_length++;
    editor_content[editor_length] = '\0';
    editor_modified = true;
}

// Function to delete character before cursor
void editor_delete_char(void)
{
    if (cursor_pos <= 0)
    {
        return; // At beginning of buffer
    }

    // Shift characters
    for (int i = cursor_pos - 1; i < editor_length - 1; i++)
    {
        editor_content[i] = editor_content[i + 1];
    }

    cursor_pos--;
    editor_length--;
    editor_content[editor_length] = '\0';
    editor_modified = true;
}

// Function to move cursor left
void editor_move_left(void)
{
    if (cursor_pos > 0)
    {
        cursor_pos--;
    }
}

// Function to move cursor right
void editor_move_right(void)
{
    if (cursor_pos < editor_length)
    {
        cursor_pos++;
    }
}

// Function to move cursor up
void editor_move_up(void)
{
    // Find current line start
    int line_start = cursor_pos;
    while (line_start > 0 && editor_content[line_start - 1] != '\n')
    {
        line_start--;
    }

    // Find column position within current line
    int column = cursor_pos - line_start;

    // Find previous line start
    int prev_line_start = line_start - 1;
    while (prev_line_start > 0 && editor_content[prev_line_start - 1] != '\n')
    {
        prev_line_start--;
    }

    // If we're already at the first line
    if (prev_line_start < 0)
    {
        cursor_pos = 0;
        return;
    }

    // Find previous line length
    int prev_line_length = line_start - prev_line_start - 1;

    // Set new cursor position
    cursor_pos = prev_line_start + (column < prev_line_length ? column : prev_line_length);
}

// Function to move cursor down
void editor_move_down(void)
{
    // Find current line start
    int line_start = cursor_pos;
    while (line_start > 0 && editor_content[line_start - 1] != '\n')
    {
        line_start--;
    }

    // Find column position within current line
    int column = cursor_pos - line_start;

    // Find next line start
    int next_line_start = cursor_pos;
    while (next_line_start < editor_length && editor_content[next_line_start] != '\n')
    {
        next_line_start++;
    }

    // Skip the newline character
    if (next_line_start < editor_length)
    {
        next_line_start++;
    }
    else
    {
        // We're at the last line already
        cursor_pos = editor_length;
        return;
    }

    // Find next line end
    int next_line_end = next_line_start;
    while (next_line_end < editor_length && editor_content[next_line_end] != '\n')
    {
        next_line_end++;
    }

    // Set new cursor position
    int next_line_length = next_line_end - next_line_start;
    cursor_pos = next_line_start + (column < next_line_length ? column : next_line_length);
}

// Function to handle editor input
void editor_handle_input(char c)
{
    switch (c)
    {
    case 27:             // ESC
        editor_mode = 0; // Switch to normal mode
        break;
    case 'i':
        if (editor_mode == 0)
        {
            editor_mode = 1; // Switch to insert mode
        }
        else
        {
            editor_insert_char(c);
        }
        break;
    case '\b': // Backspace
        if (editor_mode == 1)
        {
            editor_delete_char();
        }
        break;
    default:
        if (editor_mode == 1)
        {
            editor_insert_char(c);
        }
        else
        {
            // Handle normal mode commands
            switch (c)
            {
            case 'h':
                editor_move_left();
                break;
            case 'l':
                editor_move_right();
                break;
            case 'j':
                editor_move_down();
                break;
            case 'k':
                editor_move_up();
                break;
            }
        }
        break;
    }
}

// Function to display editor content
void editor_display(void)
{
    clear_screen();

    // Calculate visible range
    int start_line = 0;
    int line_count = 0;
    int i = 0;

    // Count lines to find start position with scroll offset
    while (i < editor_length)
    {
        if (editor_content[i] == '\n')
        {
            line_count++;
            if (line_count == editor_scroll_offset)
            {
                start_line = i + 1;
            }
        }
        i++;
    }

    // Print header
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY));
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putentryat(' ', terminal_color, i, 0);
    }

    char header[VGA_WIDTH];
    if (current_filename[0] != '\0')
    {
        strcpy(header, " File: ");
        strcat(header, current_filename);
    }
    else
    {
        strcpy(header, " [New File]");
    }

    if (editor_modified)
    {
        strcat(header, " [modified]");
    }

    terminal_row = 0;
    terminal_column = 0;
    terminal_writestring(header);

    char mode_str[20];
    strcpy(mode_str, editor_mode == 0 ? "NORMAL" : "INSERT");

    terminal_column = VGA_WIDTH - strlen(mode_str) - 2;
    terminal_writestring(mode_str);

    // Reset color for content
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    // Display content with line numbers
    int display_row = editor_start_row;
    int line_num = editor_scroll_offset + 1;
    terminal_row = display_row;

    // Display line numbers and line content
    i = start_line;
    while (i < editor_length && display_row < editor_start_row + editor_visible_rows)
    {
        // Display line number
        terminal_column = 0;
        char num_buf[8];
        itoa(line_num, num_buf, 10);
        terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
        for (int j = 0; j < 4; j++)
        {
            if (j < strlen(num_buf))
            {
                terminal_putentryat(num_buf[j], terminal_color, j, display_row);
            }
            else
            {
                terminal_putentryat(' ', terminal_color, j, display_row);
            }
        }
        terminal_putentryat('|', terminal_color, 4, display_row);

        // Display line content
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_column = 5;
        while (i < editor_length && editor_content[i] != '\n')
        {
            if (terminal_column < VGA_WIDTH)
            {
                terminal_putentryat(editor_content[i], terminal_color, terminal_column, display_row);
                terminal_column++;
            }
            i++;
        }

        // Move to next line
        if (i < editor_length && editor_content[i] == '\n')
        {
            i++; // Skip the newline character
        }
        display_row++;
        line_num++;
    }

    // Display status line
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY));
    for (int i = 0; i < VGA_WIDTH; i++)
    {
        terminal_putentryat(' ', terminal_color, i, terminal_row);
    }

    char status[VGA_WIDTH];
    strcpy(status, " Ln ");

    // Count current line
    int cursor_line = 1;
    for (i = 0; i < cursor_pos; i++)
    {
        if (editor_content[i] == '\n')
        {
            cursor_line++;
        }
    }

    char line_buf[10];
    itoa(cursor_line, line_buf, 10);
    strcat(status, line_buf);

    strcat(status, ", Col ");

    // Count current column
    int cursor_col = 0;
    i = cursor_pos;
    while (i > 0 && editor_content[i - 1] != '\n')
    {
        cursor_col++;
        i--;
    }

    char col_buf[10];
    itoa(cursor_col + 1, col_buf, 10);
    strcat(status, col_buf);

    // Display help hint
    strcat(status, " | Press ESC for normal mode, i for insert mode");

    terminal_column = 0;
    terminal_writestring(status);

    // Position the cursor
    int cursor_display_row = editor_start_row;
    int cursor_display_col = 5 + cursor_col;

    // Find cursor position relative to view
    i = 0;
    int line = 0;
    while (i < cursor_pos)
    {
        if (editor_content[i] == '\n')
        {
            line++;
        }
        i++;
    }

    // Adjust cursor display position based on scroll offset
    cursor_display_row = editor_start_row + (line - editor_scroll_offset);

    // Ensure cursor is visible by scrolling if needed
    if (cursor_display_row < editor_start_row)
    {
        editor_scroll_offset = line;
    }
    else if (cursor_display_row >= editor_start_row + editor_visible_rows)
    {
        editor_scroll_offset = line - editor_visible_rows + 1;
    }

    // Set cursor position
    if (cursor_display_row >= editor_start_row &&
        cursor_display_row < editor_start_row + editor_visible_rows)
    {
        terminal_row = cursor_display_row;
        terminal_column = cursor_display_col;
    }
}

// Run the editor with a given filename
void run_editor(const char *filename)
{
    editor_init();

    if (filename != NULL && filename[0] != '\0')
    {
        editor_load_file(filename);
    }

    bool running = true;
    editor_display();

    while (running)
    {
        char c = get_keyboard_input();

        // Special key handling for editor commands
        if (c == 0)
        {
            // No character input
            continue;
        }

        // Check for Ctrl+S (save)
        if (c == 19)
        { // Ctrl+S
            editor_save_file();
            editor_display();
            continue;
        }

        // Check for Ctrl+Q (quit)
        if (c == 17)
        { // Ctrl+Q
            if (!editor_modified || confirm_action("Quit without saving?"))
            {
                running = false;
            }
            editor_display();
            continue;
        }

        editor_handle_input(c);
        editor_display();
    }

    clear_screen();
}

// Implementation of other editor functions
void editor_new_file(void)
{
    editor_init();
    run_editor(NULL);
}

void editor_open_file(const char *filename)
{
    run_editor(filename);
}

bool editor_is_modified(void)
{
    return editor_modified;
}

const char *editor_get_filename(void)
{
    return current_filename;
}

void editor_set_scroll(int offset)
{
    if (offset >= 0)
    {
        editor_scroll_offset = offset;
    }
}

// Function to search text in editor
int editor_search(const char *query)
{
    if (!query || query[0] == '\0')
    {
        return -1;
    }

    // Simple search implementation
    int query_len = strlen(query);
    int match_count = 0;

    for (int i = 0; i <= editor_length - query_len; i++)
    {
        bool match = true;
        for (int j = 0; j < query_len; j++)
        {
            if (editor_content[i + j] != query[j])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            match_count++;

            // Set cursor to the first match
            if (match_count == 1)
            {
                cursor_pos = i;

                // Adjust scroll to make cursor visible
                int line = 0;
                for (int k = 0; k < cursor_pos; k++)
                {
                    if (editor_content[k] == '\n')
                    {
                        line++;
                    }
                }

                // Set scroll position to show the match
                editor_scroll_offset = line > 5 ? line - 5 : 0;
            }
        }
    }

    return match_count;
}