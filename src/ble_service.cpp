#include "ble_service.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static const char *TAG = "BLEService";
bool CustomBLEService::captureEnabled = false;
bool CustomBLEService::previewEnabled = false;
bool CustomBLEService::inferenceEnabled = false;
CustomBLEService *globalBLEService = nullptr;

static const unsigned long STATUS_UPDATE_INTERVAL = 5000;  // 5 seconds for general status
static const unsigned long SERVICE_UPDATE_INTERVAL = 1000; // 1 second for service updates

class ControlCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        if (globalBLEService)
        {
            globalBLEService->handleControlCallback(pCharacteristic);
        }
    }
};

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        ESP_LOGI(TAG, "Client Connected - Address: %s",
                 NimBLEAddress(desc->peer_ota_addr).toString().c_str());
        if (globalBLEService)
        {
            globalBLEService->updateConnectionState(CustomBLEService::CONNECTED);
        }
    }

    void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
    {
        if (desc != nullptr)
        {
            ESP_LOGI(TAG, "Client Disconnected - Details:"
                          "\n\tPeer Address: %s"
                          "\n\tConnection Handle: %d"
                          "\n\tConnection Interval: %d ms"
                          "\n\tConnection Timeout: %d ms",
                     NimBLEAddress(desc->peer_ota_addr).toString().c_str(),
                     desc->conn_handle,
                     (desc->conn_itvl * 1.25),          // Convert to milliseconds
                     (desc->supervision_timeout * 10)); // Convert to milliseconds
        }
        else
        {
            ESP_LOGI(TAG, "Client Disconnected - No descriptor available");
        }

        if (globalBLEService)
        {
            globalBLEService->updateConnectionState(CustomBLEService::DISCONNECTED);
            globalBLEService->handleDisconnection();
        }

        pServer->startAdvertising();
    }
};

class PreviewInfoCallbacks : public NimBLECharacteristicCallbacks
{
    void onRead(NimBLECharacteristic *pCharacteristic)
    {
        ESP_LOGD("BLEService", "Preview info read: %s", pCharacteristic->getValue().c_str());
    }
};

CustomBLEService::CustomBLEService()
{
    globalBLEService = this;
    connectionState = DISCONNECTED;
    lastKeepAlive = millis();
    // Create mutex
    mutex = xSemaphoreCreateMutex();
}

void CustomBLEService::handleControlCallback(NimBLECharacteristic *pCharacteristic)
{
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0)
    {
        ESP_LOGD(TAG, "Received control value: %c (ASCII: %d)", value[0], (int)value[0]);

        switch (value[0])
        {
        case Command::START_PREVIEW: // Start Preview
            updateState(previewEnabled, true);
            if (previewCallback)
                previewCallback(true);
            notifyClients("Preview Started");
            ESP_LOGI(TAG, "Preview Started - Streaming enabled");
            break;
        case Command::STOP_PREVIEW: // Stop Preview
            previewEnabled = false;
            if (previewCallback)
                previewCallback(false);
            notifyClients("Preview Stopped");
            ESP_LOGI(TAG, "Preview Stopped - Streaming disabled");
            break;
        case Command::START_DATA_COLLECTION:
            captureEnabled = true;
            if (operationCallback)
                operationCallback(true);
            notifyClients("Operation Started");
            ESP_LOGI(TAG, "Operation Started - Device is now capturing");
            break;
        case Command::STOP_DATA_COLLECTION:
            captureEnabled = false;
            if (operationCallback)
                operationCallback(false);
            notifyClients("Operation Stopped");
            ESP_LOGI(TAG, "Operation Stopped - Device is now idle");
            break;
        case Command::START_INFERENCE:
            inferenceEnabled = true;
            notifyClients("Inference Started");
            ESP_LOGI(TAG, "Inference Started - Device is now infering");
            break;
        case Command::STOP_INFERENCE:
            inferenceEnabled = false;
            notifyClients("Inference Stopped");
            ESP_LOGI(TAG, "Inference Stopped - Device is now idle");
            break;
        default:
            ESP_LOGW(TAG, "Unknown control command received: %c (ASCII: %d)",
                     value[0], (int)value[0]);
            notifyClients("Unknown Command: " + std::string(1, value[0]) + " (ASCII: " + std::to_string((int)value[0]) + ")");
            break;
        }
    }
    else
    {
        ESP_LOGW(TAG, "Received empty control value");
    }
}

void CustomBLEService::begin()
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to take mutex in begin()");
        return;
    }
    
    // Add check to prevent double initialization
    static bool initialized = false;
    if (initialized) {
        ESP_LOGW(TAG, "BLE Service already initialized");
        xSemaphoreGive(mutex);
        return;
    }
    
    ESP_LOGI(TAG, "Initializing BLE Service...");

    // Add delay for stable initialization
    delay(100);

    NimBLEDevice::init(HOSTNAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setMTU(185);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    // Create service FIRST
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Create all characteristics
    pControlCharacteristic = pService->createCharacteristic(
        CONTROL_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    pControlCharacteristic->setCallbacks(new ControlCallbacks());

    // Menu characteristic
    NimBLECharacteristic *pMenuCharacteristic = pService->createCharacteristic(
        "BEB5483E-36E1-4688-B7F5-EA07361B26AB",
        NIMBLE_PROPERTY::READ);
    std::string commandDescription =
        "Available Commands:\n"
        "1: Start Preview\n"
        "2: Stop Preview\n"
        "3: Start Data Collection\n"
        "4: Stop Data Collection\n"
        "5: Start Inference\n"
        "6: Stop Inference";
    pMenuCharacteristic->setValue(commandDescription);

    // Status characteristic
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Preview info characteristic
    pPreviewInfoCharacteristic = pService->createCharacteristic(
        PREVIEW_INFO_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Service Status characteristic
    pServiceStatusCharacteristic = pService->createCharacteristic(
        SERVICE_STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Service Metrics characteristic
    pServiceMetricsCharacteristic = pService->createCharacteristic(
        SERVICE_METRICS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Set initial values and start service
    if (!pPreviewInfoCharacteristic)
    {
        ESP_LOGE(TAG, "Failed to create preview info characteristic!");
        xSemaphoreGive(mutex);
        return;
    }

    JsonDocument doc;
    doc["status"] = "Preview service not enabled";
    std::string initialValue;
    serializeJsonPretty(doc, initialValue);
    pPreviewInfoCharacteristic->setValue(initialValue);
    pPreviewInfoCharacteristic->setCallbacks(new PreviewInfoCallbacks());

    // Start the service BEFORE advertising
    if (!pService->start())
    {
        ESP_LOGE(TAG, "Failed to start BLE service!");
        xSemaphoreGive(mutex);
        return;
    }

    // Configure and start advertising LAST
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinInterval(0x20);
    pAdvertising->setMaxInterval(0x40);
    pAdvertising->setMinPreferred(0x0);
    pAdvertising->setMaxPreferred(0x0);

    if (!pAdvertising->start(0))
    {
        ESP_LOGE(TAG, "Failed to start advertising!");
        xSemaphoreGive(mutex);
        return;
    }

    ESP_LOGI(TAG, "BLE Service Started Successfully - Device Name: %s", HOSTNAME);
    xSemaphoreGive(mutex);

    initialized = true;
}

void CustomBLEService::loop()
{
    static unsigned long lastCheck = 0;
    const unsigned long CHECK_INTERVAL = 5000; // Check every 5 seconds
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        unsigned long now = millis();
        
        // Periodic connection check
        if (now - lastCheck >= CHECK_INTERVAL) {
            lastCheck = now;
            
            if (isConnected() && !pServer->getConnectedCount()) {
                ESP_LOGW(TAG, "Connection state mismatch detected");
                updateConnectionState(DISCONNECTED);
                handleDisconnection();
            }
        }
        
        // Keep-alive mechanism
        if (isConnected() && (now - lastKeepAlive >= KEEPALIVE_INTERVAL)) {
            lastKeepAlive = now;
            notifyClients("keepalive");
        }
        
        xSemaphoreGive(mutex);
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); // Shorter delay for better responsiveness
}

void CustomBLEService::notifyClients(const std::string &message)
{
    if (pStatusCharacteristic && isConnected())
    {
        JsonDocument doc;
        doc["type"] = "notification";
        doc["message"] = message;

        std::string output;
        serializeJsonPretty(doc, output);

        pStatusCharacteristic->setValue(output);
        pStatusCharacteristic->notify();
    }
}

void CustomBLEService::updateConnectionState(ConnectionState newState)
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        connectionState = newState;
        ESP_LOGI(TAG, "Connection state changed to: %s (Heap: %d)",
                 newState == CONNECTED ? "Connected" : newState == CONNECTING ? "Connecting"
                                                                              : "Disconnected",
                 esp_get_free_heap_size());

        // If disconnected, ensure advertising is restarted
        if (newState == DISCONNECTED && !NimBLEDevice::getAdvertising()->isAdvertising())
        {
            ESP_LOGI(TAG, "Restarting advertising after disconnect");
            NimBLEDevice::getAdvertising()->start(0);
        }

        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::updateServiceStatus(const std::string &service, const std::string &status)
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (pStatusCharacteristic && isConnected())
        {
            JsonDocument doc;
            doc["type"] = "service_status";
            doc["service"] = service;
            doc["status"] = status;

            std::string output;
            serializeJsonPretty(doc, output);

            pStatusCharacteristic->setValue(output);
            pStatusCharacteristic->notify();
        }
        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::updateServiceMetrics(const std::string &service, const std::string &metrics)
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (pStatusCharacteristic && isConnected())
        {
            JsonDocument doc;
            doc["service"] = service;
            doc["type"] = "metrics";
            doc["data"] = metrics;

            std::string output;
            serializeJsonPretty(doc, output);

            pStatusCharacteristic->setValue(output);
            pStatusCharacteristic->notify();
        }
        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::updatePreviewInfo(const std::string &info)
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        // Verify the JSON is valid before setting
        JsonDocument verify;
        DeserializationError error = deserializeJson(verify, info);

        if (error)
        {
            ESP_LOGE(TAG, "Invalid JSON in updatePreviewInfo: %s", error.c_str());
            return;
        }

        pPreviewInfoCharacteristic->setValue(info);
        pPreviewInfoCharacteristic->notify();
        ESP_LOGD(TAG, "Preview info updated: %s", info.c_str());
        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::handleDisconnection()
{
    ESP_LOGI(TAG, "Handling disconnection...");
    
    // Reset states
    captureEnabled = false;
    previewEnabled = false;
    inferenceEnabled = false;
    
    // Notify callbacks if needed
    if (operationCallback) operationCallback(false);
    if (previewCallback) previewCallback(false);
    if (inferenceCallback) inferenceCallback(false);
    
    // Restart advertising with retry mechanism
    const int MAX_RETRY = 3;
    const int RETRY_DELAY = 1000; // 1 second
    
    for (int i = 0; i < MAX_RETRY; i++) {
        if (NimBLEDevice::getAdvertising()->start()) {
            ESP_LOGI(TAG, "Advertising restarted successfully");
            return;
        }
        ESP_LOGW(TAG, "Failed to restart advertising, attempt %d of %d", i + 1, MAX_RETRY);
        delay(RETRY_DELAY);
    }
    
    ESP_LOGE(TAG, "Failed to restart advertising after all retries");
}

void CustomBLEService::cleanup() {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        if (pServer) {
            pServer->stopAdvertising();
            pServer = nullptr;
        }
        xSemaphoreGive(mutex);
    }
    
    if (mutex) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}

bool CustomBLEService::updateState(bool& stateVar, bool newValue) {
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        stateVar = newValue;
        xSemaphoreGive(mutex);
        return true;
    }
    return false;
}

void CustomBLEService::checkConnectionHealth() {
    static unsigned long lastCheck = 0;
    const unsigned long CHECK_INTERVAL = 30000; // 30 seconds
    
    if (millis() - lastCheck >= CHECK_INTERVAL) {
        if (isConnected()) {
            // Verify connection is still active
            if (!pServer->getConnectedCount()) {
                ESP_LOGW(TAG, "Connection state mismatch detected");
                updateConnectionState(DISCONNECTED);
            }
        }
        lastCheck = millis();
    }
}
