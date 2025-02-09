#pragma once

#include <ArduinoJson.h>
#include "config.h"
#include <NimBLEDevice.h>
#include "esp_log.h"
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CONTROL_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define STATUS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define PREVIEW_INFO_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define MENU_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ab"
#define SERVICE_STATUS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ad"
#define SERVICE_METRICS_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ac"

#define BLE_LOG_LEVEL ESP_LOG_INFO // Change to ESP_LOG_DEBUG or ESP_LOG_VERBOSE for more detail

class CustomBLEService;                    // Forward declaration
extern CustomBLEService *globalBLEService; // Global instance pointer

class ServerCallbacks : public NimBLEServerCallbacks
{
private:
    static const char *TAG;

public:
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
    void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override;
};

class ControlCallbacks : public NimBLECharacteristicCallbacks
{
private:
    static const char *TAG;

public:
    void onWrite(NimBLECharacteristic *pCharacteristic) override;
};

class PreviewInfoCallbacks : public NimBLECharacteristicCallbacks
{
private:
    static const char *TAG;

public:
    void onRead(NimBLECharacteristic *pCharacteristic) override;
};

class CustomBLEService
{
public:
    enum Command
    {
        START_PREVIEW = '1',
        STOP_PREVIEW = '2',
        START_DATA_COLLECTION = '3',
        STOP_DATA_COLLECTION = '4',
        START_INFERENCE = '5',
        STOP_INFERENCE = '6'
    };

    enum ConnectionState
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    CustomBLEService();
    ~CustomBLEService()
    {
        if (mutex != nullptr)
        {
            vSemaphoreDelete(mutex);
            mutex = nullptr;
        }
        globalBLEService = nullptr;
        cleanup();
    }
    bool begin();
    void loop();
    bool isConnected() { return connectionState == CONNECTED; }
    bool isOperationEnabled() { return captureEnabled; }
    bool isPreviewEnabled() { return previewEnabled; }
    bool isInferenceEnabled() { return inferenceEnabled; }
    void updateConnectionState(ConnectionState newState);
    void handleControlCallback(NimBLECharacteristic *pCharacteristic);
    void notifyClients(const std::string &message);

    using StateChangeCallback = std::function<void(bool enabled)>;
    void setOperationCallback(StateChangeCallback cb) { operationCallback = cb; }
    void setPreviewCallback(StateChangeCallback cb) { previewCallback = cb; }
    void setInferenceCallback(StateChangeCallback cb) { inferenceCallback = cb; }

    void updateServiceStatus(const std::string &service, const std::string &status);
    void updateServiceMetrics(const std::string &service, const std::string &metrics);

    void updatePreviewInfo(const std::string &info);

    void handleDisconnection();
    void cleanup();
    bool updateState(bool &stateVar, bool newValue);
    void checkConnectionHealth();

    bool isDisconnecting() const;
    bool wasExplicitlyStopped() const;

    void setExplicitStop(bool value);

    static bool isCaptureEnabled() { return captureEnabled; }

    void handleControlCallback(Command cmd)
    {
        std::string value(1, static_cast<char>(cmd));
        handleControlCallback(value);
    }

    void handleControlCallback(const std::string &value);

    bool handleControlCommand(Command cmd)
    {
        if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            switch (cmd)
            {
            case Command::START_PREVIEW:
                previewEnabled = true;
                xSemaphoreGive(mutex);

                if (previewCallback)
                {
                    previewCallback(true);
                }
                updateServiceStatus("preview", "starting");
                notifyClients("Preview Starting");
                ESP_LOGI(TAG, "Preview Mode Activated - Initializing stream...");
                return true;

                // ... other cases if needed

            default:
                xSemaphoreGive(mutex);
                return false;
            }
        }
        return false;
    }

private:
    static const char *TAG; // Define if not already defined

    bool createControlCharacteristic();
    bool createStatusCharacteristic();
    bool createPreviewCharacteristic();
    bool createMenuCharacteristic();
    bool createServiceStatusCharacteristic();
    bool createServiceMetricsCharacteristic();

    static bool captureEnabled;
    static bool previewEnabled;
    static bool inferenceEnabled;
    bool m_explicitStop;
    bool m_disconnecting;
    ConnectionState connectionState;

    NimBLEServer *pServer;
    NimBLEService *pService;
    NimBLECharacteristic *pControlCharacteristic;
    NimBLECharacteristic *pStatusCharacteristic;
    NimBLECharacteristic *pPreviewInfoCharacteristic;
    NimBLECharacteristic *pServiceStatusCharacteristic;
    NimBLECharacteristic *pServiceMetricsCharacteristic;

    StateChangeCallback operationCallback;
    StateChangeCallback previewCallback;
    StateChangeCallback inferenceCallback;

    unsigned long lastKeepAlive;
    const unsigned long KEEPALIVE_INTERVAL = 1000;

    std::map<std::string, std::string> serviceStatus;
    void sendStatusUpdate();

    SemaphoreHandle_t mutex;
};