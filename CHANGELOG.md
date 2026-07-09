# Carputer Changelog

## Version 1.6.1 (2026-05-10)

### Maintenance Release

- Bumped application version metadata to `1.6.1` for release packaging and update checks.
- General maintenance and release preparation updates.

---

## Version 1.2 (2026-04-14)

### Apple CarPlay Integration

Added Apple CarPlay support to the carputer application.

### Features Implemented

- **CarPlayPage.qml** - Full CarPlay interface with:
  - Video rendering surface with touch input support
  - Multi-touch gestures (pinch, swipe, etc.)
  - Connection status display
  - Media controls (play/pause, next/previous track)
  - Siri activation button
  - Home button
  - Volume control slider
  - Night mode toggle
  - Auto-connect on page load
  - Keyboard shortcuts (Space=play/pause, H=home, S=Siri, N=night mode)

- **carplaymanager.{h,cpp}** - C++ backend for CarPlay:
  - USB device detection for iPhone (scans /sys/bus/usb/devices)
  - Connection state machine (Disconnected → Detecting → Connecting → Authenticating → Streaming)
  - Touch event encoding for CarPlay protocol
  - Multi-touch support with pointer IDs
  - Physical button emulation (Home, Siri, Select, Back, Menu)
  - Media control buttons (Play/Pause, Next, Previous)
  - Knob/dial input support
  - Audio routing controls (volume, microphone enable)
  - Video resolution configuration
  - Night mode support
  - H.264 video decoder placeholder (requires FFmpeg integration)
  - Audio router placeholder (requires ALSA/PulseAudio integration)

### Technical Changes

- Added CarPlayManager to QML context in main.cpp
- Extended navigation to 9 tabs (added CARPLAY)
- Updated keyboard shortcuts (keys 1-9)
- Added carplaymanager.{h,cpp} to carputer.pro
- Added CarPlayPage.qml to qml.qrc

### Hardware Requirements (for production use)

- USB port with OTG support
- Display with touchscreen (800x480 minimum)
- Audio output (I2S DAC, USB audio, or HDMI)
- Microphone for Siri
- Optional: WiFi with WiFi Direct for wireless CarPlay

### Production Integration Notes

The current implementation provides the UI framework and protocol scaffolding.
For full CarPlay functionality, integrate with:
- libusb-1.0 for USB device management
- FFmpeg/libav for H.264 video decoding
- ALSA/PulseAudio for audio routing
- OpenAuto or similar CarPlay protocol library

---

## Version 1.0 (2026-04-10)

### Conversion from Godseye (FPV Drone App)

This is the initial carputer release, converted from the Godseye FPV drone monitoring application.

### Features Implemented

- **Dashboard** - Main hub with time/date display, quick actions for Media/Cam/DVR, shortcuts to Climate/GPS/Gauges
- **Media Player** - Audio/video playback from USB/SD/Phone sources, playlist management, transport controls
- **Backup Camera** - Rear camera display with overlay for parking assistance
- **Dashcam** - Recording interface with snapshot and file browser
- **Gauge Cluster** - Speedometer, RPM, fuel level, coolant temperature display
- **Climate Control** - Temperature adjustment, fan speed (5 levels), A/C toggle, recirculation, airflow modes (Face/Feet/Defrost)
- **GPS Navigation** - Destination input, map view placeholder
- **Settings** - Display brightness, theme, audio volume, Bluetooth, camera toggles, map provider, system info

### Technical Changes

- Renamed app from "Godseye" to "Carputer"
- Removed GStreamer dependencies (drone video streaming)
- Added Qt Multimedia for media playback
- New dark car-themed UI with blue accent (#00a8e8)
- 8-tab bottom navigation (keys 1-8)
- Full-screen mode (M key toggles menu)

### Files Created

- `mediamanager.{h,cpp}` - Audio/video playback manager using Qt Multimedia
- `DashboardPage.qml` - Main dashboard UI
- `MediaPage.qml` - Media player UI
- `BackupCamPage.qml` - Rear camera display
- `DashcamPage.qml` - Recording interface
- `GaugesPage.qml` - Gauge cluster UI
- `ClimatePage.qml` - Climate control UI
- `GPSPage.qml` - Navigation UI
- `SettingsPage.qml` - Settings UI (updated)
- `main.qml` - New main window with car navigation
- `main.cpp` - Updated for carputer

### Files Removed (Drone-Specific)

- `videoplayer.{h,cpp}` - Drone video playback
- `mavlinkmanager.{h,cpp}` - Drone telemetry
- `wfbmanager.{h,cpp}` - Wireless video link
- `dvrmanager.{h,cpp}` - DVR recording
- `networkmanager.{h,cpp}` - Network tools
- `VideoPage.qml`, `WFBPage.qml`, `DVRPage.qml`, `SystemPage.qml`, `NetworkPage.qml`, `InfoPage.qml`, `ToolsPage.qml`, `InspectPage.qml`, `TerminalPage.qml`, `MenuButton.qml`, `FileBrowserPage.qml`, `TelemetryOverlay.qml`

### Known Limitations

- OBD2 diagnostics not included (handled by other apps)
- GPS map view is placeholder only
- Backup/Dashcam cameras need actual camera input integration
- Media playback requires Qt Multimedia GStreamer backend