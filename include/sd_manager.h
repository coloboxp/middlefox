#pragma once

#include <SD.h>
#include "esp_log.h"

class SDManager {
private:
    static SDManager* instance;
    static const char* TAG;
    const int SD_CS_PIN = D2;  // XIAO Expansion Board SD CS pin
    bool isInitialized = false;

    SDManager() {}  // Private constructor
    
public:
    static SDManager& getInstance() {
        if (instance == nullptr) {
            instance = new SDManager();
        }
        return *instance;
    }

    bool begin();
    bool end();
    bool isReady() const { return isInitialized; }
    
    // Wrapper methods for common SD operations
    File openFile(const char* path, const char* mode);
    bool exists(const char* path);
    bool remove(const char* path);
    File openDir(const char* path);
}; 