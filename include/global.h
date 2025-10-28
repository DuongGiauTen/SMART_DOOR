#ifndef GLOBAL_H
#define GLOBAL_H

// Include necessary headers
#include <Arduino.h>
// FreeRTOS semaphore type
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

enum SystemState {
    INITIAL, // <-- THÊM MỚI
    LOCKED,
    ENTERING_PASSWORD,
    CHECKING_PASSWORD,
    UNLOCKED,
    INCORRECT_PASSWORD,
    SYSTEM_LOCKED_DOWN,
    ERROR
};

// Khai báo các biến toàn cục để các file khác có thể sử dụng
extern SystemState g_systemState;
extern char g_enteredPassword[7];
extern int g_passwordIndex;
extern char g_newKey;
extern bool g_keyReady;
extern int g_wrongAttempts;
extern int g_lockoutTimer;
extern bool g_doorState; // true = Mở, false = Đóng

// Semaphore to protect access to global variables shared across tasks
extern SemaphoreHandle_t g_mutex;

// Global variables
extern int button1; // Example global variable
extern bool taskFlag;      // Example flag for task communication
extern int x;
// Function prototypes (if needed)
//void initializeGlobals();

#endif // GLOBAL_H