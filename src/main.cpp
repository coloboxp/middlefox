#include <Arduino.h>
#include "system_init.h"
#include "task_manager.h"
#include "global_instances.h"
#include "Version.h"

int sdCardLogOutput(const char *format, va_list args)
{
  Serial.println("Callback running");
  char buf[128];
  int ret = vsnprintf(buf, sizeof(buf), format, args);
  Serial.print(buf);
  return ret;
}

void setup()
{
  delay(100);
  //  esp_log_set_vprintf(sdCardLogOutput);

  if (!SystemInitializer::initializeDisplay())
  {
    ESP_LOGE("Main", "Display initialization failed");
    return;
  }

  if (!SystemInitializer::initializeBuzzer())
  {
    ESP_LOGE("Main", "Buzzer initialization failed");
    return;
  }

  SystemInitializer::showStartupIcons();

  if (!SystemInitializer::initializeBLE())
  {
    ESP_LOGE("Main", "BLE initialization failed");
    return;
  }

  // BuzzerManager::getInstance().setMute(true);
  SystemInitializer::playStartupSequence();

  if (!TaskManager::startTasks())
  {
    ESP_LOGE("Main", "Failed to start tasks");
    return;
  }
}

void loop()
{
  vTaskDelete(NULL);
}
