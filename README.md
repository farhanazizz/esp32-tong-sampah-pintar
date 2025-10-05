# 🚀 ESP32 + HC-SR04 + Servo — Smart Smooth Motion System

Project ini bikin ESP32 kamu jadi **sensorik cerdas** yang bisa **mendeteksi jarak objek pakai HC-SR04** dan **menggerakkan servo secara halus dan responsif**.
Gampangnya, kalau ada objek mendekat, servo **langsung buka cepat**, dan kalau objek menjauh, dia **nunggu 2 detik dulu baru nutup pelan-pelan**.
Perfect banget buat proyek kayak *smart trash bin*, *smart gate*, atau sistem otomatis lain yang butuh gerakan lembut tapi reaktif.

---

## 🧠 Cara Kerja Singkat

* Kalau jarak **≤ 30 cm** → servo **langsung buka cepat**.
* Kalau jarak **≥ 60 cm** → sistem **nunggu 2 detik**, pastikan objek benar-benar menjauh, lalu **servo nutup pelan (smooth)**.
* Kalau jarak di antara 30–60 cm → servo **diam aja di posisi terakhir** biar nggak goyang-goyang.
* Semua ini berjalan **tanpa delay blocking**, pakai sistem waktu `millis()` dan **filter EMA** supaya bacaan jarak lebih stabil.

---

## 🌟 Fitur Unggulan

✅ **Histeresis pintar (30 / 60 cm)** – mencegah servo flip-flop karena noise sensor
✅ **Delay 2 detik sebelum menutup** – biar nggak panik kalau objek cuma lewat sebentar
✅ **Gerakan halus dan cepat** – buka cepat, tutup smooth
✅ **Non-blocking loop** – bisa digabung sama task IoT lain (misal MQTT atau Wi-Fi)
✅ **Filter EMA** – data jarak lebih stabil, anti “loncat-loncat”
✅ **Timeout echo** – aman kalau sensor nggak dapet pantulan

---

## 🧩 Wiring (Hubungan Pin)

| Komponen     | Pin ESP32    | Keterangan                                                            |
| ------------ | ------------ | --------------------------------------------------------------------- |
| HC-SR04 VCC  | 5V           | Sumber daya sensor (butuh 5V biar akurat)                             |
| HC-SR04 GND  | GND          | Ground bersama                                                        |
| HC-SR04 TRIG | GPIO 5       | Pin pemicu sinyal ultrasonik                                          |
| HC-SR04 ECHO | GPIO 18      | **Gunakan resistor divider** ke 3.3V (misal 10kΩ ke pin, 20kΩ ke GND) |
| Servo Signal | GPIO 14      | Jalur kontrol servo                                                   |
| Servo VCC    | 5V eksternal | Gunakan power terpisah (misal dari step-down LM2596)                  |
| Servo GND    | GND          | **WAJIB** common ground dengan ESP32                                  |

> ⚠️ **PENTING:** Pin Echo HC-SR04 output-nya 5V, sedangkan ESP32 cuma tahan 3.3V.
> Jadi jangan langsung colok — pasang pembagi tegangan biar aman.

---

## ⚙️ Catatan Hardware

* Servo **nggak boleh ambil power dari pin 5V ESP32** langsung, karena arusnya kecil banget.
  Gunakan **step-down converter (LM2596)** atau adaptor 5V 2A.
* Pastikan **semua GND** (ESP32, sensor, servo) **terhubung jadi satu**.
* Kamu bisa ubah sudut buka/tutup di kode:

  ```cpp
  const int ANGLE_CLOSED = 0;
  const int ANGLE_OPEN   = 90;
  ```
* Kalau mekanikmu kebalik (servo buka saat harusnya tutup), tinggal tukar nilai di atas.

---

## 🧠 Rekomendasi Supply Power

* **Sensor HC-SR04:** 5V dari board ESP32 aman.
* **Servo MG90S / SG90:** 5V terpisah (regulator atau baterai 18650 + step-down).
* Jangan lupa pasang **kapasitor 470µF–1000µF** di jalur servo biar nggak drop saat start-up.

---

## 🔧 Config Arduino IDE

1. **Board:** ESP32 / ESP32-S3 (tergantung modul kamu)
2. **Port:** Pilih sesuai perangkat
3. **Library:** Install *ESP32Servo* via Library Manager
4. **Baudrate Serial Monitor:** 115200

---

## 🧩 Penggunaan

1. Upload kode ke ESP32.
2. Buka Serial Monitor → lihat jarak real-time dan status servo.
3. Dekatkan tangan (atau objek lain) ke sensor, servo akan **buka cepat**.
4. Jauhkan tangan, tunggu 2 detik → servo akan **nutup pelan-pelan**.

---

## 🧱 Aplikasi Ide

* 🗑️ **Tong sampah pintar**: buka otomatis pas tangan mendekat.
* 🚪 **Gerbang mini**: deteksi mobil / orang lewat.
* 🐾 **Feeder hewan otomatis**: buka wadah makanan saat objek terdeteksi.

---
