/*
 * ========================================
 * Menu Header - Kullanıcı Arayüzü
 * ========================================
 */

#ifndef MENU_H
#define MENU_H

#include "core.h"

// Menü seçenekleri
typedef enum {
    MENU_FILE_MANAGEMENT = 1,
    MENU_SYSTEM_MONITORING = 2,
    MENU_LOG_ANALYSIS = 3,
    MENU_AUTO_BACKUP = 4,
    MENU_NETWORK_MONITORING = 5,
    MENU_SECURITY_SCAN = 6,
    MENU_TASK_SCHEDULER = 7,
    MENU_SYSTEM_SETTINGS = 8,
    MENU_VIEW_REPORTS = 9,
    MENU_EXIT = 0
} MenuOption;

// Menü yapısı
typedef struct {
    int option;
    char title[256];
    char description[512];
    int (*handler)(void);
} MenuItem;

// Fonksiyon prototipleri
void show_main_menu(void);
void show_file_management_menu(void);
void show_system_monitoring_menu(void);
void show_log_analysis_menu(void);
void show_backup_menu(void);
void show_network_menu(void);
void show_security_menu(void);
void show_scheduler_menu(void);
void show_settings_menu(void);
void show_reports_menu(void);

// Menü işleyicileri
int handle_file_management(void);
int handle_system_monitoring(void);
int handle_log_analysis(void);
int handle_auto_backup(void);
int handle_network_monitoring(void);
int handle_security_scan(void);
int handle_task_scheduler(void);
int handle_system_settings(void);
int handle_view_reports(void);

// Yardımcı fonksiyonlar
int get_menu_choice(void);
void display_menu_header(const char* title);
void display_menu_footer(void);
void display_loading_animation(const char* message);
void display_progress_bar(int current, int total);

// Alt menüler
void show_file_operations_submenu(void);
void show_monitoring_options_submenu(void);
void show_backup_options_submenu(void);
void show_security_options_submenu(void);

#endif // MENU_H