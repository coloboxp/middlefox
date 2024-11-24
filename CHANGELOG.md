# Changelog

## [4.1.1] - 2024-11-24

### Added

- Single-button menu interface with long press activation
- Display update throttling for better performance
- Icon caching to prevent excessive reloading
- AceButton library integration for robust button handling

### Fixed

- Excessive screen refresh rate
- Icon rendering memory usage
- Button responsiveness issues
- Font size and layout optimization
- Display buffer management

### Changed

- Switched to u8g2_font_4x6_tf for better information density
- Improved menu navigation logic
- Enhanced status display layout
- Optimized display update timing

### Technical Details

- Display Manager:

  - Added update throttling (1 second or on-demand)
  - Implemented icon data caching
  - Improved buffer management
  - Enhanced layout organization

- Button Handler:
  - Added debounce configuration
  - Implemented multiple press types
  - Added event logging
  - Improved response timing

## [4.1.0] - 2024-11-24

### Added - 4.1.0

- Debug logging for bitmap conversion
- Validation checks for icon data

### Fixed - 4.1.0

- XBM icon rendering and orientation
- Build timestamp format (now DD.MMM.YY HH:MM:SS)
- Icon positioning in display
- Bitmap conversion for U8G2 display
- Display of startup icons
- Display of startup messages

## [1.1.0] - 2024-11-23

### Added - 1.1.0

- U8G2 display manager with comprehensive buffer management
- Buzzer manager with multiple sound patterns
- Visual and audio feedback for system states
- Boot sequence display messages
- Multiple sound patterns (boot, alerts, fox song)
- Mute functionality for buzzer
- Proper task management and core pinning
- Comprehensive error handling and logging

### Fixed

- Text rendering issues in display
- Buffer management sequence (clear->draw->send)
- Vertical text positioning using font metrics
- Camera initialization sequence
- WiFi and MJPEG server handling
- Service state transitions
- System restart handling

### Changed - 1.1.0

- Switched from U8X8 to U8G2 for better graphics
- Improved initialization sequence
- Updated pin definitions
- Standardized timeout values
- Improved task priorities
- Better error handling in tasks
- Enhanced code organization with singleton pattern

### Technical Details - 1.1.0

- Display Manager:

  - Proper buffer management with clearBuffer/sendBuffer
  - Text centering with font metrics
  - Comprehensive error checking
  - Drawing methods (lines, circles, boxes)
  - Fixed vertical text positioning

- Buzzer Manager:

  - Note-based music playback
  - Predefined patterns
  - Error handling
  - Initialization checks

- Configuration:

  - Display model in config.h
  - Operation modes enum
  - Updated pin definitions
  - USB CDC mode configuration

- Build System:
  - Added U8G2 library dependency
  - Updated debug levels
  - Multiple build environments (release, testing, debug)

### Testing - 1.1.0

- Verified display initialization and text rendering
- Tested buzzer patterns and sound quality
- Confirmed proper task execution
- Validated configuration changes
- Verified camera initialization sequence
- Tested service state transitions
- Confirmed system restart functionality

### Documentation - 1.1.0

- Added code documentation
- Improved error logging
- Added consistent logging patterns
- Updated build instructions
