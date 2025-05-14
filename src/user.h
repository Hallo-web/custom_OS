#ifndef USER_H
#define USER_H

#include <stdbool.h>

// User privilege levels
#define USER_GUEST 0
#define USER_STANDARD 1
#define USER_ADMIN 2

// Maximum number of users
#define MAX_USERS 16

// User structure
typedef struct
{
    char username[32];
    char password[32];
    int privilege_level;
    bool active;
    char real_name[64];
    char creation_date[16];
    char last_login[16];
} user_t;

// Initialize user management system
void user_system_init(void);

// Add a new user
bool add_user(const char *username, const char *password, int privilege_level);

// Authenticate a user
bool authenticate_user(const char *username, const char *password);

// Set current user
void set_current_user(const char *username);

// Get current username
const char *get_current_username(void);

// Check if the current user is admin
bool is_admin(void);

// Check if any user is logged in
bool is_logged_in(void);

// Logout current user
void logout(void);

// Change user password
bool change_password(const char *username, const char *old_password, const char *new_password);

// Delete a user (admin only)
bool delete_user(const char *username);

// Display user management screen
void display_user_management(void);

// Create new user dialog
void create_user_dialog(void);

// Login dialog
void login_dialog(void);

// Lock screen
void lock_screen(void);

// Elevate to admin with secret password
bool try_elevate_to_admin(const char *secret_password);

// Show all users (admin only)
void list_users(void);

// Validate username (no spaces, special chars, etc.)
bool validate_username(const char *username);

// Validate password (length, complexity)
bool validate_password(const char *password);

#endif // USER_H