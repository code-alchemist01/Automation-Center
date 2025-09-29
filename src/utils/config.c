/*
 * ========================================
 * Config Implementation - Konfigürasyon Yönetimi
 * ========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#include "../../include/config.h"
#include "../../include/logger.h"

// Forward declarations
int create_config_directory(void);

// Global konfigürasyon
static Config g_config = {0};
static int g_config_initialized = 0;

// Konfigürasyon başlatma
int init_config(void) {
    if (g_config_initialized) {
        return 1; // Zaten başlatılmış
    }
    
    // Konfigürasyon dosyası yolunu ayarla
    strcpy(g_config.config_file_path, CONFIG_FILE_PATH);
    g_config.entry_count = 0;
    
    g_config_initialized = 1;
    return 1;
}

// Konfigürasyon yükleme
int load_config(void) {
    if (!g_config_initialized) {
        init_config();
    }
    
    FILE* file = fopen(g_config.config_file_path, "r");
    if (!file) {
        // Konfigürasyon dosyası yoksa varsayılan oluştur
        create_default_config();
        file = fopen(g_config.config_file_path, "r");
        if (!file) {
            return 0; // Hata
        }
    }
    
    char line[512];
    g_config.entry_count = 0;
    
    while (fgets(line, sizeof(line), file) && g_config.entry_count < MAX_CONFIG_ENTRIES) {
        // Boş satırları ve yorumları atla
        if (line[0] == '\n' || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Satır sonundaki newline'ı kaldır
        line[strcspn(line, "\n")] = 0;
        
        // Key=Value formatını parse et
        char* equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';
            char* key = line;
            char* value = equals + 1;
            
            // Boşlukları temizle
            while (isspace(*key)) key++;
            while (isspace(*value)) value++;
            
            // Trailing boşlukları kaldır
            char* end = key + strlen(key) - 1;
            while (end > key && isspace(*end)) *end-- = '\0';
            
            end = value + strlen(value) - 1;
            while (end > value && isspace(*end)) *end-- = '\0';
            
            // Konfigürasyon girişini ekle
            if (strlen(key) > 0 && strlen(value) > 0) {
                strcpy(g_config.entries[g_config.entry_count].key, key);
                strcpy(g_config.entries[g_config.entry_count].value, value);
                g_config.entry_count++;
            }
        }
    }
    
    fclose(file);
    return 1;
}

// Konfigürasyon kaydetme
int save_config(void) {
    if (!g_config_initialized) {
        return 0;
    }
    
    // Dizini oluştur
    create_config_directory();
    
    FILE* file = fopen(g_config.config_file_path, "w");
    if (!file) {
        return 0;
    }
    
    fprintf(file, "# ========================================\n");
    fprintf(file, "# Sistem Otomasyon Merkezi Konfigürasyonu\n");
    fprintf(file, "# Otomatik oluşturuldu - Manuel düzenleme yapabilirsiniz\n");
    fprintf(file, "# ========================================\n\n");
    
    for (int i = 0; i < g_config.entry_count; i++) {
        fprintf(file, "%s=%s\n", g_config.entries[i].key, g_config.entries[i].value);
    }
    
    fclose(file);
    return 1;
}

// Konfigürasyon yeniden yükleme
int reload_config(void) {
    g_config.entry_count = 0;
    return load_config();
}

// Konfigürasyon değeri alma
const char* get_config_value(const char* key) {
    if (!key || !g_config_initialized) {
        return NULL;
    }
    
    for (int i = 0; i < g_config.entry_count; i++) {
        if (strcmp(g_config.entries[i].key, key) == 0) {
            return g_config.entries[i].value;
        }
    }
    
    return NULL;
}

// Konfigürasyon değeri ayarlama
int set_config_value(const char* key, const char* value) {
    if (!key || !value || !g_config_initialized) {
        return 0;
    }
    
    // Mevcut girişi bul
    for (int i = 0; i < g_config.entry_count; i++) {
        if (strcmp(g_config.entries[i].key, key) == 0) {
            strcpy(g_config.entries[i].value, value);
            return 1;
        }
    }
    
    // Yeni giriş ekle
    if (g_config.entry_count < MAX_CONFIG_ENTRIES) {
        strcpy(g_config.entries[g_config.entry_count].key, key);
        strcpy(g_config.entries[g_config.entry_count].value, value);
        g_config.entry_count++;
        return 1;
    }
    
    return 0; // Maksimum giriş sayısına ulaşıldı
}

// Integer değer alma
int get_config_int(const char* key, int default_value) {
    const char* value = get_config_value(key);
    if (value) {
        return atoi(value);
    }
    return default_value;
}

// Double değer alma
double get_config_double(const char* key, double default_value) {
    const char* value = get_config_value(key);
    if (value) {
        return atof(value);
    }
    return default_value;
}

// Boolean değer alma
int get_config_bool(const char* key, int default_value) {
    const char* value = get_config_value(key);
    if (value) {
        if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcmp(value, "yes") == 0) {
            return 1;
        } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcmp(value, "no") == 0) {
            return 0;
        }
    }
    return default_value;
}

// Konfigürasyon girişi ekleme
int add_config_entry(const char* key, const char* value) {
    return set_config_value(key, value);
}

// Konfigürasyon girişi kaldırma
int remove_config_entry(const char* key) {
    if (!key || !g_config_initialized) {
        return 0;
    }
    
    for (int i = 0; i < g_config.entry_count; i++) {
        if (strcmp(g_config.entries[i].key, key) == 0) {
            // Girişi kaldır ve diziyi kaydır
            for (int j = i; j < g_config.entry_count - 1; j++) {
                strcpy(g_config.entries[j].key, g_config.entries[j + 1].key);
                strcpy(g_config.entries[j].value, g_config.entries[j + 1].value);
            }
            g_config.entry_count--;
            return 1;
        }
    }
    
    return 0;
}

// Anahtar varlığını kontrol etme
int config_key_exists(const char* key) {
    return get_config_value(key) != NULL;
}

// Tüm konfigürasyonu yazdırma
void print_all_config(void) {
    printf("\n=== Konfigürasyon Ayarları ===\n");
    for (int i = 0; i < g_config.entry_count; i++) {
        printf("%s = %s\n", g_config.entries[i].key, g_config.entries[i].value);
    }
    printf("==============================\n\n");
}

// Varsayılan konfigürasyon oluşturma
int create_default_config(void) {
    if (!g_config_initialized) {
        init_config();
    }
    
    // Dizini oluştur
    create_config_directory();
    
    // Varsayılan değerleri ayarla
    set_config_value("system.version", "1.0.0");
    set_config_value("system.debug_mode", "false");
    set_config_value("system.auto_backup", "true");
    set_config_value("system.backup_interval", "24");
    
    set_config_value("logging.level", "INFO");
    set_config_value("logging.console_output", "true");
    set_config_value("logging.file_output", "true");
    set_config_value("logging.max_file_size", "10");
    set_config_value("logging.max_backup_files", "5");
    
    set_config_value("file_management.auto_organize", "false");
    set_config_value("file_management.organize_interval", "60");
    set_config_value("file_management.backup_before_organize", "true");
    
    set_config_value("monitoring.cpu_threshold", "80");
    set_config_value("monitoring.memory_threshold", "85");
    set_config_value("monitoring.disk_threshold", "90");
    set_config_value("monitoring.check_interval", "30");
    
    set_config_value("network.ping_hosts", "8.8.8.8,1.1.1.1");
    set_config_value("network.ping_interval", "60");
    set_config_value("network.timeout", "5");
    
    set_config_value("security.scan_enabled", "true");
    set_config_value("security.scan_interval", "120");
    set_config_value("security.quarantine_suspicious", "false");
    
    set_config_value("reporting.auto_generate", "true");
    set_config_value("reporting.report_interval", "24");
    set_config_value("reporting.format", "html");
    
    return save_config();
}

// Konfigürasyon doğrulama
int validate_config(void) {
    // Temel doğrulama kontrolü
    return g_config_initialized && g_config.entry_count > 0;
}

// Konfigürasyon yedekleme
int backup_config(void) {
    if (!g_config_initialized) {
        return 0;
    }
    
    char backup_path[512];
    snprintf(backup_path, sizeof(backup_path), "%s.backup", g_config.config_file_path);
    
    FILE* source = fopen(g_config.config_file_path, "r");
    FILE* dest = fopen(backup_path, "w");
    
    if (!source || !dest) {
        if (source) fclose(source);
        if (dest) fclose(dest);
        return 0;
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), source)) {
        fputs(buffer, dest);
    }
    
    fclose(source);
    fclose(dest);
    
    return 1;
}

// Konfigürasyon yedeğini geri yükleme
int restore_config_backup(void) {
    if (!g_config_initialized) {
        return 0;
    }
    
    char backup_path[512];
    snprintf(backup_path, sizeof(backup_path), "%s.backup", g_config.config_file_path);
    
    FILE* source = fopen(backup_path, "r");
    FILE* dest = fopen(g_config.config_file_path, "w");
    
    if (!source || !dest) {
        if (source) fclose(source);
        if (dest) fclose(dest);
        return 0;
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), source)) {
        fputs(buffer, dest);
    }
    
    fclose(source);
    fclose(dest);
    
    return reload_config();
}

// Sistem konfigürasyonu yükleme
int load_system_config(void) {
    // Sistem ayarlarını yükle
    return 1;
}

// Loglama konfigürasyonu yükleme
int load_logging_config(void) {
    // Loglama ayarlarını yükle
    return 1;
}

// Modül konfigürasyonu yükleme
int load_module_config(void) {
    // Modül ayarlarını yükle
    return 1;
}

// Güvenlik konfigürasyonu yükleme
int load_security_config(void) {
    // Güvenlik ayarlarını yükle
    return 1;
}

// Konfigürasyon dizini oluşturma
int create_config_directory(void) {
    char dir_path[256];
    strcpy(dir_path, g_config.config_file_path);
    
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