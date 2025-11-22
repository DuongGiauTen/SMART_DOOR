#ifndef __TEMP_HUMI_MONITOR__
#define __TEMP_HUMI_MONITOR__

#define FIRE_THRESHOLD 30.0  // Ngưỡng nhiệt độ để báo cháy
#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "DHT20.h"
#include "global.h"

void temp_humi_monitor(void *pvParameters);


#endif