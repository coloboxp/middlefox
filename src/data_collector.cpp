#include "data_collector.h"

using namespace Eloquent::Esp32cam;
static Camera::Camera camera;
static const char *TAG = "DataCollector";

bool convert_rgb565_to_jpeg(const uint8_t *rgb565_data, int width, int height, uint8_t **jpg_buf_out, size_t *jpg_len_out)
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
    // Try multiple times to initialize SD card
    for (int i = 0; i < 3; i++)
    {
        if (SD.begin(21))
        {
            uint8_t cardType = SD.cardType();
            if (cardType != CARD_NONE)
            {
                ESP_LOGI(TAG, "SD card OK");
                return true;
            }
        }
        delay(1000); // Wait a second before retrying
        SD.end();    // Clean up before retry
    }

    ESP_LOGE(TAG, "SD Card Mount Failed");
    return false;
}

int DataCollector::getNextImageCount()
{
    ESP_LOGI(TAG, "Starting getNextImageCount()");
    File root = SD.open("/");
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

DataCollector::DataCollector()
{
    ESP_LOGI(TAG, "Initializing DataCollector");
    lastCapture = 0;
    imageCount = 0;
    camera = nullptr;
}

bool DataCollector::begin()
{
    ESP_LOGI(TAG, "=== Starting DataCollector Initialization ===");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // LED off initially

    // Initialize SD card first
    if (!SD.begin(21))
    {
        ESP_LOGE(TAG, "SD Card Mount Failed - Check SD card connection");
        return false;
    }
    ESP_LOGI(TAG, "SD card mounted successfully");

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        ESP_LOGE(TAG, "No SD card detected - Please insert an SD card");
        return false;
    }
    ESP_LOGD(TAG, "SD Card Type: %d", cardType);

    // Get the next available image count and log it
    imageCount = getNextImageCount();
    ESP_LOGI(TAG, "Image counter initialized. Starting from: %d", imageCount);

    // Get camera instance
    camera = CameraManager::getInstance().getCamera();
    if (!camera) {
        ESP_LOGE(TAG, "Failed to get camera instance");
        return false;
    }

    ESP_LOGI(TAG, "=== DataCollector Initialization Complete ===");
    return true;
}

void DataCollector::loop()
{
    unsigned long currentTime = millis();

    if (currentTime - lastCapture >= CAPTURE_INTERVAL_MS)
    {
        ESP_LOGD(TAG, "=== Starting New Capture Cycle ===");
        ESP_LOGV(TAG, "Time since last capture: %lu ms", currentTime - lastCapture);

        digitalWrite(LED_BUILTIN, LOW); // Turn ON
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH); // Turn OFF
        ESP_LOGV(TAG, "LED ON - Starting capture");

        if (!camera->capture().isOk())
        {
            ESP_LOGE(TAG, "Camera capture failed: %s", camera->exception.toString().c_str());
            ESP_LOGD(TAG, "Waiting for next capture cycle...");
            lastCapture = currentTime;
            return;
        }
        ESP_LOGI(TAG, "Image captured successfully");

        // Save RGB565 raw data
        String rgb_path = "/picture" + String(imageCount) + ".rgb";
        File rgb_file = SD.open(rgb_path.c_str(), FILE_WRITE);
        if (rgb_file)
        {
            size_t bytesWritten = rgb_file.write(camera->frame->buf, camera->frame->len);
            rgb_file.close();
            ESP_LOGI(TAG, "RGB file saved: %s (Size: %u bytes)", rgb_path.c_str(), bytesWritten);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to create RGB file: %s", rgb_path.c_str());
        }

        // Convert and save JPEG
        uint8_t *jpg_buf = NULL;
        size_t jpg_len = 0;
        bool converted = convert_rgb565_to_jpeg(camera->frame->buf, camera->frame->width,
                                                camera->frame->height, &jpg_buf, &jpg_len);

        if (converted)
        {
            String jpg_path = "/picture" + String(imageCount) + ".jpg";
            File jpg_file = SD.open(jpg_path.c_str(), FILE_WRITE);
            if (jpg_file)
            {
                jpg_file.write(jpg_buf, jpg_len);
                jpg_file.close();
                ESP_LOGI(TAG, "Saved JPG file: %s", jpg_path.c_str());
                imageCount++;
            }
            else
            {
                ESP_LOGE(TAG, "Failed to create JPG file: %s", jpg_path.c_str());
            }
            free(jpg_buf);
        }

        ESP_LOGV(TAG, "LED OFF - Capture complete");

        lastCapture = currentTime;
        ESP_LOGI(TAG, "=== Capture Cycle Complete ===");
        ESP_LOGD(TAG, "Total images captured: %d", imageCount);
        ESP_LOGV(TAG, "Next capture in %d ms", CAPTURE_INTERVAL_MS);
    }
}