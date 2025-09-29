#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <lmcons.h>
#include "../../include/logger.h"
#define MAX_VALUE_LEN 512

// Function prototypes
void show_system_settings_menu();
void view_system_info();
void manage_startup_programs();
void configure_power_settings();
void manage_environment_variables();
void configure_system_performance();
void backup_restore_registry();
void view_installed_programs();
void configure_user_accounts();
void system_maintenance();

// Registry helper functions
int read_registry_string(HKEY hKey, const char* subKey, const char* valueName, char* buffer, DWORD bufferSize);
int write_registry_string(HKEY hKey, const char* subKey, const char* valueName, const char* value);
int delete_registry_value(HKEY hKey, const char* subKey, const char* valueName);

void run_system_settings() {
    log_message(LOG_INFO, "System Settings module started");
    
    int choice;
    do {
        show_system_settings_menu();
        printf("Seçiminizi yapın (0-9): ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Geçersiz giriş! Lütfen bir sayı girin.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        switch (choice) {
            case 1:
                view_system_info();
                break;
            case 2:
                manage_startup_programs();
                break;
            case 3:
                configure_power_settings();
                break;
            case 4:
                manage_environment_variables();
                break;
            case 5:
                configure_system_performance();
                break;
            case 6:
                backup_restore_registry();
                break;
            case 7:
                view_installed_programs();
                break;
            case 8:
                configure_user_accounts();
                break;
            case 9:
                system_maintenance();
                break;
            case 0:
                printf("Ana menüye dönülüyor...\n");
                break;
            default:
                printf("Geçersiz seçim! Lütfen 0-9 arasında bir sayı girin.\n");
        }
        
        if (choice != 0) {
            printf("\nDevam etmek için Enter tuşuna basın...");
            getchar();
            getchar();
        }
        
    } while (choice != 0);
    
    log_message(LOG_INFO, "System Settings module ended");
}

void show_system_settings_menu() {
    system("cls");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    SİSTEM AYARLARI                          ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Sistem Bilgilerini Görüntüle                            ║\n");
    printf("║  2. Başlangıç Programlarını Yönet                           ║\n");
    printf("║  3. Güç Ayarlarını Yapılandır                               ║\n");
    printf("║  4. Ortam Değişkenlerini Yönet                              ║\n");
    printf("║  5. Sistem Performansını Yapılandır                         ║\n");
    printf("║  6. Registry Yedekleme/Geri Yükleme                         ║\n");
    printf("║  7. Yüklü Programları Görüntüle                             ║\n");
    printf("║  8. Kullanıcı Hesaplarını Yapılandır                        ║\n");
    printf("║  9. Sistem Bakımı                                           ║\n");
    printf("║  0. Ana Menüye Dön                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

// Function to get real Windows version
typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

void get_real_windows_version(DWORD* major, DWORD* minor, DWORD* build) {
    HMODULE hMod = GetModuleHandle(TEXT("ntdll.dll"));
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != NULL) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fxPtr(&rovi) == 0) {
                *major = rovi.dwMajorVersion;
                *minor = rovi.dwMinorVersion;
                *build = rovi.dwBuildNumber;
                return;
            }
        }
    }
    // Fallback to registry method
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dwType, dwSize;
        
        dwSize = sizeof(DWORD);
        RegQueryValueEx(hKey, TEXT("CurrentMajorVersionNumber"), NULL, &dwType, (LPBYTE)major, &dwSize);
        RegQueryValueEx(hKey, TEXT("CurrentMinorVersionNumber"), NULL, &dwType, (LPBYTE)minor, &dwSize);
        RegQueryValueEx(hKey, TEXT("CurrentBuildNumber"), NULL, &dwType, (LPBYTE)build, &dwSize);
        
        RegCloseKey(hKey);
    }
}

const char* get_windows_version_name(DWORD major, DWORD minor, DWORD build) {
    if (major == 10) {
        if (build >= 22000) return "Windows 11";
        else return "Windows 10";
    } else if (major == 6) {
        if (minor == 3) return "Windows 8.1";
        else if (minor == 2) return "Windows 8";
        else if (minor == 1) return "Windows 7";
    }
    return "Windows";
}

void view_system_info() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    SİSTEM BİLGİLERİ                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    // Get real Windows version
    DWORD major = 0, minor = 0, build = 0;
    get_real_windows_version(&major, &minor, &build);
    
    const char* versionName = get_windows_version_name(major, minor, build);
    printf("İşletim Sistemi: %s (%d.%d Build %d)\n", versionName, major, minor, build);
    
    // Get additional version info from registry
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char displayVersion[256] = {0};
        DWORD dwType, dwSize = sizeof(displayVersion);
        if (RegQueryValueEx(hKey, TEXT("DisplayVersion"), NULL, &dwType, (LPBYTE)displayVersion, &dwSize) == ERROR_SUCCESS) {
            printf("Sürüm: %s\n", displayVersion);
        }
        
        char productName[256] = {0};
        dwSize = sizeof(productName);
        if (RegQueryValueEx(hKey, TEXT("ProductName"), NULL, &dwType, (LPBYTE)productName, &dwSize) == ERROR_SUCCESS) {
            printf("Ürün Adı: %s\n", productName);
        }
        
        RegCloseKey(hKey);
    }
    
    // Get computer name
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerName(computerName, &size)) {
        printf("Bilgisayar Adı: %s\n", computerName);
    }
    
    // Get user name
    char userName[UNLEN + 1];
    size = sizeof(userName);
    if (GetUserName(userName, &size)) {
        printf("Kullanıcı Adı: %s\n", userName);
    }
    
    // Get system directory
    char sysDir[MAX_PATH];
    if (GetSystemDirectory(sysDir, MAX_PATH)) {
        printf("Sistem Dizini: %s\n", sysDir);
    }
    
    // Get Windows directory
    char winDir[MAX_PATH];
    if (GetWindowsDirectory(winDir, MAX_PATH)) {
        printf("Windows Dizini: %s\n", winDir);
    }
    
    // Get memory information
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        printf("Toplam RAM: %.2f GB\n", (double)memInfo.ullTotalPhys / (1024*1024*1024));
        printf("Kullanılabilir RAM: %.2f GB\n", (double)memInfo.ullAvailPhys / (1024*1024*1024));
        printf("Bellek Kullanımı: %d%%\n", memInfo.dwMemoryLoad);
    }
    
    // Get processor information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    printf("İşlemci Sayısı: %d\n", sysInfo.dwNumberOfProcessors);
    printf("İşlemci Mimarisi: ");
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("x64 (AMD or Intel)\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("x86\n");
            break;
        default:
            printf("Bilinmeyen\n");
    }
    
    log_message(LOG_INFO, "System information displayed");
}

void manage_startup_programs() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                BAŞLANGIÇ PROGRAMLARI                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Mevcut başlangıç programlarını listele\n");
    printf("2. Başlangıç programı ekle\n");
    printf("3. Başlangıç programı kaldır\n");
    printf("Seçiminiz (1-3): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // List startup programs from registry
            HKEY hKey;
            const char* startupKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
            
            if (RegOpenKeyEx(HKEY_CURRENT_USER, startupKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                printf("\n=== Kullanıcı Başlangıç Programları ===\n");
                
                DWORD index = 0;
                char valueName[MAX_VALUE_LEN];
                char valueData[MAX_PATH_LEN];
                DWORD valueNameSize, valueDataSize;
                
                while (1) {
                    valueNameSize = sizeof(valueName);
                    valueDataSize = sizeof(valueData);
                    
                    LONG result = RegEnumValue(hKey, index, valueName, &valueNameSize, 
                                             NULL, NULL, (LPBYTE)valueData, &valueDataSize);
                    
                    if (result == ERROR_NO_MORE_ITEMS) break;
                    if (result == ERROR_SUCCESS) {
                        printf("%d. %s: %s\n", index + 1, valueName, valueData);
                    }
                    index++;
                }
                
                RegCloseKey(hKey);
            }
            
            // Also check system-wide startup
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, startupKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                printf("\n=== Sistem Başlangıç Programları ===\n");
                
                DWORD index = 0;
                char valueName[MAX_VALUE_LEN];
                char valueData[MAX_PATH_LEN];
                DWORD valueNameSize, valueDataSize;
                
                while (1) {
                    valueNameSize = sizeof(valueName);
                    valueDataSize = sizeof(valueData);
                    
                    LONG result = RegEnumValue(hKey, index, valueName, &valueNameSize, 
                                             NULL, NULL, (LPBYTE)valueData, &valueDataSize);
                    
                    if (result == ERROR_NO_MORE_ITEMS) break;
                    if (result == ERROR_SUCCESS) {
                        printf("%d. %s: %s\n", index + 1, valueName, valueData);
                    }
                    index++;
                }
                
                RegCloseKey(hKey);
            }
            break;
        }
        case 2: {
            // Add startup program
            char programName[MAX_VALUE_LEN];
            char programPath[MAX_PATH_LEN];
            
            printf("Program adı: ");
            scanf("%s", programName);
            printf("Program yolu: ");
            scanf("%s", programPath);
            
            if (write_registry_string(HKEY_CURRENT_USER, 
                                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                    programName, programPath)) {
                printf("Başlangıç programı başarıyla eklendi!\n");
                log_message(LOG_INFO, "Startup program added: %s", programName);
            } else {
                printf("Başlangıç programı eklenirken hata oluştu!\n");
                log_message(LOG_ERROR, "Failed to add startup program: %s", programName);
            }
            break;
        }
        case 3: {
            // Remove startup program
            char programName[MAX_VALUE_LEN];
            printf("Kaldırılacak program adı: ");
            scanf("%s", programName);
            
            if (delete_registry_value(HKEY_CURRENT_USER,
                                    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                                    programName)) {
                printf("Başlangıç programı başarıyla kaldırıldı!\n");
                log_message(LOG_INFO, "Startup program removed: %s", programName);
            } else {
                printf("Başlangıç programı kaldırılırken hata oluştu!\n");
                log_message(LOG_ERROR, "Failed to remove startup program: %s", programName);
            }
            break;
        }
    }
}

void configure_power_settings() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    GÜÇ AYARLARI                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Mevcut güç planını görüntüle\n");
    printf("2. Güç planını değiştir\n");
    printf("3. Uyku ayarlarını yapılandır\n");
    printf("4. Ekran kapatma süresini ayarla\n");
    printf("Seçiminiz (1-4): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // Show current power plan
            system("powercfg /getactivescheme");
            break;
        }
        case 2: {
            // List and change power plans
            printf("\nMevcut güç planları:\n");
            system("powercfg /list");
            
            char guid[100];
            printf("\nAktif hale getirilecek plan GUID'i: ");
            scanf("%s", guid);
            
            char command[200];
            sprintf(command, "powercfg /setactive %s", guid);
            
            if (system(command) == 0) {
                printf("Güç planı başarıyla değiştirildi!\n");
                log_message(LOG_INFO, "Power plan changed to: %s", guid);
            } else {
                printf("Güç planı değiştirilirken hata oluştu!\n");
                log_message(LOG_ERROR, "Failed to change power plan to: %s", guid);
            }
            break;
        }
        case 3: {
            // Configure sleep settings
            int minutes;
            printf("Uyku moduna geçme süresi (dakika, 0=hiçbir zaman): ");
            scanf("%d", &minutes);
            
            char command[200];
            sprintf(command, "powercfg /change standby-timeout-ac %d", minutes);
            system(command);
            sprintf(command, "powercfg /change standby-timeout-dc %d", minutes);
            system(command);
            
            printf("Uyku ayarları güncellendi!\n");
            log_message(LOG_INFO, "Sleep timeout set to %d minutes", minutes);
            break;
        }
        case 4: {
            // Configure display timeout
            int minutes;
            printf("Ekran kapatma süresi (dakika, 0=hiçbir zaman): ");
            scanf("%d", &minutes);
            
            char command[200];
            sprintf(command, "powercfg /change monitor-timeout-ac %d", minutes);
            system(command);
            sprintf(command, "powercfg /change monitor-timeout-dc %d", minutes);
            system(command);
            
            printf("Ekran kapatma ayarları güncellendi!\n");
            log_message(LOG_INFO, "Display timeout set to %d minutes", minutes);
            break;
        }
    }
}

void manage_environment_variables() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                ORTAM DEĞİŞKENLERİ                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Mevcut ortam değişkenlerini listele\n");
    printf("2. Yeni ortam değişkeni ekle\n");
    printf("3. Ortam değişkenini güncelle\n");
    printf("4. Ortam değişkenini sil\n");
    printf("Seçiminiz (1-4): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // List environment variables
            printf("\n=== Sistem Ortam Değişkenleri ===\n");
            system("set");
            break;
        }
        case 2:
        case 3: {
            // Add or update environment variable
            char varName[MAX_VALUE_LEN];
            char varValue[MAX_PATH_LEN];
            
            printf("Değişken adı: ");
            scanf("%s", varName);
            printf("Değişken değeri: ");
            scanf("%s", varValue);
            
            if (SetEnvironmentVariable(varName, varValue)) {
                printf("Ortam değişkeni başarıyla %s!\n", 
                       choice == 2 ? "eklendi" : "güncellendi");
                log_message(LOG_INFO, "Environment variable %s: %s = %s", 
                           choice == 2 ? "added" : "updated", varName, varValue);
                
                // Also set in registry for persistence
                char regPath[500];
                sprintf(regPath, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
                write_registry_string(HKEY_LOCAL_MACHINE, regPath, varName, varValue);
            } else {
                printf("Ortam değişkeni işlemi başarısız!\n");
                log_message(LOG_ERROR, "Failed to set environment variable: %s", varName);
            }
            break;
        }
        case 4: {
            // Delete environment variable
            char varName[MAX_VALUE_LEN];
            printf("Silinecek değişken adı: ");
            scanf("%s", varName);
            
            if (SetEnvironmentVariable(varName, NULL)) {
                printf("Ortam değişkeni başarıyla silindi!\n");
                log_message(LOG_INFO, "Environment variable deleted: %s", varName);
                
                // Also remove from registry
                char regPath[500];
                sprintf(regPath, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
                delete_registry_value(HKEY_LOCAL_MACHINE, regPath, varName);
            } else {
                printf("Ortam değişkeni silinemedi!\n");
                log_message(LOG_ERROR, "Failed to delete environment variable: %s", varName);
            }
            break;
        }
    }
}

void configure_system_performance() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                SİSTEM PERFORMANSI                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Sistem performans bilgilerini görüntüle\n");
    printf("2. Sanal bellek ayarlarını yapılandır\n");
    printf("3. Sistem önceliklerini ayarla\n");
    printf("4. Geçici dosyaları temizle\n");
    printf("Seçiminiz (1-4): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // Display performance information
            printf("\n=== Sistem Performans Bilgileri ===\n");
            system("wmic cpu get name,maxclockspeed,numberofcores");
            system("wmic memorychip get capacity,speed");
            system("wmic logicaldisk get size,freespace,caption");
            break;
        }
        case 2: {
            // Configure virtual memory
            printf("Sanal bellek ayarları için Sistem Özellikleri açılıyor...\n");
            system("sysdm.cpl");
            log_message(LOG_INFO, "Virtual memory configuration opened");
            break;
        }
        case 3: {
            // Set system priorities
            printf("1. Programlar için optimize et\n");
            printf("2. Arka plan servisleri için optimize et\n");
            printf("Seçiminiz (1-2): ");
            
            int priority;
            scanf("%d", &priority);
            
            DWORD value = (priority == 1) ? 2 : 24; // Programs vs Background services
            
            if (write_registry_string(HKEY_LOCAL_MACHINE,
                                    "SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
                                    "Win32PrioritySeparation", 
                                    (priority == 1) ? "2" : "24")) {
                printf("Sistem öncelikleri güncellendi! Değişikliklerin etkili olması için yeniden başlatın.\n");
                log_message(LOG_INFO, "System priority set to: %s", 
                           priority == 1 ? "Programs" : "Background services");
            }
            break;
        }
        case 4: {
            // Clean temporary files
            printf("Geçici dosyalar temizleniyor...\n");
            
            system("del /q /f /s %TEMP%\\*");
            system("del /q /f /s C:\\Windows\\Temp\\*");
            system("cleanmgr /sagerun:1");
            
            printf("Geçici dosyalar temizlendi!\n");
            log_message(LOG_INFO, "Temporary files cleaned");
            break;
        }
    }
}

void backup_restore_registry() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║            REGISTRY YEDEKLEME/GERİ YÜKLEME                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Registry yedekle\n");
    printf("2. Registry geri yükle\n");
    printf("3. Belirli bir anahtarı yedekle\n");
    printf("Seçiminiz (1-3): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // Backup entire registry
            char filename[MAX_PATH];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            
            sprintf(filename, "registry_backup_%04d%02d%02d_%02d%02d%02d.reg",
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                    t->tm_hour, t->tm_min, t->tm_sec);
            
            char command[500];
            sprintf(command, "regedit /e \"%s\"", filename);
            
            if (system(command) == 0) {
                printf("Registry yedekleme tamamlandı: %s\n", filename);
                log_message(LOG_INFO, "Registry backup created: %s", filename);
            } else {
                printf("Registry yedekleme başarısız!\n");
                log_message(LOG_ERROR, "Registry backup failed");
            }
            break;
        }
        case 2: {
            // Restore registry
            char filename[MAX_PATH];
            printf("Geri yüklenecek dosya adı: ");
            scanf("%s", filename);
            
            char command[500];
            sprintf(command, "regedit /s \"%s\"", filename);
            
            printf("DİKKAT: Bu işlem mevcut registry ayarlarını değiştirecek!\n");
            printf("Devam etmek istiyor musunuz? (E/H): ");
            
            char confirm;
            scanf(" %c", &confirm);
            
            if (confirm == 'E' || confirm == 'e') {
                if (system(command) == 0) {
                    printf("Registry geri yükleme tamamlandı!\n");
                    log_message(LOG_INFO, "Registry restored from: %s", filename);
                } else {
                    printf("Registry geri yükleme başarısız!\n");
                    log_message(LOG_ERROR, "Registry restore failed from: %s", filename);
                }
            }
            break;
        }
        case 3: {
            // Backup specific key
            char keyPath[MAX_PATH];
            char filename[MAX_PATH];
            
            printf("Yedeklenecek registry anahtarı (örn: HKEY_CURRENT_USER\\Software): ");
            scanf("%s", keyPath);
            printf("Yedek dosya adı: ");
            scanf("%s", filename);
            
            char command[500];
            sprintf(command, "regedit /e \"%s\" \"%s\"", filename, keyPath);
            
            if (system(command) == 0) {
                printf("Registry anahtarı yedeklendi: %s\n", filename);
                log_message(LOG_INFO, "Registry key backed up: %s to %s", keyPath, filename);
            } else {
                printf("Registry anahtarı yedekleme başarısız!\n");
                log_message(LOG_ERROR, "Registry key backup failed: %s", keyPath);
            }
            break;
        }
    }
}

void view_installed_programs() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  YÜKLÜ PROGRAMLAR                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("Yüklü programlar listeleniyor...\n\n");
    
    // Use WMI to get installed programs
    system("wmic product get name,version,vendor");
    
    printf("\n=== Alternatif Liste (Registry'den) ===\n");
    
    // Also check registry for installed programs
    HKEY hKey;
    const char* uninstallKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, uninstallKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD index = 0;
        char subKeyName[MAX_VALUE_LEN];
        DWORD subKeyNameSize;
        
        while (1) {
            subKeyNameSize = sizeof(subKeyName);
            LONG result = RegEnumKeyEx(hKey, index, subKeyName, &subKeyNameSize, 
                                     NULL, NULL, NULL, NULL);
            
            if (result == ERROR_NO_MORE_ITEMS) break;
            if (result == ERROR_SUCCESS) {
                // Open the subkey to get program details
                HKEY hSubKey;
                if (RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                    char displayName[MAX_VALUE_LEN];
                    char version[MAX_VALUE_LEN];
                    char publisher[MAX_VALUE_LEN];
                    DWORD dataSize;
                    
                    // Get display name
                    dataSize = sizeof(displayName);
                    if (RegQueryValueEx(hSubKey, "DisplayName", NULL, NULL, 
                                      (LPBYTE)displayName, &dataSize) == ERROR_SUCCESS) {
                        
                        // Get version
                        dataSize = sizeof(version);
                        if (RegQueryValueEx(hSubKey, "DisplayVersion", NULL, NULL, 
                                          (LPBYTE)version, &dataSize) != ERROR_SUCCESS) {
                            strcpy(version, "Bilinmiyor");
                        }
                        
                        // Get publisher
                        dataSize = sizeof(publisher);
                        if (RegQueryValueEx(hSubKey, "Publisher", NULL, NULL, 
                                          (LPBYTE)publisher, &dataSize) != ERROR_SUCCESS) {
                            strcpy(publisher, "Bilinmiyor");
                        }
                        
                        printf("Program: %s\n", displayName);
                        printf("Sürüm: %s\n", version);
                        printf("Yayıncı: %s\n", publisher);
                        printf("---\n");
                    }
                    
                    RegCloseKey(hSubKey);
                }
            }
            index++;
        }
        
        RegCloseKey(hKey);
    }
    
    log_message(LOG_INFO, "Installed programs list displayed");
}

void configure_user_accounts() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║               KULLANICI HESAPLARI                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Mevcut kullanıcıları listele\n");
    printf("2. Yeni kullanıcı oluştur\n");
    printf("3. Kullanıcı şifresini değiştir\n");
    printf("4. Kullanıcı hesabını etkinleştir/devre dışı bırak\n");
    printf("5. Kullanıcı gruplarını yönet\n");
    printf("Seçiminiz (1-5): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // List users
            printf("\n=== Sistem Kullanıcıları ===\n");
            system("net user");
            break;
        }
        case 2: {
            // Create new user
            char username[100];
            char password[100];
            
            printf("Yeni kullanıcı adı: ");
            scanf("%s", username);
            printf("Şifre: ");
            scanf("%s", password);
            
            char command[300];
            sprintf(command, "net user %s %s /add", username, password);
            
            if (system(command) == 0) {
                printf("Kullanıcı başarıyla oluşturuldu!\n");
                log_message(LOG_INFO, "User created: %s", username);
            } else {
                printf("Kullanıcı oluşturulamadı!\n");
                log_message(LOG_ERROR, "Failed to create user: %s", username);
            }
            break;
        }
        case 3: {
            // Change user password
            char username[100];
            char password[100];
            
            printf("Kullanıcı adı: ");
            scanf("%s", username);
            printf("Yeni şifre: ");
            scanf("%s", password);
            
            char command[300];
            sprintf(command, "net user %s %s", username, password);
            
            if (system(command) == 0) {
                printf("Şifre başarıyla değiştirildi!\n");
                log_message(LOG_INFO, "Password changed for user: %s", username);
            } else {
                printf("Şifre değiştirilemedi!\n");
                log_message(LOG_ERROR, "Failed to change password for user: %s", username);
            }
            break;
        }
        case 4: {
            // Enable/disable user account
            char username[100];
            char action[20];
            
            printf("Kullanıcı adı: ");
            scanf("%s", username);
            printf("İşlem (active/inactive): ");
            scanf("%s", action);
            
            char command[300];
            sprintf(command, "net user %s /%s", username, action);
            
            if (system(command) == 0) {
                printf("Kullanıcı hesabı durumu güncellendi!\n");
                log_message(LOG_INFO, "User account %s set to %s", username, action);
            } else {
                printf("Kullanıcı hesabı durumu güncellenemedi!\n");
                log_message(LOG_ERROR, "Failed to set user %s to %s", username, action);
            }
            break;
        }
        case 5: {
            // Manage user groups
            printf("\n=== Sistem Grupları ===\n");
            system("net localgroup");
            
            printf("\nBir grubun üyelerini görmek için grup adını girin (boş bırakın): ");
            char groupName[100];
            scanf("%s", groupName);
            
            if (strlen(groupName) > 0) {
                char command[200];
                sprintf(command, "net localgroup \"%s\"", groupName);
                system(command);
            }
            break;
        }
    }
}

void system_maintenance() {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   SİSTEM BAKIMI                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("1. Disk temizliği yap\n");
    printf("2. Sistem dosyalarını kontrol et (SFC)\n");
    printf("3. Disk hatalarını kontrol et (CHKDSK)\n");
    printf("4. Sistem performansını analiz et\n");
    printf("5. Windows Update durumunu kontrol et\n");
    printf("Seçiminiz (1-5): ");
    
    int choice;
    scanf("%d", &choice);
    
    switch (choice) {
        case 1: {
            // Disk cleanup
            printf("Disk temizliği başlatılıyor...\n");
            system("cleanmgr");
            log_message(LOG_INFO, "Disk cleanup initiated");
            break;
        }
        case 2: {
            // System file checker
            printf("Sistem dosyaları kontrol ediliyor (bu işlem uzun sürebilir)...\n");
            system("sfc /scannow");
            log_message(LOG_INFO, "System file check completed");
            break;
        }
        case 3: {
            // Check disk
            char drive[10];
            printf("Kontrol edilecek sürücü (örn: C:): ");
            scanf("%s", drive);
            
            char command[100];
            sprintf(command, "chkdsk %s /f", drive);
            
            printf("Disk kontrol ediliyor...\n");
            system(command);
            log_message(LOG_INFO, "Disk check completed for drive: %s", drive);
            break;
        }
        case 4: {
            // Performance analysis
            printf("Sistem performansı analiz ediliyor...\n");
            system("perfmon /report");
            log_message(LOG_INFO, "Performance analysis initiated");
            break;
        }
        case 5: {
            // Windows Update status
            printf("Windows Update durumu kontrol ediliyor...\n");
            system("powershell Get-WindowsUpdate");
            log_message(LOG_INFO, "Windows Update status checked");
            break;
        }
    }
}

// Registry helper functions implementation
int read_registry_string(HKEY hKey, const char* subKey, const char* valueName, char* buffer, DWORD bufferSize) {
    HKEY hOpenKey;
    LONG result = RegOpenKeyEx(hKey, subKey, 0, KEY_READ, &hOpenKey);
    
    if (result != ERROR_SUCCESS) {
        return 0;
    }
    
    DWORD dataType;
    result = RegQueryValueEx(hOpenKey, valueName, NULL, &dataType, (LPBYTE)buffer, &bufferSize);
    
    RegCloseKey(hOpenKey);
    
    return (result == ERROR_SUCCESS && dataType == REG_SZ);
}

int write_registry_string(HKEY hKey, const char* subKey, const char* valueName, const char* value) {
    HKEY hOpenKey;
    LONG result = RegCreateKeyEx(hKey, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                KEY_WRITE, NULL, &hOpenKey, NULL);
    
    if (result != ERROR_SUCCESS) {
        return 0;
    }
    
    result = RegSetValueEx(hOpenKey, valueName, 0, REG_SZ, 
                          (const BYTE*)value, strlen(value) + 1);
    
    RegCloseKey(hOpenKey);
    
    return (result == ERROR_SUCCESS);
}

int delete_registry_value(HKEY hKey, const char* subKey, const char* valueName) {
    HKEY hOpenKey;
    LONG result = RegOpenKeyEx(hKey, subKey, 0, KEY_WRITE, &hOpenKey);
    
    if (result != ERROR_SUCCESS) {
        return 0;
    }
    
    result = RegDeleteValue(hOpenKey, valueName);
    
    RegCloseKey(hOpenKey);
    
    return (result == ERROR_SUCCESS);
}