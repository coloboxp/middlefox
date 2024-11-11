#include "camera_manager.h"

const char *CameraManager::TAG = "CameraManager";

bool CameraManager::begin(bool isPreviewMode)
{
    if (initialized)
    {
        ESP_LOGW(TAG, "Camera already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing camera...");

    if (!initializePins())
    {
        ESP_LOGE(TAG, "Failed to initialize camera pins");
        return false;
    }

    if (!configureCamera(isPreviewMode))
    {
        ESP_LOGE(TAG, "Failed to configure camera");
        return false;
    }

    if (!camera.begin().isOk())
    {
        ESP_LOGE(TAG, "Camera initialization failed: %s", camera.exception.toString().c_str());
        return false;
    }

    initialized = true;
    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

bool CameraManager::initializePins()
{
    ESP_LOGD(TAG, "Configuring camera pins");
    camera.pinout.pins.d0 = Y2_GPIO_NUM;
    camera.pinout.pins.d1 = Y3_GPIO_NUM;
    camera.pinout.pins.d2 = Y4_GPIO_NUM;
    camera.pinout.pins.d3 = Y5_GPIO_NUM;
    camera.pinout.pins.d4 = Y6_GPIO_NUM;
    camera.pinout.pins.d5 = Y7_GPIO_NUM;
    camera.pinout.pins.d6 = Y8_GPIO_NUM;
    camera.pinout.pins.d7 = Y9_GPIO_NUM;
    camera.pinout.pins.xclk = XCLK_GPIO_NUM;
    camera.pinout.pins.pclk = PCLK_GPIO_NUM;
    camera.pinout.pins.vsync = VSYNC_GPIO_NUM;
    camera.pinout.pins.href = HREF_GPIO_NUM;
    camera.pinout.pins.sccb_sda = SIOD_GPIO_NUM;
    camera.pinout.pins.sccb_scl = SIOC_GPIO_NUM;
    camera.pinout.pins.pwdn = PWDN_GPIO_NUM;
    camera.pinout.pins.reset = RESET_GPIO_NUM;
    return true;
}

bool CameraManager::configureCamera(bool isPreviewMode)
{
    ESP_LOGD(TAG, "Configuring camera for %s mode", isPreviewMode ? "preview" : "capture");

    camera.pixformat.rgb565();
    camera.resolution.face();
    camera.quality.best();
    camera.sensor.enableAutomaticExposureControl();
    camera.sensor.enableAutomaticGainControl();
    camera.sensor.enableAutomaticWhiteBalance();
    camera.sensor.enableAutomaticWhiteBalanceGain();

    return true;
}