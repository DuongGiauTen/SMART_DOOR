#ifndef TASK_RFID_H
#define TASK_RFID_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "global.h" // Để truy cập biến cửa và mutex

void rfid_task(void *pvParameters);

#endif