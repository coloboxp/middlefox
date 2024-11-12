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
    Camera::Camera *camera;
    unsigned long lastCapture;
    int imageCount;
    CustomBLEService *bleService;
    int getNextImageCount();
    bool initSD();
    static const char *TAG;
    static bool convert_rgb565_to_jpeg(const uint8_t *rgb565_data, int width, int height, 
                                     uint8_t **jpg_buf_out, size_t *jpg_len_out);
};