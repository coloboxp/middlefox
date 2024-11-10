#pragma once

#include <Arduino.h>
#include <SD.h>
#include "esp_camera.h"
#include "config.h"
#include <eloquent_esp32cam.h>
#include "esp_log.h"
#include "camera_pins.h"

class DataCollector
{
public:
    DataCollector();
    bool begin();
    void loop();

private:
    unsigned long lastCapture;
    int imageCount;
    int getNextImageCount();
    bool initCamera();
    bool initSD();
    bool saveRGBFile(camera_fb_t *fb);
    bool saveJPEGFile(camera_fb_t *fb);
    bool convertToJPEG(const uint8_t *rgb565_data, int width, int height,
                       uint8_t **jpg_buf_out, size_t *jpg_len_out);
};