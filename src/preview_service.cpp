#include "preview_service.h"

#define WIFI_SSID HOSTNAME
#define WIFI_PASS ""

const char *PreviewService::TAG = "PreviewService";

PreviewService::PreviewService(CustomBLEService *ble) : streamEnabled(false), camera(nullptr), bleService(ble)
{
    ESP_LOGI(TAG, "Creating PreviewService instance");
    
    bleService->setPreviewCallback([this](bool enabled) {
        ESP_LOGI(TAG, "Preview callback triggered: %s", enabled ? "ENABLE" : "DISABLE");
        if (enabled) {
            streamEnabled = true;
        } else {
            this->disable();
            streamEnabled = false;
            
            // Only restart if explicitly stopped and not disconnecting
            if (bleService->wasExplicitlyStopped() && !bleService->isDisconnecting()) {
                ESP_LOGI(TAG, "Explicit stop command received - Triggering restart");
                bleService->notifyClients("Restarting device to idle state...");
                delay(1000);
                ESP.restart();
            }
        }
    });
}

bool PreviewService::begin()
{
    ESP_LOGI(TAG, "=== Starting PreviewService Initialization ===");
    ESP_LOGV(TAG, "Memory free: %lu bytes", ESP.getFreeHeap());

    // Get camera instance first
    camera = CameraManager::getInstance().getCamera();
    if (!camera)
    {
        ESP_LOGE(TAG, "Failed to get camera instance - Camera not initialized");
        return false;
    }

    ESP_LOGD(TAG, "Camera instance acquired successfully");
    
    // Configure camera for preview mode
    if (!CameraManager::getInstance().begin(true)) // true for preview mode
    {
        ESP_LOGE(TAG, "Failed to initialize camera in preview mode");
        return false;
    }

    ESP_LOGI(TAG, "=== PreviewService Ready ===");
    return true;
}

bool PreviewService::initWiFi()
{
    ESP_LOGD(TAG, "Setting up WiFi Access Point...");

    String ssid = HOSTNAME;

    if (!WiFi.softAP(ssid.c_str(), ""))
    {
        ESP_LOGE(TAG, "Failed to create Access Point");
        return false;
    }

    delay(100); // Wait for AP to start
    IPAddress ip = WiFi.softAPIP();
    ESP_LOGI(TAG, "WiFi AP started with IP: %s", ip.toString().c_str());

    return true;
}

bool PreviewService::initMJPEGServer()
{
    ESP_LOGD(TAG, "Initializing MJPEG server...");

    // Ensure camera is in preview mode
    camera->resolution.face();
    camera->quality.high();
    camera->pixformat.jpeg();  // Explicitly set JPEG format
    
    delay(100); // Small delay to ensure camera settings are applied
    
    if (!mjpeg.begin().isOk())
    {
        ESP_LOGE(TAG, "MJPEG server initialization failed: %s", mjpeg.exception.toString().c_str());
        return false;
    }

    streamAddress = mjpeg.address();
    ESP_LOGI(TAG, "Stream available at: %s", streamAddress.c_str());
    return true;
}

void PreviewService::loop()
{
    static bool lastState = false;
    static unsigned long lastMetricsLog = 0;
    const unsigned long METRICS_INTERVAL = 30000;

    // Handle state changes
    if (streamEnabled != lastState) {
        ESP_LOGI(TAG, "Stream state changed: %s", streamEnabled ? "ENABLED" : "DISABLED");
        
        if (streamEnabled) {
            // Initialize camera first if needed
            if (!camera && !begin()) {
                ESP_LOGE(TAG, "Failed to initialize preview service");
                streamEnabled = false;
                return;
            }

            // Initialize WiFi and MJPEG server
            if (!initWiFi()) {
                ESP_LOGE(TAG, "Failed to start WiFi AP");
                streamEnabled = false;
                return;
            }

            if (!initMJPEGServer()) {
                ESP_LOGE(TAG, "Failed to start MJPEG server");
                stopWiFi();
                streamEnabled = false;
                return;
            }

            // Update BLE with stream info
            JsonDocument doc;
            doc["status"] = "enabled";
            doc["wifi"]["ssid"] = String(HOSTNAME);
            doc["wifi"]["ip"] = WiFi.softAPIP().toString();
            doc["stream"]["url"] = String(streamAddress);
            doc["stream"]["type"] = "MJPEG";
            doc["stream"]["port"] = 81;

            String output;
            serializeJson(doc, output);
            bleService->updatePreviewInfo(output.c_str());

            ESP_LOGI(TAG, "Preview service enabled successfully");
        } else {
            disable();
        }
        
        lastState = streamEnabled;
    }

    // Only process stream if enabled
    if (streamEnabled) {
        // Handle metrics logging
        if (millis() - lastMetricsLog > METRICS_INTERVAL) {
            JsonDocument doc;
            doc["clients"] = WiFi.softAPgetStationNum();
            doc["heap"] = ESP.getFreeHeap();
            doc["psram"] = ESP.getFreePsram();

            std::string metrics;
            serializeJsonPretty(doc, metrics);
            bleService->updateServiceMetrics("preview", metrics);
            lastMetricsLog = millis();
        }
    }
}

void PreviewService::enable()
{
    static unsigned long lastEnableAttempt = 0;
    const unsigned long DEBOUNCE_TIME = 1000;
    
    unsigned long currentTime = millis();
    if (currentTime - lastEnableAttempt < DEBOUNCE_TIME) {
        ESP_LOGW(TAG, "Enable request ignored - too soon after last attempt");
        return;
    }
    lastEnableAttempt = currentTime;
    
    // Just set the flag, let loop() handle the initialization
    streamEnabled = true;
}

void PreviewService::disable()
{
    ESP_LOGI(TAG, "Disabling preview stream");

    if (!streamEnabled)
    {
        ESP_LOGD(TAG, "Stream already disabled");
        return;
    }

    // Set flag first to prevent re-entrancy
    streamEnabled = false;
    ESP_LOGV(TAG, "State transition: enabled -> disabled");

    // Stop services in order
    stopMJPEGServer();
    stopWiFi();

    // Reset camera mode
    if (camera) {
        CameraManager::getInstance().begin(false); // false for normal mode
    }

    JsonDocument doc;
    doc["status"] = "disabled";
    doc["message"] = "Preview service stopped";

    std::string infoJson;
    serializeJsonPretty(doc, infoJson);
    bleService->updatePreviewInfo(infoJson);
    
    ESP_LOGI(TAG, "Preview service disabled successfully");
}

void PreviewService::stopWiFi()
{
    ESP_LOGD(TAG, "Stopping WiFi AP");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
}

void PreviewService::stopMJPEGServer()
{
    ESP_LOGD(TAG, "Stopping MJPEG server");
    mjpeg.stop();
}