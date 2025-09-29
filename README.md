# Sistem Otomasyon Merkezi

Sistem Otomasyon Merkezi, Windows işletim sistemleri için geliştirilmiş kapsamlı bir sistem yönetimi ve otomasyon aracıdır. Bu uygulama, sistem yöneticilerinin ve ileri düzey kullanıcıların günlük sistem yönetimi görevlerini otomatikleştirmelerine ve sistem performansını izlemelerine yardımcı olur.

## 🚀 Özellikler

### 📁 Dosya Yönetimi ve Organizasyon
- Gelişmiş dosya arama ve filtreleme
- Toplu dosya yeniden adlandırma işlemleri
- Dosya boyutu analizi ve raporlama
- Boş klasör temizleme
- Dosya türüne göre otomatik organizasyon

### 🖥️ Sistem İzleme ve Performans
- Gerçek zamanlı sistem performansı izleme
- CPU, bellek ve disk kullanım analizi
- Sistem kaynaklarının detaylı raporlanması
- Performans geçmişi takibi

### 📊 Log Analizi ve Raporlama
- Sistem loglarının otomatik analizi
- Hata ve uyarı tespiti
- Log dosyalarının filtrelenmesi ve aranması
- Detaylı log raporları oluşturma

### 💾 Otomatik Yedekleme Sistemi
- Zamanlanmış yedekleme işlemleri
- Artımlı ve tam yedekleme seçenekleri
- Yedekleme durumu izleme
- Otomatik yedekleme doğrulama

### 🌐 Ağ İzleme ve Bağlantı Testleri
- Ağ bağlantısı durumu kontrolü
- Ping ve bağlantı testleri
- Ağ performansı analizi
- Bağlantı geçmişi raporlama

### 🔒 Güvenlik Tarama ve Kontrol
- Sistem güvenlik açıklarının taranması
- Güvenlik politikalarının kontrolü
- Şüpheli aktivite tespiti
- Güvenlik raporları oluşturma

### ⏰ Görev Zamanlayıcı
- Otomatik görev zamanlama
- Tekrarlayan görevler
- Görev durumu izleme
- Esnek zamanlama seçenekleri

### ⚙️ Sistem Ayarları ve Konfigürasyon
- Sistem ayarlarının merkezi yönetimi
- Konfigürasyon dosyası yönetimi
- Sistem parametrelerinin optimizasyonu
- Ayar yedekleme ve geri yükleme

### 📈 Raporlama Sistemi
- Kapsamlı sistem raporları
- HTML formatında detaylı raporlar
- PDF dışa aktarma desteği (wkhtmltopdf ile)
- Özelleştirilebilir rapor şablonları

## 🛠️ Kurulum

### Gereksinimler
- Windows 7 veya üzeri
- MinGW-w64 veya Visual Studio (derleme için)
- Git (kaynak kodunu indirmek için)

### Derleme
```bash
git clone https://github.com/kullanici/sistem-otomasyon-merkezi.git
cd sistem-otomasyon-merkezi
make
```

### Çalıştırma
```bash
./automation_center.exe
```

## 📋 Kullanım

Uygulama başlatıldığında ana menü görüntülenir. Menüden istediğiniz modülü seçerek ilgili işlemleri gerçekleştirebilirsiniz.

### Ana Menü Seçenekleri:
1. **Dosya Yönetimi** - Dosya işlemleri ve organizasyon
2. **Sistem İzleme** - Performans izleme ve analiz
3. **Log Analizi** - Log dosyalarının incelenmesi
4. **Yedekleme Sistemi** - Otomatik yedekleme işlemleri
5. **Ağ İzleme** - Ağ bağlantısı testleri
6. **Güvenlik Tarama** - Sistem güvenlik kontrolü
7. **Görev Zamanlayıcı** - Otomatik görev yönetimi
8. **Sistem Ayarları** - Konfigürasyon yönetimi
9. **Raporlar** - Sistem raporlarını görüntüleme

## 📁 Proje Yapısı

```
sistem-otomasyon-merkezi/
├── src/
│   ├── modules/          # Modül kaynak dosyaları
│   ├── utils/           # Yardımcı fonksiyonlar
│   └── main.c           # Ana program
├── include/             # Header dosyaları
├── lib/                 # Harici kütüphaneler
├── config/              # Konfigürasyon dosyaları
├── logs/                # Log dosyaları
├── reports/             # Oluşturulan raporlar
├── Makefile            # Derleme dosyası
└── README.md           # Bu dosya
```

## 🔧 Konfigürasyon

Uygulama `config/automation_config.ini` dosyası üzerinden yapılandırılabilir. Bu dosyada:
- Log seviyesi ayarları
- Yedekleme dizinleri
- Rapor şablonları
- Sistem izleme parametreleri

gibi ayarlar bulunmaktadır.

## 📊 Raporlama

Sistem otomatik olarak HTML formatında raporlar oluşturur. Bu raporlar `reports/` klasöründe saklanır ve şunları içerir:
- Sistem performans verileri
- Güvenlik tarama sonuçları
- Yedekleme durumu
- Ağ bağlantısı analizi

PDF dışa aktarma için `wkhtmltopdf` aracının sisteminizde kurulu olması gerekmektedir.

## 🐛 Sorun Giderme

### Yaygın Sorunlar:
- **Derleme hataları**: MinGW-w64'ün doğru kurulduğundan emin olun
- **İzin hataları**: Uygulamayı yönetici olarak çalıştırın
- **Log dosyası oluşturulamıyor**: `logs/` klasörünün yazma izinlerini kontrol edin

### Log Dosyaları:
Uygulama çalışma sırasında `logs/automation.log` dosyasına detaylı bilgi kaydeder. Sorun yaşadığınızda bu dosyayı inceleyebilirsiniz.

## 🤝 Katkıda Bulunma

Bu proje açık kaynak kodludur ve katkılarınızı memnuniyetle karşılarız. Katkıda bulunmak için:

1. Projeyi fork edin
2. Yeni bir branch oluşturun (`git checkout -b yeni-ozellik`)
3. Değişikliklerinizi commit edin (`git commit -am 'Yeni özellik eklendi'`)
4. Branch'inizi push edin (`git push origin yeni-ozellik`)
5. Pull Request oluşturun

## 📄 Lisans

Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için `LICENSE` dosyasına bakınız.

## 📞 İletişim

Sorularınız veya önerileriniz için:
- GitHub Issues üzerinden bildirim yapabilirsiniz
- E-posta: [email@example.com]

## 🙏 Teşekkürler

Bu projenin geliştirilmesinde katkıda bulunan herkese teşekkür ederiz.

---

**Not**: Bu uygulama sistem düzeyinde işlemler gerçekleştirdiği için yönetici yetkileriyle çalıştırılması önerilir.