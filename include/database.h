#ifndef DATABASE_H
#define DATABASE_H

#include "../lib/sqlite/sqlite3.h"
#include <stdbool.h>
#include <time.h>

// Database connection
extern sqlite3 *db;

// Database initialization and cleanup
bool init_database(void);
void close_database(void);

// System monitoring data structures
typedef struct {
    int id;
    time_t timestamp;
    double cpu_usage;
    double memory_usage;
    double disk_usage;
    char hostname[256];
} SystemMetrics;

// File operation data structures
typedef struct {
    int id;
    time_t timestamp;
    char operation[64];
    char file_path[512];
    long file_size;
    char status[32];
} FileOperation;

// Network monitoring data structures
typedef struct {
    int id;
    time_t timestamp;
    char target[256];
    int ping_time;
    bool connection_status;
    char interface_name[64];
} NetworkMetrics;

// Security scan data structures
typedef struct {
    int id;
    time_t timestamp;
    char scan_type[64];
    char target[256];
    int threats_found;
    char severity[32];
    char description[512];
} SecurityScan;

// Task scheduler data structures
typedef struct {
    int id;
    char task_name[128];
    char command[512];
    char schedule[64];
    time_t next_run;
    time_t last_run;
    bool enabled;
    char status[32];
} ScheduledTask;

// Database operations for system metrics
bool insert_system_metrics(const SystemMetrics *metrics);
bool get_system_metrics_history(SystemMetrics **metrics, int *count, int limit);
bool get_system_metrics_by_date(SystemMetrics **metrics, int *count, const char *date);

// Database operations for file operations
bool insert_file_operation(const FileOperation *operation);
bool get_file_operations_history(FileOperation **operations, int *count, int limit);
bool get_file_operations_by_type(FileOperation **operations, int *count, const char *operation_type);

// Database operations for network metrics
bool insert_network_metrics(const NetworkMetrics *metrics);
bool get_network_metrics_history(NetworkMetrics **metrics, int *count, int limit);
bool get_network_metrics_by_target(NetworkMetrics **metrics, int *count, const char *target);

// Database operations for security scans
bool insert_security_scan(const SecurityScan *scan);
bool get_security_scans_history(SecurityScan **scans, int *count, int limit);
bool get_security_scans_by_severity(SecurityScan **scans, int *count, const char *severity);

// Database operations for scheduled tasks
bool insert_scheduled_task(const ScheduledTask *task);
bool update_scheduled_task(const ScheduledTask *task);
bool delete_scheduled_task(int task_id);
bool get_scheduled_tasks(ScheduledTask **tasks, int *count);
bool get_scheduled_task_by_id(ScheduledTask *task, int task_id);

// Utility functions
bool create_tables(void);
bool execute_query(const char *query);
char* get_timestamp_string(time_t timestamp);
time_t parse_timestamp_string(const char *timestamp_str);

// Database maintenance
bool cleanup_old_records(int days_to_keep);
bool vacuum_database(void);
bool backup_database(const char *backup_path);

#endif // DATABASE_H