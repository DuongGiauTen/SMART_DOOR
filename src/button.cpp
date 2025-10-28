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
            // Send key via queue to logic task (non-blocking)
            if (g_keyQueue != NULL) {
                BaseType_t ok = xQueueSend(g_keyQueue, &customKey, 0);
                if (ok != pdPASS) {
                    // Queue full or failed -> drop key
                }
            } else {
                // If queue not available, drop key
            }
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Quét phím mỗi 20ms
    }
}