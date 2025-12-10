#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <ESP32Servo.h>

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;
extern Servo g_doorServo; 

// System state enum (used by logic/lcd modules)
enum SystemState {
	INITIAL,
	LOCKED,
	ENTERING_PASSWORD,
	CHECKING_PASSWORD,
	UNLOCKED,
	INCORRECT_PASSWORD,
	SYSTEM_LOCKED_DOWN,
	ERROR,
	FIRE_ALARM,
	UNKNOWN_CARD
};

enum TemperState{
	TEMP_LOW,
	TEMP_MEDIUM,
	TEMP_HIGH
};

enum HumiState {
    HUMI_LOW,
    HUMI_MEDIUM,
    HUMI_HIGH
};

// Globals from "global copy.cpp"
extern int button1;
extern bool taskFlag;
extern int x;
// Biến lưu thông tin người/cách thức vừa mở cửa
extern String g_unlockSource;
// Mặc định chưa ai mở
extern String g_unlockSource;

// Door / password state globals
extern SystemState g_systemState;
extern TemperState g_temperState;
extern HumiState g_humiState;
extern char g_enteredPassword[7];
extern int g_passwordIndex;
extern char g_newKey;
extern bool g_keyReady;
extern int g_wrongAttempts;
extern int g_lockoutTimer;
extern bool g_doorState;

// === CÁC THAY ĐỔI QUAN TRỌNG BẮT ĐẦU TỪ ĐÂY ===

// Mutex để bảo vệ logic phím, trạng thái hệ thống, và cửa
extern SemaphoreHandle_t g_logicMutex;

// Mutex để bảo vệ trạng thái cảm biến (temp, humi)
extern SemaphoreHandle_t g_sensorMutex;

// Mutex để bảo vệ cổng Serial (tránh bị trộn chữ)
extern SemaphoreHandle_t g_serialMutex;

// Semaphore để báo hiệu cho task cửa (thay vì polling)
extern SemaphoreHandle_t g_doorSemaphore;

// Semaphores cho LED và NeoPixel (như cũ)
extern SemaphoreHandle_t xTempSemaphore;
extern SemaphoreHandle_t xHumiSemaphore;

//======== CẤU HÌNH CHO IOT============
extern const char* MQTT_SERVER;
extern const int   MQTT_PORT;
extern const char* MQTT_CLIENT_ID;
extern const char* MQTT_USER;
extern const char* MQTT_PASS;

extern const char* TOPIC_TELEMETRY;
extern const char* TOPIC_RPC_SUB;

#endif