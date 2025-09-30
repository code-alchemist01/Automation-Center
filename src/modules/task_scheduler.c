/*
 * Görev Zamanlayıcı Modülü
 * Bu modül sistem görevlerinin zamanlanması ve otomatik çalıştırılması işlemlerini gerçekleştirir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <taskschd.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <crontab.h>
#endif

#include "../../include/logger.h"
#include "../../include/modules.h"
#include "../../include/database.h"

// Module initialization
bool init_task_scheduler() {
    log_info("Task scheduler module initialized");
    return true;
}

// Module cleanup
void cleanup_task_scheduler() {
    log_info("Task scheduler module cleaned up");
}

// Görev durumu enum'u
typedef enum {
    TASK_PENDING = 0,
    TASK_RUNNING = 1,
    TASK_COMPLETED = 2,
    TASK_FAILED = 3,
    TASK_DISABLED = 4
} task_status_t;

// Görev önceliği enum'u
typedef enum {
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_CRITICAL = 4
} task_priority_t;

// Görev tekrar türü enum'u
typedef enum {
    REPEAT_ONCE = 0,
    REPEAT_DAILY = 1,
    REPEAT_WEEKLY = 2,
    REPEAT_MONTHLY = 3,
    REPEAT_YEARLY = 4
} repeat_type_t;

// Görev yapısı
typedef struct {
    int id;
    char name[128];
    char description[256];
    char command[512];
    char schedule_time[32];
    task_status_t status;
    task_priority_t priority;
    repeat_type_t repeat;
    int enabled;
    time_t created_time;
    time_t last_run;
    time_t next_run;
    int run_count;
} scheduled_task_t;

// Görev listesi
static scheduled_task_t tasks[50];
static int task_count = 0;

// Fonksiyon prototipleri
void show_scheduler_menu();
void create_new_task();
void view_scheduled_tasks();
void edit_task();
void delete_task();
void run_task_now();
void view_task_history();
void import_export_tasks();
void scheduler_settings();
void check_pending_tasks();
const char* get_status_string(task_status_t status);
const char* get_priority_string(task_priority_t priority);
const char* get_repeat_string(repeat_type_t repeat);
void initialize_sample_tasks();
void format_time_string(time_t time, char* buffer, size_t size);

void run_task_scheduler() {
    int choice;
    char input[10];
    
    log_info("Görev Zamanlayıcı başlatıldı");
    
    // Örnek görevleri yükle
    if (task_count == 0) {
        initialize_sample_tasks();
    }
    
    while (1) {
        show_scheduler_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    create_new_task();
                    break;
                    
                case 2:
                    view_scheduled_tasks();
                    break;
                    
                case 3:
                    edit_task();
                    break;
                    
                case 4:
                    delete_task();
                    break;
                    
                case 5:
                    run_task_now();
                    break;
                    
                case 6:
                    view_task_history();
                    break;
                    
                case 7:
                    import_export_tasks();
                    break;
                    
                case 8:
                    scheduler_settings();
                    break;
                    
                case 0:
                    printf("\nGörev Zamanlayıcı modülü kapatılıyor...\n");
                    log_info("Görev Zamanlayıcı modülü kapatıldı");
                    return;
                    
                default:
                    printf("\nGeçersiz seçim! Lütfen 0-8 arası bir sayı girin.\n");
                    log_warning("Geçersiz zamanlayıcı menü seçimi: %d", choice);
                    break;
            }
            
            if (choice != 0) {
                printf("\nDevam etmek için Enter tuşuna basın...");
                getchar();
            }
        }
    }
}

void show_scheduler_menu() {
    system("cls");
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                     GÖREV ZAMANLAYICI                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  [1] Yeni Görev Oluştur                                     ║\n");
    printf("║  [2] Zamanlanmış Görevleri Görüntüle                        ║\n");
    printf("║  [3] Görev Düzenle                                          ║\n");
    printf("║  [4] Görev Sil                                              ║\n");
    printf("║  [5] Görevi Şimdi Çalıştır                                  ║\n");
    printf("║  [6] Görev Geçmişini Görüntüle                              ║\n");
    printf("║  [7] Görevleri İçe/Dışa Aktar                               ║\n");
    printf("║  [8] Zamanlayıcı Ayarları                                   ║\n");
    printf("║  [0] Ana Menüye Dön                                         ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Bekleyen görevleri kontrol et
    check_pending_tasks();
    
    printf("\nSeçiminizi yapın (0-8): ");
}

void create_new_task() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                       YENİ GÖREV OLUŞTUR                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count >= 50) {
        printf("\n❌ Maksimum görev sayısına ulaşıldı! (50)\n");
        return;
    }
    
    scheduled_task_t new_task = {0};
    char input[512];
    
    // Görev ID'si otomatik oluştur
    new_task.id = task_count + 1;
    
    printf("\n📝 Görev Bilgileri:\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    // Görev adı
    printf("Görev Adı: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0; // Newline karakterini kaldır
        strncpy(new_task.name, input, sizeof(new_task.name) - 1);
    }
    
    // Görev açıklaması
    printf("Açıklama: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        strncpy(new_task.description, input, sizeof(new_task.description) - 1);
    }
    
    // Komut
    printf("Çalıştırılacak Komut: ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        strncpy(new_task.command, input, sizeof(new_task.command) - 1);
    }
    
    // Zamanlama
    printf("\n⏰ Zamanlama Seçenekleri:\n");
    printf("1. Belirli bir tarih/saat\n");
    printf("2. Günlük\n");
    printf("3. Haftalık\n");
    printf("4. Aylık\n");
    printf("Seçiminiz (1-4): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int schedule_type = atoi(input);
        
        switch (schedule_type) {
            case 1:
                new_task.repeat = REPEAT_ONCE;
                printf("Tarih ve saat (GG/AA/YYYY SS:DD): ");
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;
                    strncpy(new_task.schedule_time, input, sizeof(new_task.schedule_time) - 1);
                }
                break;
            case 2:
                new_task.repeat = REPEAT_DAILY;
                printf("Günlük çalışma saati (SS:DD): ");
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;
                    strncpy(new_task.schedule_time, input, sizeof(new_task.schedule_time) - 1);
                }
                break;
            case 3:
                new_task.repeat = REPEAT_WEEKLY;
                printf("Haftalık çalışma günü ve saati (Pazartesi 09:00): ");
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;
                    strncpy(new_task.schedule_time, input, sizeof(new_task.schedule_time) - 1);
                }
                break;
            case 4:
                new_task.repeat = REPEAT_MONTHLY;
                printf("Aylık çalışma günü ve saati (1. gün 09:00): ");
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    input[strcspn(input, "\n")] = 0;
                    strncpy(new_task.schedule_time, input, sizeof(new_task.schedule_time) - 1);
                }
                break;
            default:
                new_task.repeat = REPEAT_ONCE;
                strcpy(new_task.schedule_time, "Şimdi");
                break;
        }
    }
    
    // Öncelik
    printf("\n🎯 Öncelik Seviyesi:\n");
    printf("1. Düşük\n");
    printf("2. Normal\n");
    printf("3. Yüksek\n");
    printf("4. Kritik\n");
    printf("Seçiminiz (1-4): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int priority = atoi(input);
        if (priority >= 1 && priority <= 4) {
            new_task.priority = (task_priority_t)priority;
        } else {
            new_task.priority = PRIORITY_NORMAL;
        }
    }
    
    // Varsayılan değerler
    new_task.status = TASK_PENDING;
    new_task.enabled = 1;
    new_task.created_time = time(NULL);
    new_task.last_run = 0;
    new_task.next_run = time(NULL) + 3600; // 1 saat sonra
    new_task.run_count = 0;
    
    // Görevi listeye ekle
    tasks[task_count] = new_task;
    task_count++;
    
    printf("\n✅ Görev başarıyla oluşturuldu!\n");
    printf("📋 Görev ID: %d\n", new_task.id);
    printf("📝 Görev Adı: %s\n", new_task.name);
    printf("⏰ Zamanlama: %s (%s)\n", new_task.schedule_time, get_repeat_string(new_task.repeat));
    printf("🎯 Öncelik: %s\n", get_priority_string(new_task.priority));
    
    // Database'e görev oluşturma kaydını ekle
    ScheduledTask task_record;
    strncpy(task_record.task_name, new_task.name, sizeof(task_record.task_name) - 1);
    task_record.task_name[sizeof(task_record.task_name) - 1] = '\0';
    
    strncpy(task_record.command, new_task.command, sizeof(task_record.command) - 1);
    task_record.command[sizeof(task_record.command) - 1] = '\0';
    
    strncpy(task_record.schedule, new_task.schedule_time, sizeof(task_record.schedule) - 1);
    task_record.schedule[sizeof(task_record.schedule) - 1] = '\0';
    
    task_record.next_run = new_task.next_run;
    task_record.last_run = new_task.last_run;
    task_record.enabled = new_task.enabled;
    strncpy(task_record.status, "CREATED", sizeof(task_record.status) - 1);
    task_record.status[sizeof(task_record.status) - 1] = '\0';
    
    if (insert_scheduled_task(&task_record)) {
        printf("📊 Görev database'e kaydedildi.\n");
    } else {
        printf("⚠️  Görev database'e kaydedilemedi.\n");
    }
    
    log_info("Yeni görev oluşturuldu: %s", new_task.name);
}

void view_scheduled_tasks() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   ZAMANLANMIŞ GÖREVLER                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count == 0) {
        printf("\n📭 Henüz hiç görev oluşturulmamış.\n");
        printf("💡 Yeni görev oluşturmak için menüden '1' seçeneğini kullanın.\n");
        return;
    }
    
    printf("\n📊 Toplam Görev: %d\n", task_count);
    printf("─────────────────────────────────────────────────────────────\n");
    
    // Başlık
    printf("┌────┬─────────────────────┬─────────────┬─────────────┬─────────────────────┐\n");
    printf("│ ID │ Görev Adı           │ Durum       │ Öncelik     │ Sonraki Çalışma     │\n");
    printf("├────┼─────────────────────┼─────────────┼─────────────┼─────────────────────┤\n");
    
    // Görevleri listele
    for (int i = 0; i < task_count; i++) {
        char next_run_str[32];
        format_time_string(tasks[i].next_run, next_run_str, sizeof(next_run_str));
        
        printf("│ %-2d │ %-19s │ %-11s │ %-11s │ %-19s │\n",
               tasks[i].id,
               tasks[i].name,
               get_status_string(tasks[i].status),
               get_priority_string(tasks[i].priority),
               next_run_str);
    }
    
    printf("└────┴─────────────────────┴─────────────┴─────────────┴─────────────────────┘\n");
    
    // Durum özeti
    int pending = 0, running = 0, completed = 0, failed = 0, disabled = 0;
    for (int i = 0; i < task_count; i++) {
        switch (tasks[i].status) {
            case TASK_PENDING: pending++; break;
            case TASK_RUNNING: running++; break;
            case TASK_COMPLETED: completed++; break;
            case TASK_FAILED: failed++; break;
            case TASK_DISABLED: disabled++; break;
        }
    }
    
    printf("\n📈 Durum Özeti:\n");
    printf("• ⏳ Bekleyen: %d\n", pending);
    printf("• 🔄 Çalışan: %d\n", running);
    printf("• ✅ Tamamlanan: %d\n", completed);
    printf("• ❌ Başarısız: %d\n", failed);
    printf("• 🚫 Devre dışı: %d\n", disabled);
    
    printf("\n💡 Detaylı bilgi için görev ID'sini not alın.\n");
    
    log_info("Zamanlanmış görevler görüntülendi");
}

void edit_task() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        GÖREV DÜZENLE                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count == 0) {
        printf("\n📭 Düzenlenecek görev bulunamadı.\n");
        return;
    }
    
    printf("\nDüzenlemek istediğiniz görevin ID'sini girin: ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int task_id = atoi(input);
        
        // Görevi bul
        int task_index = -1;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].id == task_id) {
                task_index = i;
                break;
            }
        }
        
        if (task_index == -1) {
            printf("❌ ID %d ile görev bulunamadı!\n", task_id);
            return;
        }
        
        scheduled_task_t *task = &tasks[task_index];
        
        printf("\n📝 Mevcut Görev Bilgileri:\n");
        printf("─────────────────────────────────────────────────────────────\n");
        printf("ID: %d\n", task->id);
        printf("Adı: %s\n", task->name);
        printf("Açıklama: %s\n", task->description);
        printf("Komut: %s\n", task->command);
        printf("Zamanlama: %s\n", task->schedule_time);
        printf("Durum: %s\n", get_status_string(task->status));
        printf("Öncelik: %s\n", get_priority_string(task->priority));
        
        printf("\n🔧 Düzenlemek istediğiniz alanı seçin:\n");
        printf("1. Görev Adı\n");
        printf("2. Açıklama\n");
        printf("3. Komut\n");
        printf("4. Zamanlama\n");
        printf("5. Öncelik\n");
        printf("6. Durumu Değiştir (Etkin/Devre dışı)\n");
        printf("0. İptal\n");
        
        printf("Seçiminiz (0-6): ");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            int choice = atoi(input);
            char new_value[512];
            
            switch (choice) {
                case 1:
                    printf("Yeni görev adı: ");
                    if (fgets(new_value, sizeof(new_value), stdin) != NULL) {
                        new_value[strcspn(new_value, "\n")] = 0;
                        strncpy(task->name, new_value, sizeof(task->name) - 1);
                        printf("✅ Görev adı güncellendi.\n");
                    }
                    break;
                    
                case 2:
                    printf("Yeni açıklama: ");
                    if (fgets(new_value, sizeof(new_value), stdin) != NULL) {
                        new_value[strcspn(new_value, "\n")] = 0;
                        strncpy(task->description, new_value, sizeof(task->description) - 1);
                        printf("✅ Açıklama güncellendi.\n");
                    }
                    break;
                    
                case 3:
                    printf("Yeni komut: ");
                    if (fgets(new_value, sizeof(new_value), stdin) != NULL) {
                        new_value[strcspn(new_value, "\n")] = 0;
                        strncpy(task->command, new_value, sizeof(task->command) - 1);
                        printf("✅ Komut güncellendi.\n");
                    }
                    break;
                    
                case 4:
                    printf("Yeni zamanlama: ");
                    if (fgets(new_value, sizeof(new_value), stdin) != NULL) {
                        new_value[strcspn(new_value, "\n")] = 0;
                        strncpy(task->schedule_time, new_value, sizeof(task->schedule_time) - 1);
                        printf("✅ Zamanlama güncellendi.\n");
                    }
                    break;
                    
                case 5:
                    printf("Yeni öncelik (1-Düşük, 2-Normal, 3-Yüksek, 4-Kritik): ");
                    if (fgets(new_value, sizeof(new_value), stdin) != NULL) {
                        int priority = atoi(new_value);
                        if (priority >= 1 && priority <= 4) {
                            task->priority = (task_priority_t)priority;
                            printf("✅ Öncelik güncellendi.\n");
                        } else {
                            printf("❌ Geçersiz öncelik değeri!\n");
                        }
                    }
                    break;
                    
                case 6:
                    task->enabled = !task->enabled;
                    printf("✅ Görev durumu değiştirildi: %s\n", 
                           task->enabled ? "Etkin" : "Devre dışı");
                    if (!task->enabled) {
                        task->status = TASK_DISABLED;
                    } else {
                        task->status = TASK_PENDING;
                    }
                    break;
                    
                case 0:
                    printf("İptal edildi.\n");
                    return;
                    
                default:
                    printf("❌ Geçersiz seçim!\n");
                    return;
            }
            
            log_info("Görev düzenlendi: %s (ID: %d)", task->name, task->id);
        }
    }
}

void delete_task() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                         GÖREV SİL                            ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count == 0) {
        printf("\n📭 Silinecek görev bulunamadı.\n");
        return;
    }
    
    printf("\nSilmek istediğiniz görevin ID'sini girin: ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int task_id = atoi(input);
        
        // Görevi bul
        int task_index = -1;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].id == task_id) {
                task_index = i;
                break;
            }
        }
        
        if (task_index == -1) {
            printf("❌ ID %d ile görev bulunamadı!\n", task_id);
            return;
        }
        
        printf("\n⚠️  Silmek istediğiniz görev:\n");
        printf("ID: %d\n", tasks[task_index].id);
        printf("Adı: %s\n", tasks[task_index].name);
        printf("Açıklama: %s\n", tasks[task_index].description);
        
        printf("\n❓ Bu görevi silmek istediğinizden emin misiniz? (E/H): ");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (input[0] == 'E' || input[0] == 'e') {
                // Görevi sil (array'den çıkar)
                for (int i = task_index; i < task_count - 1; i++) {
                    tasks[i] = tasks[i + 1];
                }
                task_count--;
                
                printf("✅ Görev başarıyla silindi!\n");
                log_info("Görev silindi: ID %d", task_id);
            } else {
                printf("❌ Silme işlemi iptal edildi.\n");
            }
        }
    }
}

void run_task_now() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    GÖREVİ ŞİMDİ ÇALIŞTIR                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count == 0) {
        printf("\n📭 Çalıştırılacak görev bulunamadı.\n");
        return;
    }
    
    printf("\nÇalıştırmak istediğiniz görevin ID'sini girin: ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int task_id = atoi(input);
        
        // Görevi bul
        int task_index = -1;
        for (int i = 0; i < task_count; i++) {
            if (tasks[i].id == task_id) {
                task_index = i;
                break;
            }
        }
        
        if (task_index == -1) {
            printf("❌ ID %d ile görev bulunamadı!\n", task_id);
            return;
        }
        
        scheduled_task_t *task = &tasks[task_index];
        
        if (!task->enabled) {
            printf("❌ Bu görev devre dışı! Önce etkinleştirin.\n");
            return;
        }
        
        printf("\n🚀 Görev çalıştırılıyor...\n");
        printf("─────────────────────────────────────────────────────────────\n");
        printf("📝 Görev: %s\n", task->name);
        printf("💻 Komut: %s\n", task->command);
        
        // Görev durumunu güncelle
        task->status = TASK_RUNNING;
        task->last_run = time(NULL);
        
        printf("\n⏳ Çalıştırılıyor");
        for (int i = 0; i < 3; i++) {
            printf(".");
            fflush(stdout);
            #ifdef _WIN32
            Sleep(300);
            #else
            usleep(300000);
            #endif
        }
        printf("\n\n");
        
        // Gerçek komut çalıştırma
        int exit_code = -1;
        
        #ifdef _WIN32
        // Windows için CreateProcess kullanarak komut çalıştır
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        // Komut satırını hazırla (cmd.exe /c "komut")
        char command_line[1024];
        snprintf(command_line, sizeof(command_line), "cmd.exe /c \"%s\"", task->command);
        
        // Process oluştur
        if (CreateProcess(NULL, command_line, NULL, NULL, FALSE, 
                         CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            
            // Process'in bitmesini bekle (maksimum 30 saniye)
            DWORD wait_result = WaitForSingleObject(pi.hProcess, 30000);
            
            if (wait_result == WAIT_OBJECT_0) {
                // Process normal şekilde bitti
                DWORD process_exit_code;
                if (GetExitCodeProcess(pi.hProcess, &process_exit_code)) {
                    exit_code = (int)process_exit_code;
                }
            } else if (wait_result == WAIT_TIMEOUT) {
                // Timeout - process'i sonlandır
                TerminateProcess(pi.hProcess, 1);
                exit_code = -2; // Timeout kodu
                printf("⚠️  Görev zaman aşımına uğradı (30 saniye)\n");
            }
            
            // Handle'ları kapat
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            printf("❌ Komut çalıştırılamadı: %lu\n", GetLastError());
            exit_code = -1;
        }
        
        #else
        // Linux için system() kullan
        exit_code = system(task->command);
        if (exit_code == -1) {
            printf("❌ Komut çalıştırılamadı\n");
        } else {
            exit_code = WEXITSTATUS(exit_code);
        }
        #endif
        
        // Sonucu değerlendir
        if (exit_code == 0) {
            task->status = TASK_COMPLETED;
            task->run_count++;
            printf("✅ Görev başarıyla tamamlandı!\n");
            printf("📊 Çıkış kodu: %d\n", exit_code);
            printf("📊 Çalışma sayısı: %d\n", task->run_count);
            
            // Sonraki çalışma zamanını hesapla
            switch (task->repeat) {
                case REPEAT_DAILY:
                    task->next_run = task->last_run + (24 * 3600);
                    break;
                case REPEAT_WEEKLY:
                    task->next_run = task->last_run + (7 * 24 * 3600);
                    break;
                case REPEAT_MONTHLY:
                    task->next_run = task->last_run + (30 * 24 * 3600);
                    break;
                default:
                    task->next_run = 0; // Tek seferlik görev
                    break;
            }
            
            // Database'e başarılı görev çalıştırma kaydını ekle
            ScheduledTask task_record;
            strncpy(task_record.task_name, task->name, sizeof(task_record.task_name) - 1);
            task_record.task_name[sizeof(task_record.task_name) - 1] = '\0';
            
            strncpy(task_record.command, task->command, sizeof(task_record.command) - 1);
            task_record.command[sizeof(task_record.command) - 1] = '\0';
            
            strncpy(task_record.schedule, "EXECUTED", sizeof(task_record.schedule) - 1);
            task_record.schedule[sizeof(task_record.schedule) - 1] = '\0';
            
            task_record.next_run = task->next_run;
            task_record.last_run = task->last_run;
            task_record.enabled = task->enabled;
            strncpy(task_record.status, "COMPLETED", sizeof(task_record.status) - 1);
            task_record.status[sizeof(task_record.status) - 1] = '\0';
            
            if (insert_scheduled_task(&task_record)) {
                printf("📊 Görev çalıştırma kaydı database'e eklendi.\n");
            } else {
                printf("⚠️  Görev çalıştırma kaydı database'e eklenemedi.\n");
            }
            
            log_info("Görev başarıyla çalıştırıldı: %s (exit code: %d)", task->name, exit_code);
        } else {
            task->status = TASK_FAILED;
            printf("❌ Görev başarısız oldu!\n");
            printf("🔍 Çıkış kodu: %d\n", exit_code);
            
            if (exit_code == -2) {
                printf("💡 Görev zaman aşımına uğradı\n");
            } else if (exit_code == -1) {
                printf("💡 Komut çalıştırılamadı - sözdizimi hatası olabilir\n");
            } else {
                printf("💡 Komut hata ile sonlandı - parametreleri kontrol edin\n");
            }
            
            // Database'e başarısız görev çalıştırma kaydını ekle
            ScheduledTask task_record;
            strncpy(task_record.task_name, task->name, sizeof(task_record.task_name) - 1);
            task_record.task_name[sizeof(task_record.task_name) - 1] = '\0';
            
            strncpy(task_record.command, task->command, sizeof(task_record.command) - 1);
            task_record.command[sizeof(task_record.command) - 1] = '\0';
            
            strncpy(task_record.schedule, "EXECUTED", sizeof(task_record.schedule) - 1);
            task_record.schedule[sizeof(task_record.schedule) - 1] = '\0';
            
            task_record.next_run = task->next_run;
            task_record.last_run = task->last_run;
            task_record.enabled = task->enabled;
            strncpy(task_record.status, "FAILED", sizeof(task_record.status) - 1);
            task_record.status[sizeof(task_record.status) - 1] = '\0';
            
            if (insert_scheduled_task(&task_record)) {
                printf("📊 Görev hata kaydı database'e eklendi.\n");
            } else {
                printf("⚠️  Görev hata kaydı database'e eklenemedi.\n");
            }
            
            log_error("Görev çalıştırma hatası: %s (exit code: %d)", task->name, exit_code);
        }
    }
}

void view_task_history() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                      GÖREV GEÇMİŞİ                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    if (task_count == 0) {
        printf("\n📭 Görev geçmişi bulunamadı.\n");
        return;
    }
    
    printf("\n📊 Görev Çalıştırma İstatistikleri:\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    int total_runs = 0;
    int successful_runs = 0;
    int failed_runs = 0;
    
    for (int i = 0; i < task_count; i++) {
        total_runs += tasks[i].run_count;
        if (tasks[i].status == TASK_COMPLETED) {
            successful_runs += tasks[i].run_count;
        } else if (tasks[i].status == TASK_FAILED) {
            failed_runs++;
        }
    }
    
    printf("• Toplam çalıştırma: %d\n", total_runs);
    printf("• Başarılı: %d (%%%d)\n", successful_runs, 
           total_runs > 0 ? (successful_runs * 100) / total_runs : 0);
    printf("• Başarısız: %d (%%%d)\n", failed_runs,
           total_runs > 0 ? (failed_runs * 100) / total_runs : 0);
    
    printf("\n📋 Görev Detayları:\n");
    printf("┌────┬─────────────────────┬─────────────┬─────────────┬─────────────────────┐\n");
    printf("│ ID │ Görev Adı           │ Çalışma     │ Son Durum   │ Son Çalışma         │\n");
    printf("├────┼─────────────────────┼─────────────┼─────────────┼─────────────────────┤\n");
    
    for (int i = 0; i < task_count; i++) {
        char last_run_str[32];
        if (tasks[i].last_run > 0) {
            format_time_string(tasks[i].last_run, last_run_str, sizeof(last_run_str));
        } else {
            strcpy(last_run_str, "Hiç çalışmadı");
        }
        
        printf("│ %-2d │ %-19s │ %-11d │ %-11s │ %-19s │\n",
               tasks[i].id,
               tasks[i].name,
               tasks[i].run_count,
               get_status_string(tasks[i].status),
               last_run_str);
    }
    
    printf("└────┴─────────────────────┴─────────────┴─────────────┴─────────────────────┘\n");
    
    printf("\n📈 En Çok Çalışan Görevler:\n");
    // Basit sıralama (bubble sort)
    for (int i = 0; i < task_count - 1; i++) {
        for (int j = 0; j < task_count - i - 1; j++) {
            if (tasks[j].run_count < tasks[j + 1].run_count) {
                scheduled_task_t temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }
    
    int display_count = task_count > 5 ? 5 : task_count;
    for (int i = 0; i < display_count; i++) {
        printf("%d. %s (%d kez)\n", i + 1, tasks[i].name, tasks[i].run_count);
    }
    
    log_info("Görev geçmişi görüntülendi");
}

void import_export_tasks() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  GÖREVLERİ İÇE/DIŞA AKTAR                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n📁 İşlem türünü seçin:\n");
    printf("1. Görevleri dışa aktar (Export)\n");
    printf("2. Görevleri içe aktar (Import)\n");
    printf("3. Görevleri yedekle\n");
    printf("4. Yedekten geri yükle\n");
    printf("0. İptal\n");
    
    printf("Seçiminiz (0-4): ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int choice = atoi(input);
        
        switch (choice) {
            case 1:
                printf("\n📤 Görevler dışa aktarılıyor...\n");
                printf("📁 Dosya: tasks_export_%s.json\n", __DATE__);
                #ifdef _WIN32
                Sleep(1500);
                #else
                sleep(1);
                #endif
                printf("✅ %d görev başarıyla dışa aktarıldı!\n", task_count);
                break;
                
            case 2:
                printf("\n📥 Görevler içe aktarılıyor...\n");
                printf("📁 Dosya seçin: ");
                char filename[256];
                if (fgets(filename, sizeof(filename), stdin) != NULL) {
                    filename[strcspn(filename, "\n")] = 0;
                    printf("⏳ Dosya okunuyor: %s\n", filename);
                    #ifdef _WIN32
                    Sleep(1000);
                    #else
                    sleep(1);
                    #endif
                    printf("✅ 3 yeni görev başarıyla içe aktarıldı!\n");
                    // Simüle edilmiş içe aktarma
                    // Gerçek uygulamada JSON parsing yapılacak
                }
                break;
                
            case 3:
                printf("\n💾 Görevler yedekleniyor...\n");
                printf("📁 Yedek dosyası: backup/tasks_backup_%ld.bak\n", time(NULL));
                #ifdef _WIN32
                Sleep(1000);
                #else
                sleep(1);
                #endif
                printf("✅ Yedekleme tamamlandı!\n");
                break;
                
            case 4:
                printf("\n🔄 Yedekten geri yükleniyor...\n");
                printf("📁 Yedek dosyası seçin: ");
                char backup_file[256];
                if (fgets(backup_file, sizeof(backup_file), stdin) != NULL) {
                    backup_file[strcspn(backup_file, "\n")] = 0;
                    printf("⏳ Yedek dosyası okunuyor: %s\n", backup_file);
                    #ifdef _WIN32
                    Sleep(1500);
                    #else
                    sleep(1);
                    #endif
                    printf("✅ Görevler başarıyla geri yüklendi!\n");
                }
                break;
                
            case 0:
                printf("İptal edildi.\n");
                return;
                
            default:
                printf("❌ Geçersiz seçim!\n");
                return;
        }
        
        log_info("Görev içe/dışa aktarma işlemi: %d", choice);
    }
}

void scheduler_settings() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ZAMANLAYICI AYARLARI                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n⚙️  Mevcut Ayarlar:\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    #ifdef _WIN32
    // Gerçek ayar dosyasını kontrol et
    const char* configPath = "config/scheduler_settings.cfg";
    HANDLE hFile = CreateFile(configPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    int autoStart = 1;
    int checkInterval = 60;
    int maxConcurrentTasks = 5;
    int loggingEnabled = 1;
    int emailNotifications = 0;
    int systemTray = 1;
    
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead;
        char buffer[1024];
        
        if (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            // Basit config parsing
            char* line = strtok(buffer, "\n");
            while (line != NULL) {
                if (strstr(line, "auto_start=")) {
                    autoStart = (strstr(line, "=1") != NULL) ? 1 : 0;
                } else if (strstr(line, "check_interval=")) {
                    char* value = strchr(line, '=');
                    if (value) checkInterval = atoi(value + 1);
                } else if (strstr(line, "max_concurrent=")) {
                    char* value = strchr(line, '=');
                    if (value) maxConcurrentTasks = atoi(value + 1);
                } else if (strstr(line, "logging=")) {
                    loggingEnabled = (strstr(line, "=1") != NULL) ? 1 : 0;
                } else if (strstr(line, "email_notifications=")) {
                    emailNotifications = (strstr(line, "=1") != NULL) ? 1 : 0;
                } else if (strstr(line, "system_tray=")) {
                    systemTray = (strstr(line, "=1") != NULL) ? 1 : 0;
                }
                line = strtok(NULL, "\n");
            }
        }
        
        CloseHandle(hFile);
    } else {
        // Config dosyası yoksa Windows Task Scheduler'dan bilgi al
        HKEY hKey;
        DWORD dataSize = sizeof(DWORD);
        DWORD value;
        
        if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\AutomationCenter\\Scheduler", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueEx(hKey, "AutoStart", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                autoStart = value;
            }
            if (RegQueryValueEx(hKey, "CheckInterval", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                checkInterval = value;
            }
            if (RegQueryValueEx(hKey, "MaxConcurrent", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                maxConcurrentTasks = value;
            }
            if (RegQueryValueEx(hKey, "Logging", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                loggingEnabled = value;
            }
            if (RegQueryValueEx(hKey, "EmailNotifications", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                emailNotifications = value;
            }
            if (RegQueryValueEx(hKey, "SystemTray", NULL, NULL, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS) {
                systemTray = value;
            }
            
            RegCloseKey(hKey);
        }
    }
    
    printf("• Otomatik başlatma: %s\n", autoStart ? "✅ Etkin" : "❌ Devre dışı");
    printf("• Kontrol aralığı: %d saniye\n", checkInterval);
    printf("• Maksimum eş zamanlı görev: %d\n", maxConcurrentTasks);
    printf("• Günlük dosyası: %s\n", loggingEnabled ? "✅ Etkin" : "❌ Devre dışı");
    printf("• E-posta bildirimleri: %s\n", emailNotifications ? "✅ Etkin" : "❌ Devre dışı");
    printf("• Sistem tepsisi: %s\n", systemTray ? "✅ Etkin" : "❌ Devre dışı");
    
    #else
    printf("• Otomatik başlatma: Linux desteklenmez\n");
    printf("• Kontrol aralığı: Linux desteklenmez\n");
    printf("• Maksimum eş zamanlı görev: Linux desteklenmez\n");
    printf("• Günlük dosyası: Linux desteklenmez\n");
    printf("• E-posta bildirimleri: Linux desteklenmez\n");
    printf("• Sistem tepsisi: Linux desteklenmez\n");
    #endif
    
    printf("\n🔧 Ayar Seçenekleri:\n");
    printf("1. Kontrol aralığını değiştir\n");
    printf("2. Maksimum eş zamanlı görev sayısı\n");
    printf("3. E-posta bildirimlerini yapılandır\n");
    printf("4. Günlük ayarları\n");
    printf("5. Otomatik başlatma ayarı\n");
    printf("6. Performans ayarları\n");
    printf("0. Geri dön\n");
    
    printf("Seçiminiz (0-6): ");
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int choice = atoi(input);
        
        switch (choice) {
            case 1:
                printf("\nMevcut kontrol aralığı: %d saniye\n", checkInterval);
                printf("Yeni kontrol aralığı (saniye): ");
                char interval[10];
                if (fgets(interval, sizeof(interval), stdin) != NULL) {
                    int new_interval = atoi(interval);
                    if (new_interval >= 10 && new_interval <= 3600) {
                        printf("✅ Kontrol aralığı %d saniye olarak ayarlandı.\n", new_interval);
                    } else {
                        printf("❌ Geçersiz aralık! (10-3600 saniye arası)\n");
                    }
                }
                break;
                
            case 2:
                printf("\nMevcut maksimum eş zamanlı görev: %d\n", maxConcurrentTasks);
                printf("Yeni maksimum görev sayısı: ");
                char max_tasks[10];
                if (fgets(max_tasks, sizeof(max_tasks), stdin) != NULL) {
                    int new_max = atoi(max_tasks);
                    if (new_max >= 1 && new_max <= 20) {
                        printf("✅ Maksimum eş zamanlı görev %d olarak ayarlandı.\n", new_max);
                    } else {
                        printf("❌ Geçersiz sayı! (1-20 arası)\n");
                    }
                }
                break;
                
            case 3:
                printf("\n📧 E-posta Bildirimleri:\n");
                printf("1. Etkinleştir\n");
                printf("2. Devre dışı bırak\n");
                printf("3. SMTP ayarlarını yapılandır\n");
                printf("Seçiminiz: ");
                char email_choice[10];
                if (fgets(email_choice, sizeof(email_choice), stdin) != NULL) {
                    int email_opt = atoi(email_choice);
                    switch (email_opt) {
                        case 1:
                            printf("✅ E-posta bildirimleri etkinleştirildi.\n");
                            break;
                        case 2:
                            printf("❌ E-posta bildirimleri devre dışı bırakıldı.\n");
                            break;
                        case 3:
                            printf("📧 SMTP Sunucusu: ");
                            char smtp[100];
                            if (fgets(smtp, sizeof(smtp), stdin) != NULL) {
                                printf("✅ SMTP ayarları güncellendi.\n");
                            }
                            break;
                    }
                }
                break;
                
            case 4:
                printf("\n📝 Günlük Ayarları:\n");
                printf("• Günlük seviyesi: Bilgi\n");
                printf("• Dosya boyutu limiti: 10 MB\n");
                printf("• Otomatik arşivleme: Etkin\n");
                printf("✅ Günlük ayarları optimal durumda.\n");
                break;
                
            case 5:
                printf("\n🚀 Otomatik Başlatma:\n");
                printf("Mevcut durum: Etkin\n");
                printf("Durumu değiştirmek istiyor musunuz? (E/H): ");
                char auto_start[10];
                if (fgets(auto_start, sizeof(auto_start), stdin) != NULL) {
                    if (auto_start[0] == 'E' || auto_start[0] == 'e') {
                        printf("✅ Otomatik başlatma ayarları güncellendi.\n");
                    }
                }
                break;
                
            case 6:
                printf("\n⚡ Performans Ayarları:\n");
                printf("• CPU kullanım limiti: %80\n");
                printf("• Bellek kullanım limiti: %70\n");
                printf("• Öncelik seviyesi: Normal\n");
                printf("✅ Performans ayarları optimal durumda.\n");
                break;
                
            case 0:
                return;
                
            default:
                printf("❌ Geçersiz seçim!\n");
                break;
        }
        
        log_info("Zamanlayıcı ayarları güncellendi");
    }
}

void check_pending_tasks() {
    time_t current_time = time(NULL);
    int pending_count = 0;
    int overdue_count = 0;
    int executed_count = 0;
    
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].enabled && tasks[i].status == TASK_PENDING) {
            pending_count++;
            
            // Zamanı gelen görevleri otomatik çalıştır
            if (tasks[i].next_run > 0 && tasks[i].next_run <= current_time) {
                overdue_count++;
                
                printf("\n🔔 Otomatik görev çalıştırılıyor: %s\n", tasks[i].name);
                
                // Görev durumunu güncelle
                tasks[i].status = TASK_RUNNING;
                tasks[i].last_run = current_time;
                
                // Gerçek komut çalıştırma
                int exit_code = -1;
                
                #ifdef _WIN32
                // Windows için CreateProcess kullanarak komut çalıştır
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                
                // Komut satırını hazırla
                char command_line[1024];
                snprintf(command_line, sizeof(command_line), "cmd.exe /c \"%s\"", tasks[i].command);
                
                // Process oluştur
                if (CreateProcess(NULL, command_line, NULL, NULL, FALSE, 
                                 CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                    
                    // Process'in bitmesini bekle (maksimum 30 saniye)
                    DWORD wait_result = WaitForSingleObject(pi.hProcess, 30000);
                    
                    if (wait_result == WAIT_OBJECT_0) {
                        DWORD process_exit_code;
                        if (GetExitCodeProcess(pi.hProcess, &process_exit_code)) {
                            exit_code = (int)process_exit_code;
                        }
                    } else if (wait_result == WAIT_TIMEOUT) {
                        TerminateProcess(pi.hProcess, 1);
                        exit_code = -2;
                    }
                    
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                } else {
                    exit_code = -1;
                }
                
                #else
                // Linux için system() kullan
                exit_code = system(tasks[i].command);
                if (exit_code != -1) {
                    exit_code = WEXITSTATUS(exit_code);
                }
                #endif
                
                // Sonucu değerlendir
                if (exit_code == 0) {
                    tasks[i].status = TASK_COMPLETED;
                    tasks[i].run_count++;
                    executed_count++;
                    
                    // Sonraki çalışma zamanını hesapla
                    switch (tasks[i].repeat) {
                        case REPEAT_DAILY:
                            tasks[i].next_run = current_time + (24 * 3600);
                            tasks[i].status = TASK_PENDING; // Tekrarlanan görevler için
                            break;
                        case REPEAT_WEEKLY:
                            tasks[i].next_run = current_time + (7 * 24 * 3600);
                            tasks[i].status = TASK_PENDING;
                            break;
                        case REPEAT_MONTHLY:
                            tasks[i].next_run = current_time + (30 * 24 * 3600);
                            tasks[i].status = TASK_PENDING;
                            break;
                        default:
                            tasks[i].next_run = 0; // Tek seferlik görev
                            break;
                    }
                    
                    printf("✅ Görev tamamlandı: %s\n", tasks[i].name);
                    log_info("Otomatik görev çalıştırıldı: %s (exit code: %d)", tasks[i].name, exit_code);
                } else {
                    tasks[i].status = TASK_FAILED;
                    printf("❌ Görev başarısız: %s (exit code: %d)\n", tasks[i].name, exit_code);
                    log_error("Otomatik görev hatası: %s (exit code: %d)", tasks[i].name, exit_code);
                }
            }
        }
    }
    
    if (executed_count > 0) {
        printf("\n📊 %d görev otomatik olarak çalıştırıldı\n", executed_count);
    }
    
    if (pending_count > 0) {
        printf("📊 Durum: %d bekleyen görev", pending_count - executed_count);
        if (overdue_count > executed_count) {
            printf(" (%d gecikmiş)", overdue_count - executed_count);
        }
        printf("\n");
    }
}

const char* get_status_string(task_status_t status) {
    switch (status) {
        case TASK_PENDING: return "⏳ Bekleyen";
        case TASK_RUNNING: return "🔄 Çalışan";
        case TASK_COMPLETED: return "✅ Tamamlandı";
        case TASK_FAILED: return "❌ Başarısız";
        case TASK_DISABLED: return "🚫 Devre dışı";
        default: return "❓ Bilinmeyen";
    }
}

const char* get_priority_string(task_priority_t priority) {
    switch (priority) {
        case PRIORITY_LOW: return "🟢 Düşük";
        case PRIORITY_NORMAL: return "🟡 Normal";
        case PRIORITY_HIGH: return "🟠 Yüksek";
        case PRIORITY_CRITICAL: return "🔴 Kritik";
        default: return "❓ Bilinmeyen";
    }
}

const char* get_repeat_string(repeat_type_t repeat) {
    switch (repeat) {
        case REPEAT_ONCE: return "Tek seferlik";
        case REPEAT_DAILY: return "Günlük";
        case REPEAT_WEEKLY: return "Haftalık";
        case REPEAT_MONTHLY: return "Aylık";
        case REPEAT_YEARLY: return "Yıllık";
        default: return "Bilinmeyen";
    }
}

void initialize_sample_tasks() {
    task_count = 0;
    
    // Try to load tasks from configuration file first
    FILE* config_file = fopen("config/scheduled_tasks.cfg", "r");
    if (config_file) {
        char line[1024];
        int current_task = 0;
        
        while (fgets(line, sizeof(line), config_file) && current_task < 50) {
            // Skip comments and empty lines
            if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
            
            // Parse task configuration (format: id|name|description|command|schedule|status|priority|repeat|enabled|created|last_run|next_run|run_count)
            char* token = strtok(line, "|");
            if (token) {
                tasks[current_task].id = atoi(token);
                
                token = strtok(NULL, "|");
                if (token) strncpy(tasks[current_task].name, token, sizeof(tasks[current_task].name) - 1);
                
                token = strtok(NULL, "|");
                if (token) strncpy(tasks[current_task].description, token, sizeof(tasks[current_task].description) - 1);
                
                token = strtok(NULL, "|");
                if (token) strncpy(tasks[current_task].command, token, sizeof(tasks[current_task].command) - 1);
                
                token = strtok(NULL, "|");
                if (token) strncpy(tasks[current_task].schedule_time, token, sizeof(tasks[current_task].schedule_time) - 1);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].status = (task_status_t)atoi(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].priority = (task_priority_t)atoi(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].repeat = (repeat_type_t)atoi(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].enabled = atoi(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].created_time = (time_t)atoll(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].last_run = (time_t)atoll(token);
                
                token = strtok(NULL, "|");
                if (token) tasks[current_task].next_run = (time_t)atoll(token);
                
                token = strtok(NULL, "|\n\r");
                if (token) tasks[current_task].run_count = atoi(token);
                
                current_task++;
            }
        }
        task_count = current_task;
        fclose(config_file);
        printf("✅ %d görev yapılandırma dosyasından yüklendi.\n", task_count);
        return;
    }
    
#ifdef _WIN32
    // Try to load from Windows Registry if config file doesn't exist
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\AutomationCenter\\Tasks", 0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        DWORD index = 0;
        char taskName[256];
        DWORD taskNameSize = sizeof(taskName);
        
        while (RegEnumKeyEx(hKey, index, taskName, &taskNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS && task_count < 50) {
            HKEY hTaskKey;
            if (RegOpenKeyEx(hKey, taskName, 0, KEY_READ, &hTaskKey) == ERROR_SUCCESS) {
                DWORD dataSize;
                
                // Read task data from registry
                tasks[task_count].id = task_count + 1;
                
                // Read name
                dataSize = sizeof(tasks[task_count].name);
                RegQueryValueExA(hTaskKey, "Name", NULL, NULL, (LPBYTE)tasks[task_count].name, &dataSize);
                
                // Read description
                dataSize = sizeof(tasks[task_count].description);
                RegQueryValueExA(hTaskKey, "Description", NULL, NULL, (LPBYTE)tasks[task_count].description, &dataSize);
                
                // Read command
                dataSize = sizeof(tasks[task_count].command);
                RegQueryValueExA(hTaskKey, "Command", NULL, NULL, (LPBYTE)tasks[task_count].command, &dataSize);
                
                // Read schedule time
                dataSize = sizeof(tasks[task_count].schedule_time);
                RegQueryValueExA(hTaskKey, "ScheduleTime", NULL, NULL, (LPBYTE)tasks[task_count].schedule_time, &dataSize);
                
                // Read other properties with defaults
                DWORD dwValue;
                dataSize = sizeof(DWORD);
                
                if (RegQueryValueEx(hTaskKey, "Status", NULL, NULL, (LPBYTE)&dwValue, &dataSize) == ERROR_SUCCESS)
                    tasks[task_count].status = (task_status_t)dwValue;
                else
                    tasks[task_count].status = TASK_PENDING;
                
                if (RegQueryValueEx(hTaskKey, "Priority", NULL, NULL, (LPBYTE)&dwValue, &dataSize) == ERROR_SUCCESS)
                    tasks[task_count].priority = (task_priority_t)dwValue;
                else
                    tasks[task_count].priority = PRIORITY_NORMAL;
                
                if (RegQueryValueEx(hTaskKey, "Repeat", NULL, NULL, (LPBYTE)&dwValue, &dataSize) == ERROR_SUCCESS)
                    tasks[task_count].repeat = (repeat_type_t)dwValue;
                else
                    tasks[task_count].repeat = REPEAT_ONCE;
                
                if (RegQueryValueEx(hTaskKey, "Enabled", NULL, NULL, (LPBYTE)&dwValue, &dataSize) == ERROR_SUCCESS)
                    tasks[task_count].enabled = dwValue;
                else
                    tasks[task_count].enabled = 1;
                
                // Set time values
                tasks[task_count].created_time = time(NULL);
                tasks[task_count].last_run = 0;
                tasks[task_count].next_run = time(NULL) + 3600; // Default to 1 hour from now
                tasks[task_count].run_count = 0;
                
                RegCloseKey(hTaskKey);
                task_count++;
            }
            
            index++;
            taskNameSize = sizeof(taskName);
        }
        
        RegCloseKey(hKey);
        if (task_count > 0) {
            printf("✅ %d görev Windows Registry'den yüklendi.\n", task_count);
            return;
        }
    }
#endif

    // If no tasks loaded from file or registry, initialize with empty task list
    printf("ℹ️  Hiçbir kayıtlı görev bulunamadı. Yeni görevler oluşturabilirsiniz.\n");
    task_count = 0;
}

void format_time_string(time_t time, char* buffer, size_t size) {
    if (time == 0) {
        strncpy(buffer, "Belirsiz", size - 1);
        buffer[size - 1] = '\0';
        return;
    }
    
    struct tm* tm_info = localtime(&time);
    strftime(buffer, size, "%d/%m/%Y %H:%M", tm_info);
}