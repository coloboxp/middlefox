#include "wifi_config_handler.h"
#include <ArduinoJson.h>
#include <time.h>

const char* WiFiConfigHandler::TAG = "WiFiConfig";
const char* WiFiConfigHandler::WIFI_CONFIG_FILE = "/wifi_config.json";
const int SD_CS_PIN = D2;

// Bratislava timezone settings
const char* NTP_SERVER = "pool.ntp.org";
const char* TZ_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";  // Central European Time config
                                                     // CET-1CEST: UTC+1 in winter
                                                     // M3.5.0: DST starts last Sunday in March
                                                     // M10.5.0/3: DST ends last Sunday in October

bool WiFiConfigHandler::syncTimeFromWiFi() {
    ESP_LOGI(TAG, "Starting WiFi time sync process");

    if (!SDManager::getInstance().begin()) {
        ESP_LOGE(TAG, "SD Card initialization failed");
        return false;
    }

    if (!loadWiFiCredentials()) {
        ESP_LOGI(TAG, "No stored credentials, starting WiFi Manager");
        WiFiManager wifiManager;
        wifiManager.setConfigPortalTimeout(180); // 3 minute timeout
        wifiManager.setAPCallback(configModeCallback);

        if (!wifiManager.startConfigPortal("MiddleFox_Setup")) {
            ESP_LOGE(TAG, "Failed to connect and hit timeout");
            return false;
        }

        // Save the new credentials
        saveWiFiCredentials(WiFi.SSID().c_str(), WiFi.psk().c_str());
    }

    // Configure timezone and time servers
    configTzTime(TZ_INFO, NTP_SERVER, "time.nist.gov");

    ESP_LOGI(TAG, "Waiting for NTP time sync");
    time_t now = time(nullptr);
    int retry = 0;
    while (now < 8 * 3600 * 2 && retry < 10) {
        delay(500);
        now = time(nullptr);
        retry++;
    }

    if (now < 8 * 3600 * 2) {
        ESP_LOGE(TAG, "NTP sync failed");
        return false;
    }

    // Log current time details
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStringBuf[64];
        strftime(timeStringBuf, sizeof(timeStringBuf), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
        ESP_LOGI(TAG, "Current local time: %s", timeStringBuf);
        ESP_LOGI(TAG, "DST is %s", timeinfo.tm_isdst ? "active" : "inactive");
    }

    setRTCTime();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    return true;
}

bool WiFiConfigHandler::loadWiFiCredentials() {
    if (!SD.exists(WIFI_CONFIG_FILE)) {
        return false;
    }

    File configFile = SD.open(WIFI_CONFIG_FILE, "r");
    if (!configFile) {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        ESP_LOGE(TAG, "Failed to parse config file");
        return false;
    }

    const char* ssid = doc["ssid"];
    const char* pass = doc["pass"];

    return connectToWiFi(ssid, pass);
}

bool WiFiConfigHandler::saveWiFiCredentials(const char* ssid, const char* password) {
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["pass"] = password;

    File configFile = SDManager::getInstance().openFile(WIFI_CONFIG_FILE, "w");
    if (!configFile) {
        ESP_LOGE(TAG, "Failed to open config file for writing");
        return false;
    }

    serializeJson(doc, configFile);
    configFile.close();
    return true;
}

void WiFiConfigHandler::setRTCTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        ESP_LOGE(TAG, "Failed to obtain time");
        return;
    }
    
    RTCManager& rtc = RTCManager::getInstance();
    rtc.setDateTime(
        timeinfo.tm_year - 100,  // Convert from years since 1900 to years since 2000
        timeinfo.tm_mon + 1,     // tm_mon is 0-11, RTC expects 1-12
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );
    
    // Log RTC update
    char timeStringBuf[64];
    strftime(timeStringBuf, sizeof(timeStringBuf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "RTC updated to: %s", timeStringBuf);
}

bool WiFiConfigHandler::connectToWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

void WiFiConfigHandler::configModeCallback(WiFiManager* myWiFiManager) {
    ESP_LOGI(TAG, "Entered config mode: %s", WiFi.softAPIP().toString().c_str());
} 