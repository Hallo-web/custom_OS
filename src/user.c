#include "user.h"
#include "string.h" // For string operations
#include <stdbool.h>
#include <stddef.h>

// User database
static user_t users[MAX_USERS];
static int user_count = 0;
static char current_user[32] = "";
static bool logged_in = false;

// Admin password (hidden in a real system)
static const char *ADMIN_PASSWORD = "osiris1371";

// Initialize user management system
void user_system_init(void)
{
    // Clear all user entries
    for (int i = 0; i < MAX_USERS; i++)
    {
        users[i].active = false;
        users[i].username[0] = '\0';
        users[i].password[0] = '\0';
        users[i].privilege_level = USER_GUEST;
        users[i].real_name[0] = '\0';
        users[i].creation_date[0] = '\0';
        users[i].last_login[0] = '\0';
    }

    user_count = 0;
    logged_in = false;
    current_user[0] = '\0';

    // Create default admin user
    add_user("admin", "admin", USER_ADMIN);
    strcpy(users[0].real_name, "System Administrator");
    strcpy(users[0].creation_date, "2025-05-15");

    // Create default guest user
    add_user("guest", "guest", USER_GUEST);
    strcpy(users[1].real_name, "Guest User");
    strcpy(users[1].creation_date, "2025-05-15");

    // Create a standard user
    add_user("user", "password", USER_STANDARD);
    strcpy(users[2].real_name, "Default User");
    strcpy(users[2].creation_date, "2025-05-15");
}

// Add a new user
bool add_user(const char *username, const char *password, int privilege_level)
{
    // Check if we've reached the maximum number of users
    if (user_count >= MAX_USERS)
    {
        return false;
    }

    // Check if username already exists
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active && strcmp(users[i].username, username) == 0)
        {
            return false;
        }
    }

    // Validate username and password
    if (!validate_username(username) || !validate_password(password))
    {
        return false;
    }

    // Find an empty slot
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (!users[i].active)
        {
            strcpy(users[i].username, username);
            strcpy(users[i].password, password);
            users[i].privilege_level = privilege_level;
            users[i].active = true;

            // Set default values for other fields
            strcpy(users[i].real_name, username);
            strcpy(users[i].creation_date, "2025-05-15");
            users[i].last_login[0] = '\0';

            user_count++;
            return true;
        }
    }

    return false;
}

// Authenticate a user
bool authenticate_user(const char *username, const char *password)
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active &&
            strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0)
        {
            // Set current user
            strcpy(current_user, username);
            logged_in = true;

            // Update last login date (in real system, would use actual date)
            strcpy(users[i].last_login, "2025-05-15");

            return true;
        }
    }

    return false;
}

// Set current user
void set_current_user(const char *username)
{
    strcpy(current_user, username);
    logged_in = true;
}

// Get current username
const char *get_current_username(void)
{
    if (logged_in)
    {
        return current_user;
    }
    return "not logged in";
}

// Check if the current user is admin
bool is_admin(void)
{
    if (!logged_in)
    {
        return false;
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active && strcmp(users[i].username, current_user) == 0)
        {
            return users[i].privilege_level == USER_ADMIN;
        }
    }

    return false;
}

// Check if any user is logged in
bool is_logged_in(void)
{
    return logged_in;
}

// Logout current user
void logout(void)
{
    logged_in = false;
    current_user[0] = '\0';
}

// Change user password
bool change_password(const char *username, const char *old_password, const char *new_password)
{
    // Validate new password
    if (!validate_password(new_password))
    {
        return false;
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active && strcmp(users[i].username, username) == 0)
        {
            // Check old password
            if (strcmp(users[i].password, old_password) != 0)
            {
                return false;
            }

            // Set new password
            strcpy(users[i].password, new_password);
            return true;
        }
    }

    return false;
}

// Delete a user (admin only)
bool delete_user(const char *username)
{
    // Check if current user is admin
    if (!is_admin())
    {
        return false;
    }

    // Cannot delete admin user
    if (strcmp(username, "admin") == 0)
    {
        return false;
    }

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active && strcmp(users[i].username, username) == 0)
        {
            users[i].active = false;
            user_count--;
            return true;
        }
    }

    return false;
}

// Display user management screen
void display_user_management(void)
{
    // Check if current user is admin
    if (!is_admin())
    {
        printf("Access denied. Admin privileges required.\n");
        return;
    }

    printf("===== User Management =====\n\n");
    printf("Available actions:\n");
    printf("1. Create new user\n");
    printf("2. Delete user\n");
    printf("3. List users\n");
    printf("4. Change user privileges\n");
    printf("5. Return to main menu\n\n");

    // In a real implementation, we would handle keyboard input here
}

// Create new user dialog
void create_user_dialog(void)
{
    // In a real system, this would collect input from the user
    printf("===== Create New User =====\n\n");
    printf("Username: ");
    // Get input
    printf("Password: ");
    // Get password (masked)
    printf("Privilege level (0=Guest, 1=Standard, 2=Admin): ");
    // Get privilege level
    printf("Real name: ");
    // Get real name

    // Then call add_user with the collected information
}

// Login dialog
void login_dialog(void)
{
    char username[32];
    char password[32];

    // In a real system, this would collect input from the user
    printf("===== Login =====\n\n");
    printf("Username: ");
    // Here we'd collect username input
    strcpy(username, "admin"); // Simulated input

    printf("Password: ");
    // Here we'd collect password input (masked)
    strcpy(password, "admin"); // Simulated input

    if (authenticate_user(username, password))
    {
        printf("\nLogin successful! Welcome %s.\n", username);
    }
    else
    {
        printf("\nInvalid username or password.\n");
    }
}

// Lock screen
void lock_screen(void)
{
    if (!is_logged_in())
    {
        return;
    }

    // Store username temporarily
    char username[32];
    strcpy(username, current_user);

    // Log out
    logout();

    printf("===== OSIRIS LOCKED =====\n\n");
    printf("System locked. Enter password to unlock.\n\n");
    printf("User: %s\n", username);
    printf("Password: ");

    // In a real system, would collect password and authenticate
    // For now, just simulate a successful unlock
    printf("\nSystem unlocked!\n");

    // Re-login
    set_current_user(username);
}

// Elevate to admin with secret password
bool try_elevate_to_admin(const char *secret_password)
{
    if (strcmp(secret_password, ADMIN_PASSWORD) == 0)
    {
        // Find current user and elevate privileges
        for (int i = 0; i < MAX_USERS; i++)
        {
            if (users[i].active && strcmp(users[i].username, current_user) == 0)
            {
                users[i].privilege_level = USER_ADMIN;
                return true;
            }
        }
    }

    return false;
}

// Show all users (admin only)
void list_users(void)
{
    // Check if current user is admin
    if (!is_admin())
    {
        printf("Access denied. Admin privileges required.\n");
        return;
    }

    printf("===== User List =====\n\n");
    printf("%-10s %-20s %-10s %-10s %-20s\n", "Username", "Real Name", "Privilege", "Active", "Last Login");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i].active)
        {
            const char *privilege;
            switch (users[i].privilege_level)
            {
            case USER_GUEST:
                privilege = "Guest";
                break;
            case USER_STANDARD:
                privilege = "Standard";
                break;
            case USER_ADMIN:
                privilege = "Admin";
                break;
            default:
                privilege = "Unknown";
            }

            printf("%-10s %-20s %-10s %-10s %-20s\n",
                   users[i].username,
                   users[i].real_name,
                   privilege,
                   "Yes",
                   users[i].last_login[0] ? users[i].last_login : "Never");
        }
    }
}

// Validate username (no spaces, special chars, etc.)
bool validate_username(const char *username)
{
    // Check length
    int len = strlen(username);
    if (len < 3 || len > 31)
    {
        return false;
    }

    // Check characters
    for (int i = 0; i < len; i++)
    {
        char c = username[i];

        // Allow letters, numbers, underscore, and hyphen
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-'))
        {
            return false;
        }
    }

    // First character must be a letter
    char first = username[0];
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')))
    {
        return false;
    }

    return true;
}

// Validate password (length, complexity)
bool validate_password(const char *password)
{
    // Check length
    int len = strlen(password);
    if (len < 4 || len > 31)
    {
        return false;
    }

    // In a real system, would check for complexity requirements
    // like uppercase, lowercase, numbers, special chars

    return true;
}