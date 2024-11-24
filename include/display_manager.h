#pragma once
#include <Arduino.h>
#include "U8g2lib.h"
#include "xbm_icon.h"
#include "config.h"
#include "esp_log.h"

class DisplayManager
{
private:
    static const char *TAG;
    static DisplayManager *instance;
    DISPLAY_MODEL *display;
    bool initialized;

    DisplayManager();
    void setupDefaults();
    void checkInit();

public:
    static DisplayManager &getInstance();
    ~DisplayManager();

    bool begin();
    void clear();
    void refresh();
    void print(const char *text);
    void printAt(const char *text, int x, int y);
    void drawXBM(const XBMIcon &icon, int x, int y);
    void drawXBM(int x, int y, int width, int height, const unsigned char *bitmap);
    void setFont(const uint8_t *font);
    void drawFrame(int x, int y, int width, int height);
    void drawBox(int x, int y, int width, int height);

    u8g2_uint_t getDisplayWidth() const;
    u8g2_uint_t getDisplayHeight() const;
    bool isInitialized() const { return initialized && display != nullptr; }
    DISPLAY_MODEL *getDisplay() { return display; }
};