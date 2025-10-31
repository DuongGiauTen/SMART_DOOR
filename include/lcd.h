#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

void lcd_task(void *pvParameters);
extern LiquidCrystal_I2C lcd;


#endif // LCD_H