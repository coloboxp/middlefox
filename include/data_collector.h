#pragma once

#include <Arduino.h>
#include "config.h"
#include <SD.h>
#include <eloquent_esp32cam.h>
#include "esp_log.h"
#include "camera_manager.h"

class DataCollector
{
public:
    DataCollector();
    bool begin();
    void loop();

private:
    unsigned long lastCapture;
    int imageCount;
    Camera::Camera *camera;

    bool initSD();
    int getNextImageCount();
};