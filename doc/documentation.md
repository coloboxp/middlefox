# System Documentation

## 1. Class Diagram

This diagram shows the relationships and structure between the main system components.

## 2. State Diagram

This diagram illustrates the different states and transitions in the system.

## 3. Sequence Diagram

This diagram shows the interaction between different components over time.

## 4. Flow Diagram

This diagram represents the overall system flow and decision points.

## Component Descriptions

### CustomBLEService

The central communication service that handles Bluetooth Low Energy connections and manages the system's state.

### CameraManager

A singleton class that manages camera initialization and configuration for both preview and capture modes.

### PreviewService

Handles the MJPEG streaming functionality, including WiFi AP setup and server management.

### DataCollector

Manages the image capture process, including saving files to SD card and format conversion.

## Operation Modes

1. **Preview Mode**: Streams camera feed over WiFi using MJPEG
2. **Capture Mode**: Captures and saves images to SD card
3. **Inference Mode**: Processes images through ML model (if enabled)

## Task Structure

- **BLE Task**: Handles BLE communication (Core 0)
- **Main Task**: Manages camera operations (Core 1)
