#include "global_instances.h"

// Initialize global instances
CustomBLEService bleService;
PreviewService previewService(&bleService);
DataCollector collector(&bleService);

#ifdef PRODUCTION_MODE
ModelInference inference;
#endif

DISPLAY_MODEL u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/SCL, /* data=*/SDA);

// Define global references
DisplayManager &display = DisplayManager::getInstance();
BuzzerManager &buzzer = BuzzerManager::getInstance();
RTCManager &rtc = RTCManager::getInstance();