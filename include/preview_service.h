#pragma once

#include <Arduino.h>
#include "config.h"
#include <WiFi.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/viz/mjpeg.h>
#include "esp_log.h"
#include "camera_manager.h"
#include "ble_service.h"
#include <ArduinoJson.h>

using namespace Eloquent::Esp32cam;
using namespace Eloquent::Esp32cam::Viz;

class PreviewService
{
public:
    PreviewService(CustomBLEService* ble);
    bool begin();
    void loop();
    void enable();
    void disable();
    bool isEnabled() { return streamEnabled; }
    String getStreamAddress() { return streamAddress; }

private:
    static const char *TAG;
    bool streamEnabled;
    String streamAddress;
    bool initWiFi();
    bool initMJPEGServer();
    void stopWiFi();
    void stopMJPEGServer();
    Camera::Camera *camera;
    Mjpeg mjpeg;
    CustomBLEService* bleService;
};