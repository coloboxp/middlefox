# System Documentation

## System Architecture

### 1. Component Overview

- **Core Components**: Display, Buzzer, BLE, RTC
- **Service Components**: Camera, Preview, Data Collection
- **User Interface**: Menu System
- **Task Management**: BLE and Main Tasks

### 2. Component Descriptions

#### System Core

- **SystemInitializer**: Boot sequence and core component initialization
- **MenuHandler**: User interface and display management
- **BuzzerManager**: Audio feedback system
- **RTCManager**: Real-time clock operations
- **CustomBLEService**: BLE connectivity and state management

#### Camera System

- **CameraManager**: Camera configuration and control
- **PreviewService**: MJPEG streaming service
- **DataCollector**: Image capture and storage

### 3. System Initialization Sequence

1. Display Initialization
2. Buzzer Configuration
3. Startup Icons Display
4. BLE Service Setup
5. RTC Configuration
6. Menu System Initialization
7. Task Creation

### 4. Operation Modes

1. **Preview Mode**: MJPEG streaming
2. **Capture Mode**: Image acquisition
3. **Inference Mode**: ML processing

### 5. Task Structure

- **BLE Task (Core 0)**: Communication handling
- **Main Task (Core 1)**: Camera and system operations

### 6. Menu Navigation

The system provides a hierarchical menu structure:
- **Main Menu**: Primary navigation hub
  - **Settings**: System configuration options
  - **Status**: Current system status and metrics
  - **Operations**: Mode selection and control
    - Preview Mode
    - Capture Mode
    - Inference Mode

Navigation is handled through physical inputs and updates are reflected both on the display and through BLE state changes.

### 7. Error Handling & Recovery

The system implements several error handling mechanisms:
- **Initialization Failures**: Each component initialization has fallback procedures
- **Task Monitoring**: Watchdog timers monitor core tasks
- **Communication Recovery**: Automatic BLE reconnection and WiFi fallback
- **Storage Management**: SD card error detection and recovery
- **Camera Subsystem**: Auto-reset on camera initialization failure

Error states are:
1. Reported through BLE status updates
2. Displayed on the OLED screen
3. Logged to persistent storage when available
4. Accompanied by buzzer feedback patterns

## System Diagrams

### 1. Class Diagram

This diagram shows the relationships and structure between the main system components.

[View Class Diagram](classses.md)

### 2. State Diagram

This diagram illustrates the different states and transitions in the system.

[View State Diagram](state.md)

### 3. Sequence Diagram

This diagram shows the interaction between different components over time.

[View Sequence Diagram](sequence.md)

### 4. Flow Diagram

This diagram represents the overall system flow and decision points.

[View Flow Diagram](flow.md)
