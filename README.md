# MiddleFox - Lane Assistance System

A smart camera-based lane assistance system designed to help foxes stay centered while driving. The system uses computer vision and machine learning to detect lane deviations and provide real-time feedback through visual and audio alerts.

[Changelog](CHANGELOG.md)

## 🚀 Features

### Core Functionality

- **Real-time Preview Mode**: WiFi-based MJPEG streaming for camera positioning
- **Data Collection Mode**: Captures and stores images for AI training
- **Inference Mode**: Real-time lane detection with audio-visual alerts
- **BLE Control**: Remote operation and status monitoring

### User Interface

- **OLED Display**: Real-time status and feedback
  - U8G2-based graphics
  - Proper XBM icon rendering
  - Buffer management for smooth updates
  - Centered text rendering
  - Multiple font support
  - Icon support with correct orientation
- **Audio Feedback**
  - Multiple alert patterns
  - "What does the fox say?" boot sequence
  - Configurable buzzer patterns
  - Mute functionality

## 🛠️ Hardware Requirements

### Core Components

- ESP32-S3 XIAO (with built-in camera)
- SSD1306 OLED Display (128x64)
- Active Buzzer
- SD Card (for data collection)
- LED (status indicator)

### Pin Configuration

```cpp
#define LED_PIN 21      // Status LED
#define BUZZER_PIN 4    // Audio feedback
#define BUTTON_PIN 1    // Control button
// Display: Default I2C pins
// SD Card: Standard SPI pins
```

## 🔧 Operation Modes

### 1. Preview Mode

- Creates WiFi AP "MiddleFox"
- MJPEG stream on port 81
- Real-time camera positioning
- BLE status updates

### 2. Data Collection Mode

- 5-second capture intervals
- Dual format storage (RGB565 + JPEG)
- Auto-incrementing filenames
- Visual capture feedback

### 3. Inference Mode

- Real-time lane detection
- Audio-visual alerts
- BLE status updates

## 📦 Building and Flashing

### Prerequisites

- PlatformIO
- ESP32 Arduino Framework

### Required Libraries

```ini
lib_deps =
  eloquentarduino/EloquentEsp32cam
  NimBLE-Arduino
  ArduinoJson@^7.2.0
  U8g2@^2.36.2
```

## 🔌 BLE Interface

### Service Characteristics

- Control (Write)
- Status (Notify)
- Preview Info (Read/Notify)
- Service Metrics (Read/Notify)

### Commands

1. Start/Stop Preview
2. Start/Stop Data Collection
3. Start/Stop Inference

## 🏗️ System Architecture

### Core Component

- **CustomBLEService**: Communication management
- **CameraManager**: Camera operations
- **PreviewService**: MJPEG streaming
- **DataCollector**: Image capture/storage
- **DisplayManager**: UI rendering
- **BuzzerManager**: Audio feedback

### Task Management

- Core 0: BLE communications
- Core 1: Camera and processing

## 🔍 Development

### Project Structure

```
middlefox/
├── src/
│   ├── main.cpp
│   ├── ble_service.cpp
│   ├── camera_manager.cpp
│   ├── preview_service.cpp
│   ├── data_collector.cpp
│   ├── display_manager.cpp
│   └── buzzer_manager.cpp
├── include/
│   ├── config.h
│   └── camera_pins.h
└── doc/
    └── documentation.md
```

### Documentation

- Class diagrams
- State diagrams
- Sequence diagrams
- Flow diagrams

## 📝 Contributing

1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## 📄 License

MIT License - See LICENSE file for details

## 🙏 Acknowledgments

- ESP32 Community
- U8g2 Library Team
- NimBLE-Arduino Contributors
- EloquentArduino Team
