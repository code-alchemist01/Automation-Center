#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <lmcons.h>
#include <direct.h>
#include "../../include/core.h"
#include "../../include/logger.h"

#define MAX_REPORT_SIZE 50000
#define REPORTS_DIR "reports"
#define MAX_FILENAME_LEN 1024

// Function prototypes
void show_reports_menu();
void generate_system_report();
void generate_security_html_report();
void generate_performance_report();
void generate_backup_report();
void generate_network_report();
void generate_comprehensive_report();
void view_existing_reports();
void export_report_to_pdf();

// Helper functions
void create_reports_directory();
void write_html_header(FILE* file, const char* title);
void write_html_footer(FILE* file);
void get_system_info_for_report(char* buffer, int bufferSize);
void get_disk_info_for_report(char* buffer, int bufferSize);
void get_memory_info_for_report(char* buffer, int bufferSize);
void get_network_info_for_report(char* buffer, int bufferSize);
void get_process_info_for_report(char* buffer, int bufferSize);
void get_log_analysis_for_report(char* buffer, int bufferSize);
void get_backup_status_for_report(char* buffer, int bufferSize);

void run_reports() {
    log_message(LOG_INFO, "Reports module started");
    create_reports_directory();
    
    int choice;
    do {
        show_reports_menu();
        printf("Seçiminizi yapın (0-8): ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Geçersiz giriş! Lütfen bir sayı girin.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch (choice) {
            case 1:
                generate_system_report();
                break;
            case 2:
                generate_security_html_report();
                break;
            case 3:
                generate_performance_report();
                break;
            case 4:
                generate_backup_report();
                break;
            case 5:
                generate_network_report();
                break;
            case 6:
                generate_comprehensive_report();
                break;
            case 7:
                view_existing_reports();
                break;
            case 8:
                export_report_to_pdf();
                break;
            case 0:
                printf("Ana menüye dönülüyor...\n");
                break;
            default:
                printf("Geçersiz seçim! Lütfen 0-8 arasında bir sayı girin.\n");
        }
        
        if (choice != 0) {
            printf("\nDevam etmek için Enter tuşuna basın...");
            getchar();
            getchar();
        }
        
    } while (choice != 0);
    
    log_message(LOG_INFO, "Reports module ended");
}

void show_reports_menu() {
    system("cls");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                      RAPORLAR                                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Sistem Raporu Oluştur                                   ║\n");
    printf("║  2. Güvenlik Raporu Oluştur                                 ║\n");
    printf("║  3. Performans Raporu Oluştur                               ║\n");
    printf("║  4. Yedekleme Raporu Oluştur                                ║\n");
    printf("║  5. Ağ Raporu Oluştur                                       ║\n");
    printf("║  6. Kapsamlı Rapor Oluştur                                  ║\n");
    printf("║  7. Mevcut Raporları Görüntüle                              ║\n");
    printf("║  8. Raporu PDF'e Aktar                                      ║\n");
    printf("║  0. Ana Menüye Dön                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

void create_reports_directory() {
    _mkdir(REPORTS_DIR);
}

void generate_system_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    SİSTEM RAPORU                            ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Generate filename with timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/system_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Rapor dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create system report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Sistem Raporu");
    
    // System Information Section
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🖥️ Sistem Bilgileri</h2>\n");
    
    char sysInfo[2048];
    get_system_info_for_report(sysInfo, sizeof(sysInfo));
    fprintf(file, "%s", sysInfo);
    
    fprintf(file, "</div>\n");
    
    // Memory Information Section
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💾 Bellek Bilgileri</h2>\n");
    
    char memInfo[1024];
    get_memory_info_for_report(memInfo, sizeof(memInfo));
    fprintf(file, "%s", memInfo);
    
    fprintf(file, "</div>\n");
    
    // Disk Information Section
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💿 Disk Bilgileri</h2>\n");
    
    char diskInfo[2048];
    get_disk_info_for_report(diskInfo, sizeof(diskInfo));
    fprintf(file, "%s", diskInfo);
    
    fprintf(file, "</div>\n");
    
    // Process Information Section
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>⚙️ Çalışan Süreçler (Top 10)</h2>\n");
    
    char processInfo[4096];
    get_process_info_for_report(processInfo, sizeof(processInfo));
    fprintf(file, "%s", processInfo);
    
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Sistem raporu oluşturuldu: %s\n", filename);
    printf("Raporu görüntülemek için dosyayı web tarayıcısında açın.\n");
    
    // Open the report automatically
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "System report generated: %s", filename);
}

void generate_security_html_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   GÜVENLİK RAPORU                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/security_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Güvenlik raporu dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create security report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Güvenlik Raporu");
    
    // Windows Defender Status
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🛡️ Windows Defender Durumu</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    // Check Windows Defender status
    FILE* cmd = _popen("powershell Get-MpComputerStatus | Select-Object AntivirusEnabled,RealTimeProtectionEnabled,IoavProtectionEnabled", "r");
    if (cmd) {
        char buffer[1024];
        fprintf(file, "<pre>\n");
        while (fgets(buffer, sizeof(buffer), cmd)) {
            fprintf(file, "%s", buffer);
        }
        fprintf(file, "</pre>\n");
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // Firewall Status
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🔥 Güvenlik Duvarı Durumu</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    cmd = _popen("netsh advfirewall show allprofiles state", "r");
    if (cmd) {
        char buffer[1024];
        fprintf(file, "<pre>\n");
        while (fgets(buffer, sizeof(buffer), cmd)) {
            fprintf(file, "%s", buffer);
        }
        fprintf(file, "</pre>\n");
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // User Account Control
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>👤 Kullanıcı Hesap Kontrolü (UAC)</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    // Check UAC status from registry
    HKEY hKey;
    DWORD uacValue = 0;
    DWORD dataSize = sizeof(DWORD);
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        if (RegQueryValueEx(hKey, "EnableLUA", NULL, NULL, (LPBYTE)&uacValue, &dataSize) == ERROR_SUCCESS) {
            fprintf(file, "<p><strong>UAC Durumu:</strong> %s</p>\n", 
                    uacValue ? "Etkin" : "Devre Dışı");
        }
        
        RegCloseKey(hKey);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // Recent Security Events
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>📋 Son Güvenlik Olayları</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    cmd = _popen("powershell \"Get-EventLog -LogName Security -Newest 10 | Select-Object TimeGenerated,EventID,EntryType,Message | ConvertTo-Html -Fragment\"", "r");
    if (cmd) {
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), cmd)) {
            fprintf(file, "%s", buffer);
        }
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // Network Security
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🌐 Ağ Güvenliği</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    cmd = _popen("netstat -an | findstr LISTENING", "r");
    if (cmd) {
        char buffer[1024];
        fprintf(file, "<h3>Dinlenen Portlar:</h3>\n");
        fprintf(file, "<pre>\n");
        while (fgets(buffer, sizeof(buffer), cmd)) {
            fprintf(file, "%s", buffer);
        }
        fprintf(file, "</pre>\n");
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Güvenlik raporu oluşturuldu: %s\n", filename);
    
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "Security report generated: %s", filename);
}

void generate_performance_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  PERFORMANS RAPORU                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/performance_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Performans raporu dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create performance report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Performans Raporu");
    
    // CPU Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🖥️ İşlemci Bilgileri</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    FILE* cmd = _popen("wmic cpu get name,maxclockspeed,numberofcores,numberoflogicalprocessors /format:list", "r");
    if (cmd) {
        char buffer[1024];
        fprintf(file, "<pre>\n");
        while (fgets(buffer, sizeof(buffer), cmd)) {
            if (strlen(buffer) > 1) {
                fprintf(file, "%s", buffer);
            }
        }
        fprintf(file, "</pre>\n");
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // Memory Performance
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💾 Bellek Performansı</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        fprintf(file, "<table border='1' style='border-collapse: collapse; width: 100%%;'>\n");
        fprintf(file, "<tr><th>Özellik</th><th>Değer</th></tr>\n");
        fprintf(file, "<tr><td>Toplam Fiziksel Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullTotalPhys / (1024*1024*1024));
        fprintf(file, "<tr><td>Kullanılabilir Fiziksel Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullAvailPhys / (1024*1024*1024));
        fprintf(file, "<tr><td>Bellek Kullanım Oranı</td><td>%d%%</td></tr>\n", 
                memInfo.dwMemoryLoad);
        fprintf(file, "<tr><td>Toplam Sanal Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullTotalVirtual / (1024*1024*1024));
        fprintf(file, "<tr><td>Kullanılabilir Sanal Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullAvailVirtual / (1024*1024*1024));
        fprintf(file, "</table>\n");
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // Disk Performance
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💿 Disk Performansı</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    cmd = _popen("wmic logicaldisk get size,freespace,caption /format:list", "r");
    if (cmd) {
        char buffer[1024];
        fprintf(file, "<table border='1' style='border-collapse: collapse; width: 100%%;'>\n");
        fprintf(file, "<tr><th>Sürücü</th><th>Toplam Alan</th><th>Boş Alan</th><th>Kullanım Oranı</th></tr>\n");
        
        char caption[10] = "";
        char size[50] = "";
        char freespace[50] = "";
        
        while (fgets(buffer, sizeof(buffer), cmd)) {
            if (strstr(buffer, "Caption=")) {
                sscanf(buffer, "Caption=%s", caption);
            } else if (strstr(buffer, "FreeSpace=")) {
                sscanf(buffer, "FreeSpace=%s", freespace);
            } else if (strstr(buffer, "Size=")) {
                sscanf(buffer, "Size=%s", size);
                
                if (strlen(caption) > 0 && strlen(size) > 0 && strlen(freespace) > 0) {
                    long long totalSize = atoll(size);
                    long long freeSize = atoll(freespace);
                    long long usedSize = totalSize - freeSize;
                    double usagePercent = (double)usedSize / totalSize * 100;
                    
                    fprintf(file, "<tr><td>%s</td><td>%.2f GB</td><td>%.2f GB</td><td>%.1f%%</td></tr>\n",
                            caption, 
                            (double)totalSize / (1024*1024*1024),
                            (double)freeSize / (1024*1024*1024),
                            usagePercent);
                    
                    // Reset for next iteration
                    caption[0] = '\0';
                    size[0] = '\0';
                    freespace[0] = '\0';
                }
            }
        }
        
        fprintf(file, "</table>\n");
        _pclose(cmd);
    }
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // System Uptime
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>⏰ Sistem Çalışma Süresi</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    
    DWORD uptime = GetTickCount();
    DWORD days = uptime / (1000 * 60 * 60 * 24);
    DWORD hours = (uptime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    DWORD minutes = (uptime % (1000 * 60 * 60)) / (1000 * 60);
    
    fprintf(file, "<p><strong>Sistem Çalışma Süresi:</strong> %d gün, %d saat, %d dakika</p>\n", 
            days, hours, minutes);
    
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Performans raporu oluşturuldu: %s\n", filename);
    
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "Performance report generated: %s", filename);
}

void generate_backup_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  YEDEKLEME RAPORU                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/backup_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Yedekleme raporu dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create backup report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Yedekleme Raporu");
    
    // Backup Status
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💾 Yedekleme Durumu</h2>\n");
    
    char backupInfo[2048];
    get_backup_status_for_report(backupInfo, sizeof(backupInfo));
    fprintf(file, "%s", backupInfo);
    
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Yedekleme raporu oluşturuldu: %s\n", filename);
    
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "Backup report generated: %s", filename);
}

void generate_network_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                     AĞ RAPORU                               ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/network_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Ağ raporu dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create network report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Ağ Raporu");
    
    // Network Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🌐 Ağ Bilgileri</h2>\n");
    
    char networkInfo[4096];
    get_network_info_for_report(networkInfo, sizeof(networkInfo));
    fprintf(file, "%s", networkInfo);
    
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Ağ raporu oluşturuldu: %s\n", filename);
    
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "Network report generated: %s", filename);
}

void generate_comprehensive_report() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  KAPSAMLI RAPOR                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("Kapsamlı rapor oluşturuluyor... Bu işlem birkaç dakika sürebilir.\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[MAX_PATH_LEN];
    sprintf(filename, "%s/comprehensive_report_%04d%02d%02d_%02d%02d%02d.html",
            REPORTS_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Kapsamlı rapor dosyası oluşturulamadı!\n");
        log_message(LOG_ERROR, "Failed to create comprehensive report file: %s", filename);
        return;
    }
    
    write_html_header(file, "Kapsamlı Sistem Raporu");
    
    // Executive Summary
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>📊 Yönetici Özeti</h2>\n");
    fprintf(file, "<div class='info-box'>\n");
    fprintf(file, "<p>Bu rapor %02d.%02d.%04d tarihinde %02d:%02d:%02d saatinde oluşturulmuştur.</p>\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
    fprintf(file, "<p>Sistem durumu, güvenlik, performans ve ağ bilgileri aşağıda detaylandırılmıştır.</p>\n");
    fprintf(file, "</div>\n");
    fprintf(file, "</div>\n");
    
    // System Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🖥️ Sistem Bilgileri</h2>\n");
    char sysInfo[2048];
    get_system_info_for_report(sysInfo, sizeof(sysInfo));
    fprintf(file, "%s", sysInfo);
    fprintf(file, "</div>\n");
    
    // Memory Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💾 Bellek Durumu</h2>\n");
    char memInfo[1024];
    get_memory_info_for_report(memInfo, sizeof(memInfo));
    fprintf(file, "%s", memInfo);
    fprintf(file, "</div>\n");
    
    // Disk Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💿 Disk Durumu</h2>\n");
    char diskInfo[2048];
    get_disk_info_for_report(diskInfo, sizeof(diskInfo));
    fprintf(file, "%s", diskInfo);
    fprintf(file, "</div>\n");
    
    // Network Information
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>🌐 Ağ Durumu</h2>\n");
    char networkInfo[4096];
    get_network_info_for_report(networkInfo, sizeof(networkInfo));
    fprintf(file, "%s", networkInfo);
    fprintf(file, "</div>\n");
    
    // Log Analysis
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>📋 Log Analizi</h2>\n");
    char logInfo[2048];
    get_log_analysis_for_report(logInfo, sizeof(logInfo));
    fprintf(file, "%s", logInfo);
    fprintf(file, "</div>\n");
    
    // Backup Status
    fprintf(file, "<div class='section'>\n");
    fprintf(file, "<h2>💾 Yedekleme Durumu</h2>\n");
    char backupInfo[2048];
    get_backup_status_for_report(backupInfo, sizeof(backupInfo));
    fprintf(file, "%s", backupInfo);
    fprintf(file, "</div>\n");
    
    write_html_footer(file);
    fclose(file);
    
    printf("Kapsamlı rapor oluşturuldu: %s\n", filename);
    
    char command[MAX_PATH_LEN + 10];
    sprintf(command, "start %s", filename);
    system(command);
    
    log_message(LOG_INFO, "Comprehensive report generated: %s", filename);
}

void view_existing_reports() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 MEVCUT RAPORLAR                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LEN];
    
    sprintf(searchPath, "%s\\*.html", REPORTS_DIR);
    
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Henüz oluşturulmuş rapor bulunamadı.\n");
        return;
    }
    
    printf("Mevcut raporlar:\n");
    printf("================\n");
    
    int count = 0;
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            count++;
            
            // Get file size
            LARGE_INTEGER fileSize;
            fileSize.LowPart = findFileData.nFileSizeLow;
            fileSize.HighPart = findFileData.nFileSizeHigh;
            
            // Convert file time to local time
            FILETIME localFileTime;
            SYSTEMTIME systemTime;
            FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &localFileTime);
            FileTimeToSystemTime(&localFileTime, &systemTime);
            
            printf("%d. %s\n", count, findFileData.cFileName);
            printf("   Boyut: %.2f KB\n", (double)fileSize.QuadPart / 1024);
            printf("   Tarih: %02d.%02d.%04d %02d:%02d:%02d\n",
                   systemTime.wDay, systemTime.wMonth, systemTime.wYear,
                   systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
            printf("   ---\n");
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    
    if (count == 0) {
        printf("Henüz oluşturulmuş rapor bulunamadı.\n");
    } else {
        printf("\nToplam %d rapor bulundu.\n", count);
        printf("\nBir raporu açmak istiyor musunuz? (E/H): ");
        
        char choice;
        scanf(" %c", &choice);
        
        if (choice == 'E' || choice == 'e') {
            printf("Açılacak rapor numarası: ");
            int reportNum;
            scanf("%d", &reportNum);
            
            if (reportNum > 0 && reportNum <= count) {
                // Find the selected report
                hFind = FindFirstFile(searchPath, &findFileData);
                int currentCount = 0;
                
                do {
                    if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        currentCount++;
                        if (currentCount == reportNum) {
                            char fullPath[MAX_PATH_LEN];
                            sprintf(fullPath, "%s\\%s", REPORTS_DIR, findFileData.cFileName);
                            
                            char command[MAX_PATH_LEN + 10];
                            sprintf(command, "start %s", fullPath);
                            system(command);
                            
                            printf("Rapor açılıyor: %s\n", findFileData.cFileName);
                            break;
                        }
                    }
                } while (FindNextFile(hFind, &findFileData) != 0);
                
                FindClose(hFind);
            } else {
                printf("Geçersiz rapor numarası!\n");
            }
        }
    }
    
    log_message(LOG_INFO, "Existing reports viewed, found %d reports", count);
}

int check_wkhtmltopdf_available() {
    // Check if wkhtmltopdf is available by trying to run it with --version
    int result = system("wkhtmltopdf --version >nul 2>&1");
    return (result == 0);
}

void open_html_in_browser(const char* htmlPath) {
    char command[MAX_PATH_LEN + 20];
    sprintf(command, "start \"\" \"%s\"", htmlPath);
    system(command);
}

void export_report_to_pdf() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 PDF'E AKTAR                                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Check if wkhtmltopdf is available
    int wkhtmltopdf_available = check_wkhtmltopdf_available();
    
    if (!wkhtmltopdf_available) {
        printf("⚠️  wkhtmltopdf aracı bulunamadı!\n\n");
        printf("PDF dönüştürme seçenekleri:\n");
        printf("1. HTML raporunu tarayıcıda aç ve manuel PDF'e dönüştür\n");
        printf("2. wkhtmltopdf yükle ve tekrar dene\n");
        printf("3. İptal et\n\n");
        printf("Seçiminiz (1-3): ");
        
        int choice;
        scanf("%d", &choice);
        
        if (choice == 3) {
            return;
        } else if (choice == 2) {
            printf("\nwkhtmltopdf yüklemek için:\n");
            printf("1. https://wkhtmltopdf.org/downloads.html adresini ziyaret edin\n");
            printf("2. Windows sürümünü indirin ve yükleyin\n");
            printf("3. Sistem PATH'ine eklendiğinden emin olun\n");
            printf("\nYükleme tamamlandıktan sonra bu özelliği tekrar deneyin.\n");
            return;
        }
        // choice == 1, continue to open in browser
    }
    
    // List HTML reports
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LEN];
    
    sprintf(searchPath, "%s\\*.html", REPORTS_DIR);
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("❌ Dönüştürülecek HTML raporu bulunamadı.\n");
        printf("Önce bir rapor oluşturun.\n");
        return;
    }
    
    printf("\n📄 Mevcut HTML raporları:\n");
    printf("========================\n");
    
    int count = 0;
    char reportFiles[50][MAX_FILENAME_LEN]; // Store filenames
    
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            count++;
            strcpy(reportFiles[count-1], findFileData.cFileName);
            printf("%d. %s\n", count, findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    
    printf("\nDönüştürülecek rapor numarası (1-%d): ", count);
    int reportNum;
    scanf("%d", &reportNum);
    
    if (reportNum < 1 || reportNum > count) {
        printf("❌ Geçersiz rapor numarası!\n");
        return;
    }
    
    char htmlPath[MAX_PATH_LEN];
    char pdfPath[MAX_PATH_LEN];
    
    sprintf(htmlPath, "%s\\%s", REPORTS_DIR, reportFiles[reportNum-1]);
    
    // Create PDF filename
    strcpy(pdfPath, htmlPath);
    char* ext = strrchr(pdfPath, '.');
    if (ext) {
        strcpy(ext, ".pdf");
    }
    
    if (wkhtmltopdf_available) {
        // Try to convert using wkhtmltopdf
        char command[MAX_PATH_LEN * 2 + 100];
        sprintf(command, "wkhtmltopdf --page-size A4 --margin-top 0.75in --margin-right 0.75in --margin-bottom 0.75in --margin-left 0.75in \"%s\" \"%s\"", htmlPath, pdfPath);
        
        printf("\n🔄 PDF dönüştürme işlemi başlatılıyor...\n");
        
        int result = system(command);
        if (result == 0) {
            printf("✅ PDF başarıyla oluşturuldu: %s\n", pdfPath);
            log_message(LOG_INFO, "Report converted to PDF: %s", pdfPath);
            
            printf("\nPDF dosyasını açmak istiyor musunuz? (E/H): ");
            char openChoice;
            scanf(" %c", &openChoice);
            if (openChoice == 'E' || openChoice == 'e') {
                char openCommand[MAX_PATH_LEN + 20];
                sprintf(openCommand, "start \"\" \"%s\"", pdfPath);
                system(openCommand);
            }
        } else {
            printf("❌ PDF dönüştürme başarısız!\n");
            printf("HTML dosyasını tarayıcıda açıp manuel olarak PDF'e dönüştürmeyi deneyin.\n");
            log_message(LOG_WARNING, "PDF conversion failed for: %s", htmlPath);
            
            printf("\nHTML dosyasını tarayıcıda açmak istiyor musunuz? (E/H): ");
            char openChoice;
            scanf(" %c", &openChoice);
            if (openChoice == 'E' || openChoice == 'e') {
                open_html_in_browser(htmlPath);
                printf("HTML raporu tarayıcıda açıldı. Ctrl+P ile yazdırabilir ve PDF olarak kaydedebilirsiniz.\n");
            }
        }
    } else {
        // Open in browser for manual conversion
        printf("\n🌐 HTML raporu tarayıcıda açılıyor...\n");
        open_html_in_browser(htmlPath);
        printf("✅ HTML raporu tarayıcıda açıldı.\n");
        printf("📝 PDF'e dönüştürmek için:\n");
        printf("   1. Ctrl+P tuşlarına basın\n");
        printf("   2. 'Hedef' olarak 'PDF olarak kaydet' seçin\n");
        printf("   3. Kaydet butonuna tıklayın\n");
        log_message(LOG_INFO, "HTML report opened in browser for manual PDF conversion: %s", htmlPath);
    }
}

// Helper function implementations
void write_html_header(FILE* file, const char* title) {
    fprintf(file, "<!DOCTYPE html>\n");
    fprintf(file, "<html lang='tr'>\n");
    fprintf(file, "<head>\n");
    fprintf(file, "    <meta charset='UTF-8'>\n");
    fprintf(file, "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n");
    fprintf(file, "    <title>%s</title>\n", title);
    fprintf(file, "    <style>\n");
    fprintf(file, "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }\n");
    fprintf(file, "        .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 30px; border-radius: 10px; box-shadow: 0 0 20px rgba(0,0,0,0.1); }\n");
    fprintf(file, "        h1 { color: #2c3e50; text-align: center; border-bottom: 3px solid #3498db; padding-bottom: 10px; }\n");
    fprintf(file, "        h2 { color: #34495e; border-left: 4px solid #3498db; padding-left: 15px; }\n");
    fprintf(file, "        .section { margin: 30px 0; padding: 20px; border: 1px solid #ddd; border-radius: 8px; }\n");
    fprintf(file, "        .info-box { background-color: #f8f9fa; padding: 15px; border-radius: 5px; margin: 10px 0; }\n");
    fprintf(file, "        table { width: 100%%; border-collapse: collapse; margin: 15px 0; }\n");
    fprintf(file, "        th, td { padding: 12px; text-align: left; border: 1px solid #ddd; }\n");
    fprintf(file, "        th { background-color: #3498db; color: white; }\n");
    fprintf(file, "        tr:nth-child(even) { background-color: #f2f2f2; }\n");
    fprintf(file, "        .timestamp { text-align: center; color: #7f8c8d; font-style: italic; margin-top: 30px; }\n");
    fprintf(file, "        pre { background-color: #2c3e50; color: #ecf0f1; padding: 15px; border-radius: 5px; overflow-x: auto; }\n");
    fprintf(file, "    </style>\n");
    fprintf(file, "</head>\n");
    fprintf(file, "<body>\n");
    fprintf(file, "    <div class='container'>\n");
    fprintf(file, "        <h1>%s</h1>\n", title);
    
    // Add timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(file, "        <div class='timestamp'>Rapor Tarihi: %02d.%02d.%04d %02d:%02d:%02d</div>\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec);
}

void write_html_footer(FILE* file) {
    fprintf(file, "        <div class='timestamp'>\n");
    fprintf(file, "            <p>Bu rapor Sistem Otomasyon Merkezi tarafından otomatik olarak oluşturulmuştur.</p>\n");
    fprintf(file, "        </div>\n");
    fprintf(file, "    </div>\n");
    fprintf(file, "</body>\n");
    fprintf(file, "</html>\n");
}

void get_system_info_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    
    // Get Windows version
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        char temp[500];
        sprintf(temp, "<p><strong>İşletim Sistemi:</strong> Windows %d.%d Build %d</p>\n", 
                osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
        strcat(buffer, temp);
        
        sprintf(temp, "<p><strong>Service Pack:</strong> %s</p>\n", osvi.szCSDVersion);
        strcat(buffer, temp);
    }
    
    // Get computer name
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerName(computerName, &size)) {
        char temp[200];
        sprintf(temp, "<p><strong>Bilgisayar Adı:</strong> %s</p>\n", computerName);
        strcat(buffer, temp);
    }
    
    // Get user name
    char userName[UNLEN + 1];
    size = sizeof(userName);
    if (GetUserName(userName, &size)) {
        char temp[200];
        sprintf(temp, "<p><strong>Kullanıcı Adı:</strong> %s</p>\n", userName);
        strcat(buffer, temp);
    }
    
    // Get processor information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    char temp[200];
    sprintf(temp, "<p><strong>İşlemci Sayısı:</strong> %d</p>\n", sysInfo.dwNumberOfProcessors);
    strcat(buffer, temp);
    
    strcat(buffer, "</div>\n");
}

void get_memory_info_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        char temp[500];
        
        strcat(buffer, "<table border='1' style='border-collapse: collapse; width: 100%;'>\n");
        strcat(buffer, "<tr><th>Özellik</th><th>Değer</th></tr>\n");
        
        sprintf(temp, "<tr><td>Toplam Fiziksel Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullTotalPhys / (1024*1024*1024));
        strcat(buffer, temp);
        
        sprintf(temp, "<tr><td>Kullanılabilir Fiziksel Bellek</td><td>%.2f GB</td></tr>\n", 
                (double)memInfo.ullAvailPhys / (1024*1024*1024));
        strcat(buffer, temp);
        
        sprintf(temp, "<tr><td>Bellek Kullanım Oranı</td><td>%d%%</td></tr>\n", 
                memInfo.dwMemoryLoad);
        strcat(buffer, temp);
        
        strcat(buffer, "</table>\n");
    }
    
    strcat(buffer, "</div>\n");
}

void get_disk_info_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    strcat(buffer, "<table border='1' style='border-collapse: collapse; width: 100%;'>\n");
    strcat(buffer, "<tr><th>Sürücü</th><th>Toplam Alan</th><th>Boş Alan</th><th>Kullanım Oranı</th></tr>\n");
    
    DWORD drives = GetLogicalDrives();
    char driveLetter = 'A';
    
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            char drive[4];
            sprintf(drive, "%c:\\", driveLetter + i);
            
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            
            if (GetDiskFreeSpaceEx(drive, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                double totalGB = (double)totalNumberOfBytes.QuadPart / (1024*1024*1024);
                double freeGB = (double)totalNumberOfFreeBytes.QuadPart / (1024*1024*1024);
                double usedGB = totalGB - freeGB;
                double usagePercent = (usedGB / totalGB) * 100;
                
                char temp[300];
                sprintf(temp, "<tr><td>%s</td><td>%.2f GB</td><td>%.2f GB</td><td>%.1f%%</td></tr>\n",
                        drive, totalGB, freeGB, usagePercent);
                strcat(buffer, temp);
            }
        }
    }
    
    strcat(buffer, "</table>\n");
    strcat(buffer, "</div>\n");
}

void get_network_info_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    strcat(buffer, "<h3>Ağ Arayüzleri:</h3>\n");
    strcat(buffer, "<pre>\n");
    
    // This would normally contain network interface information
    // For now, we'll add a placeholder
    strcat(buffer, "Ağ arayüz bilgileri burada görüntülenecek...\n");
    strcat(buffer, "IP yapılandırması, DNS ayarları, ağ bağlantı durumu vb.\n");
    
    strcat(buffer, "</pre>\n");
    strcat(buffer, "</div>\n");
}

void get_process_info_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    strcat(buffer, "<pre>\n");
    
    // This would normally contain top processes information
    strcat(buffer, "En çok kaynak kullanan süreçler burada listelenecek...\n");
    
    strcat(buffer, "</pre>\n");
    strcat(buffer, "</div>\n");
}

void get_log_analysis_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    
    // Check if log files exist and analyze them
    FILE* logFile = fopen("logs/automation_center.log", "r");
    if (logFile) {
        int infoCount = 0, warningCount = 0, errorCount = 0;
        char line[1024];
        
        while (fgets(line, sizeof(line), logFile)) {
            if (strstr(line, "[INFO]")) infoCount++;
            else if (strstr(line, "[WARNING]")) warningCount++;
            else if (strstr(line, "[ERROR]")) errorCount++;
        }
        
        fclose(logFile);
        
        char temp[500];
        sprintf(temp, "<p><strong>Log Analizi:</strong></p>\n");
        strcat(buffer, temp);
        
        sprintf(temp, "<p>INFO mesajları: %d</p>\n", infoCount);
        strcat(buffer, temp);
        
        sprintf(temp, "<p>WARNING mesajları: %d</p>\n", warningCount);
        strcat(buffer, temp);
        
        sprintf(temp, "<p>ERROR mesajları: %d</p>\n", errorCount);
        strcat(buffer, temp);
    } else {
        strcat(buffer, "<p>Log dosyası bulunamadı.</p>\n");
    }
    
    strcat(buffer, "</div>\n");
}

void get_backup_status_for_report(char* buffer, int bufferSize) {
    strcpy(buffer, "<div class='info-box'>\n");
    
    // Check backup directory
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH_LEN];
    
    sprintf(searchPath, "backups\\*");
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        strcat(buffer, "<p>Henüz yedekleme yapılmamış.</p>\n");
    } else {
        int backupCount = 0;
        LARGE_INTEGER totalSize;
        totalSize.QuadPart = 0;
        
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                backupCount++;
                LARGE_INTEGER fileSize;
                fileSize.LowPart = findFileData.nFileSizeLow;
                fileSize.HighPart = findFileData.nFileSizeHigh;
                totalSize.QuadPart += fileSize.QuadPart;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        
        FindClose(hFind);
        
        char temp[300];
        sprintf(temp, "<p><strong>Toplam Yedek Dosyası:</strong> %d</p>\n", backupCount);
        strcat(buffer, temp);
        
        sprintf(temp, "<p><strong>Toplam Yedek Boyutu:</strong> %.2f MB</p>\n", 
                (double)totalSize.QuadPart / (1024*1024));
        strcat(buffer, temp);
    }
    
    strcat(buffer, "</div>\n");
}