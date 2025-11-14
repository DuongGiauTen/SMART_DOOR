/* --- neo_blinky.cpp --- */
#include "neo_blinky.h"
#include "global.h" // Phải include file global

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
        // Task này không tốn CPU khi đang chờ.
        xSemaphoreTake(xHumiSemaphore, portMAX_DELAY);

        // 2. THỨC DẬY! ĐÃ CÓ TÍN HIỆU
        // Đọc trạng thái global và đổi màu
        Serial.println("neo_blinky: Received signal!");

        HumiState currentHumi; // Tạo biến cục bộ

        // Dùng mutex để ĐỌC biến global một cách an toàn
        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentHumi = g_humiState; // Sao chép giá trị
            // Trả mutex ngay
            xSemaphoreGive(g_mutex);
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