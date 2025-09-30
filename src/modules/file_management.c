/*
 * ========================================
 * Dosya Yönetimi ve Organizasyon Modülü
 * ========================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif

#include "../../include/logger.h"
#include "../../include/database.h"

// Module initialization
bool init_file_management() {
    log_info("File management module initialized");
    return true;
}

// Module cleanup
void cleanup_file_management() {
    log_info("File management module cleaned up");
}

void show_file_management_menu() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    DOSYA YÖNETİMİ MODÜLÜ                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  [1] Dosya Arama ve Filtreleme                              ║\n");
    printf("║  [2] Toplu Dosya Yeniden Adlandırma                         ║\n");
    printf("║  [3] Dosya Boyutu Analizi                                   ║\n");
    printf("║  [4] Boş Klasör Temizleme                                   ║\n");
    printf("║  [5] Dosya Türü Organizasyonu                               ║\n");
    printf("║  [0] Ana Menüye Dön                                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nSeçiminizi yapın (0-5): ");
}

void file_search() {
    char search_path[256], search_pattern[100];
    
    printf("\nDosya Arama İşlemi\n");
    printf("==================\n");
    printf("Arama yapılacak dizin: ");
    fgets(search_path, sizeof(search_path), stdin);
    search_path[strcspn(search_path, "\n")] = 0;
    
    printf("Arama deseni (*.txt, *.exe vb.): ");
    fgets(search_pattern, sizeof(search_pattern), stdin);
    search_pattern[strcspn(search_pattern, "\n")] = 0;
    
    printf("\n🔍 '%s' dizininde '%s' deseni aranıyor...\n", search_path, search_pattern);
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[512];
    int fileCount = 0;
    long long totalSize = 0;
    
    // Arama yolunu hazırla
    if(strlen(search_pattern) == 0) {
        strcpy(search_pattern, "*.*");
    }
    
    snprintf(searchPath, sizeof(searchPath), "%s\\%s", search_path, search_pattern);
    
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        if (error == ERROR_ACCESS_DENIED) {
            printf("❌ Bu dizine erişim izni yok! Lütfen erişilebilir bir dizin seçin.\n");
            printf("💡 Önerilen dizinler: C:\\Users\\%s, C:\\Windows\\System32, vb.\n", getenv("USERNAME"));
        } else if (error == ERROR_PATH_NOT_FOUND) {
            printf("❌ Belirtilen dizin bulunamadı!\n");
        } else if (error == ERROR_FILE_NOT_FOUND) {
            printf("❌ Belirtilen desene uygun dosya bulunamadı!\n");
        } else {
            printf("❌ Arama hatası (Kod: %lu)\n", error);
        }
        log_error("Dosya arama hatası: %s (Hata kodu: %lu)", search_path, error);
        return;
    }
    
    printf("✅ Bulunan dosyalar:\n");
    
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            LARGE_INTEGER fileSize;
            fileSize.LowPart = findFileData.nFileSizeLow;
            fileSize.HighPart = findFileData.nFileSizeHigh;
            
            double fileSizeKB = fileSize.QuadPart / 1024.0;
            totalSize += fileSize.QuadPart;
            fileCount++;
            
            // Dosya türüne göre ikon seç
            char* icon = "📄";
            char* ext = strrchr(findFileData.cFileName, '.');
            if(ext) {
                if(strcmp(ext, ".exe") == 0) icon = "⚙️";
                else if(strcmp(ext, ".txt") == 0) icon = "📝";
                else if(strcmp(ext, ".jpg") == 0 || strcmp(ext, ".png") == 0) icon = "🖼️";
                else if(strcmp(ext, ".mp4") == 0 || strcmp(ext, ".avi") == 0) icon = "🎬";
                else if(strcmp(ext, ".zip") == 0 || strcmp(ext, ".rar") == 0) icon = "📦";
            }
            
            if(fileSizeKB < 1024) {
                printf("   %s %s (%.1f KB)\n", icon, findFileData.cFileName, fileSizeKB);
            } else {
                printf("   %s %s (%.1f MB)\n", icon, findFileData.cFileName, fileSizeKB / 1024.0);
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    
    printf("\n📊 Arama Sonuçları:\n");
    printf("   📁 Toplam dosya: %d\n", fileCount);
    if(totalSize < 1024 * 1024) {
        printf("   💾 Toplam boyut: %.1f KB\n", totalSize / 1024.0);
    } else if(totalSize < 1024 * 1024 * 1024) {
        printf("   💾 Toplam boyut: %.1f MB\n", totalSize / (1024.0 * 1024.0));
    } else {
        printf("   💾 Toplam boyut: %.1f GB\n", totalSize / (1024.0 * 1024.0 * 1024.0));
    }
    
#else
    printf("Bu özellik sadece Windows'ta desteklenmektedir.\n");
#endif
    
    log_info("Dosya arama işlemi tamamlandı: %s -> %s", search_path, search_pattern);
    
    // Veritabanına kaydet
    FileOperation operation = {0};
    strcpy(operation.operation, "SEARCH");
    snprintf(operation.file_path, sizeof(operation.file_path), "%s\\%s", search_path, search_pattern);
    operation.file_size = totalSize;
    strcpy(operation.status, "SUCCESS");
    operation.timestamp = time(NULL);
    
    if (insert_file_operation(&operation)) {
        printf("✅ Arama işlemi veritabanına kaydedildi\n");
    } else {
        printf("❌ Arama işlemi veritabanına kaydedilemedi\n");
    }
}

void bulk_rename() {
    printf("\nToplu Dosya Yeniden Adlandırma\n");
    printf("==============================\n");
    
    char directory[256];
    char prefix[64];
    char suffix[64];
    
    // Kullanıcıdan bilgileri al
    printf("📁 Hedef dizin (boş bırakırsanız mevcut dizin): ");
    fgets(directory, sizeof(directory), stdin);
    directory[strcspn(directory, "\n")] = 0; // Newline karakterini kaldır
    
    if (strlen(directory) == 0) {
        #ifdef _WIN32
        GetCurrentDirectory(sizeof(directory), directory);
        #else
        getcwd(directory, sizeof(directory));
        #endif
    }
    
    printf("🏷️  Dosya öneki (prefix): ");
    fgets(prefix, sizeof(prefix), stdin);
    prefix[strcspn(prefix, "\n")] = 0;
    
    printf("🏷️  Dosya soneki (suffix): ");
    fgets(suffix, sizeof(suffix), stdin);
    suffix[strcspn(suffix, "\n")] = 0;
    
    printf("\n📁 Seçili dizin: %s\n", directory);
    printf("🔄 Yeniden adlandırma kuralı: %s[sayı]%s\n", prefix, suffix);
    printf("\n🔍 Dosyalar taranıyor...\n");
    
    int renamed_count = 0;
    int counter = 1;
    
    #ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[512];
    
    snprintf(searchPath, sizeof(searchPath), "%s\\*.*", directory);
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // Dizinleri atla, sadece dosyaları işle
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char oldPath[512];
                char newPath[512];
                char newFileName[256];
                
                // Dosya uzantısını al
                char *extension = strrchr(findFileData.cFileName, '.');
                if (extension == NULL) extension = "";
                
                // Yeni dosya adını oluştur
                if (strlen(extension) > 0) {
                    snprintf(newFileName, sizeof(newFileName), "%s%d%s%s", 
                             prefix, counter, suffix, extension);
                } else {
                    snprintf(newFileName, sizeof(newFileName), "%s%d%s", 
                             prefix, counter, suffix);
                }
                
                snprintf(oldPath, sizeof(oldPath), "%s\\%s", directory, findFileData.cFileName);
                snprintf(newPath, sizeof(newPath), "%s\\%s", directory, newFileName);
                
                // Dosyayı yeniden adlandır
                if (MoveFile(oldPath, newPath)) {
                    printf("   ✅ %s -> %s\n", findFileData.cFileName, newFileName);
                    renamed_count++;
                    counter++;
                } else {
                    printf("   ❌ %s (yeniden adlandırılamadı)\n", findFileData.cFileName);
                }
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        
        FindClose(hFind);
    }
    #else
    // Linux/Unix implementation
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    
    dir = opendir(directory);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            char fullPath[512];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", directory, entry->d_name);
            
            if (stat(fullPath, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                char oldPath[512];
                char newPath[512];
                char newFileName[256];
                
                // Dosya uzantısını al
                char *extension = strrchr(entry->d_name, '.');
                if (extension == NULL) extension = "";
                
                // Yeni dosya adını oluştur
                if (strlen(extension) > 0) {
                    snprintf(newFileName, sizeof(newFileName), "%s%d%s%s", 
                             prefix, counter, suffix, extension);
                } else {
                    snprintf(newFileName, sizeof(newFileName), "%s%d%s", 
                             prefix, counter, suffix);
                }
                
                snprintf(oldPath, sizeof(oldPath), "%s/%s", directory, entry->d_name);
                snprintf(newPath, sizeof(newPath), "%s/%s", directory, newFileName);
                
                // Dosyayı yeniden adlandır
                if (rename(oldPath, newPath) == 0) {
                    printf("   ✅ %s -> %s\n", entry->d_name, newFileName);
                    renamed_count++;
                    counter++;
                } else {
                    printf("   ❌ %s (yeniden adlandırılamadı)\n", entry->d_name);
                }
            }
        }
        closedir(dir);
    }
    #endif
    
    if (renamed_count > 0) {
        printf("\n✅ %d dosya başarıyla yeniden adlandırıldı!\n", renamed_count);
    } else {
        printf("\n⚠️  Yeniden adlandırılacak dosya bulunamadı.\n");
    }
    
    log_info("Toplu dosya yeniden adlandırma işlemi tamamlandı");
    
    // Veritabanına kaydet
    FileOperation operation = {0};
    strcpy(operation.operation, "BULK_RENAME");
    strcpy(operation.file_path, directory);
    operation.file_size = renamed_count;
    strcpy(operation.status, renamed_count > 0 ? "SUCCESS" : "NO_FILES");
    operation.timestamp = time(NULL);
    
    if (insert_file_operation(&operation)) {
        printf("✅ Yeniden adlandırma işlemi veritabanına kaydedildi\n");
    } else {
        printf("❌ Yeniden adlandırma işlemi veritabanına kaydedilemedi\n");
    }
}

void analyze_file_sizes() {
    printf("\nDosya Boyutu Analizi\n");
    printf("===================\n");
    printf("📊 Dizin analizi sonuçları:\n\n");
    
    #ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    LARGE_INTEGER totalSize = {0};
    int fileCount = 0;
    
    // Mevcut dizini analiz et
    char searchPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, searchPath);
    strcat(searchPath, "\\*");
    
    hFind = FindFirstFile(searchPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                LARGE_INTEGER fileSize;
                fileSize.LowPart = findFileData.nFileSizeLow;
                fileSize.HighPart = findFileData.nFileSizeHigh;
                totalSize.QuadPart += fileSize.QuadPart;
                fileCount++;
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
    
    // Sonuçları göster
    double totalSizeMB = (double)totalSize.QuadPart / (1024.0 * 1024.0);
    double avgSizeMB = fileCount > 0 ? totalSizeMB / fileCount : 0;
    
    printf("📁 Toplam dosya sayısı: %d\n", fileCount);
    printf("💾 Toplam boyut: %.2f MB\n", totalSizeMB);
    printf("📈 Ortalama dosya boyutu: %.2f MB\n\n", avgSizeMB);
    
    // En büyük dosyaları bul
    printf("🔝 En büyük dosyalar:\n");
    hFind = FindFirstFile(searchPath, &findFileData);
    
    typedef struct {
        char name[MAX_PATH];
        LARGE_INTEGER size;
    } FileInfo;
    
    FileInfo largestFiles[3] = {0};
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                LARGE_INTEGER fileSize;
                fileSize.LowPart = findFileData.nFileSizeLow;
                fileSize.HighPart = findFileData.nFileSizeHigh;
                
                // En büyük 3 dosyayı bul
                for (int i = 0; i < 3; i++) {
                    if (fileSize.QuadPart > largestFiles[i].size.QuadPart) {
                        // Diğerlerini kaydır
                        for (int j = 2; j > i; j--) {
                            largestFiles[j] = largestFiles[j-1];
                        }
                        strcpy(largestFiles[i].name, findFileData.cFileName);
                        largestFiles[i].size = fileSize;
                        break;
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
    
    for (int i = 0; i < 3 && largestFiles[i].size.QuadPart > 0; i++) {
        double fileSizeMB = (double)largestFiles[i].size.QuadPart / (1024.0 * 1024.0);
        printf("   📄 %s (%.2f MB)\n", largestFiles[i].name, fileSizeMB);
    }
    
    #else
    // Linux/Unix implementation would go here
    printf("Linux/Unix dosya analizi henüz desteklenmiyor.\n");
    #endif
    
    log_info("Dosya boyutu analizi tamamlandı");
    
    // Veritabanına kaydet
    FileOperation operation = {0};
    strcpy(operation.operation, "SIZE_ANALYSIS");
    GetCurrentDirectory(sizeof(operation.file_path), operation.file_path);
    operation.file_size = (long)totalSize.QuadPart;
    strcpy(operation.status, "SUCCESS");
    operation.timestamp = time(NULL);
    
    if (insert_file_operation(&operation)) {
        printf("✅ Boyut analizi işlemi veritabanına kaydedildi\n");
    } else {
        printf("❌ Boyut analizi işlemi veritabanına kaydedilemedi\n");
    }
}

void clean_empty_folders() {
    printf("\nBoş Klasör Temizleme\n");
    printf("===================\n");
    printf("🔍 Boş klasörler taranıyor...\n\n");
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    char emptyFolders[100][MAX_PATH]; // En fazla 100 boş klasör
    int emptyFolderCount = 0;
    int deletedCount = 0;
    
    // Mevcut dizini al
    GetCurrentDirectory(MAX_PATH, searchPath);
    
    // Alt dizinleri tara
    char fullSearchPath[MAX_PATH];
    sprintf(fullSearchPath, "%s\\*", searchPath);
    
    hFind = FindFirstFile(fullSearchPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                strcmp(findFileData.cFileName, ".") != 0 &&
                strcmp(findFileData.cFileName, "..") != 0) {
                
                // Klasörün içeriğini kontrol et
                char subDirPath[MAX_PATH];
                sprintf(subDirPath, "%s\\%s\\*", searchPath, findFileData.cFileName);
                
                WIN32_FIND_DATA subFindData;
                HANDLE hSubFind = FindFirstFile(subDirPath, &subFindData);
                
                int isEmpty = 1;
                if (hSubFind != INVALID_HANDLE_VALUE) {
                    do {
                        if (strcmp(subFindData.cFileName, ".") != 0 &&
                            strcmp(subFindData.cFileName, "..") != 0) {
                            isEmpty = 0;
                            break;
                        }
                    } while (FindNextFile(hSubFind, &subFindData));
                    FindClose(hSubFind);
                }
                
                if (isEmpty && emptyFolderCount < 100) {
                    sprintf(emptyFolders[emptyFolderCount], "%s\\%s", searchPath, findFileData.cFileName);
                    emptyFolderCount++;
                }
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
    
    if (emptyFolderCount > 0) {
        printf("✅ Bulunan boş klasörler:\n");
        for (int i = 0; i < emptyFolderCount; i++) {
            printf("   📁 %s\n", emptyFolders[i]);
        }
        
        printf("\nBu klasörleri silmek istiyor musunuz? (E/H): ");
        char response;
        scanf(" %c", &response);
        
        if (response == 'E' || response == 'e') {
            for (int i = 0; i < emptyFolderCount; i++) {
                if (RemoveDirectory(emptyFolders[i])) {
                    deletedCount++;
                    printf("   ✅ Silindi: %s\n", emptyFolders[i]);
                } else {
                    printf("   ❌ Silinemedi: %s (Hata: %lu)\n", emptyFolders[i], GetLastError());
                }
            }
            printf("\n🗑️  %d boş klasör silindi!\n", deletedCount);
        } else {
            printf("\n❌ İşlem iptal edildi.\n");
        }
    } else {
        printf("ℹ️  Boş klasör bulunamadı.\n");
    }
    
#else
    // Linux/Unix implementation
    printf("Bu özellik henüz Linux/Unix sistemlerde desteklenmiyor.\n");
#endif
    
    log_info("Boş klasör temizleme işlemi tamamlandı - %d klasör silindi", deletedCount);
    
    // Veritabanına kaydet
    FileOperation operation = {0};
    strcpy(operation.operation, "CLEAN_EMPTY");
    strcpy(operation.file_path, searchPath);
    operation.file_size = deletedCount;
    strcpy(operation.status, deletedCount > 0 ? "SUCCESS" : "NO_FOLDERS");
    operation.timestamp = time(NULL);
    
    if (insert_file_operation(&operation)) {
        printf("✅ Klasör temizleme işlemi veritabanına kaydedildi\n");
    } else {
        printf("❌ Klasör temizleme işlemi veritabanına kaydedilemedi\n");
    }
}

void organize_by_type() {
    printf("\nDosya Türü Organizasyonu\n");
    printf("=======================\n");
    printf("📂 Dosyalar türlerine göre organize ediliyor...\n\n");
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    char fullSearchPath[MAX_PATH];
    
    // Dosya türü sayaçları
    int imageCount = 0, documentCount = 0, videoCount = 0, musicCount = 0, archiveCount = 0, otherCount = 0;
    
    // Mevcut dizini al
    GetCurrentDirectory(MAX_PATH, searchPath);
    sprintf(fullSearchPath, "%s\\*", searchPath);
    
    // Hedef klasörleri oluştur
    char imagePath[MAX_PATH], docPath[MAX_PATH], videoPath[MAX_PATH], musicPath[MAX_PATH], archivePath[MAX_PATH];
    sprintf(imagePath, "%s\\Images", searchPath);
    sprintf(docPath, "%s\\Documents", searchPath);
    sprintf(videoPath, "%s\\Videos", searchPath);
    sprintf(musicPath, "%s\\Music", searchPath);
    sprintf(archivePath, "%s\\Archives", searchPath);
    
    CreateDirectory(imagePath, NULL);
    CreateDirectory(docPath, NULL);
    CreateDirectory(videoPath, NULL);
    CreateDirectory(musicPath, NULL);
    CreateDirectory(archivePath, NULL);
    
    hFind = FindFirstFile(fullSearchPath, &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                char* extension = strrchr(findFileData.cFileName, '.');
                if (extension != NULL) {
                    extension++; // '.' karakterini atla
                    
                    char sourcePath[MAX_PATH], destPath[MAX_PATH];
                    sprintf(sourcePath, "%s\\%s", searchPath, findFileData.cFileName);
                    
                    // Dosya türüne göre sınıflandır
                    if (_stricmp(extension, "jpg") == 0 || _stricmp(extension, "jpeg") == 0 || 
                        _stricmp(extension, "png") == 0 || _stricmp(extension, "gif") == 0 || 
                        _stricmp(extension, "bmp") == 0 || _stricmp(extension, "tiff") == 0) {
                        sprintf(destPath, "%s\\%s", imagePath, findFileData.cFileName);
                        if (MoveFile(sourcePath, destPath)) imageCount++;
                    }
                    else if (_stricmp(extension, "txt") == 0 || _stricmp(extension, "doc") == 0 || 
                             _stricmp(extension, "docx") == 0 || _stricmp(extension, "pdf") == 0 || 
                             _stricmp(extension, "rtf") == 0 || _stricmp(extension, "odt") == 0) {
                        sprintf(destPath, "%s\\%s", docPath, findFileData.cFileName);
                        if (MoveFile(sourcePath, destPath)) documentCount++;
                    }
                    else if (_stricmp(extension, "mp4") == 0 || _stricmp(extension, "avi") == 0 || 
                             _stricmp(extension, "mkv") == 0 || _stricmp(extension, "mov") == 0 || 
                             _stricmp(extension, "wmv") == 0 || _stricmp(extension, "flv") == 0) {
                        sprintf(destPath, "%s\\%s", videoPath, findFileData.cFileName);
                        if (MoveFile(sourcePath, destPath)) videoCount++;
                    }
                    else if (_stricmp(extension, "mp3") == 0 || _stricmp(extension, "wav") == 0 || 
                             _stricmp(extension, "flac") == 0 || _stricmp(extension, "aac") == 0 || 
                             _stricmp(extension, "ogg") == 0 || _stricmp(extension, "wma") == 0) {
                        sprintf(destPath, "%s\\%s", musicPath, findFileData.cFileName);
                        if (MoveFile(sourcePath, destPath)) musicCount++;
                    }
                    else if (_stricmp(extension, "zip") == 0 || _stricmp(extension, "rar") == 0 || 
                             _stricmp(extension, "7z") == 0 || _stricmp(extension, "tar") == 0 || 
                             _stricmp(extension, "gz") == 0 || _stricmp(extension, "bz2") == 0) {
                        sprintf(destPath, "%s\\%s", archivePath, findFileData.cFileName);
                        if (MoveFile(sourcePath, destPath)) archiveCount++;
                    }
                    else {
                        otherCount++;
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
    
    printf("✅ Organizasyon tamamlandı:\n");
    if (imageCount > 0) printf("   📁 Images/ (%d dosya)\n", imageCount);
    if (documentCount > 0) printf("   📁 Documents/ (%d dosya)\n", documentCount);
    if (videoCount > 0) printf("   📁 Videos/ (%d dosya)\n", videoCount);
    if (musicCount > 0) printf("   📁 Music/ (%d dosya)\n", musicCount);
    if (archiveCount > 0) printf("   📁 Archives/ (%d dosya)\n", archiveCount);
    if (otherCount > 0) printf("   📁 Diğer dosyalar organize edilmedi: %d\n", otherCount);
    
    int totalOrganized = imageCount + documentCount + videoCount + musicCount + archiveCount;
    if (totalOrganized == 0) {
        printf("ℹ️  Organize edilecek dosya bulunamadı.\n");
    } else {
        printf("\n📊 Toplam %d dosya organize edildi.\n", totalOrganized);
    }
    
#else
    // Linux/Unix implementation
    printf("Bu özellik henüz Linux/Unix sistemlerde desteklenmiyor.\n");
#endif
    
    log_info("Dosya türü organizasyonu tamamlandı");
    
    // Veritabanına kaydet
    FileOperation operation = {0};
    strcpy(operation.operation, "ORGANIZE_TYPE");
    strcpy(operation.file_path, searchPath);
    operation.file_size = totalOrganized;
    strcpy(operation.status, totalOrganized > 0 ? "SUCCESS" : "NO_FILES");
    operation.timestamp = time(NULL);
    
    if (insert_file_operation(&operation)) {
        printf("✅ Organizasyon işlemi veritabanına kaydedildi\n");
    } else {
        printf("❌ Organizasyon işlemi veritabanına kaydedilemedi\n");
    }
}

void run_file_management() {
    int choice;
    char input[10];
    
    while (1) {
        show_file_management_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    file_search();
                    break;
                case 2:
                    bulk_rename();
                    break;
                case 3:
                    analyze_file_sizes();
                    break;
                case 4:
                    clean_empty_folders();
                    break;
                case 5:
                    organize_by_type();
                    break;
                case 0:
                    return;
                default:
                    printf("\n❌ Geçersiz seçim! Lütfen 0-5 arası bir sayı girin.\n");
                    break;
            }
            
            if (choice != 0) {
                printf("\nDevam etmek için Enter tuşuna basın...");
                getchar();
            }
        }
    }
}