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
        // Receive key from queue instead of reading shared g_newKey/g_keyReady
        bool keyReadyLocal = false;
        char newKeyLocal = 0;
        if (g_keyQueue != NULL) {
            if (xQueueReceive(g_keyQueue, &newKeyLocal, pdMS_TO_TICKS(20)) == pdPASS) {
                keyReadyLocal = true;
            } else {
                keyReadyLocal = false;
            }
        } else {
            keyReadyLocal = false;
        }

        // Chỉ xử lý khi có phím mới được nhấn
        if (keyReadyLocal) {
            // Read current state (local copy)
            SystemState stateLocal;
            if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                stateLocal = g_systemState;
                xSemaphoreGive(g_mutex);
            } else {
                // Không lấy được mutex để đọc trạng thái hệ thống -> bỏ qua
                stateLocal = (SystemState)-1;
            }

            switch (stateLocal) {
                // ================== PHẦN THÊM MỚI ==================
                case INITIAL:
                    if (newKeyLocal == 'C') { // Nếu nhấn 'C'
                        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = LOCKED; // Chuyển sang trạng thái khóa
                            xSemaphoreGive(g_mutex);
                        } else {
                            g_systemState = LOCKED;
                        }
                    }
                    break;
                // ===================================================

                case LOCKED:
                    if (newKeyLocal >= '0' && newKeyLocal <= '9') {
                        // update shared input under mutex
                        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                            reset_input();
                            g_enteredPassword[g_passwordIndex++] = newKeyLocal;
                            g_systemState = ENTERING_PASSWORD;
                            xSemaphoreGive(g_mutex);
                        } else {
                            // Không lấy được mutex để cập nhật input -> bỏ qua phím
                        }
                    }
                    if (newKeyLocal == 'C'){
                        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = INITIAL;
                            reset_input(); // Reset nhập liệu
                            xSemaphoreGive(g_mutex);
                        } else {
                            // Không lấy được mutex để chuyển về INITIAL -> bỏ qua
                        }
                    }
                    if (newKeyLocal == 'A'){
                        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            g_systemState = ERROR;
                            reset_input(); // Reset nhập liệu
                            xSemaphoreGive(g_mutex);
                        } else {
                            // Không lấy được mutex để chuyển về ERROR -> bỏ qua
                        }
                    }
                    break;

                case ENTERING_PASSWORD:
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        if (g_passwordIndex < 6 && newKeyLocal >= '0' && newKeyLocal <= '9') {
                            g_enteredPassword[g_passwordIndex++] = newKeyLocal;
                        } else if (g_passwordIndex == 6 && newKeyLocal == '#') {
                            g_systemState = CHECKING_PASSWORD;
                        } else if (newKeyLocal == '*') {
                            reset_input();
                            g_systemState = LOCKED;
                        }
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex để cập nhật đang nhập -> bỏ qua phím
                    }
                    break;
                

                default:
                    if (newKeyLocal == '*') {
                        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                            reset_input();
                            g_systemState = LOCKED;
                            xSemaphoreGive(g_mutex);
                        } else {
                            // Không lấy được mutex để xử lý '*' -> bỏ qua
                        }
                    }
                    break;
            }
        }
        // Xử lý logic không phụ thuộc vào phím nhấn (như timer)
        // Read current state under mutex to operate on a local copy
        SystemState currentState;
        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            currentState = g_systemState;
            xSemaphoreGive(g_mutex);
        } else {
            // Không lấy được mutex để đọc trạng thái hiện tại -> bỏ qua vòng xử lý này
            currentState = (SystemState)-1;
        }

        switch (currentState) {
            case CHECKING_PASSWORD: {
                // Copy enteredPassword safely
                char localEntered[7] = {0};
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    memcpy(localEntered, g_enteredPassword, sizeof(localEntered));
                    xSemaphoreGive(g_mutex);
                } else {
                    // Không lấy được mutex để đọc entered password -> bỏ qua kiểm tra
                    break; // thoát switch CHECKING_PASSWORD
                }

                if (strcmp(localEntered, CORRECT_PASSWORD) == 0) {
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        g_systemState = UNLOCKED;
                        g_wrongAttempts = 0;
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex để cập nhật state -> bỏ qua
                    }
                } else {
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                        g_wrongAttempts++;
                        if (g_wrongAttempts >= 3) {
                            g_systemState = SYSTEM_LOCKED_DOWN;
                            g_lockoutTimer = 50;
                        } else {
                            g_systemState = INCORRECT_PASSWORD;
                        }
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex để cập nhật sai mật khẩu -> bỏ qua
                    }
                }
                reset_input();
                break;
            }

            case UNLOCKED: {
                // set door true then release mutex during delay
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_doorState = true;
                    xSemaphoreGive(g_mutex);
                } else {
                    // Không lấy được mutex để mở cửa -> bỏ qua (không thay đổi g_doorState)
                }
                vTaskDelay(pdMS_TO_TICKS(5000));
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_doorState = false;
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_mutex);
                } else {
                    // Không lấy được mutex để đóng cửa/đặt lại state -> bỏ qua
                }
                break;
            }

            case INCORRECT_PASSWORD:
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_mutex);
                } else {
                    // Không lấy được mutex để đặt lại state -> bỏ qua
                }
                break;

            case SYSTEM_LOCKED_DOWN: {
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_wrongAttempts = 0;
                    xSemaphoreGive(g_mutex);
                } else {
                    g_wrongAttempts = 0;
                }
                while (true) {
                    int timerLocal = 0;
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        timerLocal = g_lockoutTimer;
                        xSemaphoreGive(g_mutex);
                    } else {
                        timerLocal = g_lockoutTimer;
                    }
                    if (timerLocal <= 0) break;
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        g_lockoutTimer--;
                        xSemaphoreGive(g_mutex);
                    } else {
                        g_lockoutTimer--;
                    }
                }
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_lockoutTimer = 50;
                    g_systemState = LOCKED;
                    xSemaphoreGive(g_mutex);
                } else {
                    g_lockoutTimer = 50;
                    g_systemState = LOCKED;
                }
                break;
            }

            case ERROR:
                vTaskDelay(pdMS_TO_TICKS(2000));
                if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                    g_systemState = SYSTEM_LOCKED_DOWN;
                    xSemaphoreGive(g_mutex);
                } else {
                    g_systemState = SYSTEM_LOCKED_DOWN;
                }
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}