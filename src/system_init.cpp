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
    try
    {
        u8g2.begin();
        DisplayManager::getInstance().begin(&u8g2);
        ESP_LOGI(TAG, "Display initialized successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Display initialization failed: %s", e.what());
        return false;
    }
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
    display.print("Starting...");
    display.refresh();
}