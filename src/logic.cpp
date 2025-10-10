#include "logic.h"
#include "global.h"

const char CORRECT_PASSWORD[7] = "123456";

// Hàm tiện ích để reset trạng thái nhập liệu
void reset_input() {
    g_passwordIndex = 0;
    memset(g_enteredPassword, 0, sizeof(g_enteredPassword));
}

void logic_task(void *pvParameters) {
    while (1) {
        // Chỉ xử lý khi có phím mới được nhấn
        if (g_keyReady) {
            g_keyReady = false; // Reset cờ báo

            switch (g_systemState) {
                // ================== PHẦN THÊM MỚI ==================
                case INITIAL:
                    if (g_newKey == 'C') { // Nếu nhấn 'C'
                        g_systemState = LOCKED; // Chuyển sang trạng thái khóa
                    }
                    break;
                // ===================================================

                case LOCKED:
                    if (g_newKey >= '0' && g_newKey <= '9') {
                        reset_input();
                        g_enteredPassword[g_passwordIndex++] = g_newKey;
                        g_systemState = ENTERING_PASSWORD;
                    }
                    if (g_newKey == 'C'){
                        g_systemState = INITIAL; 
                        reset_input(); // Reset nhập liệu
                    }
                    break;

                case ENTERING_PASSWORD:
                    if (g_passwordIndex < 6 && g_newKey >= '0' && g_newKey <= '9') {
                        g_enteredPassword[g_passwordIndex++] = g_newKey;
                    } else if (g_passwordIndex == 6 && g_newKey == '#') {
                        g_systemState = CHECKING_PASSWORD;
                    } else if (g_newKey == '*') {
                        reset_input();
                        g_systemState = LOCKED;
                    }
                    break;
                
                default:
                    if (g_newKey == '*') {
                        reset_input();
                        g_systemState = LOCKED;
                    }
                    break;
            }
        }

        // Xử lý logic không phụ thuộc vào phím nhấn (như timer)
        switch (g_systemState) {
            case CHECKING_PASSWORD:
                if (strcmp(g_enteredPassword, CORRECT_PASSWORD) == 0) {
                    g_systemState = UNLOCKED;
                    g_wrongAttempts = 0;
                } else {
                    g_wrongAttempts++;
                    if (g_wrongAttempts >= 3) {
                        g_systemState = SYSTEM_LOCKED_DOWN;
                        g_lockoutTimer = 50;
                    } else {
                        g_systemState = INCORRECT_PASSWORD;
                    }
                }
                reset_input();
                break;
            
            case UNLOCKED:
                g_doorState = true;
                vTaskDelay(pdMS_TO_TICKS(5000));
                g_doorState = false;
                g_systemState = LOCKED;
                break;
            
            case INCORRECT_PASSWORD:
                vTaskDelay(pdMS_TO_TICKS(2000));
                g_systemState = LOCKED;
                break;
            
            case SYSTEM_LOCKED_DOWN:
                g_wrongAttempts = 0;
                while(g_lockoutTimer > 0) {
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    g_lockoutTimer--;
                }
                g_systemState = LOCKED;
                break;
            
            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}