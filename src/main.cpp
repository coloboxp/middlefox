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

// Task for BLE operations
void bleTask(void *parameter) {
  ESP_LOGI("BLE Task", "Starting BLE service loop...");
  
  while (true) {
    bleService.loop();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task for main operations
void mainTask(void *parameter) {
  static bool systemInitialized = false;
  
  ESP_LOGI("Main Task", "Starting main operations task");

  while (true) {
    // Check for state changes
    bool isPreviewEnabled = bleService.isPreviewEnabled();
    bool isOperationEnabled = bleService.isOperationEnabled();
    
    // Initialize system when first command is received
    if (!systemInitialized && (isPreviewEnabled || isOperationEnabled)) {
      ESP_LOGI("Main", "Initializing system in %s mode", 
               isPreviewEnabled ? "preview" : "capture");
      
      // Initialize camera with appropriate mode
      if (!CameraManager::getInstance().begin(isPreviewEnabled)) {
        ESP_LOGE("Main", "Camera initialization failed!");
        bleService.notifyClients("Camera initialization failed!");
        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;
      }
      
      // Initialize appropriate service
      if (isPreviewEnabled) {
        if (!previewService.begin()) {
          ESP_LOGE("Main", "Preview service initialization failed!");
          bleService.notifyClients("Preview service initialization failed!");
          vTaskDelay(pdMS_TO_TICKS(1000));
          continue;
        }
        previewService.enable();
      } else {
#ifdef PRODUCTION_MODE
        inference.begin();
#else
        if (!collector.begin()) {
          ESP_LOGE("Main", "Data collector initialization failed!");
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
    if (systemInitialized && !isPreviewEnabled && !isOperationEnabled) {
      ESP_LOGI("Main", "Stop command received - Restarting device");
      bleService.notifyClients("Restarting device to idle state...");
      vTaskDelay(pdMS_TO_TICKS(1000));
      ESP.restart();
    }

    // Run normal operations
    if (systemInitialized) {
      if (isPreviewEnabled) {
        previewService.loop();
      } else if (isOperationEnabled) {
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

void setup() {
  Serial.begin(115200);
  
  // Add delay to ensure stable power and initialization
  delay(500);
  
  // Only wait for Serial in debug builds
#ifdef DEBUG
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime < 3000)) {
    delay(10);
  }
#endif
  
  Serial.println("\n\n=== Device Starting ===");
  
  // Initialize BLE before creating tasks
  bleService.begin();
  delay(100); // Give BLE time to initialize
  
  // Create BLE task with higher priority and larger stack
  xTaskCreatePinnedToCore(
    bleTask,
    "BLE Task",
    16384,
    NULL,
    2,
    &bleTaskHandle,
    0
  );

  // Create main task
  xTaskCreatePinnedToCore(
    mainTask,
    "Main Task",
    8192,
    NULL,
    1,
    &mainTaskHandle,
    1
  );
}

void loop() {
  // Empty - tasks handle everything
  vTaskDelete(NULL);
}