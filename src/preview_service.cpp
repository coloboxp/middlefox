#include "preview_service.h"

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
    ESP_LOGV(TAG, "WiFi Mode: %d", WiFi.getMode());

    // Configure AP
    String ssid = HOSTNAME;
    ESP_LOGD(TAG, "Generated SSID: %s", ssid.c_str());

    if (!WiFi.softAP(ssid.c_str(), ""))
    {
        ESP_LOGE(TAG, "Failed to create Access Point - Check WiFi configuration");
        return false;
    }

    delay(100); // Wait for AP to start
    IPAddress ip = WiFi.softAPIP();

    JsonDocument doc;
    doc["status"] = "enabled";
    doc["ssid"] = ssid.c_str();
    doc["ip"] = ip.toString().c_str();
    doc["channel"] = WiFi.channel();
    doc["stream_url"] = streamAddress.c_str();

    std::string infoJson;
    serializeJsonPretty(doc, infoJson);
    bleService->updatePreviewInfo(infoJson);

    return true;
}

bool PreviewService::initMJPEGServer()
{
    ESP_LOGD(TAG, "Initializing MJPEG server...");

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
    ESP_LOGI(TAG, "Enabling preview stream");

    if (streamEnabled)
    {
        ESP_LOGD(TAG, "Stream already enabled");
        return;
    }

    if (!initWiFi())
    {
        ESP_LOGE(TAG, "Failed to start WiFi AP");
        return;
    }

    if (!initMJPEGServer())
    {
        ESP_LOGE(TAG, "Failed to start MJPEG server");
        stopWiFi();
        return;
    }

    ESP_LOGV(TAG, "State transition: %d -> 1", streamEnabled);
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