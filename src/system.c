#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "system.h"
#include "string.h" // For string operations

// Global system information
static system_info_t sys_info = {
    .os_name = "O.S.I.R.I.S",
    .os_version = "2.0",
    .build_date = "2025-05-15",
    .kernel_version = "1.7.3",
    .uptime_seconds = 0,
    .memory_total = 1024 * 1024, // 1MB for this simple OS
    .memory_used = 0,
    .system_ticks = 0,
    .current_user = "guest",
    .num_processes = 1, // Kernel process
    .num_files = 0};

// Current system state
static int system_state = SYSTEM_RUNNING;

// System log entries
#define MAX_LOG_ENTRIES 100
static char system_logs[MAX_LOG_ENTRIES][128];
static int log_count = 0;

// Process table
#define MAX_PROCESSES 16
typedef struct
{
    int pid;
    char name[32];
    bool active;
    uint32_t memory_usage;
    uint32_t cpu_usage;
} process_t;

static process_t process_table[MAX_PROCESSES];
static int next_pid = 1; // PID 0 is reserved for kernel

// Simple memory management (just for demonstration)
typedef struct
{
    void *address;
    size_t size;
    bool used;
} memory_block_t;

#define MAX_MEMORY_BLOCKS 100
static memory_block_t memory_blocks[MAX_MEMORY_BLOCKS];
static int block_count = 0;

// System functions implementation

// Initialize system
void system_init(void)
{
    // Initialize process table
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].pid = -1;
        process_table[i].active = false;
    }

    // Create kernel process
    process_table[0].pid = 0;
    strcpy(process_table[0].name, "kernel");
    process_table[0].active = true;
    process_table[0].memory_usage = 512 * 1024; // 512KB
    process_table[0].cpu_usage = 5;             // 5%

    // Update system info
    sys_info.memory_used = process_table[0].memory_usage;

    // Initialize memory blocks
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        memory_blocks[i].used = false;
    }

    // Log system initialization
    log_message("System initialized successfully");
}

// Get system state
int get_system_state(void)
{
    return system_state;
}

// Set system state
void set_system_state(int state)
{
    system_state = state;

    // Log state change
    char log_buffer[64];

    switch (state)
    {
    case SYSTEM_RUNNING:
        strcpy(log_buffer, "System state changed to RUNNING");
        break;
    case SYSTEM_HALTED:
        strcpy(log_buffer, "System state changed to HALTED");
        break;
    case SYSTEM_REBOOT:
        strcpy(log_buffer, "System state changed to REBOOT");
        break;
    case SYSTEM_SHUTDOWN:
        strcpy(log_buffer, "System state changed to SHUTDOWN");
        break;
    default:
        strcpy(log_buffer, "System state changed to UNKNOWN");
        break;
    }

    log_message(log_buffer);
}

// Get system information
system_info_t get_system_info(void)
{
    return sys_info;
}

// Display system information
void show_system_info(void)
{
    // This function will be implemented in kernel.c to handle the display
    // since it depends on terminal functions
}

// Perform system shutdown
void perform_shutdown(void)
{
    log_message("Initiating system shutdown sequence");

    // Terminate all processes
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active && process_table[i].pid != 0)
        {
            end_process(process_table[i].pid);
        }
    }

    // Final log
    log_message("System shutdown complete");

    // Set system state to shutdown
    set_system_state(SYSTEM_SHUTDOWN);
}

// Perform system reboot
void simulate_reboot(void)
{
    log_message("Initiating system reboot sequence");

    // Terminate all processes except kernel
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active && process_table[i].pid != 0)
        {
            end_process(process_table[i].pid);
        }
    }

    // Reset system info
    sys_info.uptime_seconds = 0;
    sys_info.system_ticks = 0;
    sys_info.memory_used = process_table[0].memory_usage;
    strcpy(sys_info.current_user, "guest");

    // Set system state to reboot
    set_system_state(SYSTEM_REBOOT);

    // Final log
    log_message("System reboot complete");
}

// Run system diagnostics
void run_diagnostics(void)
{
    log_message("Running system diagnostics");

    bool all_passed = true;

    // Check memory integrity
    if (!check_integrity())
    {
        log_message("ERROR: Memory integrity check failed");
        all_passed = false;
    }
    else
    {
        log_message("Memory integrity check passed");
    }

    // Check process table integrity
    bool process_table_ok = true;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active && (process_table[i].pid < 0 || process_table[i].pid >= MAX_PROCESSES))
        {
            log_message("ERROR: Process table integrity check failed");
            process_table_ok = false;
            all_passed = false;
            break;
        }
    }

    if (process_table_ok)
    {
        log_message("Process table integrity check passed");
    }

    // Overall diagnostics result
    if (all_passed)
    {
        log_message("All diagnostics passed successfully");
    }
    else
    {
        log_message("Some diagnostics failed - system may be unstable");
    }
}

// Check system integrity
bool check_integrity(void)
{
    // Simple integrity check (for demonstration)
    return (sys_info.memory_used <= sys_info.memory_total);
}

// Set system time
void set_system_time(int hour, int minute, int second)
{
    // In a real system, this would interact with the CMOS RTC
    // For our demo, we'll just log it
    char log_buffer[64];
    sprintf(log_buffer, "System time set to %02d:%02d:%02d", hour, minute, second);
    log_message(log_buffer);
}

// Set system date
void set_system_date(int year, int month, int day)
{
    // In a real system, this would interact with the CMOS RTC
    // For our demo, we'll just log it
    char log_buffer[64];
    sprintf(log_buffer, "System date set to %04d-%02d-%02d", year, month, day);
    log_message(log_buffer);
}

// Initialize memory management
void init_memory(void)
{
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        memory_blocks[i].used = false;
    }

    block_count = 0;
    sys_info.memory_used = process_table[0].memory_usage; // Only kernel using memory initially

    log_message("Memory management initialized");
}

// Get free memory
uint32_t get_free_memory(void)
{
    return sys_info.memory_total - sys_info.memory_used;
}

// Get used memory
uint32_t get_used_memory(void)
{
    return sys_info.memory_used;
}

// Allocate memory block (very simplified)
void *system_malloc(size_t size)
{
    if (size == 0 || size > get_free_memory())
    {
        return NULL;
    }

    // Find a free block slot
    int free_index = -1;
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        if (!memory_blocks[i].used)
        {
            free_index = i;
            break;
        }
    }

    if (free_index == -1)
    {
        return NULL; // No free block slots
    }

    // Pretend to allocate memory (in a real OS, we'd use brk/sbrk or other mechanisms)
    // For this demo, we'll just use a dummy address based on the block index
    void *address = (void *)(0x100000 + (free_index * 4096)); // Start at 1MB with 4KB blocks

    // Update the block
    memory_blocks[free_index].address = address;
    memory_blocks[free_index].size = size;
    memory_blocks[free_index].used = true;

    // Update system memory usage
    sys_info.memory_used += size;

    return address;
}

// Free memory block
void system_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    // Find the block
    for (int i = 0; i < MAX_MEMORY_BLOCKS; i++)
    {
        if (memory_blocks[i].used && memory_blocks[i].address == ptr)
        {
            // Update system memory usage
            sys_info.memory_used -= memory_blocks[i].size;

            // Mark block as free
            memory_blocks[i].used = false;
            break;
        }
    }
}

// Add a process to the system
int add_process(const char *name)
{
    // Find a free slot in the process table
    int free_index = -1;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (!process_table[i].active)
        {
            free_index = i;
            break;
        }
    }

    if (free_index == -1)
    {
        log_message("ERROR: Process table full, cannot create new process");
        return -1; // No free slots
    }

    // Create the process
    process_table[free_index].pid = next_pid++;
    strcpy(process_table[free_index].name, name);
    process_table[free_index].active = true;
    process_table[free_index].memory_usage = 64 * 1024; // 64KB default
    process_table[free_index].cpu_usage = 10;           // 10% default

    // Update system info
    sys_info.num_processes++;
    sys_info.memory_used += process_table[free_index].memory_usage;

    // Log process creation
    char log_buffer[64];
    sprintf(log_buffer, "Process created: %s (PID: %d)", name, process_table[free_index].pid);
    log_message(log_buffer);

    return process_table[free_index].pid;
}

// End a process
bool end_process(int pid)
{
    if (pid == 0)
    {
        log_message("ERROR: Cannot terminate kernel process");
        return false; // Cannot terminate kernel
    }

    // Find the process
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active && process_table[i].pid == pid)
        {
            // Update system info
            sys_info.num_processes--;
            sys_info.memory_used -= process_table[i].memory_usage;

            // Mark process as inactive
            process_table[i].active = false;

            // Log process termination
            char log_buffer[64];
            sprintf(log_buffer, "Process terminated: %s (PID: %d)", process_table[i].name, pid);
            log_message(log_buffer);

            return true;
        }
    }

    return false; // Process not found
}

// Get process status
int get_process_status(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].pid == pid)
        {
            return process_table[i].active ? 1 : 0;
        }
    }

    return -1; // Process not found
}

// Display all running processes
void show_processes(void)
{
    // This function will be implemented in kernel.c to handle the display
    // since it depends on terminal functions
}

// Display CPU usage chart
void show_cpu_usage(void)
{
    // This function will be implemented in kernel.c to handle the display
    // since it depends on terminal functions
}

// Display memory usage chart
void show_memory_usage(void)
{
    // This function will be implemented in kernel.c to handle the display
    // since it depends on terminal functions
}

// Display system logs
void show_system_logs(void)
{
    // This function will be implemented in kernel.c to handle the display
    // since it depends on terminal functions
}

// Write to system log
void log_message(const char *message)
{
    if (log_count >= MAX_LOG_ENTRIES)
    {
        // Shift logs to make room for new entry
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++)
        {
            strcpy(system_logs[i], system_logs[i + 1]);
        }
        log_count = MAX_LOG_ENTRIES - 1;
    }

    // Add new log entry
    strcpy(system_logs[log_count], message);
    log_count++;
}

// Helper function to get log entry at index
const char *get_log_entry(int index)
{
    if (index >= 0 && index < log_count)
    {
        return system_logs[index];
    }
    return NULL;
}

// Get log count
int get_log_count(void)
{
    return log_count;
}

// Handle system errors
void handle_error(const char *error_message)
{
    // Log the error
    char log_buffer[128];
    sprintf(log_buffer, "ERROR: %s", error_message);
    log_message(log_buffer);

    // In a real OS, we might take more drastic actions depending on the error
}

// Create a backup of system files
void backup_system(void)
{
    log_message("System backup initiated");
    // In a real OS, this would copy files to a backup location
    // For this demo, we'll just simulate success
    log_message("System backup completed successfully");
}

// Restore from backup
bool restore_from_backup(void)
{
    log_message("System restore initiated");
    // In a real OS, this would restore files from a backup
    // For this demo, we'll just simulate success
    log_message("System restore completed successfully");
    return true;
}

// Check for system updates (simulated)
bool check_for_updates(void)
{
    log_message("Checking for system updates...");
    // Simulate update check
    log_message("No updates available");
    return false;
}

// Install system update (simulated)
bool install_update(void)
{
    log_message("Installing system update...");
    // Simulate update installation
    log_message("System update installed successfully");
    return true;
}

// Get process information by PID
process_t *get_process(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].pid == pid)
        {
            return &process_table[i];
        }
    }
    return NULL;
}

// Get all active processes
int get_active_processes(process_t **processes, int max_count)
{
    int count = 0;

    for (int i = 0; i < MAX_PROCESSES && count < max_count; i++)
    {
        if (process_table[i].active)
        {
            processes[count++] = &process_table[i];
        }
    }

    return count;
}

// Update system info based on current state
void update_system_info(void)
{
    // Update process count
    int process_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active)
        {
            process_count++;
        }
    }
    sys_info.num_processes = process_count;

    // Update memory usage
    uint32_t memory_used = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].active)
        {
            memory_used += process_table[i].memory_usage;
        }
    }
    sys_info.memory_used = memory_used;
}

// Helper function - format bytes into KB/MB/GB
char *format_size(uint32_t bytes, char *buffer)
{
    if (bytes < 1024)
    {
        sprintf(buffer, "%d B", bytes);
    }
    else if (bytes < 1024 * 1024)
    {
        sprintf(buffer, "%d KB", bytes / 1024);
    }
    else
    {
        sprintf(buffer, "%d MB", bytes / (1024 * 1024));
    }
    return buffer;
}