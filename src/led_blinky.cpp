#include "led_blinky.h"
#include "global.h"

void led_blinky(void *pvParameters){
    pinMode(2, OUTPUT); //SETUP
    static bool led_state = LOW;  // Dùng static để giữ trạng thái
    static int last_button_state = 0;

    while (1){
        // digitalWrite(2, LOW); //ON
        // Serial.println("LED ON");
        // vTaskDelay(500);  // Sử dụng vTaskDelay thay vì delay
        
        // digitalWrite(2, LOW); //OFF
        // Serial.println("LED OFF");
        // vTaskDelay(500);  // Sử dụng vTaskDelay thay vì delay
        if(button1 == 1){
            digitalWrite(2, HIGH); //ON
            //Serial.println("LED ON");
        }
        else if(button1 == 0){
            digitalWrite(2, LOW); //OFF
            //Serial.println("LED OFF");
        }
        last_button_state = button1;  // Lưu trạng thái để so sánh lần sau
        vTaskDelay(50);  // Delay để giảm tải CPU
    }
    
}