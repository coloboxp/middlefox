#include "sd_manager.h"

SDManager* SDManager::instance = nullptr;
const char* SDManager::TAG = "SDManager";

bool SDManager::begin() {
    if (isInitialized) return true;

    ESP_LOGI(TAG, "Initializing SD card...");
    pinMode(SD_CS_PIN, OUTPUT);

    // Try multiple times to initialize SD card
    for (int i = 0; i < 3; i++) {
        if (SD.begin(SD_CS_PIN)) {
            uint8_t cardType = SD.cardType();
            if (cardType != CARD_NONE) {
                isInitialized = true;
                ESP_LOGI(TAG, "SD card initialized successfully");
                return true;
            }
        }
        delay(1000);
        SD.end();
    }

    ESP_LOGE(TAG, "SD Card initialization failed");
    return false;
}

bool SDManager::end() {
    if (!isInitialized) return true;
    SD.end();
    isInitialized = false;
    return true;
}

File SDManager::openFile(const char* path, const char* mode) {
    if (!isInitialized && !begin()) {
        ESP_LOGE(TAG, "Cannot open file - SD not initialized");
        return File();
    }
    return SD.open(path, mode);
}

bool SDManager::exists(const char* path) {
    if (!isInitialized && !begin()) return false;
    return SD.exists(path);
}

bool SDManager::remove(const char* path) {
    if (!isInitialized && !begin()) return false;
    return SD.remove(path);
}

File SDManager::openDir(const char* path) {
    if (!isInitialized && !begin()) return File();
    return SD.open(path);
} 