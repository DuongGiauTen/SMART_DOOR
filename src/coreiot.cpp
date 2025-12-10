#include "coreiot.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

// --- 1. XỬ LÝ LỆNH ĐIỀU KHIỂN TỪ XA (RPC) ---
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) message += (char)payload[i];
    Serial.print("RPC Received: "); Serial.println(message);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) return;

    // Kiểm tra method: "setDoorControl"
    if (strcmp(doc["method"], "setDoorControl") == 0) {
        bool cmdOpen = doc["params"]; // true = Mở, false = Đóng
        Serial.printf(">> REMOTE COMMAND: %s\n", cmdOpen ? "OPEN" : "CLOSE");

        // Dùng Mutex để thay đổi trạng thái hệ thống an toàn
        if (xSemaphoreTake(g_logicMutex, portMAX_DELAY) == pdTRUE) {
            if (cmdOpen) {
                g_systemState = UNLOCKED; // Chuyển trạng thái hệ thống
                g_doorState = true;       // Set biến cửa mở
                g_unlockSource = "Remote Web/App"; // Ghi nguồn mở cửa
            } else {
                g_systemState = LOCKED;   // Chuyển trạng thái hệ thống
                g_doorState = false;      // Set biến cửa đóng
            }
            xSemaphoreGive(g_logicMutex);
        }
        
        // Đánh thức Door Task thực thi ngay lập tức
        xSemaphoreGive(g_doorSemaphore);
    }
}

// --- 2. KẾT NỐI MQTT (DÙNG BIẾN GLOBAL) ---
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to CoreIoT (MQTT Basic)...");
        
        // Sử dụng các biến const char* đã khai báo bên global.cpp
        if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println(" Connected!");
            client.subscribe("v1/devices/me/rpc/request/+"); // Đăng ký nhận lệnh
        } else {
            Serial.print(" Failed, rc="); Serial.print(client.state());
            Serial.println(" try again in 5s");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

// --- 3. TASK CHÍNH (LOGIC GỬI DỮ LIỆU) ---
void coreiot_task(void *pvParameters) {
    // Chờ mạng Internet
    // Serial.println("CoreIoT: Waiting for Internet...");
    // xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY); 
    // xSemaphoreGive(xBinarySemaphoreInternet);

    Serial.println("CoreIoT Task Started");

    // ---  KẾT NỐI WIFI TRỰC TIẾP ---
    Serial.println("Forcing WiFi Connection...");
    WiFi.begin(WIFI_SSID, WIFI_PASS); 
    // -------------------------------------------------

    // Vòng lặp chờ (giữ nguyên như cũ)
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("."); // In dấu chấm cho gọn
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
    
    Serial.println("\nCoreIoT: WiFi Connected! IP Address: ");
    Serial.println(WiFi.localIP()); // In IP ra để biết đã vào mạng

    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);

    // Biến theo dõi trạng thái cũ
    bool lastDoorState = false;
    unsigned long lastSensorTime = 0;
    const unsigned long SENSOR_INTERVAL = 5000; // 5 giây gửi nhiệt độ 1 lần

    // Lấy trạng thái ban đầu
    if (xSemaphoreTake(g_logicMutex, portMAX_DELAY) == pdTRUE) {
        lastDoorState = g_doorState;
        xSemaphoreGive(g_logicMutex);
    }

    while (1) {
        if (!client.connected()) reconnect();
        client.loop();

       // --- KIỂM TRA TRẠNG THÁI BÁO CHÁY ---
        SystemState currentStateIot = INITIAL;
        if (xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentStateIot = g_systemState;
            xSemaphoreGive(g_logicMutex);
        }

        // Nếu đang cháy, gửi cảnh báo liên tục hoặc ưu tiên
        if (currentStateIot == FIRE_ALARM) {
            StaticJsonDocument<200> doc;
            doc["alarm_status"] = 1; // Bật Widget Alarm đỏ rực
            doc["door_history"] = "KHẨN CẤP: CHÁY NHÀ!";
            
            char buffer[200];
            serializeJson(doc, buffer);
            client.publish(TOPIC_TELEMETRY, buffer);
            
            Serial.println(">> FIRE ALARM SENT TO CLOUD!");
            vTaskDelay(2000); // Gửi mỗi 2 giây để spam cảnh báo
            continue; // Bỏ qua các việc khác
        } 
        else {
            // Nếu không cháy thì gửi:
            // doc["alarm_status"] = "SAFE"; 
            // (Để Widget Alarm tắt đi)
        }       

        // --- A. GỬI LỊCH SỬ CỬA (EVENT-BASED) ---
        bool currentDoorState = false;
        String unlockSourceLocal = ""; // Biến tạm để copy chuỗi ra ngoài

        if (xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentDoorState = g_doorState;
            // Copy tên người mở cửa ra biến tạm để xử lý (tránh giữ mutex lâu)
            unlockSourceLocal = g_unlockSource; 
            xSemaphoreGive(g_logicMutex);
        }

        if (currentDoorState != lastDoorState) {
            StaticJsonDocument<512> doc; // Tăng size lên xíu vì chuỗi tên có thể dài
            
            if (currentDoorState == true) {
                // NẾU CỬA MỞ: Gửi kèm tên người mở
                // Ví dụ: "MỞ: Thẻ: Nguyen Van A (ID...)"
                doc["door_history"] = "MỞ: " + unlockSourceLocal;
            } else {
                // NẾU CỬA ĐÓNG
                doc["door_history"] = "ĐÃ ĐÓNG CỬA";
            }
            
            doc["door_status"] = currentDoorState;
            
            char buffer[512];
            serializeJson(doc, buffer);
            client.publish(TOPIC_TELEMETRY, buffer);
            
            Serial.println(">> HISTORY SENT: " + String(buffer));
            lastDoorState = currentDoorState;
        }
        // --- B. GỬI NHIỆT ĐỘ/ĐỘ ẨM (PERIODIC) ---
        // Dùng millis() để gửi định kỳ mà không chặn luồng A
        if (millis() - lastSensorTime > SENSOR_INTERVAL) {
            lastSensorTime = millis();
            
            float t = 0, h = 0;
            if (xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                t = glob_temperature;
                h = glob_humidity;
                xSemaphoreGive(g_sensorMutex);
            }

            StaticJsonDocument<256> doc;
            doc["temperature"] = t;
            doc["humidity"] = h;

            if (currentStateIot != FIRE_ALARM) {
                doc["alarm_status"] = 0; 
            }
            
            char buffer[256];
            serializeJson(doc, buffer);
            client.publish(TOPIC_TELEMETRY, buffer);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay nhỏ để giảm tải CPU
    }
}