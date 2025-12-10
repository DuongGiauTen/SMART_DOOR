/* --- lcd.cpp (Đã sửa lỗi) --- */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "lcd.h"
#include "global.h"

// Định nghĩa LCD được chuyển từ lcd.h sang đây để tránh lỗi "multiple definition"
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void lcd_task(void *pvParameters) {
    
    lcd.init();
    lcd.backlight();

    SystemState lastDisplayedState = (SystemState)-1; // Trạng thái đã vẽ lần trước
    
    // Khai báo các biến cục bộ để giữ dữ liệu
    SystemState sysLocal;
    int passIdxLocal = 0;
    int attemptsLocal = 0;
    int timerLocal = 0;
    float tempLocal = 0;
    float humiLocal = 0;

    while(1) {
        
        // === 1. THU THẬP DỮ LIỆU (AN TOÀN) ===

        // Đọc dữ liệu logic (dùng g_logicMutex)
        if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            sysLocal = g_systemState;
            passIdxLocal = g_passwordIndex;
            attemptsLocal = g_wrongAttempts;
            timerLocal = g_lockoutTimer;
            xSemaphoreGive(g_logicMutex); // Trả mutex logic
        } else {
            // Không lấy được mutex, giữ nguyên trạng thái cũ để không vẽ lại
            sysLocal = lastDisplayedState;
        }

        // Đọc dữ liệu cảm biến (dùng g_sensorMutex)
        // Chỉ đọc khi ở trạng thái INITIAL để tiết kiệm
        if (sysLocal == INITIAL) {
            if (g_sensorMutex != NULL && xSemaphoreTake(g_sensorMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                tempLocal = glob_temperature;
                humiLocal = glob_humidity;
                xSemaphoreGive(g_sensorMutex); // Trả mutex sensor
            }
            // Nếu không lấy được mutex, sẽ hiển thị giá trị cũ (tempLocal, humiLocal)
        }

        
        // === 2. CẬP NHẬT HIỂN THỊ ===

        // Chỉ xóa màn hình nếu trạng thái logic thay đổi
        if (sysLocal != lastDisplayedState) {
            lcd.clear();
            lastDisplayedState = sysLocal;
        }

        // Hiển thị dựa trên dữ liệu CỤC BỘ (an toàn)
        switch(sysLocal) {
            case INITIAL:
                lcd.setCursor(0, 0);
                lcd.print("T:");
                lcd.print(tempLocal, 2); // 2 số lẻ
                lcd.setCursor(9, 0);
                lcd.print("H:");
                lcd.print(humiLocal, 2); // 2 số lẻ
                lcd.setCursor(0, 1);
                if (tempLocal < 25.0) {
                    lcd.setCursor(0, 1);
                    lcd.print("LOW   ");
                } else if (tempLocal < 30.0) {
                    lcd.setCursor(0, 1);
                    lcd.print("NORMAL");
                } else {
                    lcd.setCursor(0, 1);
                    lcd.print("HIGH   ");
                }

                if(humiLocal < 65.0) {
                    lcd.setCursor(9, 1);
                    lcd.print("LOW   ");
                } else if (humiLocal < 70.0) {
                    lcd.setCursor(9, 1);
                    lcd.print("MEDIUM");
                } else {
                    lcd.setCursor(9, 1);
                    lcd.print("HIGH   ");
                }
                break;

            case LOCKED:
                lcd.setCursor(0, 0);
                lcd.print("Enter Password:");
                break;

            case ENTERING_PASSWORD:
                lcd.setCursor(0, 0);
                lcd.print("Enter Password:");
                lcd.setCursor(0, 1);
                // Hiển thị '*' dựa trên giá trị cục bộ
                for (int i = 0; i < passIdxLocal; i++) {
                    lcd.print("*");
                }
                break;

            case UNLOCKED:
                lcd.setCursor(3, 0);
                lcd.print("UNLOCKED");
                lcd.setCursor(0, 1);
                lcd.print("Door is Open");
                break;
            
            case INCORRECT_PASSWORD:
                lcd.setCursor(0, 0);
                lcd.print("Wrong Password!");
                lcd.setCursor(0, 1);
                lcd.print("Attempts: ");
                lcd.print(attemptsLocal);
                lcd.print("/3");
                break;

            case SYSTEM_LOCKED_DOWN:
                lcd.setCursor(0, 0);
                lcd.print("System Locked!");
                lcd.setCursor(0, 1);
                lcd.print("Wait: ");
                lcd.print(timerLocal);
                lcd.print(" sec");
                break;
                
            case ERROR:
                lcd.setCursor(0, 0);
                lcd.print("LOCK ABORTED");
                lcd.setCursor(0, 1);
                lcd.print("Waiting...");
                break;

            case CHECKING_PASSWORD:
                // Thêm trạng thái này để người dùng biết
                lcd.setCursor(2, 0);
                lcd.print("Checking...");
                break;
            case FIRE_ALARM:
                lcd.setCursor(0, 0);
                lcd.print("!! HOA HOAN !!"); // Dòng 1
                lcd.setCursor(0, 1);
                lcd.print("MO CUA KHAN CAP"); // Dòng 2: Emergency Open
                
                // Nhấp nháy đèn nền LCD để gây chú ý (Optional)
                lcd.noBacklight();
                vTaskDelay(pdMS_TO_TICKS(100));
                lcd.backlight();
                break;
            case UNKNOWN_CARD:
                lcd.setCursor(0, 0);
                lcd.print("ERROR");
                lcd.setCursor(0, 1);
                lcd.print("UNKNOWN CARD");
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(200)); // Cập nhật LCD mỗi 200ms
    }
}