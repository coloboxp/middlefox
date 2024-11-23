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
        bleService.begin();
        ESP_LOGI(TAG, "BLE initialized successfully");
        return true;
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