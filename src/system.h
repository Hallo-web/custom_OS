#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdbool.h>

// System state flags
#define SYSTEM_RUNNING 0
#define SYSTEM_HALTED 1
#define SYSTEM_REBOOT 2
#define SYSTEM_SHUTDOWN 3

// System info structure
typedef struct
{
    char os_name[32];
    char os_version[16];
    char build_date[16];
    char kernel_version[16];
    uint32_t uptime_seconds;
    uint32_t memory_total;
    uint32_t memory_used;
    uint32_t system_ticks;
    char current_user[32];
    int num_processes;
    int num_files;
} system_info_t;

// Initialize system
void system_init(void);

// Get system state
int get_system_state(void);

// Set system state
void set_system_state(int state);

// Get system information
system_info_t get_system_info(void);

// Display system information
void show_system_info(void);

// Perform system shutdown
void perform_shutdown(void);

// Perform system reboot
void simulate_reboot(void);

// Run system diagnostics
void run_diagnostics(void);

// Check system integrity
bool check_integrity(void);

// Set system time
void set_system_time(int hour, int minute, int second);

// Set system date
void set_system_date(int year, int month, int day);

// Initialize memory management
void init_memory(void);

// Get free memory
uint32_t get_free_memory(void);

// Get used memory
uint32_t get_used_memory(void);

// Allocate memory block
void *system_malloc(size_t size);

// Free memory block
void system_free(void *ptr);

// Add a process to the system
int add_process(const char *name);

// End a process
bool end_process(int pid);

// Get process status
int get_process_status(int pid);

// Display all running processes
void show_processes(void);

// Display CPU usage chart
void show_cpu_usage(void);

// Display memory usage chart
void show_memory_usage(void);

// Display system logs
void show_system_logs(void);

// Write to system log
void log_message(const char *message);

// Handle system errors
void handle_error(const char *error_message);

// Create a backup of system files
void backup_system(void);

// Restore from backup
bool restore_from_backup(void);

// Check for system updates (simulated)
bool check_for_updates(void);

// Install system update (simulated)
bool install_update(void);

#endif // SYSTEM_H