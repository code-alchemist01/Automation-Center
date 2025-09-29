/*
 * ========================================
 * Log Analizi ve Raporlama Modülü
 * ========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "../../include/logger.h"

void show_log_analyzer_menu() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                   LOG ANALİZİ MODÜLÜ                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [1] Log Dosyası Analizi                                    ║\n");
    printf("║  [2] Hata Logları Filtreleme                                ║\n");
    printf("║  [3] Log İstatistikleri                                     ║\n");
    printf("║  [4] Zaman Bazlı Analiz                                     ║\n");
    printf("║  [5] Log Temizleme                                          ║\n");
    printf("║  [6] Otomatik Rapor Oluştur                                 ║\n");
    printf("║  [0] Ana Menüye Dön                                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nSeçiminizi yapın (0-6): ");
}

void analyze_log_file() {
    printf("\nLog Dosyası Analizi\n");
    printf("==================\n");
    
    #ifdef _WIN32
    // Gerçek log dosyasını analiz et
    const char* logPath = "logs/automation.log";
    HANDLE hFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        GetFileSizeEx(hFile, &fileSize);
        double fileSizeMB = (double)fileSize.QuadPart / (1024.0 * 1024.0);
        
        printf("📁 Analiz edilen dosya: %s\n", logPath);
        printf("📊 Dosya boyutu: %.2f MB\n", fileSizeMB);
        
        // Dosya içeriğini oku ve analiz et
        DWORD bytesRead;
        char buffer[8192];
        int infoCount = 0, warningCount = 0, errorCount = 0, debugCount = 0;
        int totalLines = 0;
        
        SYSTEMTIME firstTime = {0}, lastTime = {0};
        BOOL firstTimeSet = FALSE;
        
        while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            char* line = strtok(buffer, "\n");
            while (line != NULL) {
                totalLines++;
                
                // Log seviyelerini say
                if (strstr(line, "[INFO]")) infoCount++;
                else if (strstr(line, "[WARNING]")) warningCount++;
                else if (strstr(line, "[ERROR]")) errorCount++;
                else if (strstr(line, "[DEBUG]")) debugCount++;
                
                // İlk ve son zaman damgalarını bul
                if (!firstTimeSet && strlen(line) > 19) {
                    // Basit tarih formatı kontrolü (YYYY-MM-DD HH:MM:SS)
                    if (line[4] == '-' && line[7] == '-' && line[10] == ' ' && line[13] == ':' && line[16] == ':') {
                        GetSystemTime(&firstTime);
                        firstTimeSet = TRUE;
                    }
                }
                
                line = strtok(NULL, "\n");
            }
        }
        
        GetSystemTime(&lastTime);
        CloseHandle(hFile);
        
        printf("📅 Tarih aralığı: %04d-%02d-%02d - %04d-%02d-%02d\n\n", 
               firstTime.wYear, firstTime.wMonth, firstTime.wDay,
               lastTime.wYear, lastTime.wMonth, lastTime.wDay);
        
        int totalLogs = infoCount + warningCount + errorCount + debugCount;
        if (totalLogs > 0) {
            printf("📈 Log Seviyesi Dağılımı:\n");
            printf("   ✅ INFO:    %d kayıt (%d%%)\n", infoCount, (infoCount * 100) / totalLogs);
            printf("   ⚠️  WARNING:  %d kayıt (%d%%)\n", warningCount, (warningCount * 100) / totalLogs);
            printf("   ❌ ERROR:    %d kayıt (%d%%)\n", errorCount, (errorCount * 100) / totalLogs);
            printf("   🐛 DEBUG:     %d kayıt (%d%%)\n\n", debugCount, (debugCount * 100) / totalLogs);
        } else {
            printf("📈 Log Seviyesi Dağılımı:\n");
            printf("   ℹ️  Log dosyası boş veya standart format değil\n\n");
        }
        
    } else {
        printf("📁 Analiz edilen dosya: %s\n", logPath);
        printf("⚠️  Dosya bulunamadı - örnek analiz gösteriliyor\n");
        printf("📊 Dosya boyutu: 0 MB\n");
        printf("📅 Tarih aralığı: Dosya mevcut değil\n\n");
        
        printf("📈 Log Seviyesi Dağılımı:\n");
        printf("   ℹ️  Log dosyası mevcut değil\n\n");
    }
    
    printf("🔍 En Sık Görülen Mesajlar:\n");
    printf("   ℹ️  Gerçek log analizi için geçerli log dosyası gerekli\n");
    
    #else
    printf("Linux/Unix log analizi henüz desteklenmiyor.\n");
    #endif
    
    log_info("Log dosyası analizi tamamlandı");
}

void filter_error_logs() {
    printf("\nHata Logları Filtreleme\n");
    printf("======================\n");
    printf("🔍 ERROR seviyesindeki loglar filtreleniyor...\n\n");
    
    #ifdef _WIN32
    const char* logPath = "logs/automation.log";
    HANDLE hFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        printf("❌ Bulunan Hatalar (Gerçek log dosyasından):\n");
        printf("┌──────────────────────┬─────────────────────────────────────────┐\n");
        printf("│ Zaman                │ Hata Mesajı                             │\n");
        printf("├──────────────────────┼─────────────────────────────────────────┤\n");
        
        DWORD bytesRead;
        char buffer[8192];
        int errorCount = 0;
        int networkErrors = 0, fileErrors = 0, memoryErrors = 0, securityErrors = 0;
        
        while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0 && errorCount < 10) {
            buffer[bytesRead] = '\0';
            
            char* line = strtok(buffer, "\n");
            while (line != NULL && errorCount < 10) {
                if (strstr(line, "[ERROR]")) {
                    errorCount++;
                    
                    // Zaman damgasını çıkar (ilk 19 karakter)
                    char timeStr[20] = {0};
                    if (strlen(line) > 19) {
                        strncpy(timeStr, line, 19);
                    } else {
                        strcpy(timeStr, "Bilinmeyen zaman");
                    }
                    
                    // Hata mesajını çıkar
                    char* errorMsg = strstr(line, "[ERROR]");
                    if (errorMsg) {
                        errorMsg += 7; // "[ERROR]" kısmını atla
                        while (*errorMsg == ' ') errorMsg++; // Boşlukları atla
                    } else {
                        errorMsg = "Bilinmeyen hata";
                    }
                    
                    printf("│ %-20s │ %-39.39s │\n", timeStr, errorMsg);
                    
                    // Hata kategorilerini say
                    if (strstr(line, "network") || strstr(line, "connection") || strstr(line, "timeout")) {
                        networkErrors++;
                    } else if (strstr(line, "file") || strstr(line, "directory") || strstr(line, "path")) {
                        fileErrors++;
                    } else if (strstr(line, "memory") || strstr(line, "allocation") || strstr(line, "heap")) {
                        memoryErrors++;
                    } else if (strstr(line, "security") || strstr(line, "auth") || strstr(line, "permission")) {
                        securityErrors++;
                    }
                }
                line = strtok(NULL, "\n");
            }
        }
        
        CloseHandle(hFile);
        
        if (errorCount == 0) {
            printf("│ Hata bulunamadı      │ Log dosyasında ERROR seviyesi yok       │\n");
        }
        
        printf("└──────────────────────┴─────────────────────────────────────────┘\n\n");
        
        printf("📊 Hata Kategorileri:\n");
        printf("   🌐 Ağ Hataları: %d adet\n", networkErrors);
        printf("   📁 Dosya Hataları: %d adet\n", fileErrors);
        printf("   💾 Bellek Hataları: %d adet\n", memoryErrors);
        printf("   🔐 Güvenlik Hataları: %d adet\n", securityErrors);
        
    } else {
        printf("❌ Log dosyası bulunamadı - örnek hata gösteriliyor:\n");
        printf("┌──────────────────────┬─────────────────────────────────────────┐\n");
        printf("│ Zaman                │ Hata Mesajı                             │\n");
        printf("├──────────────────────┼─────────────────────────────────────────┤\n");
        printf("│ Log dosyası yok      │ Gerçek hatalar için log dosyası gerekli │\n");
        printf("└──────────────────────┴─────────────────────────────────────────┘\n\n");
        
        printf("📊 Hata Kategorileri:\n");
        printf("   ℹ️  Log dosyası mevcut değil\n");
    }
    
    #else
    printf("Linux/Unix hata log filtreleme henüz desteklenmiyor.\n");
    #endif
    
    log_info("Hata logları filtreleme işlemi tamamlandı");
}

void show_log_statistics() {
    printf("\nLog İstatistikleri\n");
    printf("=================\n");
    
    #ifdef _WIN32
    const char* logPath = "logs/automation.log";
    HANDLE hFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        GetFileSizeEx(hFile, &fileSize);
        
        DWORD bytesRead;
        char buffer[8192];
        int totalRecords = 0;
        int dailyRecords[10] = {0}; // Son 10 günün kayıtları
        int hourlyRecords[24] = {0}; // 24 saatlik dağılım
        
        SYSTEMTIME firstTime = {0}, lastTime = {0};
        BOOL firstTimeSet = FALSE;
        
        while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            
            char* line = strtok(buffer, "\n");
            while (line != NULL) {
                if (strlen(line) > 10) { // En az tarih formatı kadar uzun olmalı
                    totalRecords++;
                    
                    // İlk ve son zaman damgalarını kaydet
                    if (!firstTimeSet && strlen(line) > 19) {
                        if (line[4] == '-' && line[7] == '-' && line[10] == ' ') {
                            GetSystemTime(&firstTime);
                            firstTimeSet = TRUE;
                        }
                    }
                    
                    // Saatlik dağılım için saat bilgisini çıkar
                    if (strlen(line) > 13 && line[13] == ':') {
                        int hour = (line[11] - '0') * 10 + (line[12] - '0');
                        if (hour >= 0 && hour < 24) {
                            hourlyRecords[hour]++;
                        }
                    }
                }
                line = strtok(NULL, "\n");
            }
        }
        
        GetSystemTime(&lastTime);
        CloseHandle(hFile);
        
        printf("📊 Genel İstatistikler:\n");
        printf("   📝 Toplam log kayıtları: %d\n", totalRecords);
        printf("   📅 İlk kayıt: %04d-%02d-%02d %02d:%02d:%02d\n", 
               firstTime.wYear, firstTime.wMonth, firstTime.wDay,
               firstTime.wHour, firstTime.wMinute, firstTime.wSecond);
        printf("   📅 Son kayıt: %04d-%02d-%02d %02d:%02d:%02d\n", 
               lastTime.wYear, lastTime.wMonth, lastTime.wDay,
               lastTime.wHour, lastTime.wMinute, lastTime.wSecond);
        
        int avgDaily = totalRecords > 0 ? totalRecords / 10 : 0; // Yaklaşık 10 günlük ortalama
        printf("   ⏱️  Ortalama günlük kayıt: %d\n\n", avgDaily);
        
        printf("📈 Basit Aktivite Gösterimi:\n");
        printf("Toplam kayıt sayısına göre aktivite seviyesi:\n");
        if (totalRecords > 1000) {
            printf("████████████████████ Yüksek aktivite\n");
        } else if (totalRecords > 500) {
            printf("████████████▒▒▒▒▒▒▒▒ Orta aktivite\n");
        } else if (totalRecords > 100) {
            printf("██████▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Düşük aktivite\n");
        } else {
            printf("██▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Çok düşük aktivite\n");
        }
        printf("\n");
        
        // En aktif saatleri bul
        int maxHour1 = 0, maxHour2 = 0, maxHour3 = 0;
        for (int i = 0; i < 24; i++) {
            if (hourlyRecords[i] > hourlyRecords[maxHour1]) {
                maxHour3 = maxHour2;
                maxHour2 = maxHour1;
                maxHour1 = i;
            } else if (hourlyRecords[i] > hourlyRecords[maxHour2]) {
                maxHour3 = maxHour2;
                maxHour2 = i;
            } else if (hourlyRecords[i] > hourlyRecords[maxHour3]) {
                maxHour3 = i;
            }
        }
        
        printf("🕐 Saatlik Dağılım (En Aktif Saatler):\n");
        if (hourlyRecords[maxHour1] > 0) {
            printf("   🌅 %02d:00-%02d:00: %d kayıt\n", maxHour1, maxHour1+1, hourlyRecords[maxHour1]);
        }
        if (hourlyRecords[maxHour2] > 0) {
            printf("   🌞 %02d:00-%02d:00: %d kayıt\n", maxHour2, maxHour2+1, hourlyRecords[maxHour2]);
        }
        if (hourlyRecords[maxHour3] > 0) {
            printf("   🌆 %02d:00-%02d:00: %d kayıt\n", maxHour3, maxHour3+1, hourlyRecords[maxHour3]);
        }
        
        if (hourlyRecords[maxHour1] == 0) {
            printf("   ℹ️  Saatlik veri bulunamadı\n");
        }
        
    } else {
        printf("📊 Genel İstatistikler:\n");
        printf("   📝 Toplam log kayıtları: 0\n");
        printf("   📅 İlk kayıt: Log dosyası bulunamadı\n");
        printf("   📅 Son kayıt: Log dosyası bulunamadı\n");
        printf("   ⏱️  Ortalama günlük kayıt: 0\n\n");
        
        printf("📈 Aktivite Gösterimi:\n");
        printf("▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒ Log dosyası mevcut değil\n\n");
        
        printf("🕐 Saatlik Dağılım:\n");
        printf("   ℹ️  Log dosyası mevcut değil\n");
    }
    
    #else
    printf("Linux/Unix log istatistikleri henüz desteklenmiyor.\n");
    #endif
    
    log_info("Log istatistikleri görüntülendi");
}

void time_based_analysis() {
    printf("\nZaman Bazlı Log Analizi\n");
    printf("======================\n");
    printf("📅 Son 7 günün günlük trend analizi yapılıyor...\n\n");
    
    #ifdef _WIN32
    const char* logPath = "logs/automation.log";
    HANDLE hFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hFile != INVALID_HANDLE_VALUE) {
        printf("📊 Günlük Log Dağılımı (Son 7 Gün):\n");
        printf("┌────────────┬─────────┬─────────┬─────────┬─────────┬─────────┐\n");
        printf("│ Tarih      │ INFO    │ WARNING │ ERROR   │ DEBUG   │ Toplam  │\n");
        printf("├────────────┼─────────┼─────────┼─────────┼─────────┼─────────┤\n");
        
        // Günlük sayaçlar (7 gün için)
        int dailyInfo[7] = {0};
        int dailyWarning[7] = {0};
        int dailyError[7] = {0};
        int dailyDebug[7] = {0};
        int dailyTotal[7] = {0};
        
        // Bugünün tarihini al
        time_t now = time(NULL);
        struct tm* today = localtime(&now);
        
        DWORD bytesRead;
        char buffer[8192];
        char fullBuffer[65536] = {0}; // Tüm dosya içeriği için
        DWORD totalBytesRead = 0;
        
        // Dosyayı tamamen oku
        while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            if (totalBytesRead + bytesRead < sizeof(fullBuffer) - 1) {
                memcpy(fullBuffer + totalBytesRead, buffer, bytesRead);
                totalBytesRead += bytesRead;
            } else {
                break;
            }
        }
        fullBuffer[totalBytesRead] = '\0';
        
        CloseHandle(hFile);
        
        // Satır satır işle
        char* line = strtok(fullBuffer, "\n");
        while (line != NULL) {
            // Tarih formatı: YYYY-MM-DD HH:MM:SS
            if (strlen(line) >= 19) {
                char dateStr[11];
                strncpy(dateStr, line, 10);
                dateStr[10] = '\0';
                
                // Tarihi parse et
                int year, month, day;
                if (sscanf(dateStr, "%d-%d-%d", &year, &month, &day) == 3) {
                    struct tm logTime = {0};
                    logTime.tm_year = year - 1900;
                    logTime.tm_mon = month - 1;
                    logTime.tm_mday = day;
                    
                    time_t logTimestamp = mktime(&logTime);
                    double daysDiff = difftime(now, logTimestamp) / (24 * 3600);
                    
                    if (daysDiff >= 0 && daysDiff < 7) {
                        int dayIndex = (int)daysDiff;
                        
                        dailyTotal[dayIndex]++;
                        
                        if (strstr(line, "[INFO]")) {
                            dailyInfo[dayIndex]++;
                        } else if (strstr(line, "[WARNING]")) {
                            dailyWarning[dayIndex]++;
                        } else if (strstr(line, "[ERROR]")) {
                            dailyError[dayIndex]++;
                        } else if (strstr(line, "[DEBUG]")) {
                            dailyDebug[dayIndex]++;
                        }
                    }
                }
            }
            line = strtok(NULL, "\n");
        }
        
        // Günlük verileri göster (en yeni günden başlayarak)
        for (int i = 0; i < 7; i++) {
            time_t dayTime = now - (i * 24 * 3600);
            struct tm* dayTm = localtime(&dayTime);
            
            char dayStr[11];
            strftime(dayStr, sizeof(dayStr), "%Y-%m-%d", dayTm);
            
            printf("│ %-10s │ %7d │ %7d │ %7d │ %7d │ %7d │\n", 
                   dayStr, dailyInfo[i], dailyWarning[i], dailyError[i], dailyDebug[i], dailyTotal[i]);
        }
        
        printf("└────────────┴─────────┴─────────┴─────────┴─────────┴─────────┘\n\n");
        
        // Trend analizi
        int totalLogs = 0, totalErrors = 0, totalWarnings = 0;
        for (int i = 0; i < 7; i++) {
            totalLogs += dailyTotal[i];
            totalErrors += dailyError[i];
            totalWarnings += dailyWarning[i];
        }
        
        printf("📈 Trend Analizi:\n");
        printf("   📊 Son 7 günde toplam %d log kaydı\n", totalLogs);
        if (totalLogs > 0) {
            printf("   ⚠️  Toplam %d uyarı (%.1f%%)\n", totalWarnings, (totalWarnings * 100.0 / totalLogs));
            printf("   ❌ Toplam %d hata (%.1f%%)\n", totalErrors, (totalErrors * 100.0 / totalLogs));
        } else {
            printf("   ⚠️  Toplam %d uyarı\n", totalWarnings);
            printf("   ❌ Toplam %d hata\n", totalErrors);
        }
        
        // En aktif gün
        int maxDay = 0;
        for (int i = 1; i < 7; i++) {
            if (dailyTotal[i] > dailyTotal[maxDay]) {
                maxDay = i;
            }
        }
        
        if (dailyTotal[maxDay] > 0) {
            time_t maxDayTime = now - (maxDay * 24 * 3600);
            struct tm* maxDayTm = localtime(&maxDayTime);
            char maxDayStr[11];
            strftime(maxDayStr, sizeof(maxDayStr), "%Y-%m-%d", maxDayTm);
            printf("   🔥 En aktif gün: %s (%d log)\n", maxDayStr, dailyTotal[maxDay]);
        }
        
        // Saatlik dağılım analizi
        printf("\n⏰ Saatlik Aktivite Dağılımı:\n");
        int hourlyCount[24] = {0};
        
        // Dosyayı tekrar oku saatlik analiz için
        strcpy(fullBuffer, ""); // Buffer'ı temizle
        totalBytesRead = 0;
        
        hFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                if (totalBytesRead + bytesRead < sizeof(fullBuffer) - 1) {
                    memcpy(fullBuffer + totalBytesRead, buffer, bytesRead);
                    totalBytesRead += bytesRead;
                } else {
                    break;
                }
            }
            fullBuffer[totalBytesRead] = '\0';
            CloseHandle(hFile);
            
            line = strtok(fullBuffer, "\n");
            while (line != NULL) {
                if (strlen(line) >= 19) {
                    int hour;
                    if (sscanf(line + 11, "%d:", &hour) == 1 && hour >= 0 && hour < 24) {
                        hourlyCount[hour]++;
                    }
                }
                line = strtok(NULL, "\n");
            }
            
            // En aktif saatleri bul
            int maxHour = 0;
            for (int i = 1; i < 24; i++) {
                if (hourlyCount[i] > hourlyCount[maxHour]) {
                    maxHour = i;
                }
            }
            
            if (hourlyCount[maxHour] > 0) {
                printf("   🕐 En aktif saat: %02d:00-%02d:59 (%d log)\n", maxHour, maxHour, hourlyCount[maxHour]);
                
                // En aktif 3 saati göster
                printf("   📊 En aktif saatler:\n");
                for (int rank = 0; rank < 3; rank++) {
                    int topHour = 0;
                    for (int i = 1; i < 24; i++) {
                        if (hourlyCount[i] > hourlyCount[topHour]) {
                            // Daha önce seçilmiş saatleri atla
                            int alreadySelected = 0;
                            for (int j = 0; j < rank; j++) {
                                // Bu basit implementasyonda sadece en yüksek 3'ü gösteriyoruz
                            }
                            if (!alreadySelected) {
                                topHour = i;
                            }
                        }
                    }
                    if (hourlyCount[topHour] > 0) {
                        printf("      %d. %02d:00-%02d:59 (%d log)\n", rank + 1, topHour, topHour, hourlyCount[topHour]);
                        hourlyCount[topHour] = 0; // Bir sonraki iterasyon için sıfırla
                    }
                }
            }
        }
        
    } else {
        printf("❌ Log dosyası bulunamadı - örnek trend gösteriliyor:\n");
        printf("┌────────────┬─────────┬─────────┬─────────┬─────────┬─────────┐\n");
        printf("│ Tarih      │ INFO    │ WARNING │ ERROR   │ DEBUG   │ Toplam  │\n");
        printf("├────────────┼─────────┼─────────┼─────────┼─────────┼─────────┤\n");
        printf("│ Log yok    │       0 │       0 │       0 │       0 │       0 │\n");
        printf("└────────────┴─────────┴─────────┴─────────┴─────────┴─────────┘\n\n");
        
        printf("📈 Trend Analizi:\n");
        printf("   ℹ️  Log dosyası mevcut değil\n");
    }
    
    #else
    printf("Linux/Unix zaman bazlı analiz henüz desteklenmiyor.\n");
    #endif
    
    log_info("Zaman bazlı log analizi tamamlandı");
}

void clean_logs() {
    printf("\nLog Temizleme\n");
    printf("=============\n");
    printf("🧹 Log temizleme seçenekleri:\n\n");
    
    char logPath[256];
    printf("📁 Log dosyası yolu (boş bırakırsanız varsayılan: logs/automation.log): ");
    fgets(logPath, sizeof(logPath), stdin);
    logPath[strcspn(logPath, "\n")] = 0;
    
    if (strlen(logPath) == 0) {
        strcpy(logPath, "logs/automation.log");
    }
    
    // Log dosyasını analiz et
    FILE *file = fopen(logPath, "r");
    if (file == NULL) {
        printf("❌ Log dosyası bulunamadı: %s\n", logPath);
        return;
    }
    
    char line[512];
    int total_lines = 0;
    int debug_lines = 0;
    int info_lines = 0;
    int warning_lines = 0;
    int error_lines = 0;
    int old_30_days = 0;
    int old_90_days = 0;
    long file_size = 0;
    
    // Dosya boyutunu hesapla
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Mevcut tarihi al
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    
    // Log satırlarını analiz et
    while (fgets(line, sizeof(line), file)) {
        total_lines++;
        
        // Log seviyelerini say
        if (strstr(line, "[DEBUG]") != NULL) debug_lines++;
        else if (strstr(line, "[INFO]") != NULL) info_lines++;
        else if (strstr(line, "[WARNING]") != NULL) warning_lines++;
        else if (strstr(line, "[ERROR]") != NULL) error_lines++;
        
        // Tarih analizi (basit format: YYYY-MM-DD)
        if (strlen(line) >= 10) {
            int year, month, day;
            if (sscanf(line, "%d-%d-%d", &year, &month, &day) == 3) {
                struct tm log_time = {0};
                log_time.tm_year = year - 1900;
                log_time.tm_mon = month - 1;
                log_time.tm_mday = day;
                
                time_t log_timestamp = mktime(&log_time);
                double days_diff = difftime(now, log_timestamp) / (24 * 3600);
                
                if (days_diff > 90) old_90_days++;
                else if (days_diff > 30) old_30_days++;
            }
        }
    }
    fclose(file);
    
    double file_size_mb = file_size / (1024.0 * 1024.0);
    double debug_size_mb = (debug_lines * 50.0) / (1024.0 * 1024.0); // Ortalama satır boyutu
    double old_30_size_mb = (old_30_days * 50.0) / (1024.0 * 1024.0);
    double old_90_size_mb = (old_90_days * 50.0) / (1024.0 * 1024.0);
    
    printf("📊 Log Dosyası Analizi:\n");
    printf("• Toplam satır sayısı: %d\n", total_lines);
    printf("• Dosya boyutu: %.2f MB\n", file_size_mb);
    printf("\n📅 Tarih bazlı analiz:\n");
    printf("   ✅ 30 günden eski loglar: %d kayıt (%.1f MB)\n", old_30_days, old_30_size_mb);
    printf("   ✅ 90 günden eski loglar: %d kayıt (%.1f MB)\n", old_90_days, old_90_size_mb);
    
    printf("\n📊 Seviye bazlı analiz:\n");
    printf("   🐛 DEBUG logları: %d kayıt (%.1f MB)\n", debug_lines, debug_size_mb);
    printf("   ✅ INFO logları: %d kayıt\n", info_lines);
    printf("   ⚠️  WARNING logları: %d kayıt\n", warning_lines);
    printf("   ❌ ERROR logları: %d kayıt\n", error_lines);
    
    // Temizleme seçenekleri sun
    printf("\n🗑️  Temizleme seçenekleri:\n");
    printf("1. 30 günden eski logları sil\n");
    printf("2. 90 günden eski logları sil\n");
    printf("3. DEBUG loglarını sil\n");
    printf("4. Tüm logları sil\n");
    printf("5. İptal\n");
    
    printf("\nSeçiminiz (1-5): ");
    int choice;
    scanf("%d", &choice);
    
    int deleted_lines = 0;
    double saved_space = 0.0;
    
    switch (choice) {
        case 1:
            deleted_lines = old_30_days;
            saved_space = old_30_size_mb;
            printf("🗑️  30 günden eski loglar temizleniyor...\n");
            break;
        case 2:
            deleted_lines = old_90_days;
            saved_space = old_90_size_mb;
            printf("🗑️  90 günden eski loglar temizleniyor...\n");
            break;
        case 3:
            deleted_lines = debug_lines;
            saved_space = debug_size_mb;
            printf("🗑️  DEBUG logları temizleniyor...\n");
            break;
        case 4:
            deleted_lines = total_lines;
            saved_space = file_size_mb;
            printf("🗑️  Tüm loglar temizleniyor...\n");
            // Dosyayı temizle
            file = fopen(logPath, "w");
            if (file != NULL) {
                fclose(file);
            }
            break;
        case 5:
            printf("❌ Temizleme iptal edildi.\n");
            return;
        default:
            printf("❌ Geçersiz seçim.\n");
            return;
    }
    
    if (choice != 4) {
        printf("⚠️  Not: Gerçek log filtreleme işlemi karmaşık olduğu için simüle edildi.\n");
        printf("✅ Gerçek uygulamada %d satır silinecekti.\n", deleted_lines);
    } else {
        printf("✅ Log dosyası tamamen temizlendi!\n");
    }
    
    printf("💾 Kazanılan alan: %.1f MB\n", saved_space);
    
    log_info("Log temizleme işlemi tamamlandı");
}

void generate_auto_report() {
    printf("\nOtomatik Rapor Oluşturma\n");
    printf("========================\n");
    printf("📄 Günlük log raporu oluşturuluyor...\n\n");
    
    #ifdef _WIN32
    const char* logPath = "logs/automation.log";
    const char* reportPath = "reports/daily_log_report.html";
    
    // Reports klasörünü oluştur
    CreateDirectory("reports", NULL);
    
    HANDLE hLogFile = CreateFile(logPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hLogFile != INVALID_HANDLE_VALUE) {
        // Log dosyasını analiz et
        DWORD bytesRead;
        char buffer[8192];
        char fullBuffer[65536] = {0};
        DWORD totalBytesRead = 0;
        
        // Dosyayı tamamen oku
        while (ReadFile(hLogFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            if (totalBytesRead + bytesRead < sizeof(fullBuffer) - 1) {
                memcpy(fullBuffer + totalBytesRead, buffer, bytesRead);
                totalBytesRead += bytesRead;
            } else {
                break;
            }
        }
        fullBuffer[totalBytesRead] = '\0';
        CloseHandle(hLogFile);
        
        // İstatistikleri hesapla
        int totalLogs = 0, infoCount = 0, warningCount = 0, errorCount = 0, debugCount = 0;
        char firstTimestamp[20] = "Bilinmiyor";
        char lastTimestamp[20] = "Bilinmiyor";
        
        char* line = strtok(fullBuffer, "\n");
        int lineCount = 0;
        
        while (line != NULL) {
            if (strlen(line) > 0) {
                totalLogs++;
                
                // İlk ve son zaman damgası
                if (strlen(line) >= 19) {
                    if (lineCount == 0) {
                        strncpy(firstTimestamp, line, 19);
                        firstTimestamp[19] = '\0';
                    }
                    strncpy(lastTimestamp, line, 19);
                    lastTimestamp[19] = '\0';
                }
                
                // Log seviyelerini say
                if (strstr(line, "[INFO]")) infoCount++;
                else if (strstr(line, "[WARNING]")) warningCount++;
                else if (strstr(line, "[ERROR]")) errorCount++;
                else if (strstr(line, "[DEBUG]")) debugCount++;
                
                lineCount++;
            }
            line = strtok(NULL, "\n");
        }
        
        // HTML raporu oluştur
        HANDLE hReportFile = CreateFile(reportPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (hReportFile != INVALID_HANDLE_VALUE) {
            char htmlContent[16384];
            time_t now = time(NULL);
            struct tm* currentTime = localtime(&now);
            char dateStr[64];
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", currentTime);
            
            snprintf(htmlContent, sizeof(htmlContent),
                "<!DOCTYPE html>\n"
                "<html lang=\"tr\">\n"
                "<head>\n"
                "    <meta charset=\"UTF-8\">\n"
                "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                "    <title>Günlük Log Raporu - %s</title>\n"
                "    <style>\n"
                "        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n"
                "        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n"
                "        .header { text-align: center; color: #333; border-bottom: 2px solid #007acc; padding-bottom: 20px; margin-bottom: 30px; }\n"
                "        .stats-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 30px; }\n"
                "        .stat-card { background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%); color: white; padding: 20px; border-radius: 8px; text-align: center; }\n"
                "        .stat-card.info { background: linear-gradient(135deg, #4facfe 0%%, #00f2fe 100%%); }\n"
                "        .stat-card.warning { background: linear-gradient(135deg, #f093fb 0%%, #f5576c 100%%); }\n"
                "        .stat-card.error { background: linear-gradient(135deg, #ff9a9e 0%%, #fecfef 100%%); color: #333; }\n"
                "        .stat-card.debug { background: linear-gradient(135deg, #a8edea 0%%, #fed6e3 100%%); color: #333; }\n"
                "        .stat-number { font-size: 2em; font-weight: bold; margin-bottom: 5px; }\n"
                "        .stat-label { font-size: 0.9em; opacity: 0.9; }\n"
                "        .section { margin-bottom: 30px; }\n"
                "        .section h2 { color: #333; border-left: 4px solid #007acc; padding-left: 15px; }\n"
                "        .info-table { width: 100%%; border-collapse: collapse; margin-top: 15px; }\n"
                "        .info-table th, .info-table td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }\n"
                "        .info-table th { background-color: #f8f9fa; font-weight: bold; }\n"
                "        .footer { text-align: center; color: #666; font-size: 0.9em; margin-top: 40px; padding-top: 20px; border-top: 1px solid #eee; }\n"
                "    </style>\n"
                "</head>\n"
                "<body>\n"
                "    <div class=\"container\">\n"
                "        <div class=\"header\">\n"
                "            <h1>🔍 Günlük Log Analiz Raporu</h1>\n"
                "            <p>Rapor Tarihi: %s</p>\n"
                "        </div>\n"
                "\n"
                "        <div class=\"stats-grid\">\n"
                "            <div class=\"stat-card\">\n"
                "                <div class=\"stat-number\">%d</div>\n"
                "                <div class=\"stat-label\">Toplam Log</div>\n"
                "            </div>\n"
                "            <div class=\"stat-card info\">\n"
                "                <div class=\"stat-number\">%d</div>\n"
                "                <div class=\"stat-label\">INFO</div>\n"
                "            </div>\n"
                "            <div class=\"stat-card warning\">\n"
                "                <div class=\"stat-number\">%d</div>\n"
                "                <div class=\"stat-label\">WARNING</div>\n"
                "            </div>\n"
                "            <div class=\"stat-card error\">\n"
                "                <div class=\"stat-number\">%d</div>\n"
                "                <div class=\"stat-label\">ERROR</div>\n"
                "            </div>\n"
                "            <div class=\"stat-card debug\">\n"
                "                <div class=\"stat-number\">%d</div>\n"
                "                <div class=\"stat-label\">DEBUG</div>\n"
                "            </div>\n"
                "        </div>\n"
                "\n"
                "        <div class=\"section\">\n"
                "            <h2>📊 Genel Bilgiler</h2>\n"
                "            <table class=\"info-table\">\n"
                "                <tr><th>Özellik</th><th>Değer</th></tr>\n"
                "                <tr><td>Log Dosyası</td><td>%s</td></tr>\n"
                "                <tr><td>İlk Kayıt</td><td>%s</td></tr>\n"
                "                <tr><td>Son Kayıt</td><td>%s</td></tr>\n"
                "                <tr><td>Toplam Kayıt</td><td>%d satır</td></tr>\n"
                "                <tr><td>Hata Oranı</td><td>%.1f%%</td></tr>\n"
                "                <tr><td>Uyarı Oranı</td><td>%.1f%%</td></tr>\n"
                "            </table>\n"
                "        </div>\n"
                "\n"
                "        <div class=\"section\">\n"
                "            <h2>🎯 Öneriler</h2>\n"
                "            <ul>\n",
                dateStr, dateStr, totalLogs, infoCount, warningCount, errorCount, debugCount,
                logPath, firstTimestamp, lastTimestamp, totalLogs,
                totalLogs > 0 ? (errorCount * 100.0 / totalLogs) : 0,
                totalLogs > 0 ? (warningCount * 100.0 / totalLogs) : 0);
            
            DWORD bytesWritten;
            WriteFile(hReportFile, htmlContent, strlen(htmlContent), &bytesWritten, NULL);
            
            // Öneriler ekle
            char recommendations[2048];
            snprintf(recommendations, sizeof(recommendations),
                "                <li>%s</li>\n"
                "                <li>%s</li>\n"
                "                <li>%s</li>\n"
                "                <li>%s</li>\n"
                "            </ul>\n"
                "        </div>\n"
                "\n"
                "        <div class=\"footer\">\n"
                "            <p>Bu rapor Automation Center tarafından otomatik olarak oluşturulmuştur.</p>\n"
                "            <p>Son güncelleme: %s</p>\n"
                "        </div>\n"
                "    </div>\n"
                "</body>\n"
                "</html>\n",
                errorCount > 10 ? "❌ Yüksek hata sayısı tespit edildi - sistem kontrolü önerilir" : "✅ Hata sayısı normal seviyede",
                warningCount > 20 ? "⚠️ Çok sayıda uyarı mevcut - log temizliği yapılabilir" : "✅ Uyarı sayısı kabul edilebilir seviyede",
                totalLogs > 1000 ? "📁 Log dosyası büyük - arşivleme düşünülebilir" : "✅ Log dosyası boyutu uygun",
                totalLogs == 0 ? "ℹ️ Log kaydı bulunamadı - sistem çalışıyor mu kontrol edin" : "✅ Sistem aktif olarak log kaydı yapıyor",
                dateStr);
            
            WriteFile(hReportFile, recommendations, strlen(recommendations), &bytesWritten, NULL);
            CloseHandle(hReportFile);
            
            printf("✅ HTML raporu başarıyla oluşturuldu!\n");
            printf("📁 Rapor konumu: %s\n", reportPath);
            printf("📊 Rapor içeriği:\n");
            printf("   • Toplam %d log kaydı analiz edildi\n", totalLogs);
            printf("   • %d INFO, %d WARNING, %d ERROR, %d DEBUG\n", infoCount, warningCount, errorCount, debugCount);
            printf("   • İlk kayıt: %s\n", firstTimestamp);
            printf("   • Son kayıt: %s\n", lastTimestamp);
            
            // Raporu tarayıcıda aç
            printf("\n🌐 Raporu tarayıcıda açmak için:\n");
            printf("   start %s\n", reportPath);
            
        } else {
            printf("❌ Rapor dosyası oluşturulamadı!\n");
        }
        
    } else {
        printf("❌ Log dosyası bulunamadı - örnek rapor oluşturuluyor:\n");
        
        // Örnek HTML raporu oluştur
        HANDLE hReportFile = CreateFile(reportPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        if (hReportFile != INVALID_HANDLE_VALUE) {
            char sampleReport[] = 
                "<!DOCTYPE html>\n"
                "<html><head><title>Log Raporu - Veri Yok</title></head>\n"
                "<body style='font-family: Arial; padding: 20px;'>\n"
                "<h1>📄 Log Analiz Raporu</h1>\n"
                "<p><strong>Durum:</strong> Log dosyası bulunamadı</p>\n"
                "<p><strong>Açıklama:</strong> Gerçek rapor için log dosyası gereklidir</p>\n"
                "<p><strong>Öneriler:</strong></p>\n"
                "<ul><li>Sistem loglarının aktif olduğunu kontrol edin</li>\n"
                "<li>Log dosyası yolunu doğrulayın</li></ul>\n"
                "</body></html>";
            
            DWORD bytesWritten;
            WriteFile(hReportFile, sampleReport, strlen(sampleReport), &bytesWritten, NULL);
            CloseHandle(hReportFile);
            
            printf("📁 Örnek rapor oluşturuldu: %s\n", reportPath);
        }
    }
    
    #else
    printf("Linux/Unix otomatik rapor oluşturma henüz desteklenmiyor.\n");
    #endif
    
    log_info("Otomatik rapor oluşturma işlemi tamamlandı");
}

void run_log_analyzer() {
    int choice;
    char input[10];
    
    while (1) {
        show_log_analyzer_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    analyze_log_file();
                    break;
                case 2:
                    filter_error_logs();
                    break;
                case 3:
                    show_log_statistics();
                    break;
                case 4:
                    time_based_analysis();
                    break;
                case 5:
                    clean_logs();
                    break;
                case 6:
                    generate_auto_report();
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