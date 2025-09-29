/*
 * ========================================
 * Logger Implementation - Loglama Sistemi
 * ========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "../../include/logger.h"

// Global logger konfigürasyonu
static LogConfig g_log_config = {
    .log_file_path = "logs/automation.log",
    .min_level = LOG_INFO,
    .console_output = 1,
    .file_output = 1,
    .max_file_size = 10, // 10 MB
    .max_backup_files = 5
};

static int g_logger_initialized = 0;

// Logger başlatma
int init_logger(void) {
    if (g_logger_initialized) {
        return 1; // Zaten başlatılmış
    }
    
    // Log dizinini oluştur
    create_log_directory();
    
    g_logger_initialized = 1;
    return 1;
}

// Logger temizleme
int cleanup_logger(void) {
    g_logger_initialized = 0;
    return 1;
}

// Log seviyesi ayarlama
int set_log_level(LogLevel level) {
    g_log_config.min_level = level;
    return 1;
}

// Log dosyası ayarlama
int set_log_file(const char* filepath) {
    if (filepath && strlen(filepath) < sizeof(g_log_config.log_file_path)) {
        strcpy(g_log_config.log_file_path, filepath);
        return 1;
    }
    return 0;
}

// Konsol loglama ayarlama
int enable_console_logging(int enable) {
    g_log_config.console_output = enable;
    return 1;
}

// Dosya loglama ayarlama
int enable_file_logging(int enable) {
    g_log_config.file_output = enable;
    return 1;
}

// Ana loglama fonksiyonu
void log_message(LogLevel level, const char* format, ...) {
    if (!g_logger_initialized) {
        init_logger();
    }
    
    if (level < g_log_config.min_level) {
        return;
    }
    
    va_list args;
    char message[1024];
    char timestamp[64];
    time_t now;
    struct tm* timeinfo;
    
    // Zaman damgası oluştur
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Mesajı formatla
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    // Konsola yazdır
    if (g_log_config.console_output) {
        printf("[%s] %s: %s\n", timestamp, get_log_level_string(level), message);
    }
    
    // Dosyaya yazdır
    if (g_log_config.file_output) {
        FILE* file = fopen(g_log_config.log_file_path, "a");
        if (file) {
            fprintf(file, "[%s] %s: %s\n", timestamp, get_log_level_string(level), message);
            fclose(file);
        }
    }
}

// Debug log
void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    log_message(LOG_DEBUG, "%s", message);
}

// Info log
void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    log_message(LOG_INFO, "%s", message);
}

// Warning log
void log_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    log_message(LOG_WARNING, "%s", message);
}

// Error log
void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    log_message(LOG_ERROR, "%s", message);
}

// Log seviyesi string'i al
const char* get_log_level_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

// Log dizini oluştur
int create_log_directory(void) {
    char dir_path[256];
    strcpy(dir_path, g_log_config.log_file_path);
    
    // Dosya adını kaldır, sadece dizin yolunu al
    char* last_slash = strrchr(dir_path, '/');
    if (!last_slash) {
        last_slash = strrchr(dir_path, '\\');
    }
    
    if (last_slash) {
        *last_slash = '\0';
        
        #ifdef _WIN32
            CreateDirectoryA(dir_path, NULL);
        #else
            mkdir(dir_path, 0755);
        #endif
    }
    
    return 1;
}

// Log dosyası boyutunu al
int get_log_file_size(void) {
    FILE* file = fopen(g_log_config.log_file_path, "r");
    if (!file) {
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    
    return (int)(size / (1024 * 1024)); // MB cinsinden
}

// Log dosyası rotasyonu
int rotate_log_file(void) {
    if (get_log_file_size() < g_log_config.max_file_size) {
        return 1; // Rotasyon gerekli değil
    }
    
    // Backup dosyası oluştur
    char backup_path[512];
    time_t now;
    struct tm* timeinfo;
    
    time(&now);
    timeinfo = localtime(&now);
    
    snprintf(backup_path, sizeof(backup_path), "%s.%04d%02d%02d_%02d%02d%02d",
             g_log_config.log_file_path,
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    
    rename(g_log_config.log_file_path, backup_path);
    
    return 1;
}

// Backup dosyası oluştur
int backup_log_file(void) {
    return rotate_log_file();
}

// Eski logları temizle
int clear_old_logs(void) {
    // Bu fonksiyon geliştirilecek - şimdilik basit implementasyon
    return 1;
}