/*
 * Sistem Otomasyon Merkezi - Ana Program
 * Bu program çeşitli sistem yönetimi ve otomasyon görevlerini gerçekleştirir.
 * 
 * Özellikler:
 * - Dosya yönetimi ve organizasyon
 * - Sistem izleme ve performans analizi
 * - Log analizi ve raporlama
 * - Otomatik yedekleme sistemi
 * - Ağ izleme ve bağlantı testleri
 * - Güvenlik tarama ve kontrol
 * - Görev zamanlayıcı
 * 
 * Tarih: 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

// Header dosyalarını include et
#include "../include/core.h"
#include "../include/logger.h"
#include "../include/config.h"
#include "../include/menu.h"
#include "../include/modules.h"
#include "../include/system_settings.h"
#include "../include/reports.h"

// Ana menü fonksiyonu
void show_main_menu() {
    system("cls");
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  SİSTEM OTOMASYON MERKEZİ                   ║\n");
    printf("║                           v1.0                               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  [1] Dosya Yönetimi ve Organizasyon                         ║\n");
    printf("║  [2] Sistem İzleme ve Performans                            ║\n");
    printf("║  [3] Log Analizi ve Raporlama                               ║\n");
    printf("║  [4] Otomatik Yedekleme Sistemi                             ║\n");
    printf("║  [5] Ağ İzleme ve Bağlantı Testleri                         ║\n");
    printf("║  [6] Güvenlik Tarama ve Kontrol                             ║\n");
    printf("║  [7] Görev Zamanlayıcı                                      ║\n");
    printf("║  [8] Sistem Ayarları ve Konfigürasyon                       ║\n");
    printf("║  [9] Raporları Görüntüle                                    ║\n");
    printf("║  [0] Çıkış                                                  ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nSeçiminizi yapın (0-9): ");
}

// Ana program
int main() {
    int choice;
    char input[10];
    
    // UTF-8 ve konsol ayarları
#ifdef _WIN32
    // Windows konsol UTF-8 ayarları
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // Konsol modunu ayarla (ANSI escape sequences için)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
    
    // Sistem başlatma
    printf("Sistem Otomasyon Merkezi başlatılıyor...\n");
    
    // Logger'ı başlat
    if (init_logger() == 0) {
        printf("HATA: Logger başlatılamadı!\n");
        return 1;
    }
    
    // Konfigürasyonu yükle
    if (load_config() == 0) {
        printf("HATA: Konfigürasyon yüklenemedi!\n");
        return 1;
    }
    
    log_info("Sistem Otomasyon Merkezi başlatıldı");
    
    // Ana döngü
    while (1) {
        show_main_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    run_file_management();
                    break;
                    
                case 2:
                    run_system_monitor();
                    break;
                    
                case 3:
                    run_log_analyzer();
                    break;
                    
                case 4:
                    run_backup_system();
                    break;
                    
                case 5:
                    run_network_monitor();
                    break;
                    
                case 6:
                    run_security_scanner();
                    break;
                    
                case 7:
                    run_task_scheduler();
                    break;
                    
                case 8:
                    run_system_settings();
                    break;
                    
                case 9:
                    run_reports();
                    break;
                    
                case 0:
                    printf("\nSistem kapatılıyor...\n");
                    log_info("Sistem Otomasyon Merkezi kapatıldı");
                    cleanup_logger();
                    return 0;
                    
                default:
                    printf("\nGeçersiz seçim! Lütfen 0-9 arası bir sayı girin.\n");
                    log_warning("Geçersiz menü seçimi: %d", choice);
                    break;
            }
            
            if (choice != 0) {
                printf("\nDevam etmek için bir tuşa basın...");
                getchar();
            }
        }
    }
    
    return 0;
}