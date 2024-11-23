#include <Arduino.h>
#include "system_init.h"
#include "task_manager.h"
#include "global_instances.h"

void setup() {
    delay(100);
    
    if (!SystemInitializer::initializeBuzzer() ||
        !SystemInitializer::initializeDisplay() ||
        !SystemInitializer::initializeBLE()) {
        ESP_LOGE("Main", "System initialization failed");
        return;
    }
    
    SystemInitializer::playStartupSequence();
    SystemInitializer::showStartupIcons();
    
    if (!TaskManager::startTasks()) {
        ESP_LOGE("Main", "Failed to start tasks");
        return;
    }
}

void loop() {
    vTaskDelete(NULL);
}
