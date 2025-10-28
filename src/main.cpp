#include <Arduino.h>
#include "led_blinky.h"
#include "button.h"
#include <Keypad.h>
#include "lcd.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "door.h"
#include "logic.h"
// Include globals (contains semaphore declaration)
#include "global.h"

// const byte ROWS = 4; //four rows
// const byte COLS = 4; //four columns
// //define the cymbols on the buttons of the keypads
// char hexaKeys[ROWS][COLS] = {
//   {'0','1','2','3'},
//   {'4','5','6','7'},
//   {'8','9','A','B'},
//   {'C','D','E','F'}
// };
// byte rowPins[ROWS] = {27, 14, 12, 13}; //connect to the row pinouts of the keypad
// byte colPins[COLS] = {26, 25, 33, 32}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad


void setup(){
  Serial.begin(115200);
  // Create mutex for protecting global variables
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    Serial.println("Failed to create mutex");
    // Optionally handle error: halt or continue without mutex
  }

  // Create key queue: buffer up to 16 keys (tune as needed)
  g_keyQueue = xQueueCreate(16, sizeof(char));
  if (g_keyQueue == NULL) {
    Serial.println("Failed to create key queue");
  }

  //xTaskCreate(led_blinky, "Task LED Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(keypad_task, "KeyPad Task", 2048, NULL, 2, NULL);
  xTaskCreate(lcd_task, "LCD Task", 2048, NULL, 2, NULL);
  xTaskCreate(door_task, "Door Task", 2048, NULL, 2, NULL);
  xTaskCreate(logic_task, "Logic Task", 4096, NULL, 3, NULL);
}
  
void loop(){

}