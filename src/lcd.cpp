#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "lcd.h"
#include "global.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcd_task(void *pvParameters) {
    
    lcd.init();
    lcd.backlight();

    SystemState lastDisplayedState = (SystemState)-1;
    
    while(1) {
        // Read system state atomically to avoid tearing
        SystemState sysLocal;
        if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            sysLocal = g_systemState;
            xSemaphoreGive(g_mutex);
        } else {
            // Không lấy được mutex để đọc trạng thái -> không cập nhật hiển thị trong vòng này
            sysLocal = lastDisplayedState; // giữ nguyên để không clear/rewrite
        }

        if (sysLocal != lastDisplayedState) {
            lcd.clear();
            lastDisplayedState = sysLocal;
        }

        switch(sysLocal) {
            // ================== PHẦN THÊM MỚI ==================
            case INITIAL:
                lcd.setCursor(0, 0);
                // Avoid using printf with %f (pulls heavy float printf code and
                // increases stack usage). Use print with fixed decimals instead.
                lcd.setCursor(0, 0);
                lcd.print("Temperature:");
                lcd.print(glob_temperature, 2); // 2 decimal places
                lcd.setCursor(0, 1);
                lcd.print("Humidity:");
                lcd.print(glob_humidity, 2);
                break;
            // ===================================================

            case LOCKED:
                lcd.setCursor(0, 0);
                lcd.print("Enter Password:");
                break;

            case ENTERING_PASSWORD:
                lcd.setCursor(0, 0);
                lcd.print("Enter Password:");
                lcd.setCursor(0, 1);
                // Read password index under mutex
                {
                    int idx = 0;
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        idx = g_passwordIndex;
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex -> bỏ qua hiển thị dấu * (giữ trống)
                        idx = 0;
                    }
                    for (int i = 0; i < idx; i++) {
                        lcd.print("*");
                    }
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
                {
                    int attempts = 0;
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        attempts = g_wrongAttempts;
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex -> bỏ qua, hiển thị 0
                        attempts = 0;
                    }
                    lcd.print("Attempts: ");
                    lcd.print(attempts);
                    lcd.print("/3");
                }
                break;

            case SYSTEM_LOCKED_DOWN:
                lcd.setCursor(0, 0);
                lcd.print("System Locked!");
                lcd.setCursor(0, 1);
                {
                    int timerLocal = 0;
                    if (g_mutex != NULL && xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                        timerLocal = g_lockoutTimer;
                        xSemaphoreGive(g_mutex);
                    } else {
                        // Không lấy được mutex -> bỏ qua, hiển thị 0
                        timerLocal = 0;
                    }
                    lcd.print("Wait: ");
                    lcd.print(timerLocal);
                    lcd.print(" sec");
                }
                lastDisplayedState = (SystemState)-1;
                break;
            case ERROR:
                lcd.setCursor(0, 0);
                lcd.print("LOCK ABORTED");
                lcd.setCursor(0, 1);
                lcd.print("Waiting...");
                break;

            
            case CHECKING_PASSWORD:
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}