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

    void onDisconnect(NimBLEServer *pServer)
    {
        ESP_LOGI(TAG, "Client Disconnected");
        if (globalBLEService)
        {
            globalBLEService->updateConnectionState(CustomBLEService::DISCONNECTED);
        }
        pServer->startAdvertising();
    }
};

class PreviewInfoCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
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
            previewEnabled = true;
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
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        ESP_LOGI(TAG, "Initializing BLE Service...");
        
        // Stop any existing advertising/server
        if (NimBLEDevice::getInitialized()) {
            NimBLEDevice::deinit(true);
            delay(500); // Give time for cleanup
        }

        // Initialize NimBLE with higher stability settings
        NimBLEDevice::init(HOSTNAME);
        
        // Configure device
        NimBLEDevice::setPower(ESP_PWR_LVL_P9); // Set max power
        NimBLEDevice::setSecurityAuth(false, false, true); // No bonding, no MITM, but encryption
        NimBLEDevice::setMTU(185); // Lower MTU for better stability
        
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(new ServerCallbacks());

        // Create service
        NimBLEService *pService = pServer->createService(SERVICE_UUID);

        // Control characteristic (WRITE only)
        pControlCharacteristic = pService->createCharacteristic(
            CONTROL_CHAR_UUID,
            NIMBLE_PROPERTY::WRITE);
        pControlCharacteristic->setCallbacks(new ControlCallbacks());

        // Menu characteristic (READ only)
        NimBLECharacteristic *pMenuCharacteristic = pService->createCharacteristic(
            "BEB5483E-36E1-4688-B7F5-EA07361B26AB",  // Changed last digit to avoid conflict
            NIMBLE_PROPERTY::READ
        );
        std::string commandDescription =
            "Available Commands:\n"
            "1: Start Preview\n"
            "2: Stop Preview\n"
            "3: Start Data Collection\n"
            "4: Stop Data Collection\n"
            "5: Start Inference\n"
            "6: Stop Inference";
        pMenuCharacteristic->setValue(commandDescription);

        // Status characteristic (READ + NOTIFY)
        pStatusCharacteristic = pService->createCharacteristic(
            STATUS_CHAR_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

        // Preview info characteristic (READ + NOTIFY)
        pPreviewInfoCharacteristic = pService->createCharacteristic(
            PREVIEW_INFO_CHAR_UUID,
            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
        );

        if (!pPreviewInfoCharacteristic) {
            ESP_LOGE(TAG, "Failed to create preview info characteristic!");
            return;
        }

        // Set initial value
        JsonDocument doc;
        doc["status"] = "Preview service not enabled";
        std::string initialValue;
        serializeJsonPretty(doc, initialValue);
        
        pPreviewInfoCharacteristic->setValue(initialValue);
        pPreviewInfoCharacteristic->setCallbacks(new PreviewInfoCallbacks());

        ESP_LOGI(TAG, "Preview info characteristic initialized with value: %s", initialValue.c_str());

        if (!pService->start()) {
            ESP_LOGE(TAG, "Failed to start BLE service!");
            xSemaphoreGive(mutex);
            return;
        }

        // Configure advertising with more conservative settings
        NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinInterval(0x20); // Slower advertising interval
        pAdvertising->setMaxInterval(0x40); // (x * 0.625) ms
        pAdvertising->setMinPreferred(0x0);  // Remove min preferred
        pAdvertising->setMaxPreferred(0x0);  // Remove max preferred
        
        if (!pAdvertising->start(0)) { // Advertise forever
            ESP_LOGE(TAG, "Failed to start advertising!");
            xSemaphoreGive(mutex);
            return;
        }

        ESP_LOGI(TAG, "BLE Service Started Successfully - Device Name: %s", HOSTNAME);
        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::loop()
{
    static unsigned long lastAdvertisingCheck = 0;
    unsigned long currentTime = millis();

    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Check if we need to restart advertising
        if (!isConnected() && (currentTime - lastAdvertisingCheck >= 5000)) { // Check every 5 seconds
            lastAdvertisingCheck = currentTime;
            
            if (!NimBLEDevice::getAdvertising()->isAdvertising()) {
                ESP_LOGI(TAG, "Restarting advertising...");
                NimBLEDevice::getAdvertising()->start(0);
            }
        }

        // Keep-alive for connected clients
        if (isConnected() && (currentTime - lastKeepAlive >= KEEPALIVE_INTERVAL)) {
            lastKeepAlive = currentTime;
            notifyClients("Device alive");
        }

        xSemaphoreGive(mutex);
    }
    
    vTaskDelay(pdMS_TO_TICKS(20)); // Slightly longer delay
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
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        connectionState = newState;
        ESP_LOGI(TAG, "Connection state changed to: %s",
                 newState == CONNECTED ? "Connected" : 
                 newState == CONNECTING ? "Connecting" : "Disconnected");

        if (newState == DISCONNECTED) {
            // Restart advertising on disconnect
            if (!NimBLEDevice::getAdvertising()->start(0)) {
                ESP_LOGE(TAG, "Failed to restart advertising!");
            }
        }

        xSemaphoreGive(mutex);
    }
}

void CustomBLEService::updateServiceStatus(const std::string &service, const std::string &status)
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
}

void CustomBLEService::updateServiceMetrics(const std::string &service, const std::string &metrics)
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
}

void CustomBLEService::updatePreviewInfo(const std::string& info) {
    if (pPreviewInfoCharacteristic) {
        // Verify the JSON is valid before setting
        JsonDocument verify;
        DeserializationError error = deserializeJson(verify, info);
        
        if (error) {
            ESP_LOGE(TAG, "Invalid JSON in updatePreviewInfo: %s", error.c_str());
            return;
        }

        pPreviewInfoCharacteristic->setValue(info);
        pPreviewInfoCharacteristic->notify();
        ESP_LOGD(TAG, "Preview info updated: %s", info.c_str());
    } else {
        ESP_LOGE(TAG, "Preview info characteristic is null!");
    }
}