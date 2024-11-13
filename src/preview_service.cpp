#include "preview_service.h"

#define WIFI_SSID HOSTNAME
#define WIFI_PASS ""

const char *PreviewService::TAG = "PreviewService";

PreviewService::PreviewService(CustomBLEService *ble) : streamEnabled(false), camera(nullptr), bleService(ble)
{
    ESP_LOGI(TAG, "Creating PreviewService instance");
    ESP_LOGV(TAG, "Initial state: streamEnabled=%d, camera=nullptr", streamEnabled);
}

bool PreviewService::begin()
{
    ESP_LOGI(TAG, "=== Starting PreviewService Initialization ===");
    ESP_LOGV(TAG, "Memory free: %lu bytes", ESP.getFreeHeap());

    camera = CameraManager::getInstance().getCamera();
    if (!camera)
    {
        ESP_LOGE(TAG, "Failed to get camera instance - Camera not initialized");
        return false;
    }

    ESP_LOGD(TAG, "Camera instance acquired successfully");

    ESP_LOGI(TAG, "=== PreviewService Ready ===");
    ESP_LOGV(TAG, "Final memory free: %lu bytes", ESP.getFreeHeap());
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

    // Configure camera for streaming
    camera->resolution.vga();
    camera->quality.high();
    
    if (!mjpeg.begin().isOk())
    {
        ESP_LOGE(TAG, "MJPEG server initialization failed: %s", mjpeg.exception.toString().c_str());
        return false;
    }

    streamAddress = mjpeg.address();
    ESP_LOGI(TAG, "Stream available at: %s", streamAddress.c_str());
    ESP_LOGV(TAG, "Server metrics - Heap: %lu, PSRAM: %lu", ESP.getFreeHeap(), ESP.getFreePsram());

    return true;
}

void PreviewService::enable()
{
    static unsigned long lastEnableAttempt = 0;
    const unsigned long DEBOUNCE_TIME = 1000; // 1 second debounce
    
    unsigned long currentTime = millis();
    
    // Add timing check to prevent rapid repeated calls
    if (currentTime - lastEnableAttempt < DEBOUNCE_TIME) {
        ESP_LOGW(TAG, "Enable request ignored - too soon after last attempt (%lu ms)", 
                 currentTime - lastEnableAttempt);
        return;
    }
    
    lastEnableAttempt = currentTime;
    
    // Check if already enabled
    if (streamEnabled)
    {
        ESP_LOGW(TAG, "Stream already enabled - state: %d", streamEnabled);
        return;
    }

    ESP_LOGI(TAG, "Enabling preview stream");
    
    // Set flag first to prevent re-entrancy
    streamEnabled = true;

    if (!initWiFi())
    {
        ESP_LOGE(TAG, "Failed to start WiFi AP");
        streamEnabled = false;  // Reset flag on failure
        return;
    }

    if (!initMJPEGServer())
    {
        ESP_LOGE(TAG, "Failed to start MJPEG server");
        stopWiFi();
        streamEnabled = false;  // Reset flag on failure
        return;
    }

    // Create custom allocator for PSRAM
    struct PsramAllocator : ArduinoJson::Allocator
    {
        void *allocate(size_t size)
        {
            return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        }

        void deallocate(void *ptr)
        {
            heap_caps_free(ptr);
        }

        void *reallocate(void *ptr, size_t new_size)
        {
            return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
        }
    };

    // Create document with PSRAM allocator
    PsramAllocator allocator;
    JsonDocument doc(&allocator);

    // Basic status
    doc["status"] = "enabled";

    // WiFi information using to<JsonObject>()
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["ssid"] = String(HOSTNAME);
    wifi["ip"] = WiFi.softAPIP().toString();
    wifi["channel"] = WiFi.channel();

    // Stream information using to<JsonObject>()
    JsonObject stream = doc["stream"].to<JsonObject>();
    stream["url"] = String(streamAddress);
    stream["type"] = "MJPEG";
    stream["port"] = 81;

    // Debug log before serialization
    String debugOutput;
    serializeJsonPretty(doc, debugOutput);
    ESP_LOGD(TAG, "Preview info to send: %s", debugOutput.c_str());

    // Convert to string for BLE
    String output;
    serializeJson(doc, output);
    bleService->updatePreviewInfo(output.c_str());

    ESP_LOGV(TAG, "State transition: disabled -> enabled");
}

void PreviewService::disable()
{
    ESP_LOGI(TAG, "Disabling preview stream");

    if (!streamEnabled)
    {
        ESP_LOGD(TAG, "Stream already disabled");
        return;
    }

    ESP_LOGV(TAG, "State transition: %d -> 0", streamEnabled);

    stopMJPEGServer();
    stopWiFi();
    streamEnabled = false;

    JsonDocument doc;
    doc["status"] = "Preview service not enabled";

    std::string infoJson;
    serializeJsonPretty(doc, infoJson);
    bleService->updatePreviewInfo(infoJson);
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

void PreviewService::loop()
{
    static bool lastState = false;
    static unsigned long lastMetricsLog = 0;
    const unsigned long METRICS_INTERVAL = 30000; // Log metrics every 30 seconds

    // Log state changes
    if (streamEnabled != lastState)
    {
        ESP_LOGI(TAG, "Stream state changed: %s", streamEnabled ? "ENABLED" : "DISABLED");
        ESP_LOGD(TAG, "Connected clients: %d", WiFi.softAPgetStationNum());
        lastState = streamEnabled;
    }

    // Periodic metrics logging
    if (streamEnabled && millis() - lastMetricsLog > METRICS_INTERVAL)
    {
        ESP_LOGD(TAG, "Stream metrics - Clients: %d, Heap: %lu, PSRAM: %lu",
                 WiFi.softAPgetStationNum(),
                 ESP.getFreeHeap(),
                 ESP.getFreePsram());

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