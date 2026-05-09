# Carputer

A custom Qt5/QML in-car head unit application built for embedded Linux (Buildroot). Designed to run fullscreen on a dedicated car PC, providing media playback, real-time vehicle telemetry, climate control, and remote vehicle functions through a clean, themeable interface.

---

## Features

### Media Player
- Audio playback via GStreamer `playbin` pipeline
- ID3 tag reading — displays track title, artist, and album
- Embedded album artwork display with folder art fallback (`folder.jpg` / `cover.jpg`)
- Real-time audio spectrum visualizer (32-band FFT via GStreamer `spectrum` element)
- Repeat modes: None, Repeat All, Repeat One
- Shuffle with Fisher-Yates algorithm
- Playlist and Album browse views
- USB/SD media scanning

### Dashboard
- Large analog speedometer and RPM gauges
- Centre panel with 3 tappable views: Now Playing, Trip Computer, Performance
- Real-time sensor data: speed, RPM, fuel level, coolant temp, oil pressure, oil temp, battery, intake/ambient temp, brake fluid
- Warning system with caution/danger/critical levels and dismissable popup overlay
- Compact quick-controls bar: climate, fan speed, HVAC/AC/Auto, door lock, windows, remote start

### Theming
- 7 built-in themes: Dark, Light, Blue, Red, Green, Purple, Orange
- Custom accent colour picker
- Theme and accent colour persist across reboots via QSettings

### Other Pages
- Backup camera feed
- Dashcam / DVR recording
- Settings page

### Connectivity
- ESP32 wireless module support via WiFi (car control, sensor data)
- Bluetooth (bluez5)
- SSH access via Dropbear

---

## Hardware Target

| Component | Details |
|---|---|
| Machine | anything 
| CPU | Intel Core 
| GPU | Intel 
| OS | Custom Buildroot 2026.02 image |
| Init | systemd |
| Display | Fullscreen via Qt5 EGLFS / linuxfb |
| Audio | ALSA |

---

## Software Stack

| Layer | Technology |
|---|---|
| UI Framework | Qt 5 / QML |
| Media Backend | GStreamer 1.24 |
| GPU / OpenGL | Mesa3D — Iris / Crocus / I915 Gallium |
| Compositor | Weston (DRM) |
| Build System | Buildroot 2026.02 |
| Kernel | Linux 6.19.5 |
| Compiler | GCC 14.3.0 |

### Qt Modules Used
`Qt5Base` `Qt5Declarative` `Qt5Quick` `Qt5QuickControls2` `Qt5Multimedia` `Qt5SerialPort` `Qt5Wayland`

### GStreamer Plugins Required
`gstreamer-1.0` `gstreamer-app-1.0` `gstreamer-pbutils-1.0` `gst-plugins-good` (spectrum, rtpjitterbuffer) `gst-plugins-base` (jpegdec, videoconvert)

---

## Project Structure

```
carputer/
├── main.cpp                  # App entry point, manager registration
├── carputer.pro              # Qt project file
│
├── # Managers (C++ backend)
├── mediamanager.h/cpp        # GStreamer media player, tags, spectrum
├── artworkprovider.h/cpp     # Qt image provider for album art
├── thememanager.h            # Runtime theming
├── configmanager.h/cpp       # QSettings persistence
├── carcontrolmanager.h/cpp   # ESP32 vehicle control
├── sensormanager.h/cpp       # Vehicle sensor data
├── cameramanager.h/cpp       # Backup/dashcam feeds
├── dvrmanager.h/cpp          # DVR recording
├── audiomanager.h/cpp        # ALSA audio routing
├── wifimanager.h/cpp         # Module wireless connectivity
├── systemmanager.h/cpp       # OS-level controls
├── diagnosticmanager.h/cpp   # Log capture
├── updatemanager.h/cpp       # OTA updates
│
├── # QML Frontend
├── main.qml                  # Root window, navigation
├── DashboardPage.qml         # Main dash — gauges, media, climate
├── MediaPage.qml             # Full media player
├── BackupCamPage.qml         # Rear camera
├── DashcamPage.qml           # DVR / dashcam
├── SettingsPage.qml          # Themes, audio, system settings
└── AnalogGauge.qml           # Reusable analog gauge component
```

---

## Building

This app is built as part of a custom Buildroot image. It is not intended to be built standalone on a desktop Linux system.

```bash
# From the Buildroot tree root
make carputer-dirclean && make carputer -j$(nproc)
```

Deploy to target:
```bash
scp output/target/usr/bin/carputer root@<target-ip>:/usr/bin/carputer
```

Restart the service:
```bash
systemctl restart carputer
```

---

## Configuration

Settings are stored via QSettings at:
```
~/.config
```

Key settings include video port, audio sink, theme, accent colour, DVR directory, climate defaults, and serial port assignments for the ESP32 modules.

---

## Origins

Carputer was built for replacing broken modules in my old toyota just project is just fun and i like the fact i can just build what i want .... 
---

## License


