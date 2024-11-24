#pragma once

#include <PCF8563.h>
#include <Wire.h>
#include "esp_log.h"

class RTCManager {
private:
    static RTCManager* instance;
    static const char* TAG;
    
    PCF8563 rtc;
    bool isInitialized;
    
    RTCManager() : isInitialized(false) {}

public:
    static RTCManager& getInstance() {
        if (instance == nullptr) {
            instance = new RTCManager();
        }
        return *instance;
    }

    bool begin() {
        if (!isInitialized) {
            Wire.begin();
            rtc.init();
            isInitialized = true;
            ESP_LOGI(TAG, "RTC initialized");
        }
        return isInitialized;
    }

    PCF8563* getRTC() { return &rtc; }

    void setDateTime(int year, int month, int day, int hour, int minute, int second) {
        if (!isInitialized) begin();
        rtc.stopClock();
        rtc.setYear(year);
        rtc.setMonth(month);
        rtc.setDay(day);
        rtc.setHour(hour);
        rtc.setMinut(minute);
        rtc.setSecond(second);
        rtc.startClock();
    }

    String getFormattedDateTime() {
        if (!isInitialized) begin();
        Time now = rtc.getTime();
        char timeStr[20];
        snprintf(timeStr, sizeof(timeStr), "%02d.%02d.%02d %02d:%02d",
                now.day, now.month, now.year, now.hour, now.minute);
        return String(timeStr);
    }
}; 