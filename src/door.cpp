#include <Arduino.h>
#include "door.h"
#include "global.h"

// Các define này đã được chuyển ra main.cpp, 
// nhưng để ở đây cũng không sao
#define ANGLE_CLOSED 0    
#define ANGLE_OPEN 90     

void door_task(void *pvParameters){
    
    // Khởi tạo servo đã được làm trong setup() của main.cpp
    // Task này chỉ chờ tín hiệu
    
    while (1) {
      
        
        // 1. NGỦ (Block) cho đến khi logic_task GỬI TÍN HIỆU
        if (xSemaphoreTake(g_doorSemaphore, portMAX_DELAY) == pdTRUE) {
            
            // 2. ĐÃ THỨC DẬY! Lấy mutex để đọc trạng thái MỚI NHẤT
            bool doorLocal = false;
            if (g_logicMutex != NULL && xSemaphoreTake(g_logicMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                doorLocal = g_doorState;
                xSemaphoreGive(g_logicMutex);
            }

            // 3. Hành động dựa trên trạng thái vừa đọc
            if (g_doorServo.attached()) {
                if (doorLocal == true) {
                    g_doorServo.write(ANGLE_OPEN); // Mở cửa
                } else {
                    g_doorServo.write(ANGLE_CLOSED); // Đóng cửa
                }


                // if (button1 == 1){
                //     g_doorServo.write(ANGLE_OPEN); // Mở cửa
                // }
                // else{
                //     g_doorServo.write(ANGLE_CLOSED); // Đóng cửa
                // } 

             
            }


            
        }
        // Không cần vTaskDelay, task sẽ tự động ngủ lại ở xSemaphoreTake
    }
}