/*
 * ========================================
 * Logger Header - Loglama Sistemi
 * ========================================
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "core.h"

// Log seviyeleri
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3
} LogLevel;

// Log konfigürasyonu
typedef struct {
    char log_file_path[MAX_PATH_LEN];
    LogLevel min_level;
    int console_output;
    int file_output;
    int max_file_size; // MB cinsinden
    int max_backup_files;
} LogConfig;

// Fonksiyon prototipleri
int init_logger(void);
int cleanup_logger(void);
int set_log_level(LogLevel level);
int set_log_file(const char* filepath);
int enable_console_logging(int enable);
int enable_file_logging(int enable);

// Ana loglama fonksiyonları
void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);

// Yardımcı fonksiyonlar
void log_message(LogLevel level, const char* format, ...);
const char* get_log_level_string(LogLevel level);
int rotate_log_file(void);
int get_log_file_size(void);

// Log dosyası yönetimi
int create_log_directory(void);
int backup_log_file(void);
int clear_old_logs(void);

#endif // LOGGER_H