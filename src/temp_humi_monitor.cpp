/* --- src/temp_humi_monitor.cpp (ĐÃ SỬA LỖI) --- */
#include "temp_humi_monitor.h"
#include "global.h"
#include "lcd.h" // Để in debug nếu cần
DHT20 dht20;

void read_real_sensors() {
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();

    glob_temperature = temperature;
    glob_humidity = humidity;
    
    // In ra Serial để kiểm tra
    if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        Serial.print("Humi: "); Serial.print(humidity);
        Serial.print("%  Temp: "); Serial.print(temperature);
        Serial.println("°C");
        xSemaphoreGive(g_serialMutex);
    }
}

void temp_humi_monitor(void *pvParameters) {
    // Chờ cảm biến ổn định
    vTaskDelay(pdMS_TO_TICKS(2000));

    while (1) {
        read_real_sensors();
        float temp = glob_temperature;
        float humi = glob_humidity;

        // === LOGIC BÁO CHÁY ===
        if (temp >= FIRE_THRESHOLD) {
            // Dùng Mutex để kiểm tra và cập nhật trạng thái
            if (xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                
                // Chỉ kích hoạt nếu chưa ở trạng thái báo cháy (Tránh spam)
                if (g_systemState != FIRE_ALARM) {
                    Serial.println("!!! ALARM: TEMP TOO HIGH - OPENING DOOR !!!");
                    
                    g_systemState = FIRE_ALARM; // Cập nhật trạng thái hệ thống
                    g_doorState = true;         // Cập nhật biến cửa MỞ
                    
                    // Đánh thức Door Task ngay lập tức
                    xSemaphoreGive(g_doorSemaphore);
                }
                
                // QUAN TRỌNG: Chỉ trả Mutex khi đã lấy thành công
                xSemaphoreGive(g_logicMutex);
            }
        }

        // === LOGIC CẬP NHẬT BIẾN TOÀN CỤC (CHO ĐÈN LED) ===
        TemperState newTempState;
        if (temp < 25.0) newTempState = TEMP_LOW;
        else if (temp < 30.0) newTempState = TEMP_MEDIUM;
        else newTempState = TEMP_HIGH;

        HumiState newHumiState;
        if (humi < 65.0) newHumiState = HUMI_LOW;
        else if (humi < 70.0) newHumiState = HUMI_MEDIUM;
        else newHumiState = HUMI_HIGH;

        bool tempChanged = false;
        bool humiChanged = false;

        if (g_sensorMutex != NULL && xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (newTempState != g_temperState) {
                g_temperState = newTempState;
                tempChanged = true;
            }
            if (newHumiState != g_humiState) {
                g_humiState = newHumiState;
                humiChanged = true;
            }
            xSemaphoreGive(g_sensorMutex); 
        }

        // Gửi tín hiệu cho đèn LED Blinky
        if (tempChanged) xSemaphoreGive(xTempSemaphore);
        if (humiChanged) xSemaphoreGive(xHumiSemaphore);

        vTaskDelay(pdMS_TO_TICKS(500)); // Kiểm tra mỗi 0.5 giây
    }
}