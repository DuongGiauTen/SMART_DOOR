#include <Arduino.h>
#include "door.h"
#include "global.h"

#define DOOR_PIN 2 // Dùng chân GPIO 2 cho đèn LED/Relay cửa

void door_task(void *pvParameters){
    pinMode(DOOR_PIN, OUTPUT);
    while (1){
        if(g_doorState == true){ // Nếu trạng thái là Mở
            digitalWrite(DOOR_PIN, HIGH); // Mở cửa
        }
        else { // Nếu trạng thái là Đóng
            digitalWrite(DOOR_PIN, LOW); // Đóng cửa
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Kiểm tra trạng thái mỗi 100ms
    }
}