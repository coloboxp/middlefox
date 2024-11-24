#include "menu_handler.h"
#include "global_instances.h"
#include "Version.h"
#include "wifi_config_handler.h"

const char *TAG = "MenuHandler";

MenuHandler *MenuHandler::instance = nullptr;

// Define the static menu arrays
const char *const MenuHandler::menuItems[MENU_ITEMS] = {
    "Start Preview",
    "Start Capturing",
    "Start Inferring",
    "Sync Time (WiFi)",
    "Back"};

const char *const MenuHandler::stopMenuItems[2] = {
    "Stop Service",
    "Back"};

const uint8_t MenuHandler::STOP_MENU_ITEMS_COUNT = 2;

MenuHandler::MenuHandler(U8G2 &display) : display(display),
                                          menuPosition(0),
                                          is_redraw(1),
                                          menuActive(false)
{
    instance = this;
}

void MenuHandler::begin()
{
    ESP_LOGI("MenuHandler", "Initializing button on pin %d", BUTTON_PIN);

    // Initialize button with proper configuration
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Configure button event handling
    buttonConfig.setEventHandler(buttonEventHandler);
    buttonConfig.setFeature(ButtonConfig::kFeatureClick);
    buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);

    // Adjusted timings for better response
    buttonConfig.setDebounceDelay(20);
    buttonConfig.setClickDelay(200);
    buttonConfig.setDoubleClickDelay(400);
    buttonConfig.setLongPressDelay(1000);

    button.init(&buttonConfig, BUTTON_PIN, HIGH, 0); // HIGH when not pressed (pull-up)

    ESP_LOGI("MenuHandler", "Button initialized successfully");
}

void MenuHandler::buttonEventHandler(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    ESP_LOGV("MenuHandler", "Raw button event: type=%d state=%d", eventType, buttonState);
    if (instance)
    {
        instance->handleEvent(button, eventType, buttonState);
    }
}

void MenuHandler::handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    ESP_LOGV("MenuHandler", "Button event received: %d", eventType);

    switch (eventType)
    {
    case AceButton::kEventLongPressed:
        if (!menuActive)
        {
            ESP_LOGI("MenuHandler", "Long press detected - showing menu");
            menuActive = true;
            is_redraw = true;
        }
        else
        {
            ESP_LOGI("MenuHandler", "Long press detected - executing item %d (%s)", menuPosition, menuItems[menuPosition]);
            executeMenuItem();
            is_redraw = true;
        }
        break;

    case AceButton::kEventClicked:
        if (menuActive)
        {
            ESP_LOGI("MenuHandler", "Menu click - next item");
            menuPosition = (menuPosition + 1) % MENU_ITEMS;
            is_redraw = true;
        }
        break;
    }
}

void MenuHandler::executeMenuItem()
{
    // If any service is active, handle stop action
    if (bleService.isPreviewEnabled() || bleService.isCaptureEnabled() || bleService.isInferenceEnabled())
    {
        if (menuPosition == 0)
        { // "Stop Service"
            ESP_LOGI(TAG, "Stopping active service and restarting device");

            // Stop all services
            if (bleService.isPreviewEnabled())
            {
                bleService.handleControlCommand(CustomBLEService::Command::STOP_PREVIEW);
            }
            if (bleService.isCaptureEnabled())
            {
                bleService.handleControlCommand(CustomBLEService::Command::STOP_DATA_COLLECTION);
            }
            if (bleService.isInferenceEnabled())
            {
                bleService.handleControlCommand(CustomBLEService::Command::STOP_INFERENCE);
            }

            // Wait a moment for services to stop
            delay(1000);

            // Restart device
            ESP.restart();
        }
        menuActive = false;
        return;
    }

    // Original menu handling for when no service is active
    switch (menuPosition)
    {
    case 0: // Start Preview
        ESP_LOGI(TAG, "Starting preview from menu");
        bleService.handleControlCommand(CustomBLEService::Command::START_PREVIEW);
        menuActive = false;
        break;

    case 1: // Start Capturing
        ESP_LOGI(TAG, "Starting data collection from menu");
        bleService.handleControlCommand(CustomBLEService::Command::START_DATA_COLLECTION);
        menuActive = false;
        break;

    case 2: // Start Inferring
#ifdef PRODUCTION_MODE
        ESP_LOGI(TAG, "Starting inference from menu");
        bleService.handleControlCommand(CustomBLEService::Command::START_INFERENCE);
#endif
        menuActive = false;
        break;

    case 3: // Sync Time (WiFi)
        ESP_LOGI(TAG, "Starting WiFi time sync");
        display.clearBuffer();
        display.setFont(u8g2_font_4x6_tf);
        display.drawStr(0, 20, "Syncing time...");
        display.sendBuffer();

        if (WiFiConfigHandler::syncTimeFromWiFi())
        {
            display.clearBuffer();
            display.drawStr(0, 20, "Time synced!");
            display.sendBuffer();
            delay(2000);
            ESP.restart();
        }
        else
        {
            display.clearBuffer();
            display.drawStr(0, 20, "Sync failed!");
            display.sendBuffer();
            delay(2000);
        }
        menuActive = false;
        break;

    case 4: // Back
        menuActive = false;
        break;
    }
}

void MenuHandler::drawMenu()
{
    display.clearBuffer();
    display.setFont(u8g2_font_4x6_tf);
    display.drawStr(0, 6, "Menu");

    const char *const *currentMenuItems;
    uint8_t itemCount;

    // Determine which menu to show
    if (bleService.isPreviewEnabled() || bleService.isCaptureEnabled() || bleService.isInferenceEnabled())
    {
        currentMenuItems = stopMenuItems;
        itemCount = STOP_MENU_ITEMS_COUNT;
        // Ensure menuPosition is valid for shorter menu
        if (menuPosition >= itemCount)
        {
            menuPosition = 0;
        }
    }
    else
    {
        currentMenuItems = menuItems;
        itemCount = MENU_ITEMS;
    }

    // Draw menu items
    for (uint8_t i = 0; i < itemCount; i++)
    {
        if (i == menuPosition)
        {
            display.drawStr(0, 20 + (i * 10), ">");
        }
        display.drawStr(8, 20 + (i * 10), currentMenuItems[i]);
    }

    display.drawStr(0, 63, "Click: Next  Long: Select");
    display.sendBuffer();
}

void MenuHandler::drawDefaultScreen()
{
    ESP_LOGV("MenuHandler", "Drawing default screen");
    display.clearBuffer();

    // Cache icon data to avoid reloading every frame
    static uint8_t *iconData = nullptr;
    static bool iconLoaded = false;

    if (!iconLoaded)
    {
        Icons::IconInfo iconInfo = Icons::getIconXBM(Icons::Type::FOX_3, Icons::Polarity::POSITIVE);
        if (iconInfo.data != nullptr)
        {
            iconData = const_cast<uint8_t *>(iconInfo.data);
            iconLoaded = true;
            ESP_LOGI("MenuHandler", "Fox icon loaded successfully");
        }
    }

    // Draw the icon if loaded
    if (iconData)
    {
        display.drawXBM(64, 0, 64, 64, iconData);
    }

    // Use smaller font for all text
    display.setFont(u8g2_font_4x6_tf);

    // Draw version and build info
    display.drawStr(0, 6, ("MiddleFox v" + String(VERSION)).c_str());
    display.drawStr(0, 12, ("b:" + String(BUILD_TIMESTAMP)).c_str());
    // Get current time from RTC singleton
    display.drawStr(0, 18, rtc.getFormattedDateTime().c_str());

    // Draw status information
    display.drawStr(0, 24, "BLE:");
    display.drawStr(20, 24, bleService.isConnected() ? "Connected" : "---");

    display.drawStr(0, 32, "Mode:");
    if (bleService.isCaptureEnabled())
    {
        display.drawStr(20, 32, "Capturing");
    }
    else if (bleService.isInferenceEnabled())
    {
        display.drawStr(20, 32, "Inferring");
    }
    else
    {
        display.drawStr(20, 32, "Ready");
    }

    // Draw hint at bottom
    display.drawStr(0, 63, "Long press for menu");

    display.sendBuffer();
}

void MenuHandler::update()
{
    // Check button state
    button.check();

    // Only update display if needed
    static unsigned long lastUpdate = 0;
    static bool firstDraw = true;

    if (firstDraw || is_redraw || (millis() - lastUpdate > 1000))
    { // Update every second or when needed
        ESP_LOGV("MenuHandler", "Updating display");

        if (menuActive)
        {
            drawMenu();
        }
        else
        {
            drawDefaultScreen();
        }

        lastUpdate = millis();
        firstDraw = false;
        is_redraw = false;
    }
}