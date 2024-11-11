/**
 * Auto-capture images to SD card every 10 seconds
 * with consecutive filenames
 */

#include <Arduino.h>
#include "data_collector.h"
#include "config.h"
#include "ble_service.h"

#ifdef PRODUCTION_MODE
ModelInference inference;
#else
DataCollector collector;
#endif

CustomBLEService bleService;

void setup()
{
  Serial.begin(115200);
  delay(3000); // Give time for serial monitor to connect

  bleService.begin();

#ifdef PRODUCTION_MODE
  inference.begin();
#else
  collector.begin();
#endif
}

void loop()
{
  bleService.loop();

  if (bleService.isOperationEnabled()) {
#ifdef PRODUCTION_MODE
    inference.loop();
#else
    collector.loop();
#endif
  }
}