#include "menu_handler.h"
#include "global_instances.h"
#include "Version.h"

const char *TAG = "MenuHandler";

MenuHandler *MenuHandler::instance = nullptr;
const char *MenuHandler::menuItems[MENU_ITEMS] = {
    "Start Preview",
    "Start Capturing",
    "Start Inferring",
    "Back"};

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
    ESP_LOGD("MenuHandler", "Raw button event: type=%d state=%d", eventType, buttonState);
    if (instance)
    {
        instance->handleEvent(button, eventType, buttonState);
    }
}

void MenuHandler::handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
    ESP_LOGD("MenuHandler", "Button event received: %d", eventType);

    switch (eventType)
    {
    case AceButton::kEventLongPressed:
        if (!menuActive) {
            ESP_LOGI("MenuHandler", "Long press detected - showing menu");
            menuActive = true;
            is_redraw = true;
        } else {
            ESP_LOGI("MenuHandler", "Long press detected - executing item %d", menuPosition);
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
    switch (menuPosition)
    {
    case 0: // Start Preview
        previewService.begin();
        menuActive = false;
        break;

    case 1: // Start Capturing
        collector.begin();
        menuActive = false;
        break;

    case 2: // Start Inferring
#ifdef PRODUCTION_MODE
        inference.begin();
#endif
        menuActive = false;
        break;

    case 3: // Back
        menuActive = false;
        break;
    }
}

void MenuHandler::drawMenu()
{
    display.clearBuffer();

    // Draw menu title
    display.setFont(u8g2_font_4x6_tf);
    display.drawStr(0, 6, "Menu");

    // Draw menu items
    for (uint8_t i = 0; i < MENU_ITEMS; i++)
    {
        if (i == menuPosition)
        {
            display.drawStr(0, 20 + (i * 10), ">");
        }
        display.drawStr(8, 20 + (i * 10), menuItems[i]);
    }

    // Draw controls hint at bottom
    display.drawStr(0, 63, "Click: Next  Long: Select");

    display.sendBuffer();
}

void MenuHandler::drawDefaultScreen()
{
    ESP_LOGD("MenuHandler", "Drawing default screen");
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
        ESP_LOGD("MenuHandler", "Updating display");

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