/* --- neo_blinky.cpp --- */
#include "neo_blinky.h"
#include "global.h" // Cần thiết cho các semaphores

void neo_blinky(void *pvParameters) {
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();

    // Đặt màu mặc định (tương ứng HUMI_LOW)
    strip.setPixelColor(0, strip.Color(0, 0, 255)); // Xanh dương = Khô
    strip.show();

    while (1) {
        // 1. CHỜ TÍN HIỆU (BLOCKING)
        // "Tôi sẽ 'ngủ' ở đây cho đến khi có tín hiệu."
        xSemaphoreTake(xHumiSemaphore, portMAX_DELAY);

        // 2. THỨC DẬY! ĐÃ CÓ TÍN HIỆU
        // Báo hiệu an toàn (dùng g_serialMutex)
        if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            Serial.println("neo_blinky: Received signal!");
            xSemaphoreGive(g_serialMutex);
        }

        HumiState currentHumi; // Tạo biến cục bộ

        // Dùng g_sensorMutex để ĐỌC biến global một cách an toàn
        if (g_sensorMutex != NULL && xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentHumi = g_humiState; // Sao chép giá trị
            // Trả mutex ngay
            xSemaphoreGive(g_sensorMutex);
        }

        // Xử lý logic bên ngoài mutex
        switch (currentHumi) {
            case HUMI_LOW: // Khô
                strip.setPixelColor(0, strip.Color(0, 0, 255)); // Xanh dương
                break;
            case HUMI_MEDIUM: // Vừa
                strip.setPixelColor(0, strip.Color(0, 255, 0)); // Xanh lá
                break;
            case HUMI_HIGH: // Ẩm
                strip.setPixelColor(0, strip.Color(255, 100, 0)); // Cam
                break;
        }
        strip.show(); // Cập nhật màu

        // KHÔNG CẦN vTaskDelay.
        // Vòng lặp sẽ quay lại và "ngủ" ngay ở xSemaphoreTake.
    }
}