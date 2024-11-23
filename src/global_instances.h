#pragma once

#include "config.h"
#include "ble_service.h"
#include "preview_service.h"
#include "data_collector.h"
#include "display_manager.h"
#include "buzzer_manager.h"

// Global instances declaration
extern CustomBLEService bleService;
extern PreviewService previewService;
#ifdef PRODUCTION_MODE
extern ModelInference inference;
#else
extern DataCollector collector;
#endif

extern DISPLAY_MODEL u8g2;
extern DisplayManager &display;
extern BuzzerManager &buzzer;