#pragma once

#include <U8g2lib.h>
#include <vector>
#include <string>
#include <Wire.h>
#include "esp_log.h"
#include "esp_task_wdt.h"

class DisplayManager
{
private:
    static const char *TAG;
    static DisplayManager *instance;
    DISPLAY_MODEL *display;
    bool isInitialized;

    // Screen properties
    static const uint8_t SCREEN_WIDTH = 16; // Characters in width
    static const uint8_t SCREEN_HEIGHT = 8; // Characters in height

    // Log buffer
    static const uint8_t MAX_LOG_LINES = 32;
    std::vector<std::string> logBuffer;
    int logScrollPosition = 0;

    // Scroll state
    bool needsRefresh = false;

    static const uint32_t I2C_TIMEOUT_MS = 1000; // 1 second timeout
    static const uint8_t MAX_RETRIES = 3;
    static const uint32_t WDT_TIMEOUT_MS = 3000;       // 3 second watchdog timeout
    static const uint32_t OPERATION_TIMEOUT_MS = 1000; // 1 second operation timeout

    bool performWithTimeout(std::function<bool()> operation, const char *operationName)
    {
        unsigned long startTime = millis();
        esp_task_wdt_reset(); // Reset watchdog timer

        ESP_LOGI(TAG, "Starting %s", operationName);
        bool result = operation();

        unsigned long duration = millis() - startTime;
        if (duration > 100)
        { // Log warning if operation takes more than 100ms
            ESP_LOGW(TAG, "%s took %lu ms", operationName, duration);
        }

        return result;
    }

    bool initializeDisplay()
    {
        ESP_LOGI(TAG, "Starting display initialization sequence");

        // Configure watchdog
        esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);
        esp_task_wdt_add(NULL);

        bool success = performWithTimeout([this]()
                                          {
            if (!display->begin()) return false;
            delay(10);
            
            display->setFlipMode(1);
            display->setFont(u8g2_font_boutique_bitmap_9x9_tf);
            display->clearBuffer();
            
            return true; }, "display initialization");

        esp_task_wdt_delete(NULL);
        return success;
    }

    // Private constructor for singleton
    DisplayManager() : display(nullptr), isInitialized(false), logScrollPosition(0), needsRefresh(false)
    {
        ESP_LOGI(TAG, "Creating DisplayManager instance");
        logBuffer.reserve(MAX_LOG_LINES);
    }

public:
    static DisplayManager &getInstance()
    {
        if (instance == nullptr)
        {
            instance = new DisplayManager();
            ESP_LOGI(TAG, "DisplayManager instance created");
        }
        return *instance;
    }

    bool begin(DISPLAY_MODEL *displayPtr)
    {
        ESP_LOGI(TAG, "Initializing display manager");
        unsigned long startTime = millis();

        if (!displayPtr)
        {
            ESP_LOGE(TAG, "Display pointer is null");
            return false;
        }

        display = displayPtr;

        if (!initializeDisplay())
        {
            ESP_LOGE(TAG, "Display initialization failed after %d ms",
                     (int)(millis() - startTime));
            display = nullptr;
            isInitialized = false;
            return false;
        }

        isInitialized = true;
        ESP_LOGI(TAG, "Display manager initialized successfully in %d ms",
                 (int)(millis() - startTime));
        return true;
    }

    bool isReady() const
    {
        if (!isInitialized || !display)
        {
            ESP_LOGW(TAG, "Display not ready - initialized: %d, display: %p",
                     isInitialized, display);
        }
        return isInitialized && display != nullptr;
    }

    void clear()
    {
        if (!isReady())
        {
            ESP_LOGW(TAG, "Cannot clear - display not ready");
            return;
        }
        performWithTimeout([this]()
                           {
            display->clearBuffer();
            display->sendBuffer();
            return true; }, "clear display");
    }

    void refresh()
    {
        if (!isReady())
        {
            ESP_LOGW(TAG, "Cannot refresh - display not ready");
            return;
        }
        if (needsRefresh)
        {
            ESP_LOGD(TAG, "Refreshing display");
            performWithTimeout([this]()
                               {
                display->sendBuffer();
                return true; }, "refresh display");
            needsRefresh = false;
        }
    }

    void setCursor(uint8_t x, uint8_t y)
    {
        if (!isReady())
        {
            ESP_LOGW(TAG, "Cannot set cursor - display not ready");
            return;
        }
        ESP_LOGV(TAG, "Setting cursor to (%d, %d)", x, y);
        display->setCursor(x, y);
    }

    void print(const char *text)
    {
        if (!isReady() || !text)
        {
            ESP_LOGW(TAG, "Cannot print - display not ready or null text");
            return;
        }
        ESP_LOGD(TAG, "Printing text: %s", text);
        display->clearBuffer();
        display->drawStr(0, 10, text); // Y position 10 to account for font height
        display->sendBuffer();
        needsRefresh = true;
    }

    void println(const char *text)
    {
        if (!isReady())
        {
            ESP_LOGW(TAG, "Cannot println - display not ready");
            return;
        }
        if (!text)
        {
            ESP_LOGW(TAG, "Cannot println - null text");
            return;
        }
        ESP_LOGD(TAG, "Printing line: %s", text);
        display->print(text);
        display->print("\n");
        needsRefresh = true;
    }

    void writeCenter(const char *text, uint8_t row)
    {
        if (!isReady() || !text)
        {
            ESP_LOGW(TAG, "Cannot write centered - display not ready or null text");
            return;
        }

        performWithTimeout([this, text, row]()
        {
            // Calculate horizontal center
            int16_t width = display->getStrWidth(text);
            int16_t x = (display->getDisplayWidth() - width) / 2;
            if (x < 0) x = 0;
            
            // Calculate vertical position
            // Get the total height of display and divide it into rows
            int16_t displayHeight = display->getDisplayHeight();
            int16_t rowHeight = displayHeight / 8;  // Typical 8 rows for 64 pixel display
            
            // Calculate Y position: multiply row by height and add font ascent for proper alignment
            int16_t yPos = (row * rowHeight) + display->getAscent() + (rowHeight/2);
            
            display->drawStr(x, yPos, text);
            needsRefresh = true;
            return true;
        }, "write centered text");
    }

    void log(const char *text)
    {
        if (!text)
        {
            ESP_LOGW(TAG, "Cannot log - null text");
            return;
        }

        ESP_LOGD(TAG, "Logging text: %s", text);

        if (logBuffer.size() >= MAX_LOG_LINES)
        {
            ESP_LOGV(TAG, "Log buffer full, removing oldest entry");
            logBuffer.erase(logBuffer.begin());
        }
        logBuffer.push_back(std::string(text));

        if (!isReady())
        {
            ESP_LOGW(TAG, "Cannot update display with log - display not ready");
            return;
        }

        clear();
        int startIdx = logBuffer.size() - SCREEN_HEIGHT + logScrollPosition;
        if (startIdx < 0)
            startIdx = 0;

        ESP_LOGV(TAG, "Updating display with logs, starting from index %d", startIdx);
        for (int i = 0; i < SCREEN_HEIGHT && (startIdx + i) < logBuffer.size(); i++)
        {
            setCursor(0, i);
            print(logBuffer[startIdx + i].c_str());
        }
    }

    void scrollLog(int direction)
    {
        ESP_LOGD(TAG, "Scrolling log, direction: %d", direction);
        logScrollPosition += direction;

        if (logScrollPosition > 0)
        {
            ESP_LOGV(TAG, "Scroll position capped at top");
            logScrollPosition = 0;
        }
        if (logScrollPosition < -(int)(logBuffer.size() - SCREEN_HEIGHT))
        {
            ESP_LOGV(TAG, "Scroll position capped at bottom");
            logScrollPosition = -(logBuffer.size() - SCREEN_HEIGHT);
        }

        log(""); // Trigger redraw
    }

    void drawBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t cnt, u8g2_uint_t h, const uint8_t *bitmap)
    {
        display->drawBitmap(x, y, cnt, h, bitmap);
    }

    // Drawing methods
    void drawLine(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2)
    {
        if (!isReady())
            return;
        display->drawLine(x1, y1, x2, y2);
        needsRefresh = true;
    }

    void drawCircle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad)
    {
        if (!isReady())
            return;
        display->drawCircle(x0, y0, rad);
        needsRefresh = true;
    }

    void drawDisc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad)
    {
        if (!isReady())
            return;
        display->drawDisc(x0, y0, rad);
        needsRefresh = true;
    }

    void drawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
    {
        if (!isReady())
            return;
        display->drawBox(x, y, w, h);
        needsRefresh = true;
    }

    void drawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
    {
        if (!isReady())
            return;
        display->drawFrame(x, y, w, h);
        needsRefresh = true;
    }

    // Font and text methods
    void setFont(const uint8_t *font)
    {
        if (!isReady())
            return;
        display->setFont(font);
    }

    void setFontMode(uint8_t mode)
    {
        if (!isReady())
            return;
        display->setFontMode(mode);
    }

    void setDrawColor(uint8_t color)
    {
        if (!isReady())
            return;
        display->setDrawColor(color);
    }

    int8_t getAscent() const
    {
        return isReady() ? display->getAscent() : 0;
    }

    int8_t getDescent() const
    {
        return isReady() ? display->getDescent() : 0;
    }

    // Display control methods
    void sendBuffer()
    {
        if (!isReady())
            return;
        performWithTimeout([this]()
                           {
            display->sendBuffer();
            return true; }, "send buffer");
    }

    void clearBuffer()
    {
        if (!isReady())
            return;
        display->clearBuffer();
    }

    // Drawing state methods
    void setClipWindow(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t x1, u8g2_uint_t y1)
    {
        if (!isReady())
            return;
        display->setClipWindow(x0, y0, x1, y1);
    }

    void setMaxClipWindow()
    {
        if (!isReady())
            return;
        display->setMaxClipWindow();
    }

    // Additional utility methods
    u8g2_uint_t getDisplayWidth() const
    {
        return isReady() ? display->getDisplayWidth() : 0;
    }

    u8g2_uint_t getDisplayHeight() const
    {
        return isReady() ? display->getDisplayHeight() : 0;
    }

    // Prevent copying
    DisplayManager(DisplayManager const &) = delete;
    void operator=(DisplayManager const &) = delete;
};

// Initialize the static member
DisplayManager *DisplayManager::instance = nullptr;
const char *DisplayManager::TAG = "DisplayManager";