#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "sd_manager.h"
#include "rtc_manager.h"
#include "esp_log.h"

class WiFiConfigHandler
{
public:
    static bool syncTimeFromWiFi();
    static bool loadWiFiCredentials();
    static bool saveWiFiCredentials(const char *ssid, const char *password);
    static void setRTCTime();

private:
    static const char *TAG;
    static const char *WIFI_CONFIG_FILE;
    static bool connectToWiFi(const char *ssid, const char *password);
    static void configModeCallback(WiFiManager *myWiFiManager);
};