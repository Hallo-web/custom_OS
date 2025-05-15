#include "fs.h"
#include "string.h" // For string operations
#include <stdbool.h>

// Static file system storage
static file_t file_system[FS_MAX_FILES];
static int file_count = 0;
static char current_directory[FS_MAX_PATH] = "/";

// Initialize file system
void fs_init(void)
{
    // Clear all file entries
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        file_system[i].exists = false;
        file_system[i].size = 0;
        file_system[i].filename[0] = '\0';
        file_system[i].content[0] = '\0';
        file_system[i].owner[0] = '\0';
        file_system[i].created_date[0] = '\0';
        file_system[i].modified_date[0] = '\0';
        file_system[i].type = FS_TYPE_REGULAR;
        file_system[i].permissions = FS_PERM_READ | FS_PERM_WRITE;
    }

    file_count = 0;
    strcpy(current_directory, "/");
}

// Create a file
bool fs_create_file(const char *filename, const char *owner)
{
    // Check if we've reached the maximum number of files
    if (file_count >= FS_MAX_FILES)
    {
        return false;
    }

    // Check if file already exists
    if (fs_file_exists(filename))
    {
        return false;
    }

    // Find an empty slot
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (!file_system[i].exists)
        {
            strcpy(file_system[i].filename, filename);
            file_system[i].exists = true;
            file_system[i].size = 0;
            file_system[i].content[0] = '\0';
            strcpy(file_system[i].owner, owner);

            // Set current date (in real system, would use actual date)
            strcpy(file_system[i].created_date, "2025-05-15");
            strcpy(file_system[i].modified_date, "2025-05-15");

            file_system[i].type = FS_TYPE_REGULAR;
            file_system[i].permissions = FS_PERM_READ | FS_PERM_WRITE;

            file_count++;
            return true;
        }
    }

    return false;
}

// Delete a file
bool fs_delete_file(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            // Check if it's a system file
            if (file_system[i].type == FS_TYPE_SYSTEM)
            {
                // Can't delete system files
                return false;
            }

            file_system[i].exists = false;
            file_count--;
            return true;
        }
    }

    return false;
}

// Write to a file
bool fs_write_file(const char *filename, const char *content)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            // Check write permission
            if (!(file_system[i].permissions & FS_PERM_WRITE))
            {
                return false;
            }

            // Check content length
            int content_len = strlen(content);
            if (content_len >= FS_MAX_CONTENT)
            {
                return false;
            }

            strcpy(file_system[i].content, content);
            file_system[i].size = content_len;

            // Update modification date (in real system, would use actual date)
            strcpy(file_system[i].modified_date, "2025-05-15");

            return true;
        }
    }

    return false;
}

// Read from a file
const char *fs_read_file(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            // Check read permission
            if (!(file_system[i].permissions & FS_PERM_READ))
            {
                return "Permission denied";
            }

            return file_system[i].content;
        }
    }

    return NULL;
}

// Check if a file exists
bool fs_file_exists(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            return true;
        }
    }

    return false;
}

// Get file information
file_t *fs_get_file_info(const char *filename)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            return &file_system[i];
        }
    }

    return NULL;
}

// List all files
void fs_list_files(void)
{
    int count = 0;

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists)
        {
            // Skip hidden files unless in admin mode
            if (file_system[i].type == FS_TYPE_HIDDEN)
            {
                // In a real implementation, we'd check user permissions here
                continue;
            }

            count++;
        }
    }

    // If no files found
    if (count == 0)
    {
        printf("No files found in directory %s\n", current_directory);
        return;
    }

    printf("Files in directory %s:\n", current_directory);
    printf("%-20s %-6s %-12s %-12s %-5s\n", "Filename", "Size", "Created", "Modified", "Type");
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists)
        {
            // Skip hidden files unless in admin mode
            if (file_system[i].type == FS_TYPE_HIDDEN)
            {
                // In a real implementation, we'd check user permissions here
                continue;
            }

            char type_char = 'F'; // Regular file

            if (file_system[i].type == FS_TYPE_DIRECTORY)
                type_char = 'D';
            else if (file_system[i].type == FS_TYPE_SYSTEM)
                type_char = 'S';

            printf("%-20s %-6d %-12s %-12s %c\n",
                   file_system[i].filename,
                   file_system[i].size,
                   file_system[i].created_date,
                   file_system[i].modified_date,
                   type_char);
        }
    }
}

// Create initial system files
void fs_create_system_files(void)
{
    // Create root directory
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (!file_system[i].exists)
        {
            strcpy(file_system[i].filename, "/");
            file_system[i].exists = true;
            file_system[i].size = 0;
            strcpy(file_system[i].owner, "system");
            strcpy(file_system[i].created_date, "2025-05-15");
            strcpy(file_system[i].modified_date, "2025-05-15");
            file_system[i].type = FS_TYPE_DIRECTORY;
            file_system[i].permissions = FS_PERM_READ;
            file_count++;
            break;
        }
    }

    // Create system info file
    fs_create_file("system.cfg", "system");
    fs_write_file("system.cfg", "OS: OSIRIS\nVersion: 2.0\nBuild: 2025-05-15\n");

    // Set it as a system file
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, "system.cfg") == 0)
        {
            file_system[i].type = FS_TYPE_SYSTEM;
            file_system[i].permissions = FS_PERM_READ | FS_PERM_ADMIN;
            break;
        }
    }

    // Create a welcome file
    fs_create_file("welcome.txt", "system");
    fs_write_file("welcome.txt", "Welcome to OSIRIS Operating System!\n\nType 'help' to see available commands.\n");

    // Create a hidden file with the secret message
    fs_create_file(".secret", "system");
    fs_write_file(".secret", "The key to enlightenment is found in the year the temple was built: osiris1371");

    // Set it as a hidden file
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, ".secret") == 0)
        {
            file_system[i].type = FS_TYPE_HIDDEN;
            file_system[i].permissions = FS_PERM_READ | FS_PERM_ADMIN;
            break;
        }
    }
}

// Format file system
void fs_format(void)
{
    // Clear all file entries
    fs_init();

    // Then create system files
    fs_create_system_files();
}

// Get file count
int fs_get_file_count(void)
{
    return file_count;
}

// Create directory
bool fs_create_directory(const char *dirname)
{
    // Check if we've reached the maximum number of files
    if (file_count >= FS_MAX_FILES)
    {
        return false;
    }

    // Check if directory already exists
    if (fs_file_exists(dirname))
    {
        return false;
    }

    // Find an empty slot
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (!file_system[i].exists)
        {
            strcpy(file_system[i].filename, dirname);
            file_system[i].exists = true;
            file_system[i].size = 0;
            strcpy(file_system[i].owner, "system"); // Would use current user
            strcpy(file_system[i].created_date, "2025-05-15");
            strcpy(file_system[i].modified_date, "2025-05-15");
            file_system[i].type = FS_TYPE_DIRECTORY;
            file_system[i].permissions = FS_PERM_READ | FS_PERM_WRITE;
            file_count++;
            return true;
        }
    }

    return false;
}

// Set current directory
bool fs_set_directory(const char *dirname)
{
    // Check if directory exists
    bool found = false;
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists &&
            strcmp(file_system[i].filename, dirname) == 0 &&
            file_system[i].type == FS_TYPE_DIRECTORY)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        return false;
    }

    strcpy(current_directory, dirname);
    return true;
}

// Get current directory
const char *fs_get_current_directory(void)
{
    return current_directory;
}

// Check user permissions for a file
bool fs_check_permission(const char *filename, int permission)
{
    file_t *file = fs_get_file_info(filename);
    if (file == NULL)
    {
        return false;
    }

    return (file->permissions & permission) != 0;
}

// Set file permissions
bool fs_set_permission(const char *filename, int permission)
{
    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists && strcmp(file_system[i].filename, filename) == 0)
        {
            file_system[i].permissions = permission;
            return true;
        }
    }

    return false;
}

// Get file size
int fs_get_file_size(const char *filename)
{
    file_t *file = fs_get_file_info(filename);
    if (file == NULL)
    {
        return -1;
    }

    return file->size;
}

// Display file browser
void display_file_browser(void)
{
    // Clear screen for the browser
    // In a real system we would use terminal functions

    printf("===== OSIRIS File Browser =====\n");
    printf("Current Directory: %s\n\n", current_directory);

    // Display files with pagination
    fs_list_files();

    printf("\nCommands: [O]pen, [E]dit, [D]elete, [C]reate, [B]ack, [Q]uit\n");

    // In a real implementation, we would handle keyboard input here
}

// Search for files
void fs_search(const char *query)
{
    bool found = false;

    printf("Search results for \"%s\":\n", query);
    printf("%-20s %-6s %-12s %-5s\n", "Filename", "Size", "Modified", "Type");
    printf("---------------------------------------------\n");

    for (int i = 0; i < FS_MAX_FILES; i++)
    {
        if (file_system[i].exists)
        {
            // Skip hidden files unless in admin mode
            if (file_system[i].type == FS_TYPE_HIDDEN)
            {
                // In a real implementation, we'd check user permissions here
                continue;
            }

            // Search in filename
            if (strstr(file_system[i].filename, query) != NULL)
            {
                char type_char = 'F';
                if (file_system[i].type == FS_TYPE_DIRECTORY)
                    type_char = 'D';
                else if (file_system[i].type == FS_TYPE_SYSTEM)
                    type_char = 'S';

                printf("%-20s %-6d %-12s %c\n",
                       file_system[i].filename,
                       file_system[i].size,
                       file_system[i].modified_date,
                       type_char);

                found = true;
            }

            // Also search in file content (only for text files)
            else if (file_system[i].type == FS_TYPE_REGULAR && strstr(file_system[i].content, query) != NULL)
            {
                printf("%-20s %-6d %-12s F (content match)\n",
                       file_system[i].filename,
                       file_system[i].size,
                       file_system[i].modified_date);

                found = true;
            }
        }
    }

    if (!found)
    {
        printf("No files found matching \"%s\"\n", query);
    }
}