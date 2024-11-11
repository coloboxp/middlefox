#include "ble_service.h"

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

CustomBLEService::CustomBLEService()
{
    globalBLEService = this;
    connectionState = DISCONNECTED;
    lastKeepAlive = millis(); // Initialize lastKeepAlive
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
    ESP_LOGI(TAG, "Initializing BLE Service...");
    NimBLEDevice::init(HOSTNAME);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

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
        "BEB5483E-36E1-4688-B7F5-EA07361B26AA", // New UUID for menu
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

    // Status characteristic (READ + NOTIFY)
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pPreviewInfoCharacteristic = pService->createCharacteristic(
        PREVIEW_INFO_CHAR_UUID,
        NIMBLE_PROPERTY::READ
    );
    
    // Set initial value
    JsonDocument doc;
    doc["status"] = "Preview service not enabled";
    std::string initialValue;
    serializeJsonPretty(doc, initialValue);
    pPreviewInfoCharacteristic->setValue(initialValue);

    if (!pService->start())
    {
        ESP_LOGE(TAG, "Failed to start BLE service!");
        return;
    }

    // Advertising configuration
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);

    if (!pAdvertising->start())
    {
        ESP_LOGE(TAG, "Failed to start advertising!");
        return;
    }

    ESP_LOGI(TAG, "BLE Service Started Successfully");
}

void CustomBLEService::loop()
{
    static unsigned long lastStatusUpdate = 0;
    static unsigned long lastServiceUpdate = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastStatusUpdate > STATUS_UPDATE_INTERVAL)
    {
        if (isConnected() && !previewEnabled && !captureEnabled && !inferenceEnabled)
        {
            JsonDocument doc;
            doc["type"] = "status";
            doc["op"] = captureEnabled;
            doc["preview"] = previewEnabled;
            doc["inference"] = inferenceEnabled;
            doc["uptime"] = currentTime / 1000;

            std::string output;
            serializeJsonPretty(doc, output);

            pStatusCharacteristic->setValue(output);
            pStatusCharacteristic->notify();
        }
        lastStatusUpdate = currentTime;
    }

    // Keep-alive check
    if (isConnected() && currentTime - lastKeepAlive > KEEPALIVE_INTERVAL)
    {
        lastKeepAlive = currentTime;
    }
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
    connectionState = newState;
    ESP_LOGI(TAG, "Connection state changed to: %s",
             newState == CONNECTED ? "Connected" : newState == CONNECTING ? "Connecting"
                                                                          : "Disconnected");

    // Notify clients about connection state change
    if (pStatusCharacteristic && isConnected())
    {
        char statusMsg[200];
        snprintf(statusMsg, sizeof(statusMsg),
                 "{\"connection\":\"%s\"}",
                 newState == CONNECTED ? "connected" : "disconnected");
        pStatusCharacteristic->setValue(statusMsg);
        pStatusCharacteristic->notify();
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
        pPreviewInfoCharacteristic->setValue(info);
    }
}