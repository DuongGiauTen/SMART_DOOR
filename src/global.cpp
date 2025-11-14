#include "global.h"
#include <ESP32Servo.h>

float glob_temperature = 0;
float glob_humidity = 0;

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

// --- Merged from global copy.cpp ---
int button1 = 0;
bool taskFlag = false;
int x = 0;

// Định nghĩa (cấp phát bộ nhớ) cho các biến toàn cục liên quan đến khóa/cửa
SystemState g_systemState = INITIAL; 
char g_enteredPassword[7] = {0};
int g_passwordIndex = 0;
char g_newKey = 0;
bool g_keyReady = false;
int g_wrongAttempts = 0;
int g_lockoutTimer = 50;
bool g_doorState = false; // Mặc định cửa đóng
Servo g_doorServo;


// === CÁC THAY ĐỔI QUAN TRỌNG BẮT ĐẦU TỪ ĐÂY ===

// Khởi tạo tất cả handle là NULL
SemaphoreHandle_t g_logicMutex = NULL;
SemaphoreHandle_t g_sensorMutex = NULL;
SemaphoreHandle_t g_serialMutex = NULL;
SemaphoreHandle_t g_doorSemaphore = NULL;
SemaphoreHandle_t xTempSemaphore = NULL;
SemaphoreHandle_t xHumiSemaphore = NULL;

TemperState g_temperState = TEMP_LOW;
HumiState g_humiState = HUMI_LOW;