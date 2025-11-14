#include "temp_humi_monitor.h"
#include "global.h"
#include "lcd.h"
DHT20 dht20;



void read_real_sensors() {
    // === ĐÂY LÀ CODE GIẢ LẬP ===
    // Trong dự án thật, bạn sẽ dùng thư viện DHT.h hoặc SHT.h ở đây
    // và cập nhật 2 biến global
    dht20.read();
        // Reading temperature in Celsius
    float temperature = dht20.getTemperature();
        // Reading humidity
    float humidity = dht20.getHumidity();

        // Update global variables
    glob_temperature = temperature;
    glob_humidity = humidity;

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
    
}


void temp_humi_monitor(void *pvParameters) {
    // dht.begin(); // Ví dụ
    
    while (1) {
        // 1. Đọc giá trị từ cảm biến
        // Chú ý: Việc đọc cảm biến có thể tốn thời gian,
        // không nên đặt trong mutex.
        read_real_sensors();
        float temp = glob_temperature; // Đọc giá trị thật: dht.readTemperature();
        float humi = glob_humidity; // Đọc giá trị thật: dht.readHumidity();

        // 2. Xử lý trạng thái mới
        TemperState newTempState;
        if (temp < 25.0) newTempState = TEMP_LOW;
        else if (temp < 30.0) newTempState = TEMP_MEDIUM;
        else newTempState = TEMP_HIGH;

        HumiState newHumiState;
        if (humi < 65.0) newHumiState = HUMI_LOW;
        else if (humi < 70.0) newHumiState = HUMI_MEDIUM;
        else newHumiState = HUMI_HIGH;

        bool tempChanged = false;
        bool humiChanged = false;

        // 3. Cập nhật biến global (PHẢI DÙNG MUTEX)
        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            
            // Kiểm tra xem trạng thái CÓ THAY ĐỔI KHÔNG
            if (newTempState != g_temperState) {
                g_temperState = newTempState;
                tempChanged = true;
            }
            if (newHumiState != g_humiState) {
                g_humiState = newHumiState;
                humiChanged = true;
            }
            
            // Trả mutex
            xSemaphoreGive(g_mutex);

        } else {
            Serial.println("temp_humi_monitor: FAILED to get mutex!");
        }

        // 4. GỬI TÍN HIỆU (BÊN NGOÀI MUTEX)
        // Chỉ gửi tín hiệu nếu trạng thái đã thực sự thay đổi
        if (tempChanged) {
            Serial.println("temp_humi_monitor: Sending Temp signal!");
            xSemaphoreGive(xTempSemaphore);
        }
        if (humiChanged) {
            Serial.println("temp_humi_monitor: Sending Humi signal!");
            xSemaphoreGive(xHumiSemaphore);
        }

        // Chờ 5 giây cho lần đọc tiếp theo
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}