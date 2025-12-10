#include "task_rfid.h"

// Cấu hình chân SPI (Sửa lại theo dây em nối)
#define SS_PIN  21  
#define RST_PIN 1
#define SCK_PIN 4
#define MOSI_PIN 3
#define MISO_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);

// === 1. TẠO CẤU TRÚC DỮ LIỆU NGƯỜI DÙNG ===
struct User {
    String name;          // Tên người dùng
    byte uid[4];          // Mã thẻ (4 bytes)
};

// === 2. DANH SÁCH NGƯỜI DÙNG (DATABASE) ===
// Em hãy điền mã thẻ thật vào đây
User authorizedUsers[] = {
    {"Tran Minh Duong : 2310609",  {0x62, 0x5C, 0xDC, 0x73}}, // Người số 1
    {"Nguyen Minh Hien",    {0xA4, 0xB2, 0xC5, 0xD9}}, // Người số 2
    {"Giang Vien",    {0x12, 0x34, 0x56, 0x78}}  // Người số 3
};

// Tính số lượng người trong danh sách
const int numUsers = sizeof(authorizedUsers) / sizeof(authorizedUsers[0]);

// Hàm chuyển đổi UID sang chuỗi Hex để hiển thị (VD: "DE AD BE EF")
String getUIDString(byte *uid, byte size) {
    String uidStr = "";
    for (byte i = 0; i < size; i++) {
        if (uid[i] < 0x10) uidStr += "0";
        uidStr += String(uid[i], HEX);
        if (i < size - 1) uidStr += " "; // Thêm dấu cách
    }
    uidStr.toUpperCase();
    return uidStr;
}

void rfid_task(void *pvParameters) {
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    rfid.PCD_Init();
    
    Serial.println("RFID Task: Database loaded with " + String(numUsers) + " users.");

    while (1) {
        // Kiểm tra thẻ mới
        if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Đã có thẻ!
        String currentUIDStr = getUIDString(rfid.uid.uidByte, rfid.uid.size);
        Serial.println(">>> Quẹt thẻ: " + currentUIDStr);

        bool accessGranted = false;
        String detectedName = "";

        // === 3. QUÉT DATABASE TÌM NGƯỜI DÙNG ===
        for (int i = 0; i < numUsers; i++) {
            bool match = true;
            // So sánh từng byte
            for (int j = 0; j < 4; j++) {
                if (rfid.uid.uidByte[j] != authorizedUsers[i].uid[j]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                accessGranted = true;
                detectedName = authorizedUsers[i].name;
                break; // Tìm thấy rồi thì dừng vòng lặp
            }
        }

        // === 4. XỬ LÝ KẾT QUẢ ===
        if (accessGranted) {
            Serial.println(">>> HELLO: " + detectedName);

            // Dùng Mutex cập nhật trạng thái
            if (xSemaphoreTake(g_logicMutex, portMAX_DELAY) == pdTRUE) {
                g_systemState = UNLOCKED;
                g_doorState = true; 
                g_wrongAttempts = 0;
                
                // *** QUAN TRỌNG: LƯU TÊN NGƯỜI VÀO BIẾN TOÀN CỤC ***
                // Format: "Thẻ: Nguyen Van A (ID: DE AD...)"
                g_unlockSource = "Thẻ: " + detectedName; 
                
                xSemaphoreGive(g_logicMutex);
            }
            xSemaphoreGive(g_doorSemaphore); // Mở cửa
            
            // Có thể thêm còi bíp ngắn ở đây
        } else {
            Serial.println(">>> ACCESS DENIED! Unknown Card.");
            // Có thể thêm code báo lỗi
            if (xSemaphoreTake(g_logicMutex, portMAX_DELAY) == pdTRUE) {
                g_systemState = UNKNOWN_CARD;
                
                xSemaphoreGive(g_logicMutex);
            }
            
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        vTaskDelay(pdMS_TO_TICKS(2000)); // Chống quẹt liên tục
    }
}