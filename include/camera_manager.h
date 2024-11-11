#pragma once

#include <Arduino.h>
#include "config.h"  // Must be first for camera model selection
#include <eloquent_esp32cam.h>
#include "esp_log.h"

using namespace Eloquent::Esp32cam;

class CameraManager {
public:
    static CameraManager& getInstance() {
        static CameraManager instance;
        return instance;
    }

    bool begin(bool isPreviewMode = false);
    Camera::Camera* getCamera() { return &camera; }
    
private:
    CameraManager() {}
    static const char* TAG;
    Camera::Camera camera;
    bool initialized = false;
    
    bool initializePins();
    bool configureCamera(bool isPreviewMode);
}; 