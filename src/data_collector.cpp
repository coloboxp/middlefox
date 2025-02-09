#include "data_collector.h"

using namespace Eloquent::Esp32cam;
static Camera::Camera camera;

// Define the static TAG member
const char *DataCollector::TAG = "DataCollector";

bool DataCollector::convert_rgb565_to_jpeg(const uint8_t *rgb565_data, int width, int height,
                                           uint8_t **jpg_buf_out, size_t *jpg_len_out)
{
    if (!rgb565_data || !jpg_buf_out || !jpg_len_out)
    {
        ESP_LOGE(TAG, "Invalid parameters");
        return false;
    }

    camera_fb_t fb = {
        .buf = (uint8_t *)rgb565_data,
        .len = static_cast<size_t>(width * height * 2),
        .width = static_cast<size_t>(width),
        .height = static_cast<size_t>(height),
        .format = PIXFORMAT_RGB565};

    bool success = frame2jpg(&fb, 100, jpg_buf_out, jpg_len_out);

    if (success && *jpg_len_out > 0)
    {
        ESP_LOGI(TAG, "JPEG conversion successful, size: %d", *jpg_len_out);
        return true;
    }

    ESP_LOGE(TAG, "JPEG conversion failed");
    return false;
}

bool DataCollector::initSD()
{
    return SDManager::getInstance().begin();
}

int DataCollector::getNextImageCount()
{
    ESP_LOGI(TAG, "Starting getNextImageCount()");
    File root = SDManager::getInstance().openDir("/");
    int maxCount = 0;

    if (!root)
    {
        ESP_LOGE(TAG, "Failed to open root directory");
        return 0;
    }

    ESP_LOGI(TAG, "Scanning files...");
    while (true)
    {
        File entry = root.openNextFile();
        if (!entry)
            break;

        String fileName = String(entry.name());
        ESP_LOGD(TAG, "Found file: %s", fileName.c_str());

        if (fileName.startsWith("picture") && fileName.endsWith(".jpg"))
        {
            String numStr = fileName.substring(7, fileName.length() - 4);
            int num = numStr.toInt();
            ESP_LOGD(TAG, "Found picture number: %d", num);
            if (num > maxCount)
                maxCount = num;
        }
        entry.close();
    }

    root.close();
    ESP_LOGI(TAG, "Scan complete. Next image number will be: %d", maxCount + 1);
    return maxCount + 1;
}

DataCollector::DataCollector(CustomBLEService *ble) : bleService(ble)
{
    ESP_LOGI(TAG, "Initializing DataCollector");
    lastCapture = 0;
    imageCount = 0;
    camera = nullptr;
    cameraMutex = xSemaphoreCreateMutex();

    bleService->setOperationCallback([this](bool enabled)
                                     {
        ESP_LOGI(TAG, "Data collection callback triggered: %s", enabled ? "START" : "STOP");
        if (enabled) {
            if (!camera && !begin()) {
                ESP_LOGE(TAG, "Failed to initialize data collector");
                return;
            }
            // Start capturing
            lastCapture = 0;  // Force immediate first capture
        } else {
            // Only restart if this was triggered by a stop command, not disconnection
            if (bleService->wasExplicitlyStopped() && !bleService->isDisconnecting()) {
                ESP_LOGI(TAG, "Explicit stop command received - Triggering restart");
                bleService->notifyClients("Restarting device to idle state...");
                delay(1000);
                ESP.restart();
            }
        } });
}

DataCollector::~DataCollector()
{
    cleanup();
}

void DataCollector::cleanup()
{
    if (cameraMutex)
    {
        vSemaphoreDelete(cameraMutex);
        cameraMutex = nullptr;
    }
    camera = nullptr; // Don't delete, it's managed by CameraManager
}

bool DataCollector::begin()
{
    ESP_LOGI(TAG, "=== Starting DataCollector Initialization ===");

    // Initialize SD card first with better error handling
    if (!initSD())
    {
        ESP_LOGE(TAG, "SD Card initialization failed");
        return false;
    }

    // Verify SD card is writable
    File testFile = SD.open("/test.txt", FILE_WRITE);
    if (!testFile)
    {
        ESP_LOGE(TAG, "Failed to create test file - SD card may be write-protected or not properly mounted");
        return false;
    }
    testFile.close();
    SD.remove("/test.txt");

    // Get camera instance and configure for capture mode
    camera = CameraManager::getInstance().getCamera();
    if (!camera)
    {
        ESP_LOGE(TAG, "Failed to get camera instance");
        return false;
    }

    // Configure camera for capture mode (not preview)
    if (!CameraManager::getInstance().begin(false))
    {
        ESP_LOGE(TAG, "Failed to initialize camera in capture mode");
        return false;
    }

    // Get the next available image count
    imageCount = getNextImageCount();
    ESP_LOGI(TAG, "Image counter initialized. Starting from: %d", imageCount);

    ESP_LOGI(TAG, "=== DataCollector Initialization Complete ===");
    return true;
}

void DataCollector::loop()
{
    if (!bleService->isCaptureEnabled())
    {
        return;
    }

    if (!camera)
    {
        ESP_LOGE(TAG, "Camera not initialized");
        BuzzerManager::getInstance().playHighImportance(); // Error sound
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastCapture >= CAPTURE_INTERVAL_MS)
    {
        ESP_LOGD(TAG, "=== Starting New Capture Cycle ===");

        // Visual feedback
        digitalWrite(LED_BUILTIN, LOW); // Turn ON
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH); // Turn OFF

        if (!camera->capture().isOk())
        {
            std::string errorMsg = "Capture failed: ";
            errorMsg += camera->exception.toString().c_str();
            bleService->updateServiceStatus("collector", errorMsg);
            BuzzerManager::getInstance().playHighImportance(); // Error sound
            return;
        }

        bleService->updateServiceStatus("collector", "Image captured successfully");

        // Save RGB565 raw data with better error handling
        String rgb_path = "/picture" + String(imageCount) + ".rgb";
        File rgb_file = SD.open(rgb_path.c_str(), FILE_WRITE);
        if (!rgb_file)
        {
            ESP_LOGE(TAG, "Failed to create RGB file: %s", rgb_path.c_str());
            bleService->updateServiceStatus("collector", "Failed to create RGB file");
            BuzzerManager::getInstance().playMediumImportance(); // Warning sound
            return;
        }

        size_t bytesWritten = rgb_file.write(camera->frame->buf, camera->frame->len);
        rgb_file.close();

        if (bytesWritten != camera->frame->len)
        {
            ESP_LOGE(TAG, "Failed to write complete RGB data. Written: %u, Expected: %u",
                     bytesWritten, camera->frame->len);
            bleService->updateServiceStatus("collector", "Failed to write complete RGB data");
            BuzzerManager::getInstance().playMediumImportance(); // Warning sound
            return;
        }

        ESP_LOGI(TAG, "RGB file saved: %s (Size: %u bytes)", rgb_path.c_str(), bytesWritten);

        // Convert and save JPEG with better error handling
        uint8_t *jpg_buf = NULL;
        size_t jpg_len = 0;
        bool converted = convert_rgb565_to_jpeg(camera->frame->buf, camera->frame->width,
                                                camera->frame->height, &jpg_buf, &jpg_len);

        if (converted && jpg_buf != NULL && jpg_len > 0)
        {
            String jpg_path = "/picture" + String(imageCount) + ".jpg";
            File jpg_file = SD.open(jpg_path.c_str(), FILE_WRITE);
            if (!jpg_file)
            {
                ESP_LOGE(TAG, "Failed to create JPG file: %s", jpg_path.c_str());
                free(jpg_buf);
                bleService->updateServiceStatus("collector", "Failed to create JPG file");
                BuzzerManager::getInstance().playMediumImportance(); // Warning sound
                return;
            }

            size_t jpgBytesWritten = jpg_file.write(jpg_buf, jpg_len);
            jpg_file.close();

            if (jpgBytesWritten != jpg_len)
            {
                ESP_LOGE(TAG, "Failed to write complete JPG data. Written: %u, Expected: %u",
                         jpgBytesWritten, jpg_len);
                free(jpg_buf);
                bleService->updateServiceStatus("collector", "Failed to write complete JPG data");
                BuzzerManager::getInstance().playMediumImportance(); // Warning sound
                return;
            }

            ESP_LOGI(TAG, "Saved JPG file: %s (Size: %u bytes)", jpg_path.c_str(), jpg_len);
            free(jpg_buf);
            imageCount++;
            // BuzzerManager::getInstance().playLowImportance(); // Success beep
        }
        else
        {
            ESP_LOGE(TAG, "JPEG conversion failed");
            bleService->updateServiceStatus("collector", "JPEG conversion failed");
            if (jpg_buf)
                free(jpg_buf);
            BuzzerManager::getInstance().playHighImportance(); // Error sound
            return;
        }

        lastCapture = currentTime;
        ESP_LOGI(TAG, "=== Capture Cycle Complete ===");
        ESP_LOGD(TAG, "Total images captured: %d", imageCount);

        // Update metrics
        JsonDocument doc;
        doc["image_count"] = imageCount;
        doc["last_capture"] = millis() - lastCapture;
        doc["next_capture"] = CAPTURE_INTERVAL_MS - (millis() - lastCapture);

        std::string metrics;
        serializeJsonPretty(doc, metrics);
        bleService->updateServiceMetrics("collector", metrics);
    }
}