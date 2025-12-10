# 🚪 Smart Door System with IoT & Fire Safety (FreeRTOS based)

![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue?style=for-the-badge&logo=espressif)
![Framework](https://img.shields.io/badge/Framework-Arduino-green?style=for-the-badge&logo=arduino)
![OS](https://img.shields.io/badge/OS-FreeRTOS-orange?style=for-the-badge)
![IoT](https://img.shields.io/badge/IoT-CoreIoT%20(ThingsBoard)-red?style=for-the-badge)

> **Đồ án Môn học:** Hệ thống nhúng & IoT  
> **Giảng viên hướng dẫn:** Thầy Lê Trọng Nhân  
> **Kit phát triển:** Yolo Uno (ESP32-S3)

---

## 📖 Giới thiệu (Introduction)

**Smart Door System** là hệ thống kiểm soát ra vào thông minh được thiết kế nhằm giải quyết các vấn đề về an ninh và an toàn trong mô hình Smart Home hiện đại. Dự án tích hợp đa phương thức xác thực (RFID, Keypad, Remote), giám sát môi trường thời gian thực và đặc biệt là cơ chế **Fire Escape** (Tự động mở cửa khi có cháy).

Hệ thống được xây dựng trên kiến trúc đa nhiệm (Multi-tasking) sử dụng hệ điều hành thời gian thực **FreeRTOS**, đảm bảo khả năng phản hồi tức thì và hoạt động ổn định.

---

## ✨ Tính năng nổi bật (Key Features)

* 🔐 **Đa phương thức xác thực (Multi-factor Auth):**
    * Quẹt thẻ từ RFID (RC522) với Database định danh người dùng.
    * Mật khẩu số qua Keypad 4x4 (có cơ chế chống dò mật khẩu).
    * Điều khiển từ xa qua Web/App (CoreIoT Dashboard).
* 🔥 **Cơ chế an toàn (Fire Safety Mode):**
    * Giám sát nhiệt độ liên tục.
    * **Tự động mở cửa** và cảnh báo khi nhiệt độ vượt ngưỡng (>30°C) để hỗ trợ thoát hiểm.
* 🌐 **IoT & Traceability (Truy xuất nguồn gốc):**
    * Giám sát nhiệt độ/độ ẩm từ xa qua Dashboard.
    * Ghi lại lịch sử truy cập chi tiết: Biết chính xác **AI** đã mở cửa (VD: *"MỞ: Thẻ Nguyễn Văn A"*, *"MỞ: Remote App"*).
* ⚡ **Hệ điều hành FreeRTOS:**
    * Quản lý 7 tác vụ (Tasks) song song.
    * Sử dụng **Mutex** để bảo vệ dữ liệu và **Semaphore** để đồng bộ hóa sự kiện (Event-driven).

---



## ⚙️ Cấu hình Phần cứng (Pin Mapping)

Dựa trên thiết kế mạch cho Kit **Yolo Uno (ESP32-S3)**, các chân GPIO được quy hoạch tối ưu để tránh xung đột tín hiệu:

| Giao tiếp | Chân chức năng | GPIO (Yolo Uno) | Ghi chú kỹ thuật |
| :--- | :--- | :--- | :--- |
| **SPI (RFID)** | SDA (SS) | 21 | Chip Select |
| | SCK | 4 | Serial Clock |
| | MOSI | 3 | Master Out Slave In |
| | MISO | 2 | Master In Slave Out |
| | RST | 1 | Reset |
| **I2C** | SDA | 11 | Dùng chung LCD & DHT20 |
| | SCL | 12 | |
| **PWM** | Signal | 38 | Servo Motor |
| **GPIO (Keypad)** | Rows (Hàng) | 8, 7, 6, 5 | Cấu hình Input Pull-up |
| | Cols (Cột) | 9, 10, 17, 18 | Cấu hình Output |

---

## 🚀 Cài đặt và Sử dụng (Installation)

### 1. Yêu cầu phần mềm
* **Visual Studio Code** + Extension **PlatformIO**.
* Driver USB cho Yolo Uno/ESP32-S3.

### 2. Thiết lập dự án
1.  **Clone repository:**
    ```bash
    git clone [https://github.com/your-username/smart-door-system.git](https://github.com/your-username/smart-door-system.git)
    ```
2.  **Cấu hình WiFi & MQTT:**
    * Mở file `src/global.cpp`.
    * Điền thông tin WiFi (`WIFI_SSID`, `WIFI_PASS`).
    * Điền thông tin CoreIoT (`MQTT_SERVER`, `TOKEN`...).
3.  **Nạp Code:**
    * Kết nối mạch Yolo Uno với máy tính.
    * Nhấn nút **Upload** trên PlatformIO.

### 3. Hướng dẫn sử dụng
* **Mở bằng Mật khẩu:** Nhấn 'C' để khóa -> Nhập mật khẩu -> Nhấn '#' để mở.
* **Mở bằng Thẻ:** Quẹt thẻ đã đăng ký vào đầu đọc.
* **Khóa cửa:** Cửa tự động đóng sau 5 giây hoặc nhấn phím 'C'.
* **Reset Báo cháy:** Khi hết nhiệt độ cao, nhấn phím 'B' để đưa hệ thống về bình thường.

---



## 📝 Kết quả đạt được (Credits Achievement)

Dự án đã hoàn thành đầy đủ 6/6 Credit theo yêu cầu:

| Credit | Mô tả chức năng | Trạng thái |
| :--- | :--- | :---: |
| **1** | Cảnh báo Nhiệt độ bằng LED đơn (Nháy theo tần số) | ✅ |
| **2** | Cảnh báo Độ ẩm bằng NeoPixel (Đổi màu RGB) | ✅ |
| **3** | Hiển thị LCD đa trạng thái (Locked, Unlocked, Fire...) | ✅ |
| **4** | Khóa điện tử Keypad + Servo (Logic FSM & Lockout) | ✅ |
| **5** | Xác thực RFID với Database định danh người dùng | ✅ |
| **6** | IoT Dashboard: Traceability (Truy xuất nguồn gốc) & Remote Control | ✅ |

---

**© 2024 Smart Door Project.** *Developed with ❤️ using ESP32-S3 & FreeRTOS.*
