/*
 * ========================================
 * Core Header - Temel Tanımlamalar
 * ========================================
 */

#ifndef CORE_H
#define CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Platform bağımlı include'lar
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define PATH_SEPARATOR "\\"
    #define MAX_PATH_LEN 260
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define PATH_SEPARATOR "/"
    #define MAX_PATH_LEN 4096
#endif

// Genel sabitler
#define VERSION "1.0.0"
#define PROGRAM_NAME "Sistem Otomasyon Merkezi"
#define MAX_BUFFER_SIZE 4096
#define MAX_FILENAME_LEN 256
#define MAX_COMMAND_LEN 512

// Hata kodları
#define SUCCESS 0
#define ERROR_GENERAL -1
#define ERROR_FILE_NOT_FOUND_CUSTOM -2
#define ERROR_PERMISSION_DENIED -3
#define ERROR_MEMORY_ALLOCATION -4
#define ERROR_INVALID_PARAMETER_CUSTOM -5

// Renk kodları (Windows için)
#ifdef _WIN32
    #define COLOR_RESET ""
    #define COLOR_RED ""
    #define COLOR_GREEN ""
    #define COLOR_YELLOW ""
    #define COLOR_BLUE ""
    #define COLOR_MAGENTA ""
    #define COLOR_CYAN ""
    #define COLOR_WHITE ""
#else
    #define COLOR_RESET "\033[0m"
    #define COLOR_RED "\033[31m"
    #define COLOR_GREEN "\033[32m"
    #define COLOR_YELLOW "\033[33m"
    #define COLOR_BLUE "\033[34m"
    #define COLOR_MAGENTA "\033[35m"
    #define COLOR_CYAN "\033[36m"
    #define COLOR_WHITE "\033[37m"
#endif

// Yapılar
typedef struct {
    char name[MAX_FILENAME_LEN];
    char path[MAX_PATH_LEN];
    long size;
    time_t modified_time;
    int is_directory;
} FileInfo;

typedef struct {
    char message[MAX_BUFFER_SIZE];
    time_t timestamp;
    int level; // 0=INFO, 1=WARNING, 2=ERROR, 3=DEBUG
} LogEntry;

typedef struct {
    char key[256];
    char value[512];
} ConfigEntry;

// Fonksiyon prototipleri
int init_system(void);
int cleanup_system(void);
char* get_current_time_string(void);
int create_directory(const char* path);
int file_exists(const char* path);
int directory_exists(const char* path);
long get_file_size(const char* path);
int copy_file(const char* source, const char* destination);
int move_file(const char* source, const char* destination);
int delete_file(const char* path);

// Utility fonksiyonları
void clear_screen(void);
void pause_system(void);
int get_user_input(char* buffer, int max_size);
void print_header(const char* title);
void print_separator(void);

#endif // CORE_H