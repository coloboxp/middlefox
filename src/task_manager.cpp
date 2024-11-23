#include "task_manager.h"
#include "global_instances.h"

TaskHandle_t TaskManager::bleTaskHandle = nullptr;
TaskHandle_t TaskManager::mainTaskHandle = nullptr;

bool TaskManager::startTasks() {
    // Wait for BLE to be fully initialized
    vTaskDelay(pdMS_TO_TICKS(200));

    // Start BLE task
    BaseType_t result = xTaskCreatePinnedToCore(
        bleTask,
        "BLE_Task",
        4096,
        nullptr,
        1,
        &bleTaskHandle,
        0  // Core 0
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
        1  // Core 1
    );
    
    if (result != pdPASS) {
        ESP_LOGE("TaskManager", "Failed to create main task");
        vTaskDelete(bleTaskHandle);
        return false;
    }

    return true;
}

void TaskManager::bleTask(void* parameter) {
    while (true) {
        bleService.loop();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void TaskManager::mainTask(void* parameter) {
    while (true) {
        if (bleService.isCaptureEnabled()) {
            collector.loop();
        } else if (bleService.isPreviewEnabled()) {
            previewService.loop();
        }
        #ifdef PRODUCTION_MODE
        else if (bleService.isInferenceEnabled()) {
            inference.loop();
        }
        #endif
        
        display.refresh();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
} 