# 🎙️ Marci

> ॐ क्रतो स्मर कृतं स्मर क्रतो स्मर कृतं स्मर ॥

First-stage runtime for Manas: **Marci**&mdash;*The Mute Mighty Maiden!*

A touch-activated wearable audio recorder for the Seeed Studio XIAO ESP32S3 Sense. Records continuous audio in 1-minute WAV segments to a microSD card, with optional cloud backup to a remote file server.

## ✨ Features

- **Segmented Recording**: Save audio in 1-minute WAV files to minimize data loss from interruptions.
- **Touch Control**: Start/stop recording with a capacitive touch on the designated pin.
- **Run-Based Organization**: Keep each recording session separated by directories (e.g., `/0/`, `/1/`).
- **Unique Filenames**: Use device uptime (milliseconds) for filenames to avoid overwrites.
- **Visual Feedback**: Indicate recording status with onboard LED (ON = recording, OFF = idle).
- **WiFi Backup**: [optionally] connect to WiFi after recording stops to upload files to a file server.

## 🛠️ Hardware

- **Board**: Seeed Studio XIAO ESP32S3 Sense
- **Microphone**: Onboard PDM microphone
- **Storage**: MicroSD card (FAT32 formatted)
- **Touch Sensor**: Capacitive surface connected to touch pin
- **WiFi**: Access to a network (for uploads)

## 🚀 Usage

1. **Setup Arduino IDE**:
   - Select board: `Seeed Studio XIAO ESP32S3`
   - Enable PSRAM: `Tools` → `PSRAM` → `OPI PSRAM`
   - Install required libraries: `WiFi`, `SD`, `ESP_I2S`
   - Open Serial Monitor at 115200 baud for logs.
   - Open `marci.ino` in Arduino IDE.

2. **Adjust Config.h**:
   - Set `WIFI_SSID` and `WIFI_PASSWORD` for your network.
   - Set `SERVER_HOST` and `SERVER_PORT` (tip: use `copyparty`).
   - Adjust pins, recording duration, or other settings as needed.

3. **Flash Firmware**:
   - Connect the ESP32 module with USB
   - Ensure Arduino IDE can recognize it
   - Verify, compile, and upload the sketch

4. **Try it out**:
   - Touch pin 1 (perhaps with a pin, in lieu of capacitive surface) to start (LED on).
   - Talk for as long as you want, or until the battery or the TF card runs out...
   - Touch pin 1 again to stop the recording (LED off)
   - If auto-upload is configured, the device will attempt to connect and upload.

### 🌐 Configuring Backup

To simply simulate a cloud backup, we recommend setting up a `copyparty` file server:

1. Install: `pip install copyparty`
2. Run: `copyparty /path/to/backup/dir`
3. We skipped authentication as it's only for personal/demonstrative use.

Files are uploaded to `/session/filename.wav` (e.g., `/0/60000.wav`).

## 📁 SD Card Structure

- Directories: One per recording session (e.g., `/0/`, `/1/`).
- Files: Named by uptime in ms (e.g., `60000.wav`, `120000.wav`).

Example:
```
/0/
  ├── 60000.wav
  └── 120000.wav
/1/
  └── 60000.wav
```

---

*Built with ❤️*
