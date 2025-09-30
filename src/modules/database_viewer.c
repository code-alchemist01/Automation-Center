/*
 * Database Viewer Module
 * Veritabanında saklanan verileri görüntüleme ve analiz etme modülü
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/logger.h"
#include "../../include/modules.h"
#include "../../include/database.h"

// Function prototypes
void show_database_menu();
void view_system_metrics();
void view_file_operations();
void view_network_metrics();
void view_security_scans();
void view_database_scheduled_tasks();
void generate_reports();
void database_statistics();
void export_data();
void cleanup_old_data();
void format_timestamp(time_t timestamp, char* buffer, size_t size);
void print_separator();

void run_database_viewer() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    DATABASE VIEWER                          ║\n");
    printf("║                Veritabanı Görüntüleyici                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (!db) {
        printf("\n❌ Veritabanı bağlantısı bulunamadı!\n");
        printf("💡 Lütfen önce veritabanını başlatın.\n");
        return;
    }
    
    int choice;
    do {
        show_database_menu();
        printf("\n🔍 Seçiminizi yapın: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("❌ Geçersiz giriş! Lütfen bir sayı girin.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch (choice) {
            case 1:
                view_system_metrics();
                break;
            case 2:
                view_file_operations();
                break;
            case 3:
                view_network_metrics();
                break;
            case 4:
                view_security_scans();
                break;
            case 5:
                view_database_scheduled_tasks();
                break;
            case 6:
                generate_reports();
                break;
            case 7:
                database_statistics();
                break;
            case 8:
                export_data();
                break;
            case 9:
                cleanup_old_data();
                break;
            case 0:
                printf("\n👋 Database Viewer'dan çıkılıyor...\n");
                break;
            default:
                printf("❌ Geçersiz seçim! Lütfen 0-9 arasında bir sayı girin.\n");
        }
        
        if (choice != 0) {
            printf("\n⏸️  Devam etmek için Enter tuşuna basın...");
            getchar(); // Clear newline
            getchar(); // Wait for Enter
        }
        
    } while (choice != 0);
    
    log_info("Database Viewer modülü kapatıldı");
}

void show_database_menu() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    DATABASE VIEWER MENÜ                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  1. 📊 Sistem Metrikleri                                    ║\n");
    printf("║  2. 📁 Dosya İşlemleri                                      ║\n");
    printf("║  3. 🌐 Ağ Metrikleri                                        ║\n");
    printf("║  4. 🔒 Güvenlik Taramaları                                  ║\n");
    printf("║  5. ⏰ Zamanlanmış Görevler                                 ║\n");
    printf("║  6. 📈 Raporlar Oluştur                                     ║\n");
    printf("║  7. 📋 Veritabanı İstatistikleri                            ║\n");
    printf("║  8. 💾 Veri Dışa Aktarma                                    ║\n");
    printf("║  9. 🧹 Eski Verileri Temizle                               ║\n");
    printf("║  0. 🚪 Çıkış                                                ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

void view_system_metrics() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      SİSTEM METRİKLERİ                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    SystemMetrics *metrics = NULL;
    int count = 0;
    
    if (!get_system_metrics_history(&metrics, &count, 50)) {
        printf("\n❌ Sistem metrikleri alınamadı!\n");
        return;
    }
    
    if (count == 0) {
        printf("\n📭 Henüz sistem metriği kaydı bulunamadı.\n");
        return;
    }
    
    printf("\n📊 Son %d sistem metriği:\n", count);
    print_separator();
    printf("%-20s %-10s %-12s %-12s %-20s\n", 
           "Zaman", "CPU (%)", "Bellek (%)", "Disk (%)", "Hostname");
    print_separator();
    
    for (int i = 0; i < count; i++) {
        char time_str[32];
        format_timestamp(metrics[i].timestamp, time_str, sizeof(time_str));
        
        printf("%-20s %-10.1f %-12.1f %-12.1f %-20s\n",
               time_str,
               metrics[i].cpu_usage,
               metrics[i].memory_usage,
               metrics[i].disk_usage,
               metrics[i].hostname);
    }
    
    print_separator();
    printf("📈 Toplam kayıt: %d\n", count);
    
    // Calculate averages
    double avg_cpu = 0, avg_memory = 0, avg_disk = 0;
    for (int i = 0; i < count; i++) {
        avg_cpu += metrics[i].cpu_usage;
        avg_memory += metrics[i].memory_usage;
        avg_disk += metrics[i].disk_usage;
    }
    
    if (count > 0) {
        avg_cpu /= count;
        avg_memory /= count;
        avg_disk /= count;
        
        printf("📊 Ortalamalar - CPU: %.1f%%, Bellek: %.1f%%, Disk: %.1f%%\n",
               avg_cpu, avg_memory, avg_disk);
    }
    
    free(metrics);
    log_info("Sistem metrikleri görüntülendi: %d kayıt", count);
}

void view_file_operations() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      DOSYA İŞLEMLERİ                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    FileOperation *operations = NULL;
    int count = 0;
    
    if (!get_file_operations_history(&operations, &count, 50)) {
        printf("\n❌ Dosya işlemleri alınamadı!\n");
        return;
    }
    
    if (count == 0) {
        printf("\n📭 Henüz dosya işlemi kaydı bulunamadı.\n");
        return;
    }
    
    printf("\n📁 Son %d dosya işlemi:\n", count);
    print_separator();
    printf("%-20s %-15s %-40s %-12s %-10s\n", 
           "Zaman", "İşlem", "Dosya Yolu", "Boyut", "Durum");
    print_separator();
    
    for (int i = 0; i < count; i++) {
        char time_str[32];
        format_timestamp(operations[i].timestamp, time_str, sizeof(time_str));
        
        // Truncate long paths
        char short_path[41];
        if (strlen(operations[i].file_path) > 40) {
            strncpy(short_path, operations[i].file_path, 37);
            short_path[37] = '.';
            short_path[38] = '.';
            short_path[39] = '.';
            short_path[40] = '\0';
        } else {
            strcpy(short_path, operations[i].file_path);
        }
        
        printf("%-20s %-15s %-40s %-12ld %-10s\n",
               time_str,
               operations[i].operation,
               short_path,
               operations[i].file_size,
               operations[i].status);
    }
    
    print_separator();
    printf("📈 Toplam kayıt: %d\n", count);
    
    free(operations);
    log_info("Dosya işlemleri görüntülendi: %d kayıt", count);
}

void view_network_metrics() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        AĞ METRİKLERİ                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    NetworkMetrics *metrics = NULL;
    int count = 0;
    
    if (!get_network_metrics_history(&metrics, &count, 50)) {
        printf("\n❌ Ağ metrikleri alınamadı!\n");
        return;
    }
    
    if (count == 0) {
        printf("\n📭 Henüz ağ metriği kaydı bulunamadı.\n");
        return;
    }
    
    printf("\n🌐 Son %d ağ metriği:\n", count);
    print_separator();
    printf("%-20s %-20s %-20s %-10s %-15s\n", 
           "Zaman", "Arayüz", "Hedef", "Ping (ms)", "Bağlantı");
    print_separator();
    
    for (int i = 0; i < count; i++) {
        char time_str[32];
        format_timestamp(metrics[i].timestamp, time_str, sizeof(time_str));
        
        printf("%-20s %-20s %-20s %-10d %-15s\n",
               time_str,
               metrics[i].interface_name,
               metrics[i].target,
               metrics[i].ping_time,
               metrics[i].connection_status ? "Başarılı" : "Başarısız");
    }
    
    print_separator();
    printf("📈 Toplam kayıt: %d\n", count);
    
    free(metrics);
    log_info("Ağ metrikleri görüntülendi: %d kayıt", count);
}

void view_security_scans() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    GÜVENLİK TARAMALARI                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    SecurityScan *scans = NULL;
    int count = 0;
    
    if (!get_security_scans_history(&scans, &count, 50)) {
        printf("\n❌ Güvenlik taramaları alınamadı!\n");
        return;
    }
    
    if (count == 0) {
        printf("\n📭 Henüz güvenlik taraması kaydı bulunamadı.\n");
        return;
    }
    
    printf("\n🔒 Son %d güvenlik taraması:\n", count);
    print_separator();
    printf("%-20s %-20s %-15s %-10s %-10s\n", 
           "Zaman", "Tarama Türü", "Hedef", "Tehdit", "Önem");
    print_separator();
    
    for (int i = 0; i < count; i++) {
        char time_str[32];
        format_timestamp(scans[i].timestamp, time_str, sizeof(time_str));
        
        printf("%-20s %-20s %-15s %-10d %-10s\n",
               time_str,
               scans[i].scan_type,
               scans[i].target,
               scans[i].threats_found,
               scans[i].severity);
    }
    
    print_separator();
    printf("📈 Toplam kayıt: %d\n", count);
    
    // Count threats by severity
    int critical = 0, high = 0, medium = 0, low = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(scans[i].severity, "CRITICAL") == 0) critical++;
        else if (strcmp(scans[i].severity, "HIGH") == 0) high++;
        else if (strcmp(scans[i].severity, "MEDIUM") == 0) medium++;
        else if (strcmp(scans[i].severity, "LOW") == 0) low++;
    }
    
    printf("🚨 Önem Dağılımı - Kritik: %d, Yüksek: %d, Orta: %d, Düşük: %d\n",
           critical, high, medium, low);
    
    free(scans);
    log_info("Güvenlik taramaları görüntülendi: %d kayıt", count);
}

void view_database_scheduled_tasks() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   ZAMANLANMIŞ GÖREVLER                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    ScheduledTask *tasks = NULL;
    int count = 0;
    
    if (!get_scheduled_tasks(&tasks, &count)) {
        printf("\n❌ Zamanlanmış görevler alınamadı!\n");
        return;
    }
    
    if (count == 0) {
        printf("\n📭 Henüz zamanlanmış görev kaydı bulunamadı.\n");
        return;
    }
    
    printf("\n⏰ Toplam %d zamanlanmış görev:\n", count);
    print_separator();
    printf("%-5s %-25s %-15s %-10s %-12s\n", 
           "ID", "Görev Adı", "Zamanlama", "Durum", "Etkin");
    print_separator();
    
    for (int i = 0; i < count; i++) {
        printf("%-5d %-25s %-15s %-10s %-12s\n",
               tasks[i].id,
               tasks[i].task_name,
               tasks[i].schedule,
               tasks[i].status,
               tasks[i].enabled ? "Evet" : "Hayır");
    }
    
    print_separator();
    printf("📈 Toplam kayıt: %d\n", count);
    
    // Count by status
    int created = 0, completed = 0, failed = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(tasks[i].status, "CREATED") == 0) created++;
        else if (strcmp(tasks[i].status, "COMPLETED") == 0) completed++;
        else if (strcmp(tasks[i].status, "FAILED") == 0) failed++;
    }
    
    printf("📊 Durum Dağılımı - Oluşturulan: %d, Tamamlanan: %d, Başarısız: %d\n",
           created, completed, failed);
    
    free(tasks);
    log_info("Zamanlanmış görevler görüntülendi: %d kayıt", count);
}

void generate_reports() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      RAPOR OLUŞTURMA                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n📈 Özet rapor oluşturuluyor...\n");
    
    // Get counts from each table
    SystemMetrics *sys_metrics = NULL;
    FileOperation *file_ops = NULL;
    NetworkMetrics *net_metrics = NULL;
    SecurityScan *sec_scans = NULL;
    ScheduledTask *tasks = NULL;
    
    int sys_count = 0, file_count = 0, net_count = 0, sec_count = 0, task_count = 0;
    
    get_system_metrics_history(&sys_metrics, &sys_count, 1000);
    get_file_operations_history(&file_ops, &file_count, 1000);
    get_network_metrics_history(&net_metrics, &net_count, 1000);
    get_security_scans_history(&sec_scans, &sec_count, 1000);
    get_scheduled_tasks(&tasks, &task_count);
    
    printf("\n📊 VERİTABANI ÖZET RAPORU\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("📈 Sistem Metrikleri      : %d kayıt\n", sys_count);
    printf("📁 Dosya İşlemleri        : %d kayıt\n", file_count);
    printf("🌐 Ağ Metrikleri          : %d kayıt\n", net_count);
    printf("🔒 Güvenlik Taramaları    : %d kayıt\n", sec_count);
    printf("⏰ Zamanlanmış Görevler   : %d kayıt\n", task_count);
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("📊 TOPLAM KAYIT           : %d\n", 
           sys_count + file_count + net_count + sec_count + task_count);
    
    // Generate timestamp for report
    time_t now = time(NULL);
    char time_str[64];
    format_timestamp(now, time_str, sizeof(time_str));
    printf("🕒 Rapor Tarihi           : %s\n", time_str);
    
    // Free allocated memory
    if (sys_metrics) free(sys_metrics);
    if (file_ops) free(file_ops);
    if (net_metrics) free(net_metrics);
    if (sec_scans) free(sec_scans);
    if (tasks) free(tasks);
    
    log_info("Database özet raporu oluşturuldu");
}

void database_statistics() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  VERİTABANI İSTATİSTİKLERİ                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n📊 Veritabanı istatistikleri hesaplanıyor...\n");
    
    // This would require additional SQL queries to get table sizes, etc.
    // For now, we'll show basic information
    
    printf("\n📈 VERİTABANI BİLGİLERİ\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("📂 Veritabanı Dosyası     : data/automation.db\n");
    printf("🔧 SQLite Versiyonu       : %s\n", sqlite3_libversion());
    printf("📊 Aktif Bağlantı         : %s\n", db ? "Evet" : "Hayır");
    
    // Get database file size (approximate)
    FILE *db_file = fopen("data/automation.db", "rb");
    if (db_file) {
        fseek(db_file, 0, SEEK_END);
        long size = ftell(db_file);
        fclose(db_file);
        printf("💾 Dosya Boyutu           : %ld bytes (%.2f KB)\n", 
               size, (double)size / 1024.0);
    }
    
    printf("═══════════════════════════════════════════════════════════════\n");
    
    log_info("Veritabanı istatistikleri görüntülendi");
}

void export_data() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      VERİ DIŞA AKTARMA                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n💾 Veri dışa aktarma özelliği geliştirilme aşamasında...\n");
    printf("🔧 Bu özellik gelecek sürümlerde eklenecektir.\n");
    
    // Future implementation:
    // - Export to CSV
    // - Export to JSON
    // - Export to XML
    // - Backup database
    
    log_info("Veri dışa aktarma menüsü görüntülendi");
}

void cleanup_old_data() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ESKİ VERİLERİ TEMİZLE                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n🧹 Kaç günden eski verileri temizlemek istiyorsunuz? (varsayılan: 30): ");
    
    int days = 30;
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int temp = atoi(input);
        if (temp > 0 && temp <= 365) {
            days = temp;
        }
    }
    
    printf("\n⚠️  %d günden eski tüm veriler silinecek!\n", days);
    printf("❓ Devam etmek istediğinizden emin misiniz? (E/H): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        if (input[0] == 'E' || input[0] == 'e') {
            printf("\n🧹 Eski veriler temizleniyor...\n");
            
            if (cleanup_old_records(days)) {
                printf("✅ %d günden eski veriler başarıyla temizlendi!\n", days);
                
                // Vacuum database to reclaim space
                if (vacuum_database()) {
                    printf("🗜️  Veritabanı optimize edildi.\n");
                }
            } else {
                printf("❌ Veri temizleme işlemi başarısız!\n");
            }
        } else {
            printf("❌ Temizleme işlemi iptal edildi.\n");
        }
    }
    
    log_info("Eski veri temizleme işlemi: %d gün", days);
}

void format_timestamp(time_t timestamp, char* buffer, size_t size) {
    struct tm *tm_info = localtime(&timestamp);
    strftime(buffer, size, "%Y-%m-%d %H:%M", tm_info);
}

void print_separator() {
    printf("────────────────────────────────────────────────────────────────\n");
}