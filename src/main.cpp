/**
 * Auto-capture images to SD card every 10 seconds
 * with consecutive filenames
 */

#include <Arduino.h>
#include "data_collector.h"
#include "config.h"

// Future implementation
#ifdef PRODUCTION_MODE
ModelInference inference;
#else
DataCollector collector;
#endif

void setup()
{
  Serial.begin(115200);
  delay(3000); // Give time for serial monitor to connect

#ifdef PRODUCTION_MODE
  inference.begin();
#else
  collector.begin();
#endif
}

void loop()
{
#ifdef PRODUCTION_MODE
  inference.loop();
#else
  collector.loop();
#endif
}