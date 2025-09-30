#include "../include/core.h"
#include "../include/logger.h"
#include "../include/database.h"
#include "../include/web_server.h"
#include "../include/modules.h"
#include "../include/reports.h"
#include "../include/system_settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

// Global variables
static bool running = true;
static bool web_server_enabled = false;

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\n🛑 Sistem kapatılıyor...\n");
        running = false;
        if (web_server_enabled) {
            stop_web_server();
        }
    }
}

// Display main menu
void display_menu() {
    printf("\n" COLOR_CYAN "========================================\n");
    printf("🖥️  SİSTEM OTOMASYON MERKEZİ v%s\n", VERSION);
    printf("========================================" COLOR_RESET "\n");
    printf(COLOR_GREEN "1.  📊 Sistem İzleme\n");
    printf("2.  📁 Dosya Yönetimi\n");
    printf("3.  🌐 Ağ İzleme\n");
    printf("4.  🔒 Güvenlik Taraması\n");
    printf("5.  ⏰ Görev Zamanlayıcı\n");
    printf("6.  💾 Yedekleme Sistemi\n");
    printf("7.  📋 Log Analizi\n");
    printf("8.  📈 Raporlar\n");
    printf("9.  ⚙️  Sistem Ayarları\n");
    printf("10. 🗄️  Veritabanı Görüntüleyici\n");
    printf("11. 🌐 Web Dashboard (Başlat/Durdur)\n" COLOR_RESET);
    printf(COLOR_RED "0.  🚪 Çıkış\n" COLOR_RESET);
    printf("\n" COLOR_YELLOW "Seçiminizi yapın: " COLOR_RESET);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf(COLOR_CYAN "🚀 Sistem Otomasyon Merkezi başlatılıyor...\n" COLOR_RESET);

    // Initialize logger
    if (!init_logger()) {
        printf(COLOR_RED "❌ Logger başlatılamadı!\n" COLOR_RESET);
        return 1;
    }
    log_info("Logger initialized successfully");

    // Initialize database
    if (!init_database()) {
        printf(COLOR_RED "❌ Veritabanı başlatılamadı!\n" COLOR_RESET);
        cleanup_logger();
        return 1;
    }
    log_info("Database initialized successfully");

    // Initialize modules
    printf(COLOR_GREEN "✅ Modüller yükleniyor...\n" COLOR_RESET);
    
    if (!init_system_monitor()) {
        log_warning("System monitor initialization failed");
    }
    
    if (!init_file_management()) {
        log_warning("File management initialization failed");
    }
    
    if (!init_network_monitor()) {
        log_warning("Network monitor initialization failed");
    }
    
    if (!init_security_scanner()) {
        log_warning("Security scanner initialization failed");
    }
    
    if (!init_task_scheduler()) {
        log_warning("Task scheduler initialization failed");
    }
    
    if (!init_backup_system()) {
        log_warning("Backup system initialization failed");
    }

    printf(COLOR_GREEN "✅ Sistem başarıyla başlatıldı!\n" COLOR_RESET);
    log_info("System startup completed successfully");

    // Main program loop
    int choice;
    while (running) {
        display_menu();
        
        if (scanf("%d", &choice) != 1) {
            // Clear invalid input
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf(COLOR_RED "❌ Geçersiz giriş! Lütfen bir sayı girin.\n" COLOR_RESET);
            continue;
        }

        // Clear input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        switch (choice) {
            case 1:
                printf(COLOR_CYAN "\n📊 Sistem İzleme Modülü\n" COLOR_RESET);
                run_system_monitor();
                break;
                
            case 2:
                printf(COLOR_CYAN "\n📁 Dosya Yönetimi Modülü\n" COLOR_RESET);
                run_file_management();
                break;
                
            case 3:
                printf(COLOR_CYAN "\n🌐 Ağ İzleme Modülü\n" COLOR_RESET);
                run_network_monitor();
                break;
                
            case 4:
                printf(COLOR_CYAN "\n🔒 Güvenlik Taraması Modülü\n" COLOR_RESET);
                run_security_scanner();
                break;
                
            case 5:
                printf(COLOR_CYAN "\n⏰ Görev Zamanlayıcı Modülü\n" COLOR_RESET);
                run_task_scheduler();
                break;
                
            case 6:
                printf(COLOR_CYAN "\n💾 Yedekleme Sistemi Modülü\n" COLOR_RESET);
                run_backup_system();
                break;
                
            case 7:
                printf(COLOR_CYAN "\n📋 Log Analizi Modülü\n" COLOR_RESET);
                run_log_analyzer();
                break;
                
            case 8:
                printf(COLOR_CYAN "\n📈 Raporlar Modülü\n" COLOR_RESET);
                run_reports();
                break;
                
            case 9:
                printf(COLOR_CYAN "\n⚙️ Sistem Ayarları Modülü\n" COLOR_RESET);
                run_system_settings();
                break;
                
            case 10:
                printf(COLOR_CYAN "\n🗄️ Veritabanı Görüntüleyici Modülü\n" COLOR_RESET);
                run_database_viewer();
                break;
                
            case 11:
                if (!web_server_enabled) {
                    printf(COLOR_CYAN "\n🌐 Web Dashboard başlatılıyor...\n" COLOR_RESET);
                    if (init_web_server(8080)) {
                        printf(COLOR_GREEN "✅ Web server başlatıldı: http://localhost:8080\n" COLOR_RESET);
                        printf(COLOR_YELLOW "💡 Dashboard'a tarayıcınızdan erişebilirsiniz!\n" COLOR_RESET);
                        log_info("Web dashboard started on port 8080");
                        web_server_enabled = true;
                        
                        // Start web server in a separate thread (simplified approach)
                        printf(COLOR_YELLOW "⚠️  Web server çalışıyor. Ana menüye dönmek için Enter'a basın.\n" COLOR_RESET);
                        getchar(); // Wait for user input
                    } else {
                        printf(COLOR_RED "❌ Web server başlatılamadı!\n" COLOR_RESET);
                        log_error("Failed to start web dashboard");
                    }
                } else {
                    printf(COLOR_CYAN "\n🛑 Web Dashboard durduruluyor...\n" COLOR_RESET);
                    stop_web_server();
                    cleanup_web_server();
                    web_server_enabled = false;
                    printf(COLOR_GREEN "✅ Web server durduruldu.\n" COLOR_RESET);
                    log_info("Web dashboard stopped");
                }
                break;
                
            case 0:
                printf(COLOR_YELLOW "\n👋 Sistem kapatılıyor...\n" COLOR_RESET);
                running = false;
                break;
                
            default:
                printf(COLOR_RED "❌ Geçersiz seçim! Lütfen 0-11 arasında bir sayı girin.\n" COLOR_RESET);
                break;
        }
        
        if (running && choice != 11) {
            printf(COLOR_YELLOW "\nDevam etmek için Enter'a basın..." COLOR_RESET);
            getchar();
        }
    }

    // Cleanup
    printf(COLOR_CYAN "🧹 Sistem temizleniyor...\n" COLOR_RESET);
    
    if (web_server_enabled) {
        stop_web_server();
        cleanup_web_server();
    }
    
    cleanup_system_monitor();
    cleanup_file_management();
    cleanup_network_monitor();
    cleanup_security_scanner();
    cleanup_task_scheduler();
    cleanup_backup_system();
    
    close_database();
    cleanup_logger();
    
    printf(COLOR_GREEN "✅ Sistem başarıyla kapatıldı. Güle güle!\n" COLOR_RESET);
    return 0;
}