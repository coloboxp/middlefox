#pragma once

// Operation modes
enum OperationMode
{
    DATA_COLLECTION,
    PRODUCTION
};

// Camera settings
#define CAMERA_MODEL_XIAO_ESP32S3
#define LED_PIN 21

// Time intervals
#define CAPTURE_INTERVAL_MS 5000 // 5 seconds

// File paths and formats
#define IMAGE_PREFIX "picture"
#define RGB_EXTENSION ".rgb"
#define JPG_EXTENSION ".jpg"
