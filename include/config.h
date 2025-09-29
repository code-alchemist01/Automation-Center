/*
 * ========================================
 * Config Header - Konfigürasyon Yönetimi
 * ========================================
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "core.h"

// Konfigürasyon dosyası yolu
#define CONFIG_FILE_PATH "config/automation_config.ini"
#define MAX_CONFIG_ENTRIES 100

// Konfigürasyon yapısı
typedef struct {
    ConfigEntry entries[MAX_CONFIG_ENTRIES];
    int entry_count;
    char config_file_path[MAX_PATH_LEN];
} Config;

// Fonksiyon prototipleri
int init_config(void);
int load_config(void);
int save_config(void);
int reload_config(void);

// Konfigürasyon okuma/yazma
const char* get_config_value(const char* key);
int set_config_value(const char* key, const char* value);
int get_config_int(const char* key, int default_value);
double get_config_double(const char* key, double default_value);
int get_config_bool(const char* key, int default_value);

// Konfigürasyon yönetimi
int add_config_entry(const char* key, const char* value);
int remove_config_entry(const char* key);
int config_key_exists(const char* key);
void print_all_config(void);

// Varsayılan konfigürasyon
int create_default_config(void);
int validate_config(void);
int backup_config(void);
int restore_config_backup(void);

// Konfigürasyon kategorileri
int load_system_config(void);
int load_logging_config(void);
int load_module_config(void);
int load_security_config(void);

#endif // CONFIG_H