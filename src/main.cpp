#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
// #include "tinyml.h"
// #include "coreiot.h"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "button.h"
#include "lcd.h"
#include "door.h"
#include "logic.h"    

void setup()
{
  Serial.begin(115200);
  check_info_File(0);
  Wire.begin(11, 12); //SDA, SCL
   
  g_mutex = xSemaphoreCreateMutex();
  if (g_mutex == NULL) {
    Serial.println("Failed to create mutex");
    // Optionally handle error: halt or continue without mutex
  }

  //xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  //xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  //xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);

  //new Task
  xTaskCreate(keypad_task, "KeyPad Task", 2048, NULL, 2, NULL);
  xTaskCreate(lcd_task, "LCD Task", 2048, NULL, 2, NULL);
  xTaskCreate(door_task, "Door Task", 2048, NULL, 2, NULL);
  xTaskCreate(logic_task, "Logic Task", 4096, NULL, 3, NULL);
}

void loop()
{
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();
}