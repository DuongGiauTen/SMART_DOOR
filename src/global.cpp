#include "global.h"

int button1 = 0;
bool taskFlag = false;
int x = 0;

// Định nghĩa (cấp phát bộ nhớ) cho các biến toàn cục
SystemState g_systemState = INITIAL; // <-- THAY ĐỔI
char g_enteredPassword[7] = {0};
int g_passwordIndex = 0;
char g_newKey = 0;
bool g_keyReady = false;
int g_wrongAttempts = 0;
int g_lockoutTimer = 50;
bool g_doorState = false; // Mặc định cửa đóng