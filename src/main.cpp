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
  delay(3000); // Give time for serial monitor to connect

  // Initialize camera first
  if (!CameraManager::getInstance().begin(true))
  {
    ESP_LOGE("Main", "Failed to initialize camera - System halted");
    while (1)
    {
      delay(1000);
    }
  }
  ESP_LOGI("Main", "Camera initialized successfully");

  bleService.begin();
  previewService.begin();

#ifdef PRODUCTION_MODE
  inference.begin();
#else
  collector.begin();
#endif
}

void loop()
{
  bleService.loop();

  if (bleService.isPreviewEnabled())
  {
    previewService.enable();
    previewService.loop();
  }
  else if (previewService.isEnabled())
  {
    previewService.disable();
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