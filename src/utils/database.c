#include "../../include/database.h"
#include "../../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Global database connection
sqlite3 *db = NULL;

bool init_database(void) {
    int rc;
    char db_path[512];
    
    // Create database path
    snprintf(db_path, sizeof(db_path), "data/automation.db");
    
    // Create data directory if it doesn't exist
    system("mkdir data 2>nul");
    
    // Open database
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        log_error("Cannot open database: %s", sqlite3_errmsg(db));
        return false;
    }
    
    log_info("Database opened successfully: %s", db_path);
    
    // Create tables
    if (!create_tables()) {
        log_error("Failed to create database tables");
        close_database();
        return false;
    }
    
    return true;
}

void close_database(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        log_info("Database connection closed");
    }
}

bool create_tables(void) {
    const char *create_system_metrics = 
        "CREATE TABLE IF NOT EXISTS system_metrics ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "cpu_usage REAL NOT NULL,"
        "memory_usage REAL NOT NULL,"
        "disk_usage REAL NOT NULL,"
        "hostname TEXT NOT NULL"
        ");";
    
    const char *create_file_operations = 
        "CREATE TABLE IF NOT EXISTS file_operations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "operation TEXT NOT NULL,"
        "file_path TEXT NOT NULL,"
        "file_size INTEGER,"
        "status TEXT NOT NULL"
        ");";
    
    const char *create_network_metrics = 
        "CREATE TABLE IF NOT EXISTS network_metrics ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "target TEXT NOT NULL,"
        "ping_time INTEGER,"
        "connection_status INTEGER NOT NULL,"
        "interface_name TEXT"
        ");";
    
    const char *create_security_scans = 
        "CREATE TABLE IF NOT EXISTS security_scans ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp INTEGER NOT NULL,"
        "scan_type TEXT NOT NULL,"
        "target TEXT NOT NULL,"
        "threats_found INTEGER NOT NULL,"
        "severity TEXT NOT NULL,"
        "description TEXT"
        ");";
    
    const char *create_scheduled_tasks = 
        "CREATE TABLE IF NOT EXISTS scheduled_tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "task_name TEXT UNIQUE NOT NULL,"
        "command TEXT NOT NULL,"
        "schedule TEXT NOT NULL,"
        "next_run INTEGER,"
        "last_run INTEGER,"
        "enabled INTEGER NOT NULL DEFAULT 1,"
        "status TEXT DEFAULT 'pending'"
        ");";
    
    // Create indexes for better performance
    const char *create_indexes[] = {
        "CREATE INDEX IF NOT EXISTS idx_system_metrics_timestamp ON system_metrics(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_file_operations_timestamp ON file_operations(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_network_metrics_timestamp ON network_metrics(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_security_scans_timestamp ON security_scans(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_scheduled_tasks_next_run ON scheduled_tasks(next_run);"
    };
    
    // Execute table creation queries
    if (!execute_query(create_system_metrics) ||
        !execute_query(create_file_operations) ||
        !execute_query(create_network_metrics) ||
        !execute_query(create_security_scans) ||
        !execute_query(create_scheduled_tasks)) {
        return false;
    }
    
    // Create indexes
    for (int i = 0; i < 5; i++) {
        if (!execute_query(create_indexes[i])) {
            log_warning("Failed to create index: %s", create_indexes[i]);
        }
    }
    
    log_info("Database tables created successfully");
    return true;
}

bool execute_query(const char *query) {
    char *err_msg = 0;
    int rc = sqlite3_exec(db, query, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        log_error("SQL error: %s", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

// System metrics operations
bool insert_system_metrics(const SystemMetrics *metrics) {
    const char *sql = "INSERT INTO system_metrics (timestamp, cpu_usage, memory_usage, disk_usage, hostname) "
                      "VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, metrics->timestamp);
    sqlite3_bind_double(stmt, 2, metrics->cpu_usage);
    sqlite3_bind_double(stmt, 3, metrics->memory_usage);
    sqlite3_bind_double(stmt, 4, metrics->disk_usage);
    sqlite3_bind_text(stmt, 5, metrics->hostname, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_error("Failed to insert system metrics: %s", sqlite3_errmsg(db));
        return false;
    }
    
    return true;
}

bool get_system_metrics_history(SystemMetrics **metrics, int *count, int limit) {
    const char *sql = "SELECT id, timestamp, cpu_usage, memory_usage, disk_usage, hostname "
                      "FROM system_metrics ORDER BY timestamp DESC LIMIT ?;";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    
    // Count rows first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    
    if (*count == 0) {
        sqlite3_finalize(stmt);
        *metrics = NULL;
        return true;
    }
    
    // Reset statement
    sqlite3_reset(stmt);
    
    // Allocate memory
    *metrics = malloc(sizeof(SystemMetrics) * (*count));
    if (!*metrics) {
        log_error("Memory allocation failed");
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch data
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*metrics)[i].id = sqlite3_column_int(stmt, 0);
        (*metrics)[i].timestamp = sqlite3_column_int64(stmt, 1);
        (*metrics)[i].cpu_usage = sqlite3_column_double(stmt, 2);
        (*metrics)[i].memory_usage = sqlite3_column_double(stmt, 3);
        (*metrics)[i].disk_usage = sqlite3_column_double(stmt, 4);
        strncpy((*metrics)[i].hostname, (const char*)sqlite3_column_text(stmt, 5), 
                sizeof((*metrics)[i].hostname) - 1);
        (*metrics)[i].hostname[sizeof((*metrics)[i].hostname) - 1] = '\0';
        i++;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// File operations
bool insert_file_operation(const FileOperation *operation) {
    const char *sql = "INSERT INTO file_operations (timestamp, operation, file_path, file_size, status) "
                      "VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, operation->timestamp);
    sqlite3_bind_text(stmt, 2, operation->operation, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, operation->file_path, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, operation->file_size);
    sqlite3_bind_text(stmt, 5, operation->status, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_error("Failed to insert file operation: %s", sqlite3_errmsg(db));
        return false;
    }
    
    return true;
}

// Network metrics
bool insert_network_metrics(const NetworkMetrics *metrics) {
    const char *sql = "INSERT INTO network_metrics (timestamp, target, ping_time, connection_status, interface_name) "
                      "VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, metrics->timestamp);
    sqlite3_bind_text(stmt, 2, metrics->target, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, metrics->ping_time);
    sqlite3_bind_int(stmt, 4, metrics->connection_status ? 1 : 0);
    sqlite3_bind_text(stmt, 5, metrics->interface_name, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_error("Failed to insert network metrics: %s", sqlite3_errmsg(db));
        return false;
    }
    
    return true;
}

// Security scans
bool insert_security_scan(const SecurityScan *scan) {
    const char *sql = "INSERT INTO security_scans (timestamp, scan_type, target, threats_found, severity, description) "
                      "VALUES (?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, scan->timestamp);
    sqlite3_bind_text(stmt, 2, scan->scan_type, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, scan->target, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, scan->threats_found);
    sqlite3_bind_text(stmt, 5, scan->severity, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, scan->description, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_error("Failed to insert security scan: %s", sqlite3_errmsg(db));
        return false;
    }
    
    return true;
}

// Scheduled tasks
bool insert_scheduled_task(const ScheduledTask *task) {
    const char *sql = "INSERT INTO scheduled_tasks (task_name, command, schedule, next_run, last_run, enabled, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, task->task_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, task->command, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, task->schedule, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, task->next_run);
    sqlite3_bind_int64(stmt, 5, task->last_run);
    sqlite3_bind_int(stmt, 6, task->enabled ? 1 : 0);
    sqlite3_bind_text(stmt, 7, task->status, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        log_error("Failed to insert scheduled task: %s", sqlite3_errmsg(db));
        return false;
    }
    
    return true;
}

bool get_scheduled_tasks(ScheduledTask **tasks, int *count) {
    const char *sql = "SELECT id, task_name, command, schedule, next_run, last_run, enabled, status "
                      "FROM scheduled_tasks ORDER BY task_name;";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    // Count rows first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    
    if (*count == 0) {
        sqlite3_finalize(stmt);
        *tasks = NULL;
        return true;
    }
    
    // Reset statement
    sqlite3_reset(stmt);
    
    // Allocate memory
    *tasks = malloc(sizeof(ScheduledTask) * (*count));
    if (!*tasks) {
        log_error("Memory allocation failed");
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch data
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*tasks)[i].id = sqlite3_column_int(stmt, 0);
        strncpy((*tasks)[i].task_name, (const char*)sqlite3_column_text(stmt, 1), 
                sizeof((*tasks)[i].task_name) - 1);
        (*tasks)[i].task_name[sizeof((*tasks)[i].task_name) - 1] = '\0';
        
        strncpy((*tasks)[i].command, (const char*)sqlite3_column_text(stmt, 2), 
                sizeof((*tasks)[i].command) - 1);
        (*tasks)[i].command[sizeof((*tasks)[i].command) - 1] = '\0';
        
        strncpy((*tasks)[i].schedule, (const char*)sqlite3_column_text(stmt, 3), 
                sizeof((*tasks)[i].schedule) - 1);
        (*tasks)[i].schedule[sizeof((*tasks)[i].schedule) - 1] = '\0';
        
        (*tasks)[i].next_run = sqlite3_column_int64(stmt, 4);
        (*tasks)[i].last_run = sqlite3_column_int64(stmt, 5);
        (*tasks)[i].enabled = sqlite3_column_int(stmt, 6) ? true : false;
        
        strncpy((*tasks)[i].status, (const char*)sqlite3_column_text(stmt, 7), 
                sizeof((*tasks)[i].status) - 1);
        (*tasks)[i].status[sizeof((*tasks)[i].status) - 1] = '\0';
        
        i++;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Utility functions
char* get_timestamp_string(time_t timestamp) {
    static char buffer[32];
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}

time_t parse_timestamp_string(const char *timestamp_str) {
    struct tm tm_info = {0};
    
#ifdef _WIN32
    // Windows için sscanf kullanarak timestamp parse et
    int year, month, day, hour, minute, second;
    if (sscanf(timestamp_str, "%d-%d-%d %d:%d:%d", 
               &year, &month, &day, &hour, &minute, &second) == 6) {
        tm_info.tm_year = year - 1900;
        tm_info.tm_mon = month - 1;
        tm_info.tm_mday = day;
        tm_info.tm_hour = hour;
        tm_info.tm_min = minute;
        tm_info.tm_sec = second;
        return mktime(&tm_info);
    }
#else
    if (strptime(timestamp_str, "%Y-%m-%d %H:%M:%S", &tm_info) != NULL) {
        return mktime(&tm_info);
    }
#endif
    return 0;
}

bool cleanup_old_records(int days_to_keep) {
    time_t cutoff_time = time(NULL) - (days_to_keep * 24 * 60 * 60);
    
    const char *cleanup_queries[] = {
        "DELETE FROM system_metrics WHERE timestamp < ?;",
        "DELETE FROM file_operations WHERE timestamp < ?;",
        "DELETE FROM network_metrics WHERE timestamp < ?;",
        "DELETE FROM security_scans WHERE timestamp < ?;"
    };
    
    for (int i = 0; i < 4; i++) {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, cleanup_queries[i], -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            log_error("Failed to prepare cleanup statement: %s", sqlite3_errmsg(db));
            continue;
        }
        
        sqlite3_bind_int64(stmt, 1, cutoff_time);
        
        rc = sqlite3_step(stmt);
        int deleted_rows = sqlite3_changes(db);
        sqlite3_finalize(stmt);
        
        if (rc == SQLITE_DONE) {
            log_info("Cleaned up %d old records from table %d", deleted_rows, i + 1);
        } else {
            log_error("Failed to cleanup old records: %s", sqlite3_errmsg(db));
        }
    }
    
    return true;
}

bool vacuum_database(void) {
    return execute_query("VACUUM;");
}

bool backup_database(const char *backup_path) {
    sqlite3 *backup_db;
    sqlite3_backup *backup;
    int rc;
    
    rc = sqlite3_open(backup_path, &backup_db);
    if (rc != SQLITE_OK) {
        log_error("Cannot open backup database: %s", sqlite3_errmsg(backup_db));
        return false;
    }
    
    backup = sqlite3_backup_init(backup_db, "main", db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }
    
    rc = sqlite3_errcode(backup_db);
    sqlite3_close(backup_db);
    
    if (rc == SQLITE_OK) {
        log_info("Database backup created successfully: %s", backup_path);
        return true;
    } else {
        log_error("Database backup failed: %s", sqlite3_errmsg(backup_db));
        return false;
    }
}

// Get file operations history
bool get_file_operations_history(FileOperation **operations, int *count, int limit) {
    const char *sql = "SELECT id, timestamp, operation, file_path, file_size, status "
                      "FROM file_operations ORDER BY timestamp DESC LIMIT ?;";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    
    // Count rows first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    
    if (*count == 0) {
        sqlite3_finalize(stmt);
        *operations = NULL;
        return true;
    }
    
    sqlite3_reset(stmt);
    
    // Allocate memory
    *operations = malloc(sizeof(FileOperation) * (*count));
    if (!*operations) {
        log_error("Memory allocation failed");
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch data
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*operations)[i].id = sqlite3_column_int(stmt, 0);
        (*operations)[i].timestamp = sqlite3_column_int64(stmt, 1);
        strncpy((*operations)[i].operation, (const char*)sqlite3_column_text(stmt, 2), 
                sizeof((*operations)[i].operation) - 1);
        (*operations)[i].operation[sizeof((*operations)[i].operation) - 1] = '\0';
        strncpy((*operations)[i].file_path, (const char*)sqlite3_column_text(stmt, 3), 
                sizeof((*operations)[i].file_path) - 1);
        (*operations)[i].file_path[sizeof((*operations)[i].file_path) - 1] = '\0';
        (*operations)[i].file_size = sqlite3_column_int64(stmt, 4);
        strncpy((*operations)[i].status, (const char*)sqlite3_column_text(stmt, 5), 
                sizeof((*operations)[i].status) - 1);
        (*operations)[i].status[sizeof((*operations)[i].status) - 1] = '\0';
        i++;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Get network metrics history
bool get_network_metrics_history(NetworkMetrics **metrics, int *count, int limit) {
    const char *sql = "SELECT id, timestamp, target, ping_time, connection_status, interface_name "
                      "FROM network_metrics ORDER BY timestamp DESC LIMIT ?;";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    
    // Count rows first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    
    if (*count == 0) {
        sqlite3_finalize(stmt);
        *metrics = NULL;
        return true;
    }
    
    sqlite3_reset(stmt);
    
    // Allocate memory
    *metrics = malloc(sizeof(NetworkMetrics) * (*count));
    if (!*metrics) {
        log_error("Memory allocation failed");
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch data
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*metrics)[i].id = sqlite3_column_int(stmt, 0);
        (*metrics)[i].timestamp = sqlite3_column_int64(stmt, 1);
        strncpy((*metrics)[i].target, (const char*)sqlite3_column_text(stmt, 2), 
                sizeof((*metrics)[i].target) - 1);
        (*metrics)[i].target[sizeof((*metrics)[i].target) - 1] = '\0';
        (*metrics)[i].ping_time = sqlite3_column_int(stmt, 3);
        (*metrics)[i].connection_status = sqlite3_column_int(stmt, 4);
        strncpy((*metrics)[i].interface_name, (const char*)sqlite3_column_text(stmt, 5), 
                sizeof((*metrics)[i].interface_name) - 1);
        (*metrics)[i].interface_name[sizeof((*metrics)[i].interface_name) - 1] = '\0';
        i++;
    }
    
    sqlite3_finalize(stmt);
    return true;
}

// Get security scans history
bool get_security_scans_history(SecurityScan **scans, int *count, int limit) {
    const char *sql = "SELECT id, timestamp, scan_type, target, threats_found, severity, description "
                      "FROM security_scans ORDER BY timestamp DESC LIMIT ?;";
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        log_error("Failed to prepare statement: %s", sqlite3_errmsg(db));
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    
    // Count rows first
    *count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }
    
    if (*count == 0) {
        sqlite3_finalize(stmt);
        *scans = NULL;
        return true;
    }
    
    sqlite3_reset(stmt);
    
    // Allocate memory
    *scans = malloc(sizeof(SecurityScan) * (*count));
    if (!*scans) {
        log_error("Memory allocation failed");
        sqlite3_finalize(stmt);
        return false;
    }
    
    // Fetch data
    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < *count) {
        (*scans)[i].id = sqlite3_column_int(stmt, 0);
        (*scans)[i].timestamp = sqlite3_column_int64(stmt, 1);
        strncpy((*scans)[i].scan_type, (const char*)sqlite3_column_text(stmt, 2), 
                sizeof((*scans)[i].scan_type) - 1);
        (*scans)[i].scan_type[sizeof((*scans)[i].scan_type) - 1] = '\0';
        strncpy((*scans)[i].target, (const char*)sqlite3_column_text(stmt, 3), 
                sizeof((*scans)[i].target) - 1);
        (*scans)[i].target[sizeof((*scans)[i].target) - 1] = '\0';
        (*scans)[i].threats_found = sqlite3_column_int(stmt, 4);
        strncpy((*scans)[i].severity, (const char*)sqlite3_column_text(stmt, 5), 
                sizeof((*scans)[i].severity) - 1);
        (*scans)[i].severity[sizeof((*scans)[i].severity) - 1] = '\0';
        strncpy((*scans)[i].description, (const char*)sqlite3_column_text(stmt, 6), 
                sizeof((*scans)[i].description) - 1);
        (*scans)[i].description[sizeof((*scans)[i].description) - 1] = '\0';
        i++;
    }
    
    sqlite3_finalize(stmt);
    return true;
}