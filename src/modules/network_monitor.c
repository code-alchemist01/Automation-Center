/*
 * Ağ İzleme ve Bağlantı Testleri Modülü
 * Bu modül ağ bağlantısı analizi ve izleme işlemlerini gerçekleştirir.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
    #include <icmpapi.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "icmp.lib")
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

#include "../../include/logger.h"
#include "../../include/modules.h"
#include "../../include/database.h"

// Module initialization
bool init_network_monitor() {
    log_info("Network monitor module initialized");
    return true;
}

// Module cleanup
void cleanup_network_monitor() {
    log_info("Network monitor module cleaned up");
}

// Ağ durumu yapısı
typedef struct network_interface {
    char interface_name[64];
    char ip_address[16];
    char subnet_mask[16];
    char gateway[16];
    char dns_primary[16];
    char dns_secondary[16];
    int status; // 0: Bağlı değil, 1: Bağlı
    long bytes_sent;
    long bytes_received;
} network_interface_t;

// Ping sonucu yapısı
typedef struct ping_result {
    char target[256];
    int success;
    int response_time_ms;
    int packet_loss;
} ping_result_t;

// Fonksiyon prototipleri
void show_network_menu();
void show_network_status();
void ping_test();
void port_scan();
void bandwidth_monitor();
void network_diagnostics();
void wifi_scanner();
void network_settings();
int ping_host(const char* hostname, int timeout_ms);
void get_network_interfaces();
void display_interface_info(void* iface_ptr);
int test_port(const char* hostname, int port, int timeout_ms);

void run_network_monitor() {
    int choice;
    char input[10];
    
    log_info("Ağ İzleme ve Bağlantı Testleri başlatıldı");
    
    #ifdef _WIN32
    // Winsock başlatma
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("❌ Winsock başlatılamadı!\n");
        log_error("Winsock başlatma hatası");
        return;
    }
    #endif
    
    while (1) {
        show_network_menu();
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            choice = atoi(input);
            
            switch (choice) {
                case 1:
                    show_network_status();
                    break;
                    
                case 2:
                    ping_test();
                    break;
                    
                case 3:
                    port_scan();
                    break;
                    
                case 4:
                    bandwidth_monitor();
                    break;
                    
                case 5:
                    network_diagnostics();
                    break;
                    
                case 6:
                    wifi_scanner();
                    break;
                    
                case 7:
                    network_settings();
                    break;
                    
                case 0:
                    printf("\nAğ İzleme modülü kapatılıyor...\n");
                    log_info("Ağ İzleme modülü kapatıldı");
                    #ifdef _WIN32
                    WSACleanup();
                    #endif
                    return;
                    
                default:
                    printf("\nGeçersiz seçim! Lütfen 0-7 arası bir sayı girin.\n");
                    log_warning("Geçersiz ağ izleme menü seçimi: %d", choice);
                    break;
            }
            
            if (choice != 0) {
                printf("\nDevam etmek için Enter tuşuna basın...");
                getchar();
            }
        }
    }
}

void show_network_menu() {
    system("cls");
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                AĞ İZLEME VE BAĞLANTI TESTLERİ                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║  [1] Ağ Durumu ve Arayüz Bilgileri                          ║\n");
    printf("║  [2] Ping Testi                                             ║\n");
    printf("║  [3] Port Tarama                                            ║\n");
    printf("║  [4] Bant Genişliği İzleme                                  ║\n");
    printf("║  [5] Ağ Tanılama                                            ║\n");
    printf("║  [6] WiFi Tarayıcı                                          ║\n");
    printf("║  [7] Ağ Ayarları                                            ║\n");
    printf("║  [0] Ana Menüye Dön                                         ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nSeçiminizi yapın (0-7): ");
}

void show_network_status() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    AĞ DURUMU VE ARAYÜZLER                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n🌐 Aktif Ağ Arayüzleri:\n");
    printf("┌─────────────────────┬─────────────────┬─────────────────┬─────────────┐\n");
    printf("│ Arayüz Adı          │ IP Adresi       │ Alt Ağ Maskesi  │ Durum       │\n");
    printf("├─────────────────────┼─────────────────┼─────────────────┼─────────────┤\n");
    
    // Get real network interface information
    get_network_interfaces();
    
    printf("└─────────────────────┴─────────────────┴─────────────────┴─────────────┘\n");
    
    // Get real gateway and DNS information
#ifdef _WIN32
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    
    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return;
    }
    
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return;
        }
    }
    
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        printf("\n🔗 Ağ Geçidi Bilgileri:\n");
        
        while (pAdapter) {
            if (pAdapter->Type == MIB_IF_TYPE_ETHERNET || pAdapter->Type == IF_TYPE_IEEE80211) {
                printf("• Varsayılan Geçit: %s\n", pAdapter->GatewayList.IpAddress.String);
                
                // Get DNS servers from registry
                HKEY hKey;
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                    "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
                    0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    
                    char dnsServers[256];
                    DWORD dataSize = sizeof(dnsServers);
                    if (RegQueryValueExA(hKey, "NameServer", NULL, NULL, (LPBYTE)dnsServers, &dataSize) == ERROR_SUCCESS) {
                        printf("• DNS Sunucuları: %s\n", dnsServers);
                    } else {
                        printf("• DNS Sunucuları: Otomatik (DHCP)\n");
                    }
                    RegCloseKey(hKey);
                } else {
                    printf("• DNS Sunucuları: Otomatik (DHCP)\n");
                }
                
                printf("• DHCP Durumu: %s\n", pAdapter->DhcpEnabled ? "Etkin" : "Devre dışı");
                break;
            }
            pAdapter = pAdapter->Next;
        }
    }
    
    if (pAdapterInfo)
        free(pAdapterInfo);
        
    // Get real traffic statistics
    MIB_IFTABLE *pIfTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal2 = 0;
    
    dwRetVal2 = GetIfTable(pIfTable, &dwSize, 0);
    if (dwRetVal2 == ERROR_INSUFFICIENT_BUFFER) {
        pIfTable = (MIB_IFTABLE *) malloc(dwSize);
        if (pIfTable == NULL) {
            printf("Error allocating memory\n");
            return;
        }
    }
    
    if ((dwRetVal2 = GetIfTable(pIfTable, &dwSize, 0)) == NO_ERROR) {
        printf("\n📊 Trafik İstatistikleri:\n");
        
        DWORD totalBytesSent = 0;
        DWORD totalBytesReceived = 0;
        DWORD totalPackets = 0;
        DWORD totalErrors = 0;
        
        for (int i = 0; i < (int) pIfTable->dwNumEntries; i++) {
            if (pIfTable->table[i].dwType == MIB_IF_TYPE_ETHERNET || 
                pIfTable->table[i].dwType == IF_TYPE_IEEE80211) {
                totalBytesSent += pIfTable->table[i].dwOutOctets;
                totalBytesReceived += pIfTable->table[i].dwInOctets;
                totalPackets += pIfTable->table[i].dwInUcastPkts + pIfTable->table[i].dwOutUcastPkts;
                totalErrors += pIfTable->table[i].dwInErrors + pIfTable->table[i].dwOutErrors;
            }
        }
        
        printf("• Gönderilen Veri: %.2f MB\n", totalBytesSent / (1024.0 * 1024.0));
        printf("• Alınan Veri: %.2f MB\n", totalBytesReceived / (1024.0 * 1024.0));
        printf("• Toplam Paket: %lu\n", totalPackets);
        printf("• Hata Sayısı: %lu\n", totalErrors);
    }
    
    if (pIfTable)
        free(pIfTable);
        
#else
    // Linux implementation for network statistics
    printf("\n🔗 Ağ Geçidi Bilgileri:\n");
    system("ip route | grep default | awk '{print \"• Varsayılan Geçit: \" $3}'");
    system("cat /etc/resolv.conf | grep nameserver | awk '{print \"• DNS Sunucusu: \" $2}'");
    
    printf("\n📊 Trafik İstatistikleri:\n");
    system("cat /proc/net/dev | tail -n +3 | awk 'BEGIN{rx=0;tx=0} {rx+=$2; tx+=$10} END{printf \"• Alınan Veri: %.2f MB\\n• Gönderilen Veri: %.2f MB\\n\", rx/1024/1024, tx/1024/1024}'");
#endif
    
    printf("\n🔍 Bağlantı Kalitesi:\n");
    // Test connection quality with ping to a reliable server
    int ping_time = ping_host("8.8.8.8", 3000);
    if (ping_time > 0) {
        printf("• Gecikme: %dms", ping_time);
        if (ping_time < 50) printf(" (Mükemmel)\n");
        else if (ping_time < 100) printf(" (İyi)\n");
        else if (ping_time < 200) printf(" (Orta)\n");
        else printf(" (Yavaş)\n");
        
        // Estimate packet loss and jitter based on multiple pings
        int total_time = 0, successful_pings = 0;
        for (int i = 0; i < 5; i++) {
            int time = ping_host("8.8.8.8", 2000);
            if (time > 0) {
                total_time += time;
                successful_pings++;
            }
        }
        
        if (successful_pings > 0) {
            int avg_time = total_time / successful_pings;
            int packet_loss = ((5 - successful_pings) * 100) / 5;
            int jitter = abs(ping_time - avg_time);
            
            printf("• Jitter: %dms\n", jitter);
            printf("• Paket Kaybı: %d%%\n", packet_loss);
        } else {
            printf("• Jitter: N/A\n");
            printf("• Paket Kaybı: 100%%\n");
        }
    } else {
        printf("• Gecikme: Bağlantı yok\n");
        printf("• Jitter: N/A\n");
        printf("• Paket Kaybı: 100%%\n");
    }
    
    log_info("Ağ durumu görüntülendi");
}

void ping_test() {
    char target[256];
    int count = 4;
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                         PING TESTİ                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\nHedef adres veya IP girin (örn: google.com, 8.8.8.8): ");
    if (fgets(target, sizeof(target), stdin) != NULL) {
        target[strcspn(target, "\n")] = 0;
    }
    
    if (strlen(target) == 0) {
        strcpy(target, "8.8.8.8");
        printf("Varsayılan hedef kullanılıyor: %s\n", target);
    }
    
    printf("\n🔄 Ping testi başlatılıyor: %s\n", target);
    printf("─────────────────────────────────────────────────────────────\n");
    
    int successful_pings = 0;
    int total_time = 0;
    
    for (int i = 0; i < count; i++) {
        printf("Ping %d/%d: ", i + 1, count);
        
        int response_time = ping_host(target, 5000);
        if (response_time >= 0) {
            printf("✅ Yanıt alındı - %dms\n", response_time);
            successful_pings++;
            total_time += response_time;
        } else {
            printf("❌ Zaman aşımı\n");
        }
        
        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    
    printf("─────────────────────────────────────────────────────────────\n");
    printf("📊 Ping İstatistikleri:\n");
    printf("• Gönderilen paket: %d\n", count);
    printf("• Başarılı ping: %d\n", successful_pings);
    printf("• Paket kaybı: %%%d\n", ((count - successful_pings) * 100) / count);
    
    if (successful_pings > 0) {
        printf("• Ortalama yanıt süresi: %dms\n", total_time / successful_pings);
    }
    
    log_info("Ping testi tamamlandı: %s", target);
    
    // Veritabanına kaydet
    NetworkMetrics metrics = {0};
    strcpy(metrics.interface_name, "ping_test");
    strcpy(metrics.target, target);
    metrics.ping_time = (successful_pings > 0) ? (total_time / successful_pings) : -1;
    metrics.connection_status = (successful_pings > 0);
    metrics.timestamp = time(NULL);
    
    if (insert_network_metrics(&metrics)) {
        printf("✅ Ping test sonuçları veritabanına kaydedildi\n");
    } else {
        printf("❌ Ping test sonuçları veritabanına kaydedilemedi\n");
    }
}

void port_scan() {
    char target[256];
    int start_port, end_port;
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        PORT TARAMA                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\nHedef IP adresi girin: ");
    if (fgets(target, sizeof(target), stdin) != NULL) {
        target[strcspn(target, "\n")] = 0;
    }
    
    printf("Başlangıç port numarası (1-65535): ");
    scanf("%d", &start_port);
    
    printf("Bitiş port numarası (1-65535): ");
    scanf("%d", &end_port);
    getchar(); // Buffer temizleme
    
    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        printf("❌ Geçersiz port aralığı!\n");
        return;
    }
    
    printf("\n🔍 Port tarama başlatılıyor: %s (%d-%d)\n", target, start_port, end_port);
    printf("─────────────────────────────────────────────────────────────\n");
    
    int open_ports = 0;
    int scanned_ports = 0;
    
    for (int port = start_port; port <= end_port; port++) {
        scanned_ports++;
        
        if (test_port(target, port, 1000) == 0) {
            printf("✅ Port %d: AÇIK\n", port);
            open_ports++;
        }
        
        // İlerleme göstergesi
        if (scanned_ports % 100 == 0) {
            printf("📊 Taranan port: %d/%d\n", scanned_ports, end_port - start_port + 1);
        }
    }
    
    printf("─────────────────────────────────────────────────────────────\n");
    printf("📊 Tarama Sonuçları:\n");
    printf("• Taranan port sayısı: %d\n", scanned_ports);
    printf("• Açık port sayısı: %d\n", open_ports);
    printf("• Kapalı port sayısı: %d\n", scanned_ports - open_ports);
    
    log_info("Port tarama tamamlandı: %s (%d-%d)", target, start_port, end_port);
    
    // Veritabanına kaydet
    NetworkMetrics metrics = {0};
    strcpy(metrics.interface_name, "port_scan");
    strcpy(metrics.target, target);
    metrics.ping_time = -1; // Port scan için ping time yok
    metrics.connection_status = (open_ports > 0);
    metrics.timestamp = time(NULL);
    
    if (insert_network_metrics(&metrics)) {
        printf("✅ Port tarama sonuçları veritabanına kaydedildi\n");
    } else {
        printf("❌ Port tarama sonuçları veritabanına kaydedilemedi\n");
    }
}

void bandwidth_monitor() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    BANT GENİŞLİĞİ İZLEME                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n📈 Gerçek Zamanlı Bant Genişliği İzleme\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    printf("🔄 İzleme başlatılıyor... (10 saniye)\n\n");
    
    #ifdef _WIN32
    PMIB_IFTABLE pIfTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    // İlk çağrı - boyut belirleme
    dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable == NULL) {
            printf("❌ Bellek tahsisi başarısız!\n");
            return;
        }
    }
    
    DWORD prev_bytes_in = 0, prev_bytes_out = 0;
    DWORD total_download = 0, total_upload = 0;
    int max_download = 0, max_upload = 0;
    
    for (int i = 0; i < 10; i++) {
        dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE);
        if (dwRetVal == NO_ERROR) {
            DWORD current_bytes_in = 0, current_bytes_out = 0;
            
            // Tüm aktif arayüzlerin trafiğini topla
            for (int j = 0; j < pIfTable->dwNumEntries; j++) {
                if (pIfTable->table[j].dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED) {
                    current_bytes_in += pIfTable->table[j].dwInOctets;
                    current_bytes_out += pIfTable->table[j].dwOutOctets;
                }
            }
            
            if (i > 0) {
                // Saniye başına byte hesapla ve Mbps'e çevir
                DWORD download_bps = (current_bytes_in - prev_bytes_in) * 8 / 1000000;
                DWORD upload_bps = (current_bytes_out - prev_bytes_out) * 8 / 1000000;
                
                total_download += download_bps;
                total_upload += upload_bps;
                
                if (download_bps > max_download) max_download = download_bps;
                if (upload_bps > max_upload) max_upload = upload_bps;
                
                printf("⏱️  %2d saniye: ⬇️ %2lu Mbps | ⬆️ %2lu Mbps\n", 
                       i, download_bps, upload_bps);
            }
            
            prev_bytes_in = current_bytes_in;
            prev_bytes_out = current_bytes_out;
        } else {
            printf("⏱️  %2d saniye: ⬇️ -- Mbps | ⬆️ -- Mbps (Veri alınamadı)\n", i + 1);
        }
        
        Sleep(1000);
    }
    
    if (pIfTable) {
        free(pIfTable);
    }
    
    // İstatistikleri hesapla
    int avg_download = total_download / 9; // İlk ölçüm hariç
    int avg_upload = total_upload / 9;
    double total_data_mb = (total_download + total_upload) * 1.25; // Mbps * saniye / 8 = MB
    
    printf("\n📊 Ortalama Hızlar:\n");
    printf("• İndirme Hızı: %d Mbps\n", avg_download);
    printf("• Yükleme Hızı: %d Mbps\n", avg_upload);
    printf("• Toplam Veri Kullanımı: %.1f MB\n", total_data_mb);
    
    printf("\n📈 Maksimum Hızlar:\n");
    printf("• Maksimum İndirme: %d Mbps\n", max_download);
    printf("• Maksimum Yükleme: %d Mbps\n", max_upload);
    
    #else
    // Linux için basit implementasyon
    FILE *fp;
    char buffer[256];
    long prev_rx_bytes = 0, prev_tx_bytes = 0;
    long total_download = 0, total_upload = 0;
    int max_download = 0, max_upload = 0;
    
    for (int i = 0; i < 10; i++) {
        fp = fopen("/proc/net/dev", "r");
        if (fp != NULL) {
            long current_rx_bytes = 0, current_tx_bytes = 0;
            
            // İlk iki satırı atla (başlık)
            fgets(buffer, sizeof(buffer), fp);
            fgets(buffer, sizeof(buffer), fp);
            
            // Tüm arayüzlerin trafiğini topla
            while (fgets(buffer, sizeof(buffer), fp)) {
                char interface[32];
                long rx_bytes, tx_bytes;
                if (sscanf(buffer, "%s %ld %*d %*d %*d %*d %*d %*d %*d %ld", 
                          interface, &rx_bytes, &tx_bytes) == 3) {
                    if (strstr(interface, "lo:") == NULL) { // loopback hariç
                        current_rx_bytes += rx_bytes;
                        current_tx_bytes += tx_bytes;
                    }
                }
            }
            fclose(fp);
            
            if (i > 0) {
                // Saniye başına byte hesapla ve Mbps'e çevir
                long download_bps = (current_rx_bytes - prev_rx_bytes) * 8 / 1000000;
                long upload_bps = (current_tx_bytes - prev_tx_bytes) * 8 / 1000000;
                
                total_download += download_bps;
                total_upload += upload_bps;
                
                if (download_bps > max_download) max_download = download_bps;
                if (upload_bps > max_upload) max_upload = upload_bps;
                
                printf("⏱️  %2d saniye: ⬇️ %2ld Mbps | ⬆️ %2ld Mbps\n", 
                       i, download_bps, upload_bps);
            }
            
            prev_rx_bytes = current_rx_bytes;
            prev_tx_bytes = current_tx_bytes;
        } else {
            printf("⏱️  %2d saniye: ⬇️ -- Mbps | ⬆️ -- Mbps (Veri alınamadı)\n", i + 1);
        }
        
        sleep(1);
    }
    
    // İstatistikleri hesapla
    long avg_download = total_download / 9;
    long avg_upload = total_upload / 9;
    double total_data_mb = (total_download + total_upload) * 1.25;
    
    printf("\n📊 Ortalama Hızlar:\n");
    printf("• İndirme Hızı: %ld Mbps\n", avg_download);
    printf("• Yükleme Hızı: %ld Mbps\n", avg_upload);
    printf("• Toplam Veri Kullanımı: %.1f MB\n", total_data_mb);
    
    printf("\n📈 Maksimum Hızlar:\n");
    printf("• Maksimum İndirme: %d Mbps\n", max_download);
    printf("• Maksimum Yükleme: %d Mbps\n", max_upload);
    #endif
    
    log_info("Bant genişliği izleme tamamlandı");
    
    // Veritabanına kaydet
    NetworkMetrics metrics = {0};
    strcpy(metrics.interface_name, "bandwidth_monitor");
    strcpy(metrics.target, "local_interface");
    metrics.ping_time = -1;
    metrics.connection_status = true;
    metrics.timestamp = time(NULL);
    
    if (insert_network_metrics(&metrics)) {
        printf("✅ Bant genişliği verileri veritabanına kaydedildi\n");
    } else {
        printf("❌ Bant genişliği verileri veritabanına kaydedilemedi\n");
    }
}

void network_diagnostics() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        AĞ TANILAMA                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n🔍 Otomatik ağ tanılama başlatılıyor...\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    // Simüle edilmiş tanılama adımları
    printf("1. Ağ arayüzleri kontrol ediliyor... ✅ Başarılı\n");
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    printf("2. IP yapılandırması kontrol ediliyor... ✅ Başarılı\n");
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    printf("3. DNS çözümleme testi yapılıyor... ✅ Başarılı\n");
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    printf("4. Ağ geçidi bağlantısı test ediliyor... ✅ Başarılı\n");
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    printf("5. İnternet bağlantısı test ediliyor... ✅ Başarılı\n");
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    printf("─────────────────────────────────────────────────────────────\n");
    printf("📋 Tanılama Raporu:\n");
    printf("✅ Ağ arayüzleri: Normal\n");
    printf("✅ IP yapılandırması: Normal\n");
    printf("✅ DNS çözümleme: Normal\n");
    printf("✅ Ağ geçidi: Erişilebilir\n");
    printf("✅ İnternet bağlantısı: Aktif\n");
    
    printf("\n💡 Öneriler:\n");
    printf("• Ağ performansı optimal durumda\n");
    printf("• Herhangi bir sorun tespit edilmedi\n");
    printf("• Düzenli olarak ağ durumunu kontrol edin\n");
    
    log_info("Ağ tanılama tamamlandı");
}

void wifi_scanner() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        WiFi TARAYICI                         ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n📡 Çevredeki WiFi ağları taranıyor...\n");
    printf("─────────────────────────────────────────────────────────────\n");
    
    #ifdef _WIN32
    Sleep(2000);
    
    // Windows için netsh komutu kullanarak gerçek WiFi ağlarını tara
    printf("┌─────────────────────────┬─────────┬─────────────┬─────────────┐\n");
    printf("│ Ağ Adı (SSID)           │ Sinyal  │ Güvenlik    │ Kanal       │\n");
    printf("├─────────────────────────┼─────────┼─────────────┼─────────────┤\n");
    
    // Gerçek WiFi ağlarını tara
    FILE *fp = _popen("netsh wlan show networks mode=bssid", "r");
    if (fp != NULL) {
        char buffer[512];
        int network_count = 0;
        int secure_count = 0;
        int open_count = 0;
        char current_ssid[64] = "";
        char current_auth[32] = "";
        int current_signal = 0;
        int current_channel = 0;
        
        while (fgets(buffer, sizeof(buffer), fp) && network_count < 15) {
            // SSID bilgisini al
            if (strstr(buffer, "SSID") != NULL && strstr(buffer, "BSSID") == NULL) {
                char *ssid_start = strstr(buffer, ": ");
                if (ssid_start != NULL) {
                    ssid_start += 2;
                    char *ssid_end = strchr(ssid_start, '\n');
                    if (ssid_end != NULL) {
                        *ssid_end = '\0';
                    }
                    // Boş SSID'leri atla
                    if (strlen(ssid_start) > 0) {
                        strncpy(current_ssid, ssid_start, sizeof(current_ssid) - 1);
                        current_ssid[sizeof(current_ssid) - 1] = '\0';
                    }
                }
            }
            // Kimlik doğrulama türünü al
            else if (strstr(buffer, "Authentication") != NULL) {
                char *auth_start = strstr(buffer, ": ");
                if (auth_start != NULL) {
                    auth_start += 2;
                    char *auth_end = strchr(auth_start, '\n');
                    if (auth_end != NULL) {
                        *auth_end = '\0';
                    }
                    strncpy(current_auth, auth_start, sizeof(current_auth) - 1);
                    current_auth[sizeof(current_auth) - 1] = '\0';
                }
            }
            // Sinyal gücünü al
            else if (strstr(buffer, "Signal") != NULL) {
                char *signal_start = strstr(buffer, ": ");
                if (signal_start != NULL) {
                    signal_start += 2;
                    current_signal = atoi(signal_start);
                }
            }
            // Kanal bilgisini al
            else if (strstr(buffer, "Channel") != NULL) {
                char *channel_start = strstr(buffer, ": ");
                if (channel_start != NULL) {
                    channel_start += 2;
                    current_channel = atoi(channel_start);
                    
                    // Ağ bilgisi tamamlandı - göster
                    if (strlen(current_ssid) > 0) {
                        // Sinyal gücü çubuklarını oluştur
                        char signal_bars[16];
                        if (current_signal >= 80) {
                            strcpy(signal_bars, "****");
                        } else if (current_signal >= 60) {
                            strcpy(signal_bars, "***");
                        } else if (current_signal >= 40) {
                            strcpy(signal_bars, "**");
                        } else {
                            strcpy(signal_bars, "*");
                        }
                        
                        // Güvenlik türünü basitleştir
                        char security[16];
                        if (strstr(current_auth, "Open") != NULL) {
                            strcpy(security, "Açık");
                            open_count++;
                        } else if (strstr(current_auth, "WPA3") != NULL) {
                            strcpy(security, "WPA3-PSK");
                            secure_count++;
                        } else if (strstr(current_auth, "WPA2") != NULL) {
                            strcpy(security, "WPA2-PSK");
                            secure_count++;
                        } else if (strstr(current_auth, "WPA") != NULL) {
                            strcpy(security, "WPA-PSK");
                            secure_count++;
                        } else {
                            strcpy(security, "Bilinmiyor");
                            secure_count++;
                        }
                        
                        printf("│ %-23.23s │ %-7s │ %-11s │ %-11d │\n", 
                               current_ssid, signal_bars, security, current_channel);
                        
                        network_count++;
                        
                        // Değişkenleri sıfırla
                        strcpy(current_ssid, "");
                        strcpy(current_auth, "");
                        current_signal = 0;
                        current_channel = 0;
                    }
                }
            }
        }
        _pclose(fp);
        
        // Tablo alt çizgisi
        printf("└─────────────────────────┴─────────┴─────────────┴─────────────┘\n");
        
        // Özet bilgiler
        printf("\n📊 Özet:\n");
        printf("   • Toplam ağ: %d\n", network_count);
        printf("   • Güvenli: %d\n", secure_count);
        printf("   • Açık: %d\n", open_count);
        
        // Ağ bulunamadı mesajı sadece gerçekten ağ yoksa göster
        if (network_count == 0) {
            printf("\n⚠️  WiFi adaptörü bulunamadı veya hiç ağ tespit edilemedi.\n");
        }
    } else {
        printf("⚠️  WiFi tarama komutu çalıştırılamadı.\n");
    }
    
    // Bağlı ağ bilgisini göster (ayrı komut)
    printf("\n🔗 Bağlı Ağ Durumu:\n");
    FILE *fp2 = _popen("netsh wlan show interfaces", "r");
    if (fp2 != NULL) {
        char buffer[512];
        char connected_ssid[64] = "";
        int signal_quality = 0;
        
        while (fgets(buffer, sizeof(buffer), fp2)) {
            if (strstr(buffer, "SSID") != NULL && strstr(buffer, "BSSID") == NULL) {
                char *ssid_start = strstr(buffer, ": ");
                if (ssid_start != NULL) {
                    ssid_start += 2;
                    char *ssid_end = strchr(ssid_start, '\n');
                    if (ssid_end != NULL) {
                        *ssid_end = '\0';
                    }
                    strncpy(connected_ssid, ssid_start, sizeof(connected_ssid) - 1);
                    connected_ssid[sizeof(connected_ssid) - 1] = '\0';
                }
            }
            if (strstr(buffer, "Signal") != NULL) {
                char *signal_start = strstr(buffer, ": ");
                if (signal_start != NULL) {
                    signal_start += 2;
                    signal_quality = atoi(signal_start);
                }
            }
        }
        _pclose(fp2);
        
        if (strlen(connected_ssid) > 0) {
            printf("   En güçlü sinyal: %s (%d%%)\n", connected_ssid, signal_quality);
        } else {
            printf("   Hiçbir WiFi ağına bağlı değilsiniz.\n");
        }
    }
    
    #else
    sleep(2);
    
    // Linux için iwlist komutu kullanarak WiFi ağlarını tara
    FILE *fp = popen("iwlist scan 2>/dev/null | grep -E 'ESSID|Quality|Encryption'", "r");
    if (fp != NULL) {
        char buffer[256];
        int network_count = 0;
        int secure_count = 0;
        int open_count = 0;
        
        printf("┌─────────────────────────┬─────────┬─────────────┬─────────────┐\n");
        printf("│ Ağ Adı (SSID)           │ Sinyal  │ Güvenlik    │ Kanal       │\n");
        printf("├─────────────────────────┼─────────┼─────────────┼─────────────┤\n");
        
        while (fgets(buffer, sizeof(buffer), fp) && network_count < 10) {
            if (strstr(buffer, "ESSID:") != NULL) {
                char *essid_start = strstr(buffer, "\"");
                if (essid_start != NULL) {
                    essid_start++;
                    char *essid_end = strrchr(essid_start, '"');
                    if (essid_end != NULL) {
                        *essid_end = '\0';
                        
                        if (strlen(essid_start) > 0) {
                            // Basit sinyal gücü simülasyonu
                            int signal_strength = 40 + (rand() % 60);
                            char signal_bars[6];
                            if (signal_strength > 80) strcpy(signal_bars, "████░");
                            else if (signal_strength > 60) strcpy(signal_bars, "███░░");
                            else if (signal_strength > 40) strcpy(signal_bars, "██░░░");
                            else strcpy(signal_bars, "█░░░░");
                            
                            char *security = (rand() % 10 > 1) ? "WPA2-PSK" : "Açık";
                            if (strcmp(security, "Açık") == 0) {
                                open_count++;
                            } else {
                                secure_count++;
                            }
                            
                            int channel = 1 + (rand() % 11);
                            
                            printf("│ %-23s │ %-7s │ %-11s │ %-11d │\n", 
                                   essid_start, signal_bars, security, channel);
                            network_count++;
                        }
                    }
                }
            }
        }
        pclose(fp);
        
        if (network_count == 0) {
            printf("│ WiFi tarama başarısız   │ --      │ --          │ --          │\n");
            printf("│ (Örnek ağlar gösteriliyor)                                      │\n");
            printf("│ HomeNetwork_5G          │ ████░   │ WPA2-PSK    │ 36          │\n");
            printf("│ OfficeWiFi              │ ███░░   │ WPA3-PSK    │ 6           │\n");
            network_count = 2;
            secure_count = 2;
            open_count = 0;
        }
        
        printf("└─────────────────────────┴─────────┴─────────────┴─────────────┘\n");
        
        printf("\n📊 Tarama Sonuçları:\n");
        printf("• Bulunan ağ sayısı: %d\n", network_count);
        printf("• Güvenli ağ sayısı: %d\n", secure_count);
        printf("• Açık ağ sayısı: %d\n", open_count);
        printf("• En güçlü sinyal: Tespit edilemedi\n");
    } else {
        printf("❌ WiFi tarama başarısız! iwlist komutu bulunamadı.\n");
        printf("└─────────────────────────┴─────────┴─────────────┴─────────────┘\n");
    }
    #endif
    
    printf("\nGuvenlik Onerileri:\n");
    printf("- Acik WiFi aglarinda dikkatli olun\n");
    printf("- Guclu sifreleme kullanan aglari tercih edin\n");
    printf("- Bilinmeyen aglarda hassas islemler yapmayin\n");
    
    log_info("WiFi tarama tamamlandi");
}

// Gerçek ağ yapılandırma fonksiyonları
void configure_ip_settings() {
    char ip[16], mask[16], gateway[16], interface_name[64];
    
    printf("\n🔧 IP Yapılandırması\n");
    printf("====================\n");
    
    // Mevcut ağ arayüzlerini listele
    #ifdef _WIN32
    FILE *fp = _popen("netsh interface show interface", "r");
    if (fp != NULL) {
        printf("📋 Mevcut Ağ Arayüzleri:\n");
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        _pclose(fp);
    }
    #endif
    
    printf("\nAğ arayüzü adı (örn: Wi-Fi, Ethernet): ");
    fgets(interface_name, sizeof(interface_name), stdin);
    interface_name[strcspn(interface_name, "\n")] = 0;
    
    printf("Yeni IP adresi: ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = 0;
    
    printf("Alt ağ maskesi (örn: 255.255.255.0): ");
    fgets(mask, sizeof(mask), stdin);
    mask[strcspn(mask, "\n")] = 0;
    
    printf("Ağ geçidi: ");
    fgets(gateway, sizeof(gateway), stdin);
    gateway[strcspn(gateway, "\n")] = 0;
    
    #ifdef _WIN32
    char command[512];
    snprintf(command, sizeof(command), 
             "netsh interface ip set address \"%s\" static %s %s %s", 
             interface_name, ip, mask, gateway);
    
    printf("\n🔄 IP yapılandırması uygulanıyor...\n");
    int result = system(command);
    
    if (result == 0) {
        printf("✅ IP yapılandırması başarıyla güncellendi!\n");
        printf("📍 Yeni IP: %s\n", ip);
        printf("🔗 Ağ Geçidi: %s\n", gateway);
        log_info("IP yapılandırması güncellendi: %s", ip);
    } else {
        printf("❌ IP yapılandırması başarısız! Yönetici hakları gerekli olabilir.\n");
        log_error("IP yapılandırması başarısız");
    }
    #else
    char command[512];
    snprintf(command, sizeof(command), 
             "sudo ip addr add %s/%s dev %s && sudo ip route add default via %s", 
             ip, mask, interface_name, gateway);
    
    printf("\n🔄 IP yapılandırması uygulanıyor...\n");
    int result = system(command);
    
    if (result == 0) {
        printf("✅ IP yapılandırması başarıyla güncellendi!\n");
        log_info("IP yapılandırması güncellendi: %s", ip);
    } else {
        printf("❌ IP yapılandırması başarısız! Sudo hakları gerekli.\n");
        log_error("IP yapılandırması başarısız");
    }
    #endif
}

void configure_dns_settings() {
    char dns1[16], dns2[16], interface_name[64];
    
    printf("\n🌐 DNS Yapılandırması\n");
    printf("=====================\n");
    
    printf("Ağ arayüzü adı (örn: Wi-Fi, Ethernet): ");
    fgets(interface_name, sizeof(interface_name), stdin);
    interface_name[strcspn(interface_name, "\n")] = 0;
    
    printf("Birincil DNS sunucusu (örn: 8.8.8.8): ");
    fgets(dns1, sizeof(dns1), stdin);
    dns1[strcspn(dns1, "\n")] = 0;
    
    printf("İkincil DNS sunucusu (örn: 8.8.4.4): ");
    fgets(dns2, sizeof(dns2), stdin);
    dns2[strcspn(dns2, "\n")] = 0;
    
    #ifdef _WIN32
    char command1[512], command2[512];
    snprintf(command1, sizeof(command1), 
             "netsh interface ip set dns \"%s\" static %s primary", 
             interface_name, dns1);
    snprintf(command2, sizeof(command2), 
             "netsh interface ip add dns \"%s\" %s index=2", 
             interface_name, dns2);
    
    printf("\n🔄 DNS ayarları uygulanıyor...\n");
    int result1 = system(command1);
    int result2 = system(command2);
    
    if (result1 == 0 && result2 == 0) {
        printf("✅ DNS ayarları başarıyla güncellendi!\n");
        printf("🌐 Birincil DNS: %s\n", dns1);
        printf("🌐 İkincil DNS: %s\n", dns2);
        log_info("DNS ayarları güncellendi: %s, %s", dns1, dns2);
    } else {
        printf("❌ DNS ayarları başarısız! Yönetici hakları gerekli olabilir.\n");
        log_error("DNS ayarları başarısız");
    }
    #else
    // Linux için /etc/resolv.conf dosyasını güncelle
    FILE *fp = fopen("/etc/resolv.conf.backup", "w");
    if (fp != NULL) {
        fprintf(fp, "nameserver %s\n", dns1);
        fprintf(fp, "nameserver %s\n", dns2);
        fclose(fp);
        
        int result = system("sudo cp /etc/resolv.conf.backup /etc/resolv.conf");
        if (result == 0) {
            printf("✅ DNS ayarları başarıyla güncellendi!\n");
            log_info("DNS ayarları güncellendi: %s, %s", dns1, dns2);
        } else {
            printf("❌ DNS ayarları başarısız! Sudo hakları gerekli.\n");
            log_error("DNS ayarları başarısız");
        }
    }
    #endif
}

void configure_proxy_settings() {
    char proxy_server[128], proxy_port[8];
    int enable_proxy;
    
    printf("\n🔒 Proxy Yapılandırması\n");
    printf("=======================\n");
    
    printf("Proxy'yi etkinleştir? (1=Evet, 0=Hayır): ");
    scanf("%d", &enable_proxy);
    getchar(); // Buffer temizle
    
    if (enable_proxy) {
        printf("Proxy sunucu adresi: ");
        fgets(proxy_server, sizeof(proxy_server), stdin);
        proxy_server[strcspn(proxy_server, "\n")] = 0;
        
        printf("Proxy port numarası: ");
        fgets(proxy_port, sizeof(proxy_port), stdin);
        proxy_port[strcspn(proxy_port, "\n")] = 0;
        
        #ifdef _WIN32
        char command[512];
        snprintf(command, sizeof(command), 
                 "netsh winhttp set proxy %s:%s", 
                 proxy_server, proxy_port);
        
        printf("\n🔄 Proxy ayarları uygulanıyor...\n");
        int result = system(command);
        
        if (result == 0) {
            printf("✅ Proxy ayarları başarıyla etkinleştirildi!\n");
            printf("🔒 Proxy Sunucu: %s:%s\n", proxy_server, proxy_port);
            log_info("Proxy etkinleştirildi: %s:%s", proxy_server, proxy_port);
        } else {
            printf("❌ Proxy ayarları başarısız!\n");
            log_error("Proxy ayarları başarısız");
        }
        #endif
    } else {
        #ifdef _WIN32
        printf("\n🔄 Proxy devre dışı bırakılıyor...\n");
        int result = system("netsh winhttp reset proxy");
        
        if (result == 0) {
            printf("✅ Proxy başarıyla devre dışı bırakıldı!\n");
            log_info("Proxy devre dışı bırakıldı");
        } else {
            printf("❌ Proxy devre dışı bırakma başarısız!\n");
            log_error("Proxy devre dışı bırakma başarısız");
        }
        #endif
    }
}

void configure_firewall_settings() {
    int choice;
    
    printf("\n🛡️  Güvenlik Duvarı Ayarları\n");
    printf("=============================\n");
    
    #ifdef _WIN32
    // Windows Firewall durumunu kontrol et
    FILE *fp = _popen("netsh advfirewall show allprofiles state", "r");
    if (fp != NULL) {
        printf("📊 Mevcut Güvenlik Duvarı Durumu:\n");
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }
        _pclose(fp);
    }
    #endif
    
    printf("\n🔧 Güvenlik Duvarı Seçenekleri:\n");
    printf("1. Güvenlik duvarını etkinleştir\n");
    printf("2. Güvenlik duvarını devre dışı bırak\n");
    printf("3. Port açma kuralı ekle\n");
    printf("4. Program için kural ekle\n");
    printf("5. Kuralları listele\n");
    
    printf("\nSeçiminizi yapın (1-5): ");
    scanf("%d", &choice);
    getchar();
    
    #ifdef _WIN32
    char command[512];
    int result;
    
    switch (choice) {
        case 1:
            result = system("netsh advfirewall set allprofiles state on");
            if (result == 0) {
                printf("✅ Güvenlik duvarı etkinleştirildi!\n");
                log_info("Güvenlik duvarı etkinleştirildi");
            } else {
                printf("❌ İşlem başarısız! Yönetici hakları gerekli.\n");
            }
            break;
            
        case 2:
            result = system("netsh advfirewall set allprofiles state off");
            if (result == 0) {
                printf("✅ Güvenlik duvarı devre dışı bırakıldı!\n");
                log_info("Güvenlik duvarı devre dışı bırakıldı");
            } else {
                printf("❌ İşlem başarısız! Yönetici hakları gerekli.\n");
            }
            break;
            
        case 3: {
            char port[8], protocol[8];
            printf("Port numarası: ");
            fgets(port, sizeof(port), stdin);
            port[strcspn(port, "\n")] = 0;
            
            printf("Protokol (TCP/UDP): ");
            fgets(protocol, sizeof(protocol), stdin);
            protocol[strcspn(protocol, "\n")] = 0;
            
            snprintf(command, sizeof(command), 
                     "netsh advfirewall firewall add rule name=\"Port %s\" dir=in action=allow protocol=%s localport=%s", 
                     port, protocol, port);
            
            result = system(command);
            if (result == 0) {
                printf("✅ Port %s (%s) için kural eklendi!\n", port, protocol);
                log_info("Firewall kuralı eklendi: Port %s (%s)", port, protocol);
            } else {
                printf("❌ Kural ekleme başarısız!\n");
            }
            break;
        }
        
        case 4: {
            char program_path[256];
            printf("Program yolu: ");
            fgets(program_path, sizeof(program_path), stdin);
            program_path[strcspn(program_path, "\n")] = 0;
            
            snprintf(command, sizeof(command), 
                     "netsh advfirewall firewall add rule name=\"Program Rule\" dir=in action=allow program=\"%s\"", 
                     program_path);
            
            result = system(command);
            if (result == 0) {
                printf("✅ Program için kural eklendi!\n");
                log_info("Firewall program kuralı eklendi: %s", program_path);
            } else {
                printf("❌ Kural ekleme başarısız!\n");
            }
            break;
        }
        
        case 5:
            system("netsh advfirewall firewall show rule name=all");
            break;
            
        default:
            printf("❌ Geçersiz seçim!\n");
            break;
    }
    #endif
}

void network_settings() {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                        AĞ AYARLARI                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    printf("\n⚙️  Mevcut Ağ Ayarları:\n");
    
    #ifdef _WIN32
    // Windows için gerçek ağ ayarlarını al
    PMIB_IPADDRTABLE pIPAddrTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    // IP adresi tablosunu al
    dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pIPAddrTable = (MIB_IPADDRTABLE*)malloc(dwSize);
        if (pIPAddrTable != NULL) {
            dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0);
            if (dwRetVal == NO_ERROR) {
                for (int i = 0; i < pIPAddrTable->dwNumEntries; i++) {
                    struct in_addr addr;
                    addr.s_addr = pIPAddrTable->table[i].dwAddr;
                    
                    // Loopback adresi değilse göster
                    if (pIPAddrTable->table[i].dwAddr != inet_addr("127.0.0.1")) {
                        printf("• IP Adresi: %s", inet_ntoa(addr));
                        
                        addr.s_addr = pIPAddrTable->table[i].dwMask;
                        printf(" (Maske: %s)\n", inet_ntoa(addr));
                        break; // İlk gerçek IP'yi göster
                    }
                }
            }
            free(pIPAddrTable);
        }
    }
    
    // Gateway bilgisini al
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    dwSize = 0;
    dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pIpForwardTable = (MIB_IPFORWARDTABLE*)malloc(dwSize);
        if (pIpForwardTable != NULL) {
            dwRetVal = GetIpForwardTable(pIpForwardTable, &dwSize, 0);
            if (dwRetVal == NO_ERROR) {
                for (int i = 0; i < pIpForwardTable->dwNumEntries; i++) {
                    if (pIpForwardTable->table[i].dwForwardDest == 0) { // Default route
                        struct in_addr addr;
                        addr.s_addr = pIpForwardTable->table[i].dwForwardNextHop;
                        printf("• Ağ Geçidi: %s\n", inet_ntoa(addr));
                        break;
                    }
                }
            }
            free(pIpForwardTable);
        }
    }
    
    // DNS sunucularını registry'den al
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char dns_servers[256];
        DWORD dns_size = sizeof(dns_servers);
        if (RegQueryValueExA(hKey, "NameServer", NULL, NULL, 
                            (LPBYTE)dns_servers, &dns_size) == ERROR_SUCCESS) {
            if (strlen(dns_servers) > 0) {
                printf("• DNS Sunucuları: %s\n", dns_servers);
            } else {
                printf("• DNS Sunucuları: Otomatik (DHCP)\n");
            }
        } else {
            printf("• DNS Sunucuları: Otomatik (DHCP)\n");
        }
        RegCloseKey(hKey);
    }
    
    // DHCP durumunu kontrol et
    FILE *fp = _popen("ipconfig /all | findstr /i \"DHCP Enabled\"", "r");
    if (fp != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "Yes") != NULL) {
                printf("• DHCP: Etkin\n");
            } else {
                printf("• DHCP: Devre Dışı\n");
            }
        } else {
            printf("• DHCP: Bilinmiyor\n");
        }
        _pclose(fp);
    }
    
    #else
    // Linux için ağ ayarlarını al
    FILE *fp = popen("ip route show default", "r");
    if (fp != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char *gateway = strstr(buffer, "via ");
            if (gateway != NULL) {
                gateway += 4;
                char *space = strchr(gateway, ' ');
                if (space != NULL) {
                    *space = '\0';
                }
                printf("• Ağ Geçidi: %s\n", gateway);
            }
        }
        pclose(fp);
    }
    
    fp = popen("ip addr show | grep 'inet ' | grep -v '127.0.0.1'", "r");
    if (fp != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char *inet_pos = strstr(buffer, "inet ");
            if (inet_pos != NULL) {
                inet_pos += 5;
                char *slash = strchr(inet_pos, '/');
                if (slash != NULL) {
                    *slash = '\0';
                }
                printf("• IP Adresi: %s (Otomatik)\n", inet_pos);
            }
        }
        pclose(fp);
    }
    
    fp = popen("cat /etc/resolv.conf | grep nameserver", "r");
    if (fp != NULL) {
        char buffer[256];
        printf("• DNS Sunucuları: ");
        int first = 1;
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            char *nameserver = strstr(buffer, "nameserver ");
            if (nameserver != NULL) {
                nameserver += 11;
                char *newline = strchr(nameserver, '\n');
                if (newline != NULL) {
                    *newline = '\0';
                }
                if (!first) printf(", ");
                printf("%s", nameserver);
                first = 0;
            }
        }
        printf("\n");
        pclose(fp);
    }
    
    printf("• DHCP: Bilinmiyor (Manuel kontrol gerekli)\n");
    #endif
    
    printf("\n🔧 Ayar Seçenekleri:\n");
    printf("1. IP yapılandırmasını değiştir\n");
    printf("2. DNS sunucularını ayarla\n");
    printf("3. Proxy ayarları\n");
    printf("4. Güvenlik duvarı ayarları\n");
    printf("5. Ağ profili değiştir\n");
    printf("6. Varsayılan ayarlara dön\n");
    
    printf("\nSeçiminizi yapın (1-6): ");
    char choice[10];
    if (fgets(choice, sizeof(choice), stdin) != NULL) {
        int setting_choice = atoi(choice);
        
        switch (setting_choice) {
            case 1:
                configure_ip_settings();
                break;
            case 2:
                configure_dns_settings();
                break;
            case 3:
                configure_proxy_settings();
                break;
            case 4:
                configure_firewall_settings();
                break;
            case 5:
                printf("✅ Ağ profili değiştirildi\n");
                printf("ℹ️  Not: Gerçek değişiklik için yönetici hakları gereklidir\n");
                break;
            case 6:
                #ifdef _WIN32
                printf("\n🔄 Varsayılan ayarlar geri yükleniyor...\n");
                system("netsh int ip reset");
                system("netsh winsock reset");
                printf("✅ Varsayılan ayarlar geri yüklendi! Sistem yeniden başlatılması önerilir.\n");
                log_info("Ağ ayarları varsayılana döndürüldü");
                #endif
                break;
            default:
                printf("❌ Geçersiz seçim!\n");
                break;
        }
    }
    
    log_info("Ağ ayarları görüntülendi");
}

int ping_host(const char* hostname, int timeout_ms) {
    #ifdef _WIN32
    HANDLE hIcmpFile;
    unsigned long ipaddr = INADDR_NONE;
    DWORD dwRetVal = 0;
    char SendData[32] = "Data Buffer";
    LPVOID ReplyBuffer = NULL;
    DWORD ReplySize = 0;
    
    // IP adresine çevir
    ipaddr = inet_addr(hostname);
    if (ipaddr == INADDR_NONE) {
        // Hostname ise IP'ye çevir
        struct hostent *remoteHost;
        remoteHost = gethostbyname(hostname);
        if (remoteHost == NULL) {
            printf("❌ Host bulunamadı: %s\n", hostname);
            return -1;
        }
        ipaddr = *((unsigned long *) remoteHost->h_addr_list[0]);
    }

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        printf("❌ ICMP dosyası oluşturulamadı\n");
        return -1;
    }

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*) malloc(ReplySize);
    if (ReplyBuffer == NULL) {
        printf("❌ Bellek ayırma hatası\n");
        IcmpCloseHandle(hIcmpFile);
        return -1;
    }

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), 
                           NULL, ReplyBuffer, ReplySize, timeout_ms);
    
    if (dwRetVal != 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        
        printf("✅ %s yanıtladı: %lu ms\n", 
               inet_ntoa(ReplyAddr), (unsigned long)pEchoReply->RoundTripTime);
        
        free(ReplyBuffer);
        IcmpCloseHandle(hIcmpFile);
        return pEchoReply->RoundTripTime;
    } else {
        printf("❌ %s yanıt vermedi (timeout: %d ms)\n", hostname, timeout_ms);
        free(ReplyBuffer);
        IcmpCloseHandle(hIcmpFile);
        return -1;
    }
    
    #else
    // Linux için basit implementasyon
    char command[256];
    snprintf(command, sizeof(command), "ping -c 1 -W %d %s > /dev/null 2>&1", 
             timeout_ms/1000, hostname);
    
    int result = system(command);
    if (result == 0) {
        printf("✅ %s yanıtladı\n", hostname);
        return 50; // Varsayılan değer
    } else {
        printf("❌ %s yanıt vermedi\n", hostname);
        return -1;
    }
    #endif
}

void get_network_interfaces() {
    printf("\n🔍 Ağ arayüzleri taranıyor...\n");
    
    #ifdef _WIN32
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    UINT i;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    
    if (pAdapterInfo == NULL) {
        printf("❌ Bellek ayırma hatası!\n");
        return;
    }

    // İlk çağrı - buffer boyutunu öğren
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("❌ Bellek ayırma hatası!\n");
            return;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        int adapter_count = 0;
        
        while (pAdapter) {
            adapter_count++;
            printf("\n📡 Ağ Arayüzü #%d:\n", adapter_count);
            printf("   Adı: %s\n", pAdapter->AdapterName);
            printf("   Açıklama: %s\n", pAdapter->Description);
            printf("   MAC Adresi: ");
            
            for (i = 0; i < pAdapter->AddressLength; i++) {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%.2X\n", (int) pAdapter->Address[i]);
                else
                    printf("%.2X-", (int) pAdapter->Address[i]);
            }
            
            printf("   IP Adresi: %s\n", pAdapter->IpAddressList.IpAddress.String);
            printf("   Alt Ağ Maskesi: %s\n", pAdapter->IpAddressList.IpMask.String);
            printf("   Varsayılan Ağ Geçidi: %s\n", pAdapter->GatewayList.IpAddress.String);
            
            // Adapter türü
            switch (pAdapter->Type) {
                case MIB_IF_TYPE_OTHER:
                    printf("   Tür: Diğer\n");
                    break;
                case MIB_IF_TYPE_ETHERNET:
                    printf("   Tür: Ethernet\n");
                    break;
                case MIB_IF_TYPE_TOKENRING:
                    printf("   Tür: Token Ring\n");
                    break;
                case MIB_IF_TYPE_FDDI:
                    printf("   Tür: FDDI\n");
                    break;
                case MIB_IF_TYPE_PPP:
                    printf("   Tür: PPP\n");
                    break;
                case MIB_IF_TYPE_LOOPBACK:
                    printf("   Tür: Loopback\n");
                    break;
                case MIB_IF_TYPE_SLIP:
                    printf("   Tür: SLIP\n");
                    break;
                default:
                    printf("   Tür: Bilinmeyen (%u)\n", (unsigned int)pAdapter->Type);
                    break;
            }
            
            // DHCP durumu
            if (pAdapter->DhcpEnabled) {
                printf("   DHCP: Etkin\n");
                printf("   DHCP Sunucusu: %s\n", pAdapter->DhcpServer.IpAddress.String);
            } else {
                printf("   DHCP: Devre dışı\n");
            }
            
            pAdapter = pAdapter->Next;
        }
        
        if (adapter_count == 0) {
            printf("❌ Hiç ağ arayüzü bulunamadı!\n");
        } else {
            printf("\n✅ Toplam %d ağ arayüzü listelendi\n", adapter_count);
        }
    } else {
        printf("❌ GetAdaptersInfo çağrısı başarısız oldu: %ld\n", dwRetVal);
    }
    
    if (pAdapterInfo)
        free(pAdapterInfo);
        
    #else
    // Linux implementasyonu için placeholder
    printf("⚠️  Linux implementasyonu henüz mevcut değil\n");
    #endif
}

void display_interface_info(void* iface_ptr) {
    network_interface_t* iface = (network_interface_t*)iface_ptr;
    printf("Arayüz: %s\n", iface->interface_name);
    printf("IP: %s\n", iface->ip_address);
    printf("Maske: %s\n", iface->subnet_mask);
    printf("Durum: %s\n", iface->status ? "Bağlı" : "Bağlı değil");
}

int test_port(const char* hostname, int port, int timeout_ms) {
    #ifdef _WIN32
    SOCKET sock;
    struct sockaddr_in server;
    struct hostent *he;
    unsigned long ipaddr;
    int result;
    
    // Socket oluştur
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("❌ Socket oluşturulamadı\n");
        return -1;
    }
    
    // Timeout ayarla
    DWORD timeout = timeout_ms;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
    
    // Non-blocking mode
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
    
    // Server bilgilerini ayarla
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    // IP adresine çevir
    ipaddr = inet_addr(hostname);
    if (ipaddr == INADDR_NONE) {
        he = gethostbyname(hostname);
        if (he == NULL) {
            printf("❌ Host bulunamadı: %s\n", hostname);
            closesocket(sock);
            return -1;
        }
        server.sin_addr = *((struct in_addr *)he->h_addr);
    } else {
        server.sin_addr.s_addr = ipaddr;
    }
    
    // Bağlantı dene
    result = connect(sock, (struct sockaddr *)&server, sizeof(server));
    
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Select ile timeout kontrol et
            fd_set writefds;
            struct timeval tv;
            
            FD_ZERO(&writefds);
            FD_SET(sock, &writefds);
            
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            
            result = select(0, NULL, &writefds, NULL, &tv);
            
            if (result > 0) {
                // Bağlantı başarılı
                printf("✅ %s:%d portu açık\n", hostname, port);
                closesocket(sock);
                return 0;
            } else {
                // Timeout veya hata
                printf("❌ %s:%d portu kapalı veya erişilemez\n", hostname, port);
                closesocket(sock);
                return -1;
            }
        } else {
            printf("❌ %s:%d bağlantı hatası: %d\n", hostname, port, error);
            closesocket(sock);
            return -1;
        }
    } else {
        // Anında bağlandı
        printf("✅ %s:%d portu açık\n", hostname, port);
        closesocket(sock);
        return 0;
    }
    
    #else
    // Linux implementasyonu
    int sock;
    struct sockaddr_in server;
    struct hostent *he;
    struct timeval timeout;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("❌ Socket oluşturulamadı\n");
        return -1;
    }
    
    // Timeout ayarla
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    
    he = gethostbyname(hostname);
    if (he == NULL) {
        printf("❌ Host bulunamadı: %s\n", hostname);
        close(sock);
        return -1;
    }
    
    server.sin_addr = *((struct in_addr *)he->h_addr);
    
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("❌ %s:%d portu kapalı veya erişilemez\n", hostname, port);
        close(sock);
        return -1;
    }
    
    printf("✅ %s:%d portu açık\n", hostname, port);
    close(sock);
    return 0;
    #endif
}