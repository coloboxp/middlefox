#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class TaskManager {
public:
    static bool startTasks();
    static void stopTasks();
    
private:
    static void mainTask(void* parameter);
    static void bleTask(void* parameter);
    
    static TaskHandle_t bleTaskHandle;
    static TaskHandle_t mainTaskHandle;
}; 