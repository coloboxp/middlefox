#include "task_manager.h"
#include "global_instances.h"

TaskHandle_t TaskManager::bleTaskHandle = nullptr;
TaskHandle_t TaskManager::mainTaskHandle = nullptr;

bool TaskManager::startTasks() {
    BaseType_t result;
    
    // Start BLE task
    result = xTaskCreatePinnedToCore(
        bleTask,
        "BLE_Task",
        4096,
        nullptr,
        1,
        &bleTaskHandle,
        0
    );
    
    if (result != pdPASS) {
        ESP_LOGE("TaskManager", "Failed to create BLE task");
        return false;
    }

    // Start main task
    result = xTaskCreatePinnedToCore(
        mainTask,
        "Main_Task",
        8192,
        nullptr,
        2,
        &mainTaskHandle,
        1
    );
    
    if (result != pdPASS) {
        ESP_LOGE("TaskManager", "Failed to create main task");
        vTaskDelete(bleTaskHandle);
        return false;
    }

    return true;
}

void TaskManager::stopTasks() {
    if (bleTaskHandle != nullptr) {
        vTaskDelete(bleTaskHandle);
        bleTaskHandle = nullptr;
    }
    
    if (mainTaskHandle != nullptr) {
        vTaskDelete(mainTaskHandle);
        mainTaskHandle = nullptr;
    }
}

void TaskManager::bleTask(void* parameter) {
    while (true) {
        bleService.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void TaskManager::mainTask(void* parameter) {
    while (true) {
        #ifdef PRODUCTION_MODE
        inference.loop();
        #else
        collector.loop();
        #endif
        
        display.refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
} 