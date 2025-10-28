#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "lcd.h"
#include "global.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcd_task(void *pvParameters) {
    Wire.begin(22, 21);
    lcd.init();
    lcd.backlight();

    SystemState lastDisplayedState = (SystemState)-1;
    
    while(1) {
        if (g_systemState != lastDisplayedState) {
            lcd.clear();
            lastDisplayedState = g_systemState;
        }

        switch(g_systemState) {
            // ================== PHẦN THÊM MỚI ==================
            case INITIAL:
                lcd.setCursor(0, 0);
                lcd.print("C: UNLOCK DOOR");
                lcd.setCursor(0, 1);
                lcd.print("B: CHANGE PASS");
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
                for (int i = 0; i < g_passwordIndex; i++) {
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
                lcd.printf("Attempts: %d/3", g_wrongAttempts);
                break;

            case SYSTEM_LOCKED_DOWN:
                lcd.setCursor(0, 0);
                lcd.print("System Locked!");
                lcd.setCursor(0, 1);
                lcd.printf("Wait: %2d sec", g_lockoutTimer); 
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