/* --- button.cpp (Đã sửa lỗi) --- */
#include <Keypad.h>
#include "button.h"
#include "global.h"

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; // R1, R2, R3, R4
byte colPins[COLS] = {9, 10, 17, 18}; // C1, C2, C3, C4

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void keypad_task(void *pvParameters){
    while(1){
        char customKey = customKeypad.getKey();
        
        if (customKey){
            
            // 1. Lấy mutex logic để cập nhật biến
            // Phải dùng 'g_logicMutex' để đồng bộ với 'logic_task'
            if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                
                g_newKey = customKey;
                g_keyReady = true; // Báo hiệu có phím mới
                
                if(customKey == 'B'){
                    button1 = !button1; // Toggle button1 state
                    Serial.print("Button1 state changed to: ");
                    Serial.println(button1);
                    
                }

                // Trả mutex logic NGAY LẬP TỨC
                xSemaphoreGive(g_logicMutex);

            } else {
                // Không lấy được logic_mutex, bỏ qua phím này
                // Báo lỗi (an toàn)
                if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    Serial.println("keypad_task: FAILED to get g_logicMutex! Key dropped.");
                    xSemaphoreGive(g_serialMutex);
                }
                continue; // Bỏ qua phím này
            }

            // 2. Lấy mutex serial để in (Hành động này giờ độc lập)
            if (g_serialMutex != NULL && xSemaphoreTake(g_serialMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                Serial.print("Key Pressed: ");
                Serial.println(customKey);
                xSemaphoreGive(g_serialMutex);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // Quét phím mỗi 20ms
    }
}