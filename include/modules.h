/*
 * ========================================
 * Modül Fonksiyonları Header Dosyası
 * ========================================
 */

#ifndef MODULES_H
#define MODULES_H

#include <stdbool.h>

// Dosya Yönetimi Modülü
void run_file_management();
bool init_file_management();
void cleanup_file_management();

// Sistem İzleme Modülü
void run_system_monitor();
bool init_system_monitor();
void cleanup_system_monitor();

// Log Analizi Modülü
void run_log_analyzer();

// Yedekleme Modülü
void run_backup_system();
bool init_backup_system();
void cleanup_backup_system();

// Ağ İzleme Modülü
void run_network_monitor();
bool init_network_monitor();
void cleanup_network_monitor();

// Güvenlik Modülü
void run_security_scanner();
bool init_security_scanner();
void cleanup_security_scanner();

// Görev Zamanlayıcı Modülü
void run_task_scheduler();
bool init_task_scheduler();
void cleanup_task_scheduler();

// Sistem Ayarları Modülü
void run_system_settings();

// Raporlama Modülü
void run_reports();

// Database Viewer Modülü
void run_database_viewer();

#endif // MODULES_H