#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ble_service.h"
#include "camera_manager.h"
#include "config.h"
#include "esp_log.h"
#include "sd_manager.h"
#include "buzzer_manager.h"

class DataCollector
{
public:
    DataCollector(CustomBLEService *ble);
    ~DataCollector();
    bool begin();
    void loop();
    void cleanup();

private:
    Camera::Camera *camera;
    unsigned long lastCapture;
    int imageCount;
    CustomBLEService *bleService;
    static const char *TAG;
    SemaphoreHandle_t cameraMutex;
    bool initSD();
    int getNextImageCount();
    bool convert_rgb565_to_jpeg(const uint8_t *rgb565_data, int width, int height,
                                uint8_t **jpg_buf_out, size_t *jpg_len_out);
};