#include "system_init.h"
#include "global_instances.h"
#include "xbm_icon.h"
#include "Version.h"

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
        if (success)
        {
            ESP_LOGI(TAG, "BLE initialized successfully");
        }
        else
        {
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
    auto &display = DisplayManager::getInstance();
    if (!display.isInitialized())
    {
        ESP_LOGE(TAG, "Display not initialized!");
        return;
    }

    try
    {
        display.clear();

        // Get the icon data
        Icons::IconInfo iconInfo = Icons::getIconXBM(Icons::Type::FOX_3, Icons::Polarity::POSITIVE);
        if (iconInfo.data != nullptr)
        {
            // Calculate Y position to draw from bottom-up
            int yPos = display.getDisplayHeight() - iconInfo.height;
            ESP_LOGI(TAG, "Drawing fox icon at %d, %d, %d, %d",
                     64, yPos, iconInfo.width, iconInfo.height);
            display.drawXBM(64, yPos, iconInfo.width, iconInfo.height, iconInfo.data);
            Icons::freeXBMData(iconInfo);
        }

        display.printAt(("v" + String(VERSION)).c_str(), 0, 0);
        display.printAt(("b: " + String(BUILD_TIMESTAMP)).c_str(), 0, 8);
        display.printAt("",0,16);
        display.printAt("MiddleFox...", 0, 24);
        display.printAt("Started!", 0, 32);

        display.refresh();
        ESP_LOGI(TAG, "Startup icons shown successfully");
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error showing startup icons: %s", e.what());
    }
}

bool initSystem()
{
    // Initialize BLE first
    CustomBLEService bleService;
    if (!bleService.begin())
    {
        return false;
    }

    // Initialize camera manager
    if (!CameraManager::getInstance().begin(false))
    {
        return false;
    }

    // Initialize data collector
    DataCollector dataCollector(&bleService);
    if (!dataCollector.begin())
    {
        return false;
    }

    return true;
}