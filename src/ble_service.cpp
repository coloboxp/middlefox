#include "ble_service.h"

static const char *TAG = "BLEService";
bool CustomBLEService::operationEnabled = false;
bool CustomBLEService::previewEnabled = false;
CustomBLEService *globalBLEService = nullptr;

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
        case Command::START_OPERATION:
            operationEnabled = true;
            if (operationCallback)
                operationCallback(true);
            notifyClients("Operation Started");
            ESP_LOGI(TAG, "Operation Started - Device is now capturing");
            break;
        case Command::STOP_OPERATION:
            operationEnabled = false;
            if (operationCallback)
                operationCallback(false);
            notifyClients("Operation Stopped");
            ESP_LOGI(TAG, "Operation Stopped - Device is now idle");
            break;
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
        default:
            ESP_LOGW(TAG, "Unknown control command received: %c (ASCII: %d)",
                     value[0], (int)value[0]);
            notifyClients("Unknown Command received " + value);
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

    NimBLEDevice::init("ESP32-CAM");

    // Set power level for better range
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Control characteristic
    pControlCharacteristic = pService->createCharacteristic(
        CONTROL_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pControlCharacteristic->setCallbacks(new ControlCallbacks());

    // Status characteristic for device state updates
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Fix descriptor creation - using the correct UUID format and properties
    pControlCharacteristic->createDescriptor(
                              NimBLEUUID("2901"),
                              NIMBLE_PROPERTY::READ,
                              20 // Max length
                              )
        ->setValue("Camera Control");

    pStatusCharacteristic->createDescriptor(
                             NimBLEUUID("2901"),
                             NIMBLE_PROPERTY::READ,
                             20 // Max length
                             )
        ->setValue("Device Status");

    if (!pService->start())
    {
        ESP_LOGE(TAG, "Failed to start BLE service!");
        return;
    }

    // Enhanced advertising configuration
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // Functions that help with iPhone connections issue
    pAdvertising->setMaxPreferred(0x12);

    NimBLEDevice::startSecurity(0); // Enable security

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

    // Regular status updates
    if (millis() - lastStatusUpdate > 1000)
    {
        if (isConnected())
        {
            // Create status message as JSON string
            char statusMsg[200];
            snprintf(statusMsg, sizeof(statusMsg),
                     "{\"operation\":%s,\"preview\":%s,\"uptime\":%lu}",
                     operationEnabled ? "true" : "false",
                     previewEnabled ? "true" : "false",
                     millis() / 1000);

            pStatusCharacteristic->setValue(statusMsg);
            pStatusCharacteristic->notify();
        }
        lastStatusUpdate = millis();
    }

    // Keep-alive check
    if (isConnected() && millis() - lastKeepAlive > KEEPALIVE_INTERVAL)
    {
        // Implement keep-alive mechanism
        lastKeepAlive = millis();
    }
}

void CustomBLEService::notifyClients(const std::string &message)
{
    if (pStatusCharacteristic && isConnected())
    {
        pStatusCharacteristic->setValue(message);
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