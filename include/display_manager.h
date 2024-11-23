#pragma once
#include <Arduino.h>
#include "U8g2lib.h"
#include "xbm_icon.h"
#include "config.h"

class DisplayManager
{
private:
    static const char *TAG;
    static DisplayManager *instance;

    DISPLAY_MODEL *display;
    bool isInitialized;

    // Private constructor
    DisplayManager() : display(nullptr), isInitialized(false) {}

public:
    static DisplayManager& getInstance();

    bool begin(DISPLAY_MODEL *displayDriver);
    void clear();
    void refresh();
    void print(const char *text);
    void printAt(const char *text, int x, int y);
    void drawXBM(const XBMIcon &icon, int x, int y);
    void setFont(const uint8_t *font);
    void drawFrame(int x, int y, int width, int height);
    void drawBox(int x, int y, int width, int height);
    u8g2_uint_t getDisplayWidth() const;
    u8g2_uint_t getDisplayHeight() const;
    bool isReady() const { return isInitialized && display != nullptr; }

    // Prevent copying
    DisplayManager(DisplayManager const&) = delete;
    void operator=(DisplayManager const&) = delete;
};