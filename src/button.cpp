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
byte rowPins[ROWS] = {27, 14, 12, 13};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void keypad_task(void *pvParameters){
    while(1){
        char customKey = customKeypad.getKey();
        if (customKey){
            g_newKey = customKey;
            g_keyReady = true; // Báo hiệu có phím mới
            // Chờ một chút để task logic xử lý, tránh việc đọc phím quá nhanh
            if(customKey == 'A'){
                button1 = !button1; // Toggle button1 state
            }
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Quét phím mỗi 20ms
    }
}