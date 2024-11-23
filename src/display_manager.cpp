#include "display_manager.h"
#include "esp_log.h"
#include "U8g2lib.h"
#include "xbm_icon.h"

// Initialize static members
const char* DisplayManager::TAG = "DisplayManager";

DisplayManager& DisplayManager::getInstance() {
    static DisplayManager instance;
    return instance;
}

bool DisplayManager::begin(DISPLAY_MODEL* displayDriver) {
    if (!displayDriver) {
        ESP_LOGE(TAG, "Display driver is null");
        return false;
    }
    
    this->display = displayDriver;
    this->isInitialized = true;
    ESP_LOGI(TAG, "Display initialized successfully");
    return true;
}

void DisplayManager::clear() {
    if (!isReady()) return;
    this->display->clearBuffer();
}

void DisplayManager::refresh() {
    if (!isReady()) return;
    this->display->sendBuffer();
}

void DisplayManager::print(const char* text) {
    if (!isReady() || !text) return;
    
    clear();
    this->display->drawStr(0, 10, text);
    refresh();
}

void DisplayManager::printAt(const char* text, int x, int y) {
    if (!isReady() || !text) return;
    this->display->drawStr(x, y, text);
}

void DisplayManager::drawXBM(const XBMIcon& icon, int x, int y) {
    if (!isReady() || !icon.isValid()) return;
    
    this->display->drawXBM(
        x, y,
        icon.width(),
        icon.height(),
        icon.data()
    );
}

void DisplayManager::setFont(const uint8_t* font) {
    if (!isReady() || !font) return;
    this->display->setFont(font);
}

void DisplayManager::drawFrame(int x, int y, int width, int height) {
    if (!isReady()) return;
    this->display->drawFrame(x, y, width, height);
}

void DisplayManager::drawBox(int x, int y, int width, int height) {
    if (!isReady()) return;
    this->display->drawBox(x, y, width, height);
}

u8g2_uint_t DisplayManager::getDisplayWidth() const {
    if (!isReady()) return 0;
    return this->display->getDisplayWidth();
}

u8g2_uint_t DisplayManager::getDisplayHeight() const {
    if (!isReady()) return 0;
    return this->display->getDisplayHeight();
}

// ... implement other methods ... 