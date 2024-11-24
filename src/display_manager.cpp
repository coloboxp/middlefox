#include "display_manager.h"
#include "Version.h"

const char *DisplayManager::TAG = "DisplayManager";

DisplayManager *DisplayManager::instance = nullptr;

DisplayManager &DisplayManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new DisplayManager();
    }
    return *instance;
}

DisplayManager::DisplayManager() : display(nullptr), initialized(false)
{
    ESP_LOGI(TAG, "Creating DisplayManager");
}

DisplayManager::~DisplayManager()
{
    if (display)
    {
        delete display;
    }
}

bool DisplayManager::begin()
{
    ESP_LOGI(TAG, "Initializing display...");

    if (initialized)
    {
        ESP_LOGW(TAG, "Display already initialized");
        return true;
    }

    display = new DISPLAY_MODEL(U8G2_R0, U8X8_PIN_NONE);
    if (!display || !display->begin())
    {
        ESP_LOGE(TAG, "Display initialization failed");
        return false;
    }

    setupDefaults();
    initialized = true;
    ESP_LOGI(TAG, "Display initialized successfully");
    return true;
}

void DisplayManager::setupDefaults()
{
    if (!display)
        return;
    display->setFont(u8g2_font_4x6_tf);
    display->setDrawColor(1);
    display->setFontPosTop();
    clear();
    refresh();
}

void DisplayManager::clear()
{
    if (!display)
        return;
    display->clearBuffer();
}

void DisplayManager::refresh()
{
    if (!display)
        return;
    display->sendBuffer();
}

void DisplayManager::print(const char *text)
{
    if (!text)
    {
        ESP_LOGW(TAG, "Attempted to print null text");
        return;
    }

    try
    {
        checkInit();

        ESP_LOGD(TAG, "Printing text: %s", text);

        if (strlen(text) >= 128)
        {
            ESP_LOGW(TAG, "Text too long, truncating");
            return;
        }

        // Get current cursor position
        u8g2_uint_t x = display->getCursorX();
        u8g2_uint_t y = display->getCursorY();

        // Ensure we're within display bounds
        if (y == 0)
            y = 10; // minimum y to show text
        if (x >= display->getDisplayWidth())
            x = 0;

        display->drawStr(x, y, text);
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error in print: %s", e.what());
    }
}

void DisplayManager::printAt(const char *text, int x, int y)
{
    if (!text)
        return;

    try
    {
        checkInit();
        display->drawStr(x, y, text);
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error in printAt: %s", e.what());
    }
}

void DisplayManager::drawXBM(const XBMIcon &icon, int x, int y)
{
    try
    {
        checkInit();
        display->drawXBM(x, y, icon.width(), icon.height(), icon.getData());
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error in drawXBM: %s", e.what());
    }
}

void DisplayManager::drawXBM(int x, int y, int width, int height, const unsigned char *bitmap)
{
    if (!display)
        return;
    display->drawXBM(x, y, width, height, bitmap);
}

void DisplayManager::setFont(const uint8_t *font)
{
    if (!display)
        return;
    display->setFont(font);
}

void DisplayManager::drawFrame(int x, int y, int width, int height)
{
    try
    {
        checkInit();
        display->drawFrame(x, y, width, height);
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error in drawFrame: %s", e.what());
    }
}

void DisplayManager::drawBox(int x, int y, int width, int height)
{
    try
    {
        checkInit();
        display->drawBox(x, y, width, height);
    }
    catch (const std::exception &e)
    {
        ESP_LOGE(TAG, "Error in drawBox: %s", e.what());
    }
}

u8g2_uint_t DisplayManager::getDisplayWidth() const
{
    return display ? display->getDisplayWidth() : 0;
}

u8g2_uint_t DisplayManager::getDisplayHeight() const
{
    return display ? display->getDisplayHeight() : 0;
}

void DisplayManager::checkInit()
{
    if (!isInitialized())
    {
        ESP_LOGE(TAG, "Display not initialized!");
        throw std::runtime_error("Display not initialized");
    }
}
