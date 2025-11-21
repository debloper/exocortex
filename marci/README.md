# 🎙️ Marci

> ॐ क्रतो स्मर कृतं स्मर क्रतो स्मर कृतं स्मर ॥

First stage runtime for Manas: **Marci**&mdash;*The Mute Mighty Maiden!*

Touch activated wearable audio recorder, with high local storage resiliency, and (optional) cloud back up.

---

This firmware is designed for the ***Seeed Studio XIAO ESP32S3 Sense*** to perform continuous audio recording to a microSD card. It is triggered by a capacitive touch sensor and saves audio in 1-minute WAV file segments.

## Key Features

- **Segmented Recording**: Automatically saves audio in 1-minute WAV files, minimizing data loss from power cuts or other interruptions.
- **Continuous Operation**: After each 1-minute segment is saved, a new recording starts immediately without missing any audio.
- **Touch Control**: A simple touch on the designated pin starts and stops the recording sequence.
- **Failsafe File Naming**: Creates a new directory for each power cycle (`run`) and uses the device uptime in milliseconds for filenames, preventing accidental overwrites.
- **Visual Feedback**: The onboard LED indicates the recording status (ON for recording, OFF for idle).
- **Modular Codebase**: The source code is organized into logical modules (`Recorder`, `SDManager`) for easy maintenance and future development.

## Hardware Requirements

- **Board**: **Seeed Studio XIAO ESP32S3 Sense**
- **Microphone**: The onboard PDM microphone is used.
- **Storage**: A microSD card connected to the appropriate pins.
- **Touch Sensor**: A wire or capacitive surface connected to the touch input pin.

### ❗ Important Configuration

For this firmware to function correctly, **OPI PSRAM must be enabled** in your Arduino IDE board settings.
- In Arduino IDE: `Tools` -> `PSRAM` -> `OPI PSRAM`.

## Pinout

The following pins are used by the firmware. They can be reconfigured in `Config.h`.

| Function      | Pin |
|---------------|-----|
| I2S BCLK      | 42  |
| I2S LRCLK/WS  | 41  |
| SD Card (CS)  | 21  |
| Touch Input   | 1   |

## How to Use

1.  **Setup Hardware**: Connect your microSD card reader and touch sensor to the pins listed above.
2.  **Configure IDE**: Open the project in the Arduino IDE and ensure you have selected the correct board (`Seeed Studio XIAO ESP32S3`) and enabled OPI PSRAM.
3.  **Flash Firmware**: Compile and upload the `marci.ino` sketch to your device.
4.  **Monitor (Optional)**: You can open the Serial Monitor at 115200 baud to view status messages. The firmware will wait up to 500ms for a connection before proceeding.
5.  **Start Recording**: Touch the sensor on Pin 1. The onboard LED will turn on, and the device will start saving audio to the SD card.
6.  **Stop Recording**: Touch the sensor again. The LED will turn off, and the final audio segment will be saved.

## SD Card File Structure

The firmware organizes recordings on the SD card to prevent data loss and confusion between sessions.

- A new directory is created each time the device is powered on (e.g., `/0`, `/1`, `/2`, ...).
- Inside each directory, WAV files are named after the number of milliseconds that had passed since the device booted (e.g., `60000.wav`, `120000.wav`).

Example structure:
```
/0/
  - 60000.wav
  - 120000.wav
  - 180000.wav
/1/
  - 60000.wav
  - 120000.wav
```
