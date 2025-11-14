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
#define DOOR_SERVO_PIN 38 // !!! THAY BẰNG CHÂN GPIO BẠN NỐI DÂY TÍN HIỆU
#define ANGLE_CLOSED 0    // Góc khi cửa ĐÓNG (ví dụ 0 độ)
#define ANGLE_OPEN 90     // Góc khi cửa MỞ (ví dụ 90 độ)   

void setup()
{
  Serial.begin(115200);
  check_info_File(0);
  Wire.begin(11, 12); //SDA, SCL
   
  // === TẠO CÁC SEMAPHORE ===
  g_logicMutex = xSemaphoreCreateMutex();
  g_sensorMutex = xSemaphoreCreateMutex();
  g_serialMutex = xSemaphoreCreateMutex();
  
  g_doorSemaphore = xSemaphoreCreateBinary();
  xTempSemaphore = xSemaphoreCreateBinary();
  xHumiSemaphore = xSemaphoreCreateBinary(); 

  // Khởi tạo servo 1 LẦN DUY NHẤT ở đây
  g_doorServo.attach(DOOR_SERVO_PIN, 544, 2400); // Gắn servo vào chân
  g_doorServo.write(ANGLE_CLOSED); // Đảm bảo cửa ĐÓNG khi khởi động

  // Kiểm tra tất cả
  if (g_logicMutex == NULL || g_sensorMutex == NULL || g_serialMutex == NULL ||
      g_doorSemaphore == NULL || xTempSemaphore == NULL || xHumiSemaphore == NULL) { 
    Serial.println("FATAL ERROR: Failed to create semaphores");
    // Ở đây bạn có thể dừng hoặc khởi động lại
  }

  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
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