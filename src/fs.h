#ifndef FS_H
#define FS_H

#include <stdbool.h>

// File system limits
#define FS_MAX_FILES 32
#define FS_MAX_FILENAME 32
#define FS_MAX_CONTENT 2048
#define FS_MAX_PATH 64

// File permissions
#define FS_PERM_READ 0x01
#define FS_PERM_WRITE 0x02
#define FS_PERM_EXEC 0x04
#define FS_PERM_ADMIN 0x08 // Only admins can access

// File types
#define FS_TYPE_REGULAR 0
#define FS_TYPE_DIRECTORY 1
#define FS_TYPE_SYSTEM 2
#define FS_TYPE_HIDDEN 3

// File structure
typedef struct
{
    char filename[FS_MAX_FILENAME];
    char content[FS_MAX_CONTENT];
    int size;
    bool exists;
    char owner[32];
    char created_date[16];
    char modified_date[16];
    int type;
    int permissions;
} file_t;

// Initialize file system
void fs_init(void);

// Create a file
bool fs_create_file(const char *filename, const char *owner);

// Delete a file
bool fs_delete_file(const char *filename);

// Write to a file
bool fs_write_file(const char *filename, const char *content);

// Read from a file
const char *fs_read_file(const char *filename);

// Check if a file exists
bool fs_file_exists(const char *filename);

// Get file information
file_t *fs_get_file_info(const char *filename);

// List all files
void fs_list_files(void);

// Create initial system files
void fs_create_system_files(void);

// Format file system
void fs_format(void);

// Get file count
int fs_get_file_count(void);

// Create directory
bool fs_create_directory(const char *dirname);

// Set current directory
bool fs_set_directory(const char *dirname);

// Get current directory
const char *fs_get_current_directory(void);

// Check user permissions for a file
bool fs_check_permission(const char *filename, int permission);

// Set file permissions
bool fs_set_permission(const char *filename, int permission);

// Get file size
int fs_get_file_size(const char *filename);

// Display file browser
void display_file_browser(void);

// Search for files
void fs_search(const char *query);

#endif // FS_H