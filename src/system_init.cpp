#include "system_init.h"
#include "global_instances.h"

const char *SystemInitializer::TAG = "SystemInit";

bool SystemInitializer::initializeBuzzer()
{
    try
    {
        BuzzerManager::getInstance().begin(BUZZER_PIN);
        ESP_LOGI(TAG, "Buzzer initialized successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Buzzer initialization failed: %s", e.what());
        return false;
    }
}

bool SystemInitializer::initializeDisplay()
{
    ESP_LOGI(TAG, "Initializing display...");
    return DisplayManager::getInstance().begin();
}

bool SystemInitializer::initializeBLE()
{
    try
    {
        bool success = bleService.begin();
        if (success) {
            ESP_LOGI(TAG, "BLE initialized successfully");
        } else {
            ESP_LOGE(TAG, "BLE initialization failed");
        }
        return success;
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "BLE initialization failed: %s", e.what());
        return false;
    }
}

void SystemInitializer::playStartupSequence()
{
    buzzer.playBootTone();
}

void SystemInitializer::showStartupIcons()
{
    ESP_LOGI(TAG, "Showing startup icons...");
    
    // Get display instance safely
    auto& display = DisplayManager::getInstance();
    if (!display.isInitialized()) {
        ESP_LOGE(TAG, "Display not initialized!");
        return;
    }

    // Add delay to ensure display is ready
    vTaskDelay(pdMS_TO_TICKS(100));

    try {
        display.clear();    
        
        // Print version info with bounds checking
        const char* version = "v1.0.0";  // or whatever your version is
        if (version && strlen(version) < 32) {  // sanity check
            display.print(version);
        }

        // Update display
        display.refresh();
        
        ESP_LOGI(TAG, "Startup icons shown successfully");
    } catch (const std::exception& e) {
        ESP_LOGE(TAG, "Error showing startup icons: %s", e.what());
    }
}

bool initSystem() {
    // Initialize BLE first
    CustomBLEService bleService;
    if (!bleService.begin()) {
        return false;
    }

    // Initialize camera manager
    if (!CameraManager::getInstance().begin(false)) {
        return false;
    }

    // Initialize data collector
    DataCollector dataCollector(&bleService);
    if (!dataCollector.begin()) {
        return false;
    }

    return true;
}