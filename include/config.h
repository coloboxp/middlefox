#pragma once

// Camera model selection - MUST BE BEFORE ANY CAMERA INCLUDES
#define CAMERA_MODEL_XIAO_ESP32S3
#define DISPLAY_MODEL U8G2_SSD1306_128X64_NONAME_1_HW_I2C
#include "camera_pins.h"

// Operation modes
enum OperationMode
{
    DATA_COLLECTION,
    PRODUCTION
};

#define HOSTNAME "MiddleFox"
// Camera settings
#define LED_PIN 21

// Time intervals
#define CAPTURE_INTERVAL_MS 5000 // 5 seconds

// File paths and formats
#define IMAGE_PREFIX "picture"
#define RGB_EXTENSION ".rgb"
#define JPG_EXTENSION ".jpg"
