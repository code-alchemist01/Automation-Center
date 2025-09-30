/*
 * ========================================
 * Sistem İzleme ve Performans Modülü
 * ========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
    #include <pdh.h>
    #include <tlhelp32.h>
    #include <versionhelpers.h>
#else
    #include <unistd.h>
    #include <sys/sysinfo.h>
#endif

#include "../../include/logger.h"
#include "../../include/database.h"

void show_system_monitor_menu() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SİSTEM İZLEME MODÜLÜ                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [1] CPU Kullanımı İzleme                                   ║\n");
    printf("║  [2] Bellek (RAM) Durumu                                    ║\n");
    printf("║  [3] Disk Kullanımı Analizi                                 ║\n");
    printf("║  [4] Çalışan Süreçler                                       ║\n");
    printf("║  [5] Sistem Bilgileri                                       ║\n");
    printf("║  [6] Performans Grafiği                                     ║\n");
    printf("║  [0] Ana Menüye Dön                                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nSeçiminizi yapın (0-6): ");
}

void monitor_cpu() {
    printf("\nCPU Kullanımı İzleme\n");
    printf("===================\n");
    
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    printf("🖥️  İşlemci Çekirdek Sayısı: %lu\n", sysInfo.dwNumberOfProcessors);
    
    // CPU kullanımı için PDH sorgusu oluştur
    PDH_HQUERY cpuQuery;
    PDH_HCOUNTER cpuTotal;
    
    PdhOpenQuery(NULL, 0, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
    
    // İlk ölçüm için bekle
    Sleep(1000);
    
    PdhCollectQueryData(cpuQuery);
    PDH_FMT_COUNTERVALUE counterVal;
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    double cpuUsage = counterVal.doubleValue;
    
    printf("📊 Anlık CPU Kullanımı: %.1f%%\n", cpuUsage);
    
    // CPU kullanımı grafiği
    int bars = (int)(cpuUsage / 10);
    printf("   ");
    for(int i = 0; i < 10; i++) {
        if(i < bars) printf("▓");
        else printf("░");
    }
    printf(" %.1f%%\n\n", cpuUsage);
    
    PdhCloseQuery(cpuQuery);
    
    // Sistem çalışma süresi
    ULONGLONG uptime = GetTickCount64();
    int days = uptime / (1000 * 60 * 60 * 24);
    int hours = (uptime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    int minutes = (uptime % (1000 * 60 * 60)) / (1000 * 60);
    
    printf("⏰ Sistem Çalışma Süresi: %d gün, %d saat, %d dakika\n", days, hours, minutes);
    
    // Veritabanına kaydet
    SystemMetrics metrics = {0};
    metrics.cpu_usage = cpuUsage;
    metrics.memory_usage = 0; // Şimdilik 0, memory fonksiyonunda doldurulacak
    metrics.disk_usage = 0;
    strcpy(metrics.hostname, "localhost");
    metrics.timestamp = time(NULL);
    
    if (insert_system_metrics(&metrics)) {
        printf("✅ CPU verileri veritabanına kaydedildi\n");
    } else {
        printf("❌ CPU verileri veritabanına kaydedilemedi\n");
    }
    
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("CPU kullanımı izleme işlemi tamamlandı");
}

void monitor_memory() {
    printf("\nBellek (RAM) Durumu\n");
    printf("==================\n");
    
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    DWORDLONG totalVirtualMem = memInfo.ullTotalVirtual;
    DWORDLONG virtualMemUsed = memInfo.ullTotalVirtual - memInfo.ullAvailVirtual;
    
    // Byte'ları GB'ye çevir
    double totalGB = totalPhysMem / (1024.0 * 1024.0 * 1024.0);
    double usedGB = physMemUsed / (1024.0 * 1024.0 * 1024.0);
    double availableGB = memInfo.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
    double usagePercent = (double)physMemUsed / totalPhysMem * 100.0;
    
    printf("💾 Toplam RAM: %.1f GB\n", totalGB);
    printf("✅ Kullanılabilir: %.1f GB\n", availableGB);
    printf("🔴 Kullanımda: %.1f GB (%.1f%%)\n\n", usedGB, usagePercent);
    
    // Bellek kullanım grafiği
    printf("📊 Bellek Kullanım Grafiği:\n");
    int bars = (int)(usagePercent / 10);
    printf("   ");
    for(int i = 0; i < 10; i++) {
        if(i < bars) printf("▓");
        else printf("░");
    }
    printf(" %.1f%%\n\n", usagePercent);
    
    // Sanal bellek bilgisi
    double totalVirtualGB = totalVirtualMem / (1024.0 * 1024.0 * 1024.0);
    double usedVirtualGB = virtualMemUsed / (1024.0 * 1024.0 * 1024.0);
    
    printf("🔄 Sanal Bellek:\n");
    printf("   📊 Toplam: %.1f GB\n", totalVirtualGB);
    printf("   🔴 Kullanımda: %.1f GB\n", usedVirtualGB);
    
    // Bellek yükü uyarısı
    if(usagePercent > 80) {
        printf("\n⚠️  UYARI: Bellek kullanımı %%80'in üzerinde!\n");
    } else if(usagePercent > 60) {
        printf("\n⚡ BİLGİ: Bellek kullanımı yüksek (%%60+)\n");
    }
    
    // Veritabanına kaydet
    SystemMetrics metrics = {0};
    metrics.cpu_usage = 0; // CPU verisi yok, sadece memory
    metrics.memory_usage = usagePercent;
    metrics.disk_usage = 0;
    strcpy(metrics.hostname, "localhost");
    metrics.timestamp = time(NULL);
    
    if (insert_system_metrics(&metrics)) {
        printf("✅ Bellek verileri veritabanına kaydedildi\n");
    } else {
        printf("❌ Bellek verileri veritabanına kaydedilemedi\n");
    }
    
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("Bellek durumu izleme işlemi tamamlandı");
}

void analyze_disk_usage() {
    printf("\nDisk Kullanımı Analizi\n");
    printf("=====================\n");
    printf("💿 Disk Sürücüleri:\n\n");
    
#ifdef _WIN32
    DWORD drives = GetLogicalDrives();
    char driveLetter = 'A';
    
    for(int i = 0; i < 26; i++) {
        if(drives & (1 << i)) {
            char rootPath[4];
            sprintf(rootPath, "%c:\\", driveLetter + i);
            
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            
            if(GetDiskFreeSpaceEx(rootPath, &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                double totalGB = totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                double freeGB = totalNumberOfFreeBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
                double usedGB = totalGB - freeGB;
                double usagePercent = (usedGB / totalGB) * 100.0;
                
                // Sürücü türünü belirle
                UINT driveType = GetDriveType(rootPath);
                char* driveTypeStr = "Bilinmeyen";
                char* icon = "📀";
                
                switch(driveType) {
                    case DRIVE_FIXED:
                        driveTypeStr = "Sabit Disk";
                        icon = "💾";
                        break;
                    case DRIVE_REMOVABLE:
                        driveTypeStr = "Çıkarılabilir Disk";
                        icon = "💿";
                        break;
                    case DRIVE_CDROM:
                        driveTypeStr = "CD/DVD";
                        icon = "💿";
                        break;
                    case DRIVE_REMOTE:
                        driveTypeStr = "Ağ Sürücüsü";
                        icon = "🌐";
                        break;
                }
                
                printf("%s %s (%s)\n", icon, rootPath, driveTypeStr);
                printf("   📊 Toplam: %.1f GB\n", totalGB);
                printf("   ✅ Boş: %.1f GB\n", freeGB);
                printf("   🔴 Dolu: %.1f GB (%.1f%%)\n", usedGB, usagePercent);
                
                // Kullanım grafiği
                printf("   ");
                int bars = (int)(usagePercent / 10);
                for(int j = 0; j < 10; j++) {
                    if(j < bars) printf("▓");
                    else printf("░");
                }
                printf(" %.1f%%\n", usagePercent);
                
                // Uyarı mesajları
                if(usagePercent > 90) {
                    printf("   ⚠️  KRİTİK: Disk alanı %%90'ın üzerinde!\n");
                } else if(usagePercent > 80) {
                    printf("   ⚠️  UYARI: Disk alanı %%80'in üzerinde!\n");
                } else if(usagePercent > 70) {
                    printf("   ⚡ BİLGİ: Disk alanı %%70'in üzerinde\n");
                }
                
                printf("\n");
            }
        }
    }
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("Disk kullanımı analizi tamamlandı");
}

void show_running_processes() {
    printf("\nÇalışan Süreçler\n");
    printf("===============\n");
    
#ifdef _WIN32
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    int processCount = 0;
    
    // Süreç listesinin anlık görüntüsünü al
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("❌ Süreç listesi alınamadı!\n");
        return;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    // İlk süreç bilgisini al
    if (!Process32First(hProcessSnap, &pe32)) {
        printf("❌ Süreç bilgileri alınamadı!\n");
        CloseHandle(hProcessSnap);
        return;
    }
    
    // Toplam süreç sayısını hesapla
    do {
        processCount++;
    } while (Process32Next(hProcessSnap, &pe32));
    
    printf("🔄 Aktif süreç sayısı: %d\n\n", processCount);
    
    // Süreç listesini yeniden başlat
    CloseHandle(hProcessSnap);
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pe32.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hProcessSnap, &pe32);
    
    printf("🔝 Çalışan süreçler (ilk 10):\n");
    printf("┌─────────────────────────┬─────────┬─────────────────────┐\n");
    printf("│ Süreç Adı               │ PID     │ Thread Sayısı       │\n");
    printf("├─────────────────────────┼─────────┼─────────────────────┤\n");
    
    int displayCount = 0;
    do {
        if (displayCount >= 10) break;
        
        // Süreç adını kısalt
        char processName[26];
        strncpy(processName, pe32.szExeFile, 24);
        processName[24] = '\0';
        
        printf("│ %-23s │ %-7lu │ %-19lu │\n", 
               processName, pe32.th32ProcessID, pe32.cntThreads);
        
        displayCount++;
    } while (Process32Next(hProcessSnap, &pe32));
    
    printf("└─────────────────────────┴─────────┴─────────────────────┘\n");
    
    CloseHandle(hProcessSnap);
    
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("Çalışan süreçler listesi görüntülendi");
}

void show_system_info() {
    printf("\nSistem Bilgileri\n");
    printf("===============\n");
    
#ifdef _WIN32
    // İşletim sistemi bilgisi
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        printf("🖥️  İşletim Sistemi: Windows %lu.%lu (Build %lu)\n", 
               osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    } else {
        printf("🖥️  İşletim Sistemi: Windows (Sürüm bilgisi alınamadı)\n");
    }
    
    // Bilgisayar adı
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerName(computerName, &size)) {
        printf("💻 Bilgisayar Adı: %s\n", computerName);
    } else {
        printf("💻 Bilgisayar Adı: Bilinmiyor\n");
    }
    
    // Kullanıcı adı
    printf("👤 Kullanıcı: %s\n", getenv("USERNAME") ? getenv("USERNAME") : "Bilinmiyor");
    
    // İşlemci bilgisi
    char processorName[256];
    DWORD bufferSize = sizeof(processorName);
    HKEY hKey;
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, 
                           (LPBYTE)processorName, &bufferSize) == ERROR_SUCCESS) {
            printf("🖥️  İşlemci: %s\n", processorName);
        } else {
            printf("🖥️  İşlemci: Bilgi alınamadı\n");
        }
        RegCloseKey(hKey);
    } else {
        printf("🖥️  İşlemci: Bilgi alınamadı\n");
    }
    
    // RAM bilgisi
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        double totalRAM = memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
        printf("💾 RAM: %.1f GB\n", totalRAM);
    } else {
        printf("💾 RAM: Bilgi alınamadı\n");
    }
    
    // Sistem çalışma süresi
    ULONGLONG uptime = GetTickCount64();
    ULONGLONG days = uptime / (1000 * 60 * 60 * 24);
    ULONGLONG hours = (uptime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    ULONGLONG minutes = (uptime % (1000 * 60 * 60)) / (1000 * 60);
    
    printf("\n⏰ Sistem Çalışma Süresi: %llu gün, %llu saat, %llu dakika\n", 
           days, hours, minutes);
    
    // Son boot zamanı
    time_t bootTime = time(NULL) - (uptime / 1000);
    struct tm* bootTm = localtime(&bootTime);
    printf("🔄 Son Yeniden Başlatma: %04d-%02d-%02d %02d:%02d:%02d\n",
           bootTm->tm_year + 1900, bootTm->tm_mon + 1, bootTm->tm_mday,
           bootTm->tm_hour, bootTm->tm_min, bootTm->tm_sec);
    
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("Sistem bilgileri görüntülendi");
}

void show_performance_graph() {
    printf("\nPerformans Grafiği (Gerçek Zamanlı)\n");
    printf("===================================\n");
    
    printf("📊 5 saniye boyunca sistem performansı izleniyor...\n");
    printf("    (Çıkmak için Ctrl+C tuşlayın)\n\n");
    
    #ifdef _WIN32
    // PDH sorguları oluştur
    PDH_HQUERY cpuQuery, memQuery;
    PDH_HCOUNTER cpuTotal;
    
    PdhOpenQuery(NULL, 0, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", 0, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
    
    // Bellek bilgisi için
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    printf("Zaman    CPU%%    RAM%%    CPU Graf.    RAM Graf.\n");
    printf("================================================\n");
    
    for(int i = 0; i < 10; i++) {
        Sleep(500); // 0.5 saniye bekle
        
        // CPU kullanımını al
        PdhCollectQueryData(cpuQuery);
        PDH_FMT_COUNTERVALUE counterVal;
        PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
        double cpuUsage = counterVal.doubleValue;
        
        // RAM kullanımını al
        GlobalMemoryStatusEx(&memInfo);
        double ramUsage = (double)(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / memInfo.ullTotalPhys * 100.0;
        
        // Zaman damgası
        time_t now = time(NULL);
        struct tm *timeinfo = localtime(&now);
        
        printf("%02d:%02d:%02d  %5.1f  %5.1f    ", 
               timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
               cpuUsage, ramUsage);
        
        // CPU grafiği
        int cpuBars = (int)(cpuUsage / 10);
        for(int j = 0; j < 10; j++) {
            if(j < cpuBars) printf("▓");
            else printf("░");
        }
        printf("  ");
        
        // RAM grafiği
        int ramBars = (int)(ramUsage / 10);
        for(int j = 0; j < 10; j++) {
            if(j < ramBars) printf("▓");
            else printf("░");
        }
        printf("\n");
        
        // Uyarılar
        if(cpuUsage > 90) printf("⚠️  CPU kullanımı kritik seviyede!\n");
        if(ramUsage > 90) printf("⚠️  RAM kullanımı kritik seviyede!\n");
    }
    
    PdhCloseQuery(cpuQuery);
    
    printf("\n✅ Performans izleme tamamlandı.\n");
    
    // Disk I/O bilgisi
    printf("\n💾 Disk Performansı:\n");
    ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes;
    if(GetDiskFreeSpaceEx("C:\\", &freeBytesAvailable, &totalNumberOfBytes, NULL)) {
        double totalGB = totalNumberOfBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
        double freeGB = freeBytesAvailable.QuadPart / (1024.0 * 1024.0 * 1024.0);
        double usedPercent = ((totalGB - freeGB) / totalGB) * 100.0;
        
        printf("   C:\\ Sürücüsü: %.1f%% dolu (%.1f GB / %.1f GB)\n", 
               usedPercent, totalGB - freeGB, totalGB);
    }
    
    // Ağ bağlantısı testi
    printf("\n🌐 Ağ Bağlantısı:\n");
    DWORD result = system("ping -n 1 8.8.8.8 >nul 2>&1");
    if(result == 0) {
        printf("   ✅ İnternet bağlantısı aktif\n");
    } else {
        printf("   ❌ İnternet bağlantısı yok\n");
    }
    
    #else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
    #endif
    
    log_info("Performans grafiği görüntülendi");
}

void run_system_monitor() {
    int choice;
    char input[10];
    
    while (1) {
        show_system_monitor_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    monitor_cpu();
                    break;
                case 2:
                    monitor_memory();
                    break;
                case 3:
                    analyze_disk_usage();
                    break;
                case 4:
                    show_running_processes();
                    break;
                case 5:
                    show_system_info();
                    break;
                case 6:
                    show_performance_graph();
                    break;
                case 0:
                    return;
                default:
                    printf("\n❌ Geçersiz seçim! Lütfen 0-6 arası bir sayı girin.\n");
                    break;
            }
            
            if (choice != 0) {
                printf("\nDevam etmek için Enter tuşuna basın...");
                getchar();
            }
        }
    }
}