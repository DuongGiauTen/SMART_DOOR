/* --- led_blinky.cpp --- */
#include "led_blinky.h"
// global.h đã được include từ led_blinky.h

void led_blinky(void *pvParameters) {
    pinMode(LED_GPIO, OUTPUT);
    int blink_rate = 1000; // Mặc định là TEMP_LOW (tổng chu kỳ 1000ms)

    while (1) {
        // 1. KIỂM TRA TÍN HIỆU (NON-BLOCKING)
        // "Có tín hiệu nào mới không? (Timeout = 0)"
        if (xSemaphoreTake(xTempSemaphore, 0) == pdTRUE) {
            // CÓ TÍN HIỆU! Trạng thái đã thay đổi.
            
            // Báo hiệu an toàn (dùng g_serialMutex)
            if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                Serial.println("led_blinky: Received signal!");
                xSemaphoreGive(g_serialMutex);
            }

            // Dùng g_sensorMutex để ĐỌC biến global một cách an toàn
            if (g_sensorMutex != NULL && xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                switch (g_temperState) {
                    case TEMP_LOW:
                        blink_rate = 1000; // chậm
                        break;
                    case TEMP_MEDIUM:
                        blink_rate = 500; // trung bình
                        break;
                    case TEMP_HIGH:
                        blink_rate = 200; // nhanh
                        break;
                }
                // Trả mutex ngay sau khi đọc xong
                xSemaphoreGive(g_sensorMutex);
            }
        }

        // 2. THỰC HIỆN CÔNG VIỆC CHÍNH (LUÔN LUÔN NHÁY)
        // Dù có tín hiệu hay không, task này vẫn phải nháy
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(blink_rate / 2)); // Chia 2 để đúng chu kỳ
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(blink_rate / 2));
    }
}