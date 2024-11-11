#pragma once

#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include "esp_log.h"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONTROL_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define STATUS_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a9"

class CustomBLEService;  // Forward declaration
extern CustomBLEService* globalBLEService;  // Global instance pointer

class CustomBLEService {
public:
    enum Command {
        START_OPERATION = '1',
        STOP_OPERATION = '2',
        START_PREVIEW = '3',
        STOP_PREVIEW = '4'
    };

    enum ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    CustomBLEService();
    void begin();
    void loop();
    bool isConnected() { return connectionState == CONNECTED; }
    bool isOperationEnabled() { return operationEnabled; }
    bool isPreviewEnabled() { return previewEnabled; }
    void updateConnectionState(ConnectionState newState);
    void handleControlCallback(NimBLECharacteristic* pCharacteristic);
    void notifyClients(const std::string& message);

    using StateChangeCallback = std::function<void(bool enabled)>;
    void setOperationCallback(StateChangeCallback cb) { operationCallback = cb; }
    void setPreviewCallback(StateChangeCallback cb) { previewCallback = cb; }

private:
    static bool operationEnabled;
    static bool previewEnabled;
    ConnectionState connectionState;
    
    NimBLEServer* pServer;
    NimBLECharacteristic* pControlCharacteristic;
    NimBLECharacteristic* pStatusCharacteristic;
    
    StateChangeCallback operationCallback;
    StateChangeCallback previewCallback;
    
    unsigned long lastKeepAlive;
    const unsigned long KEEPALIVE_INTERVAL = 1000;
};