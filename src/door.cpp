#include <Arduino.h>
#include "door.h"
#include "global.h"

#define DOOR_PIN 48 // Dùng chân GPIO 2 cho đèn LED/Relay cửa

void door_task(void *pvParameters){
    pinMode(DOOR_PIN, OUTPUT);
    bool doorLocal = false; // giữ giá trị cuối cùng nếu không đọc được mutex
    while (1){
        // Read door state under mutex to avoid race
        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            doorLocal = g_doorState;
            xSemaphoreGive(g_mutex);
        } else {
            // Không lấy được mutex -> bỏ qua vòng này, giữ doorLocal cũ
        }

        if(doorLocal == true){ // Nếu trạng thái là Mở
            digitalWrite(DOOR_PIN, HIGH); // Mở cửa
        }
        else { // Nếu trạng thái là Đóng
            digitalWrite(DOOR_PIN, LOW); // Đóng cửa
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Kiểm tra trạng thái mỗi 100ms
    }
}