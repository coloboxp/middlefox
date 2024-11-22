/**
 * Auto-capture images to SD card every 10 seconds
 * with consecutive filenames
 */

#include <Arduino.h>
#include "ble_service.h"
#include "config.h"
#include "data_collector.h"
#include "preview_service.h"
#include "camera_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <Wire.h>
#include "display_manager.h"
#include <string.h>
#include "buzzer_manager.h"

const char *TAG = "Main";

// Global objects
CustomBLEService bleService;
PreviewService previewService(&bleService);
#ifdef PRODUCTION_MODE
ModelInference inference(&bleService);
#else
DataCollector collector(&bleService);
#endif

// Task handles
TaskHandle_t bleTaskHandle = NULL;
TaskHandle_t mainTaskHandle = NULL;

DISPLAY_MODEL u8g2(U8G2_R1, /* reset=*/U8X8_PIN_NONE);
auto &display = DisplayManager::getInstance();
auto &buzzer = BuzzerManager::getInstance();

void mainTask(void *parameter);
void bleTask(void *parameter);

void setup()
{
  // Serial.begin(115200);
  delay(100); // Give some time for serial to initialize

  ESP_LOGI("Main", "Starting initialization sequence");

  // Initialize buzzer first
  if (!buzzer.begin(D3))
  {
    ESP_LOGE("Main", "Failed to initialize buzzer");
  }
  else
  {
    ESP_LOGI("Main", "Buzzer initialized");
    buzzer.playBootTone();
  }

  // Initialize display with error checking
  ESP_LOGI("Main", "Initializing display");

  // Create display object with explicit pin configuration
  DISPLAY_MODEL u8g2(U8G2_R0, U8X8_PIN_NONE);
  // u8x8(U8X8_PIN_NONE, 5, 4);  // SCL=5, SDA=4

  if (!display.begin(&u8g2))
  {
    ESP_LOGE("Main", "Failed to initialize display");
    return;
  }

  delay(100); // Give display time to stabilize

  while (!display.isReady())
  {
    ESP_LOGE("Main", "Display not ready after initialization");
    return;
  }

  ESP_LOGI("Main", "Clearing display");
  display.clear();

  delay(50);

  // Add delay to ensure stable power and initialization
  ESP_LOGI(TAG, "Delaying for 500ms");
  delay(500);

  // Only wait for Serial in debug builds
#ifdef DEBUG
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 3000))
  {
    delay(10);
  }
#endif

  ESP_LOGI(TAG, "Printing to display");
  display.clearBuffer();
  display.writeCenter("Starting...", 0);
  display.sendBuffer();

  ESP_LOGI(TAG, "Device starting");

  // Initialize BLE before creating tasks
  ESP_LOGI(TAG, "Initializing BLE");
  bleService.begin();
  ESP_LOGI(TAG, "BLE initialized");
  delay(100); // Give BLE time to initialize

  // Create BLE task with higher priority and larger stack
  ESP_LOGI(TAG, "Creating BLE task");
  xTaskCreatePinnedToCore(
      bleTask,
      "BLE Task",
      16384,
      NULL,
      2,
      &bleTaskHandle,
      0);

  // Create main task
  ESP_LOGI(TAG, "Creating main task");
  xTaskCreatePinnedToCore(
      mainTask,
      "Main Task",
      8192,
      NULL,
      1,
      &mainTaskHandle,
      1);

  ESP_LOGI(TAG, "Playing boot tone");
  buzzer.playBootTone();

  delay(1000);

  buzzer.playWhatTheFoxSaid();

  display.clearBuffer();
  display.writeCenter("MiddleFox Started", 3);
  display.sendBuffer();

  display.clearBuffer();
  display.writeCenter("Ready", 7);
  display.sendBuffer();
}

void loop()
{
  // Empty - tasks handle everything
  vTaskDelete(NULL);
}

// Task for main operations
void mainTask(void *parameter)
{
  static bool systemInitialized = false;

  ESP_LOGI("Main Task", "Starting main operations task");

  while (true)
  {
    // Check for state changes
    bool isPreviewEnabled = bleService.isPreviewEnabled();
    bool isOperationEnabled = bleService.isOperationEnabled();

    // Initialize system when first command is received
    if (!systemInitialized && (isPreviewEnabled || isOperationEnabled))
    {
      ESP_LOGI(TAG, "Initializing system in %s mode",
               isPreviewEnabled ? "preview" : "capture");

      // Initialize camera with appropriate mode
      if (!CameraManager::getInstance().begin(isPreviewEnabled))
      {
        ESP_LOGE(TAG, "Camera initialization failed!");
        bleService.notifyClients("Camera initialization failed!");
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }

      // Initialize appropriate service
      if (isPreviewEnabled)
      {
        if (!previewService.begin())
        {
          ESP_LOGE(TAG, "Preview service initialization failed!");
          bleService.notifyClients("Preview service initialization failed!");
          vTaskDelay(pdMS_TO_TICKS(1000));
          continue;
        }
        previewService.enable();
      }
      else
      {
#ifdef PRODUCTION_MODE
        inference.begin();
#else
        if (!collector.begin())
        {
          ESP_LOGE(TAG, "Data collector initialization failed!");
          bleService.notifyClients("Data collector initialization failed!");
          vTaskDelay(pdMS_TO_TICKS(1000));
          continue;
        }
#endif
      }

      systemInitialized = true;
      bleService.notifyClients("System initialized successfully");
    }

    // Check if we need to restart
    if (systemInitialized && !isPreviewEnabled && !isOperationEnabled)
    {
      ESP_LOGI(TAG, "Stop command received - Restarting device");
      bleService.notifyClients("Restarting device to idle state...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      ESP.restart();
    }

    // Run normal operations
    if (systemInitialized)
    {
      if (isPreviewEnabled)
      {
        previewService.loop();
      }
      else if (isOperationEnabled)
      {
#ifdef PRODUCTION_MODE
        inference.loop();
#else
        collector.loop();
#endif
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20)); // 20ms delay
  }
}

// Task for BLE operations
void bleTask(void *parameter)
{
  ESP_LOGI("BLE Task", "Starting BLE service loop...");

  while (true)
  {
    bleService.loop();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
