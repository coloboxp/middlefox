#pragma once

#include <AceButton.h>
#include <U8g2lib.h>
#include <esp_log.h>
#include "config.h"
#include "system_init.h"
#include "global_instances.h"
#include "Version.h"

using namespace ace_button;

class MenuHandler
{
public:
    MenuHandler(U8G2 &display);
    void begin();
    void update();
    void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState);

private:
    static void buttonEventHandler(AceButton *button, uint8_t eventType, uint8_t buttonState);
    void drawMenu();
    void executeMenuItem();
    void drawDefaultScreen();

    U8G2 &display;
    AceButton button;
    ButtonConfig buttonConfig;

    uint8_t menuPosition;
    uint8_t is_redraw;
    bool menuActive;

    static const uint8_t MENU_ITEMS = 3;
    static const char *menuItems[MENU_ITEMS];
    static MenuHandler *instance;
};