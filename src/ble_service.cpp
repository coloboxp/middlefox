#include "ble_service.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

const char *CustomBLEService::TAG = "BLEService";
const char *ServerCallbacks::TAG = "BLEServer";
const char *ControlCallbacks::TAG = "BLEControl";
const char *PreviewInfoCallbacks::TAG = "BLEPreview";

bool CustomBLEService::captureEnabled = false;
bool CustomBLEService::previewEnabled = false;
bool CustomBLEService::inferenceEnabled = false;
CustomBLEService *globalBLEService = nullptr;

static const unsigned long STATUS_UPDATE_INTERVAL = 5000;  // 5 seconds for general status
static const unsigned long SERVICE_UPDATE_INTERVAL = 1000; // 1 second for service updates

void ServerCallbacks::onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
    ESP_LOGI(TAG, "Client Connected - Address: %s",
             NimBLEAddress(desc->peer_ota_addr).toString().c_str());
    if (globalBLEService)
    {
        globalBLEService->updateConnectionState(CustomBLEService::CONNECTED);
    }
}

void ServerCallbacks::onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
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
                 (desc->conn_itvl * 1.25),
                 (desc->supervision_timeout * 10));
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

void ControlCallbacks::onWrite(NimBLECharacteristic *pCharacteristic)
{
    if (globalBLEService)
    {
        globalBLEService->handleControlCallback(pCharacteristic);
    }
}

void PreviewInfoCallbacks::onRead(NimBLECharacteristic *pCharacteristic)
{
    // Implementation if needed
}

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
        case Command::START_PREVIEW:
            if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                previewEnabled = true;
                xSemaphoreGive(mutex);

                // First notify the preview callback to initialize
                if (previewCallback)
                {
                    previewCallback(true);
                }

                // Then update status and notify clients
                updateServiceStatus("preview", "starting");
                notifyClients("Preview Starting");
                ESP_LOGI(TAG, "Preview Mode Activated - Initializing stream...");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to take mutex in START_PREVIEW");
            }
            break;
        case Command::STOP_PREVIEW:
            if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                previewEnabled = false;
                m_explicitStop = true; // Set flag for explicit stop
                xSemaphoreGive(mutex);

                if (previewCallback)
                    previewCallback(false);
                notifyClients("Preview Stopped");
                ESP_LOGI(TAG, "Preview Stopped - Streaming disabled");
            }
            break;
        case Command::START_DATA_COLLECTION:
            if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                captureEnabled = true;
                xSemaphoreGive(mutex);

                if (operationCallback)
                    operationCallback(true);
                notifyClients("Operation Started");
                ESP_LOGI(TAG, "Operation Started - Device is now capturing");
            }
            break;
        case Command::STOP_DATA_COLLECTION:
            if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                captureEnabled = false;
                m_explicitStop = true; // Set flag for explicit stop
                xSemaphoreGive(mutex);

                if (operationCallback)
                    operationCallback(false);
                notifyClients("Operation Stopped");
                ESP_LOGI(TAG, "Operation Stopped - Device is now idle");
            }
            break;
        case Command::START_INFERENCE:
            inferenceEnabled = true;
            notifyClients("Inference Started");
            ESP_LOGI(TAG, "Inference Started - Device is now infering");
            break;
        case Command::STOP_INFERENCE:
            inferenceEnabled = false;
            m_explicitStop = true; // Set flag for explicit stop
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

bool CustomBLEService::begin()
{
    ESP_LOGI(TAG, "=== Starting BLE Service Initialization ===");
    ESP_LOGD(TAG, "Taking mutex for initialization...");

    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        ESP_LOGE(TAG, "❌ Failed to take mutex in begin() - Timeout after 1000ms");
        return false;
    }

    static bool initialized = false;
    if (initialized)
    {
        ESP_LOGW(TAG, "⚠️ BLE Service already initialized");
        xSemaphoreGive(mutex);
        return true;
    }

    ESP_LOGI(TAG, "1️⃣ Initializing NimBLE Device...");
    NimBLEDevice::init(HOSTNAME);
    ESP_LOGD(TAG, "Setting BLE parameters...");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setSecurityAuth(false, false, true);
    NimBLEDevice::setMTU(185);

    ESP_LOGI(TAG, "2️⃣ Creating BLE Server...");
    pServer = NimBLEDevice::createServer();
    if (!pServer)
    {
        ESP_LOGE(TAG, "❌ Failed to create BLE server");
        xSemaphoreGive(mutex);
        return false;
    }
    pServer->setCallbacks(new ServerCallbacks());

    ESP_LOGI(TAG, "3️⃣ Creating BLE Service...");
    pService = pServer->createService(SERVICE_UUID);
    if (!pService)
    {
        ESP_LOGE(TAG, "❌ Failed to create BLE service");
        xSemaphoreGive(mutex);
        return false;
    }

    ESP_LOGI(TAG, "4️⃣ Creating Characteristics...");

    // Control Characteristic
    ESP_LOGD(TAG, "Creating Control characteristic...");
    pControlCharacteristic = pService->createCharacteristic(
        CONTROL_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    if (!pControlCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Control characteristic");
        xSemaphoreGive(mutex);
        return false;
    }
    pControlCharacteristic->setCallbacks(new ControlCallbacks());

    // Status Characteristic
    ESP_LOGD(TAG, "Creating Status characteristic...");
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    if (!pStatusCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Status characteristic");
        xSemaphoreGive(mutex);
        return false;
    }

    // Preview Info Characteristic
    ESP_LOGD(TAG, "Creating Preview Info characteristic...");
    pPreviewInfoCharacteristic = pService->createCharacteristic(
        PREVIEW_INFO_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    if (!pPreviewInfoCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Preview Info characteristic");
        xSemaphoreGive(mutex);
        return false;
    }

    // Menu Characteristic
    ESP_LOGD(TAG, "Creating Menu characteristic...");
    NimBLECharacteristic *pMenuCharacteristic = pService->createCharacteristic(
        MENU_CHAR_UUID,
        NIMBLE_PROPERTY::READ);
    if (!pMenuCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Menu characteristic");
        xSemaphoreGive(mutex);
        return false;
    }
    std::string commandDescription =
        "Available Commands:\n"
        "1: Start Preview\n"
        "2: Stop Preview\n"
        "3: Start Operation\n"
        "4: Stop Operation\n"
        "5: Start Inference\n"
        "6: Stop Inference";
    pMenuCharacteristic->setValue(commandDescription);

    // Service Status Characteristic
    ESP_LOGD(TAG, "Creating Service Status characteristic...");
    pServiceStatusCharacteristic = pService->createCharacteristic(
        SERVICE_STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    if (!pServiceStatusCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Service Status characteristic");
        xSemaphoreGive(mutex);
        return false;
    }

    // Service Metrics Characteristic
    ESP_LOGD(TAG, "Creating Service Metrics characteristic...");
    pServiceMetricsCharacteristic = pService->createCharacteristic(
        SERVICE_METRICS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    if (!pServiceMetricsCharacteristic)
    {
        ESP_LOGE(TAG, "❌ Failed to create Service Metrics characteristic");
        xSemaphoreGive(mutex);
        return false;
    }

    ESP_LOGI(TAG, "5️⃣ Starting BLE Service...");
    if (!pService->start())
    {
        ESP_LOGE(TAG, "❌ Failed to start BLE service");
        xSemaphoreGive(mutex);
        return false;
    }

    ESP_LOGI(TAG, "6️⃣ Starting Advertising...");
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);

    if (!pAdvertising->start())
    {
        ESP_LOGE(TAG, "❌ Failed to start advertising");
        xSemaphoreGive(mutex);
        return false;
    }

    initialized = true;
    ESP_LOGI(TAG, "✅ BLE Service Started Successfully - Device Name: %s", HOSTNAME);
    ESP_LOGI(TAG, "=== BLE Service Initialization Complete ===");

    xSemaphoreGive(mutex);
    return true;
}

bool CustomBLEService::createControlCharacteristic()
{
    pControlCharacteristic = pService->createCharacteristic(
        CONTROL_CHAR_UUID,
        NIMBLE_PROPERTY::WRITE);
    if (!pControlCharacteristic)
        return false;
    pControlCharacteristic->setCallbacks(new ControlCallbacks());
    return true;
}

bool CustomBLEService::createStatusCharacteristic()
{
    pStatusCharacteristic = pService->createCharacteristic(
        STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    return pStatusCharacteristic != nullptr;
}

bool CustomBLEService::createPreviewCharacteristic()
{
    pPreviewInfoCharacteristic = pService->createCharacteristic(
        PREVIEW_INFO_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    if (!pPreviewInfoCharacteristic)
        return false;
    pPreviewInfoCharacteristic->setCallbacks(new PreviewInfoCallbacks());
    return true;
}

bool CustomBLEService::createMenuCharacteristic()
{
    auto pMenuChar = pService->createCharacteristic(
        MENU_CHAR_UUID,
        NIMBLE_PROPERTY::READ);
    if (!pMenuChar)
        return false;

    std::string commandDescription =
        "Available Commands:\n"
        "1: Start Preview\n"
        "2: Stop Preview\n"
        "3: Start Operation\n"
        "4: Stop Operation\n"
        "5: Start Inference\n"
        "6: Stop Inference";
    pMenuChar->setValue(commandDescription);
    return true;
}

bool CustomBLEService::createServiceStatusCharacteristic()
{
    pServiceStatusCharacteristic = pService->createCharacteristic(
        SERVICE_STATUS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    return pServiceStatusCharacteristic != nullptr;
}

bool CustomBLEService::createServiceMetricsCharacteristic()
{
    pServiceMetricsCharacteristic = pService->createCharacteristic(
        SERVICE_METRICS_CHAR_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    return pServiceMetricsCharacteristic != nullptr;
}

void CustomBLEService::loop()
{
    static unsigned long lastCheck = 0;
    const unsigned long CHECK_INTERVAL = 5000;

    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        unsigned long now = millis();

        if (now - lastCheck >= CHECK_INTERVAL)
        {
            lastCheck = now;
            ESP_LOGV(TAG, "Performing connection check...");

            if (isConnected() && !pServer->getConnectedCount())
            {
                ESP_LOGW(TAG, "⚠️ Connection state mismatch detected");
                ESP_LOGD(TAG, "Internal state: Connected, Server count: 0");
                updateConnectionState(DISCONNECTED);
                handleDisconnection();
            }
        }

        if (isConnected() && (now - lastKeepAlive >= KEEPALIVE_INTERVAL))
        {
            lastKeepAlive = now;
            ESP_LOGV(TAG, "Sending keepalive...");
            notifyClients("keepalive");
        }

        xSemaphoreGive(mutex);
    }
    else
    {
        ESP_LOGW(TAG, "⚠️ Failed to take mutex in loop()");
    }

    vTaskDelay(pdMS_TO_TICKS(10));
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

    m_disconnecting = true; // Set disconnecting flag before callbacks

    // Reset states but don't restart
    captureEnabled = false;
    previewEnabled = false;
    inferenceEnabled = false;
    m_explicitStop = false; // Reset explicit stop flag on disconnection

    // Notify callbacks with explicit disconnect flag
    if (operationCallback)
        operationCallback(false);
    if (previewCallback)
        previewCallback(false);
    if (inferenceCallback)
        inferenceCallback(false);

    m_disconnecting = false; // Reset disconnecting flag after callbacks

    // Restart advertising with retry mechanism
    const int MAX_RETRY = 3;
    const int RETRY_DELAY = 1000;

    for (int i = 0; i < MAX_RETRY; i++)
    {
        if (NimBLEDevice::getAdvertising()->start())
        {
            ESP_LOGI(TAG, "Advertising restarted successfully");
            return;
        }
        ESP_LOGW(TAG, "Failed to restart advertising, attempt %d of %d", i + 1, MAX_RETRY);
        delay(RETRY_DELAY);
    }

    ESP_LOGE(TAG, "Failed to restart advertising after all retries");
}

bool CustomBLEService::isDisconnecting() const
{
    return m_disconnecting;
}

void CustomBLEService::cleanup()
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if (pServer)
        {
            pServer->stopAdvertising();
            pServer = nullptr;
        }
        xSemaphoreGive(mutex);
    }

    if (mutex)
    {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
}

bool CustomBLEService::updateState(bool &stateVar, bool newValue)
{
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        stateVar = newValue;
        xSemaphoreGive(mutex);
        return true;
    }
    return false;
}

void CustomBLEService::checkConnectionHealth()
{
    static unsigned long lastCheck = 0;
    const unsigned long CHECK_INTERVAL = 30000; // 30 seconds

    if (millis() - lastCheck >= CHECK_INTERVAL)
    {
        if (isConnected())
        {
            // Verify connection is still active
            if (!pServer->getConnectedCount())
            {
                ESP_LOGW(TAG, "Connection state mismatch detected");
                updateConnectionState(DISCONNECTED);
            }
        }
        lastCheck = millis();
    }
}

bool CustomBLEService::wasExplicitlyStopped() const { return m_explicitStop; }
void CustomBLEService::setExplicitStop(bool value) { m_explicitStop = value; }
