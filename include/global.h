#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// System state enum (used by logic/lcd modules)
enum SystemState {
	INITIAL,
	LOCKED,
	ENTERING_PASSWORD,
	CHECKING_PASSWORD,
	UNLOCKED,
	INCORRECT_PASSWORD,
	SYSTEM_LOCKED_DOWN,
	ERROR
};

// Globals from "global copy.cpp"
extern int button1;
extern bool taskFlag;
extern int x;

// Door / password state globals
extern SystemState g_systemState;
extern char g_enteredPassword[7];
extern int g_passwordIndex;
extern char g_newKey;
extern bool g_keyReady;
extern int g_wrongAttempts;
extern int g_lockoutTimer;
extern bool g_doorState;

// Mutex to protect shared globals
extern SemaphoreHandle_t g_mutex;
#endif