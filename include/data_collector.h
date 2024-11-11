#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ble_service.h"
#include "camera_manager.h"
#include "config.h"
#include "esp_log.h"
#include <SD.h>

class DataCollector
{
public:
    DataCollector(CustomBLEService *ble);
    bool begin();
    void loop();

private:
    static const char *TAG;
    Camera::Camera *camera;
    unsigned long lastCapture;
    int imageCount;
    CustomBLEService *bleService;
    int getNextImageCount();
    bool initSD();
};