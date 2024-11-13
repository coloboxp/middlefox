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

// TODO: Implement web preview server using Eloquent Esp32cam
CustomBLEService bleService;
PreviewService previewService(&bleService);

#ifdef PRODUCTION_MODE
ModelInference inference(&bleService);
#else
DataCollector collector(&bleService);
#endif

void setup()
{
  Serial.begin(115200);
  
  // Wait for USB CDC
  while (!Serial) {
    delay(10);
  }
  
  Serial.println("\n\n=== Device Starting ===");
  Serial.println("Initializing...");
  delay(1000);

  Serial.println("Starting camera init...");
  // Initialize camera first
  if (!CameraManager::getInstance().begin(true))
  {
    Serial.println("Camera init failed!");
    ESP_LOGE("Main", "Failed to initialize camera - System halted");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("Camera init success!");
  ESP_LOGI("Main", "Camera initialized successfully");

  Serial.println("Starting BLE...");
  bleService.begin();
  Serial.println("Starting Preview Service...");
  previewService.begin();

#ifdef PRODUCTION_MODE
  inference.begin();
#else
  Serial.println("Starting Data Collector...");
  collector.begin();
#endif

  Serial.println("Setup complete!");
}

void loop()
{
  bleService.loop();

  if (bleService.isPreviewEnabled())
  {
    previewService.enable();
    previewService.loop();
  }
  else
  {
    if (previewService.isEnabled())
    {
      previewService.disable();
    }
  }

  if (!bleService.isPreviewEnabled() && bleService.isOperationEnabled())
  {
#ifdef PRODUCTION_MODE
    inference.loop();
#else
    collector.loop();
#endif
  }
  delay(50);
}