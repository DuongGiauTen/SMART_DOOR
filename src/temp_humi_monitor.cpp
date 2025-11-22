#include "temp_humi_monitor.h"
#include "global.h"
#include "lcd.h"
DHT20 dht20;

void read_real_sensors() {
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();

    glob_temperature = temperature;
    glob_humidity = humidity;
    
    // BẢO VỆ SERIAL PRINT
    if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        //Serial.print("Humidity: ");
        //Serial.print(humidity);
        //Serial.print("%  Temperature: ");
        //Serial.print(temperature);
        //Serial.println("°C");
        xSemaphoreGive(g_serialMutex);
    }
}


void temp_humi_monitor(void *pvParameters) {
    while (1) {
        // 1. Đọc giá trị
        read_real_sensors();
        float temp = glob_temperature;
        float humi = glob_humidity;

        // 2. Xử lý trạng thái mới
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

        // 3. Cập nhật biến global (DÙNG g_sensorMutex)
        if (g_sensorMutex != NULL && xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            if (newTempState != g_temperState) {
                g_temperState = newTempState;
                tempChanged = true;
            }
            if (newHumiState != g_humiState) {
                g_humiState = newHumiState;
                humiChanged = true;
            }
            
            xSemaphoreGive(g_sensorMutex); // Trả g_sensorMutex

        } else {
            // Báo lỗi dùng g_serialMutex
            if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                Serial.println("temp_humi_monitor: FAILED to get g_sensorMutex!");
                xSemaphoreGive(g_serialMutex);
            }
        }

        // 4. GỬI TÍN HIỆU
        if (tempChanged) {
            if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                Serial.println("temp_humi_monitor: Sending Temp signal!");
                xSemaphoreGive(g_serialMutex);
            }
            xSemaphoreGive(xTempSemaphore);
        }
        if (humiChanged) {
            if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                Serial.println("temp_humi_monitor: Sending Humi signal!");
                xSemaphoreGive(g_serialMutex);
            }
            xSemaphoreGive(xHumiSemaphore);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}