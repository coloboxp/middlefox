#pragma once

#include <Arduino.h>

class SystemInitializer
{
public:
    static bool initializeBuzzer();
    static bool initializeDisplay();
    static bool initializeBLE();
    static void playStartupSequence();
    static void showStartupIcons();

private:
    static const char *TAG;
};