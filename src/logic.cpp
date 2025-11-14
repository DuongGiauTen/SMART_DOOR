#include "logic.h"
#include "global.h"

const char CORRECT_PASSWORD[7] = "123456";

// Hàm tiện ích để reset trạng thái nhập liệu (BÊN TRONG MUTEX)
void reset_input_unsafe() {
    g_passwordIndex = 0;
    memset(g_enteredPassword, 0, sizeof(g_enteredPassword));
}

void logic_task(void *pvParameters) {
    while (1) {
        // === PHẦN 1: XỬ LÝ PHÍM BẤM ===
        bool keyReadyLocal = false;
        char newKeyLocal = 0;
        
        // Dùng g_logicMutex
        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            keyReadyLocal = g_keyReady;
            newKeyLocal = g_newKey;
            if (keyReadyLocal) g_keyReady = false; // Tiêu thụ phím
            xSemaphoreGive(g_logicMutex);
        } else {
            keyReadyLocal = false;
            newKeyLocal = 0;
        }

        // Chỉ xử lý khi có phím mới
        if (keyReadyLocal) {
            SystemState stateLocal;
            // Đọc trạng thái (dùng g_logicMutex)
            if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                stateLocal = g_systemState;
                xSemaphoreGive(g_logicMutex);
            } else {
                stateLocal = (SystemState)-1; // -1 = không đọc được
            }

            switch (stateLocal) {
                case INITIAL:
                    if (newKeyLocal == 'C') { 
                        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = LOCKED; 
                            xSemaphoreGive(g_logicMutex);
                        }
                    }
                    break;

                case LOCKED:
                    if (newKeyLocal >= '0' && newKeyLocal <= '9') {
                        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                            reset_input_unsafe();
                            g_enteredPassword[g_passwordIndex++] = newKeyLocal;
                            g_systemState = ENTERING_PASSWORD;
                            xSemaphoreGive(g_logicMutex);
                        }
                    }
                    if (newKeyLocal == 'C'){
                        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = INITIAL;
                            reset_input_unsafe();
                            xSemaphoreGive(g_logicMutex);
                        }
                    }
                    if (newKeyLocal == 'A'){
                        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = ERROR;
                            reset_input_unsafe(); 
                            xSemaphoreGive(g_logicMutex);
                        }
                    }
                    break;

                case ENTERING_PASSWORD:
                    if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        if (g_passwordIndex < 6 && newKeyLocal >= '0' && newKeyLocal <= '9') {
                            g_enteredPassword[g_passwordIndex++] = newKeyLocal;
                        } else if (g_passwordIndex == 6 && newKeyLocal == '#') {
                            g_systemState = CHECKING_PASSWORD;
                        } else if (newKeyLocal == '*') {
                            reset_input_unsafe();
                            g_systemState = LOCKED;
                        }
                        xSemaphoreGive(g_logicMutex);
                    }
                    break;
                
                default:
                    if (newKeyLocal == '*') {
                        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            reset_input_unsafe();
                            g_systemState = LOCKED;
                            xSemaphoreGive(g_logicMutex);
                        }
                    }
                    break;
            }
        }
        
        // === PHẦN 2: XỬ LÝ LOGIC (TIMER, KIỂM TRA) ===
        SystemState currentState;
        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentState = g_systemState;
            xSemaphoreGive(g_logicMutex);
        } else {
            currentState = (SystemState)-1; // Không đọc được
        }

        switch (currentState) {
            case CHECKING_PASSWORD: {
                char localEntered[7] = {0};
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    memcpy(localEntered, g_enteredPassword, sizeof(localEntered));
                    reset_input_unsafe(); // Reset luôn ở đây
                    xSemaphoreGive(g_logicMutex);
                } else {
                    break; 
                }

                if (strcmp(localEntered, CORRECT_PASSWORD) == 0) {
                    if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        g_systemState = UNLOCKED;
                        g_wrongAttempts = 0;
                        xSemaphoreGive(g_logicMutex);
                    }
                } else {
                    if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        g_wrongAttempts++;
                        if (g_wrongAttempts >= 3) {
                            g_systemState = SYSTEM_LOCKED_DOWN;
                            g_lockoutTimer = 50;
                        } else {
                            g_systemState = INCORRECT_PASSWORD;
                        }
                        xSemaphoreGive(g_logicMutex);
                    }
                }
                break;
            }

            // SỬA LẠI CASE UNLOCKED ĐỂ DÙNG SEMAPHORE
            case UNLOCKED: {
                // Mở cửa
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_doorState = true;
                    xSemaphoreGive(g_logicMutex);
                }
                xSemaphoreGive(g_doorSemaphore); // BÁO HIỆU CHO DOOR_TASK

                vTaskDelay(pdMS_TO_TICKS(5000)); // Chờ 5 giây

                // Đóng cửa
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_doorState = false;
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_logicMutex);
                }
                xSemaphoreGive(g_doorSemaphore); // BÁO HIỆU CHO DOOR_TASK
                
                break;
            }

            case INCORRECT_PASSWORD:
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_logicMutex);
                }
                break;

            // SỬA LẠI CASE NÀY ĐỂ FIX BUG (XÓA ELSE)
            case SYSTEM_LOCKED_DOWN: {
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_wrongAttempts = 0; // Chỉ reset khi lấy được mutex
                    xSemaphoreGive(g_logicMutex);
                }
                
                while (true) {
                    int timerLocal = 0;
                    if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        timerLocal = g_lockoutTimer;
                        xSemaphoreGive(g_logicMutex);
                    }
                    
                    if (timerLocal <= 0) break; // Hết giờ

                    vTaskDelay(pdMS_TO_TICKS(1000)); // Chờ 1 giây
                    
                    if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        g_lockoutTimer--; // Chỉ trừ khi lấy được mutex
                        xSemaphoreGive(g_logicMutex);
                    }
                }
                
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_lockoutTimer = 50; // Reset lại timer
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_logicMutex);
                }
                break;
            }

            // SỬA LẠI CASE NÀY ĐỂ FIX BUG (XÓA ELSE)
            case ERROR:
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_systemState = SYSTEM_LOCKED_DOWN;
                    xSemaphoreGive(g_logicMutex);
                }
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}