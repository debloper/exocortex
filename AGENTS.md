# Exocortex — Agent Context

This file provides essential context for AI coding agents working on the **Exocortex** project. Read it fully before making any changes.

---

## What Is This Project?

**Exocortex** (Codename: *Project Manas*) is an iterative, modular personal cognitive accelerator — a wearable, always-on intelligence layer for capturing, structuring, and surfacing thought. It evolves through five firmware stages, each building on the lessons of its predecessor while shipping as a standalone, independent artifact.

Each stage is a distinct embedded firmware targeting a hardware module that's appropriate (necessary and sufficient) for its purpose. Stages are deployed to the apt physical form-factor to carry progressively deeper intelligence.

---

## Repository Layout

```
/
├── AGENTS.md                   ← This file; coding agent context
├── README.md                   ← Project overview + stage roadmap
├── marci/                      ← Stage 1: completed, frozen
│   ├── marci.ino
│   ├── Config.h
│   ├── Recorder.{h,cpp}
│   ├── SDManager.{h,cpp}
│   └── Uploader.{h,cpp}
└── moneo/                      ← Stage 2: active development
```

**Rule**: each stage folder is self-contained and independently compilable. Do not create shared libraries or cross-stage imports.

---

## Stage Roadmap

| # | Name  | Status  | Core Capability                                                  |
|---|-------|---------|------------------------------------------------------------------|
| 1 | Marci | ✅ Done | Audio recording companion; saves file locally with cloud backup  |
| 2 | Moneo | 🚧 WIP  | Evolves Marci by streaming audio to LLMs for transcripts & notes |
| 3 | Mimir | 📋 TODO | Upgrades Moneo with realtime recall, response & personality      |
| 4 | Metis | 📋 TODO | Adds biometric timeseries and retrospectives, like PPG, EEG etc. |
| 5 | Mitra | 📋 TODO | Extends cognition as an adaptive, agentic, anti-fragile, gateway |

---

## Hardware Platform

| Component    | Details                                              |
|-------------|------------------------------------------------------|
| Board        | Seeed Studio XIAO ESP32S3 Sense                     |
| MCU          | ESP32-S3 dual-core @ 240 MHz                        |
| PSRAM        | 8 MB OPI PSRAM — **must enable** `Tools → PSRAM → OPI PSRAM` |
| Microphone   | Onboard PDM mic, accessed via I2S PDM RX mode       |
| Storage      | MicroSD slot (SPI, CS on GPIO 21)                   |
| Connectivity | 802.11 b/g/n WiFi                                   |
| Touch        | Capacitive touch on configurable GPIO               |

### Critical Arduino IDE Settings (all stages)
- Board: `Seeed Studio XIAO ESP32S3`
- PSRAM: `OPI PSRAM` — **required for Moneo and later stages**
- Serial baud: 115200

---

## Coding Conventions

- **Language**: C++17 (Arduino framework on ESP-IDF)
- **Module pattern**: one `.h` / `.cpp` class pair per concern; main `.ino` wires them together and owns the global instances
- **Config**: `Config.h` holds compile-time defaults and NVS key name constants. From Moneo onward, runtime values come from `Preferences` (NVS); `Config.h` defaults are fallbacks only — never hardcode real credentials there
- **Private members**: use `_camelCase` prefix for private member variables
- **ISR functions**: must be `IRAM_ATTR`; keep body minimal (set a `volatile` flag, return immediately)
- **PSRAM allocation**: use `ps_malloc()` or `heap_caps_malloc(MALLOC_CAP_SPIRAM)` for large buffers; always check the return pointer
- **Debug output**: `Serial.println()` / `Serial.printf()` — acceptable everywhere; never block on Serial in a hot path
- **Error handling on init failure**: blink LED in a distinctive pattern and `while(true)` halt — do not silently continue with a broken subsystem
- **No dynamic polymorphism**: avoid `virtual`, RTTI, and exceptions — the Arduino/ESP-IDF runtime does not support them well

---

## Key Libraries (Moneo)

| Library | Purpose | Install |
|---------|---------|---------|
| `ESP_I2S` | I2S PDM microphone driver | Bundled with ESP32 Arduino core |
| `SD` | MicroSD card (SPI) | Bundled |
| `WiFi` | Network connectivity | Bundled |
| `Preferences` | NVS key-value storage for runtime config | Bundled |
| `time.h` + `configTime()` | SNTP/NTP time sync | Bundled (POSIX) |
| `HTTPClient` | HTTP/HTTPS requests for LLM API and uploads | Bundled |
| `WiFiClientSecure` | TLS for HTTPS endpoints | Bundled |
| `ArduinoJson` | JSON parsing for `config.json` and LLM API responses | Library Manager |

---

## Moneo — Implementation Overview

Read `moneo/README.md` for the full specification. Key architectural points for the agent:

### PSRAM Ring Buffer
- Allocate with `ps_malloc(RING_BUFFER_SIZE)` in `Recorder::begin()`; halt if null
- The buffer is a circular write-ahead cache: I2S data goes in at `_ringHead`; SD flush reads from `_ringTail`
- Flush a `FLUSH_CHUNK_SIZE` chunk to SD whenever `_ringBytesAvailable >= FLUSH_CHUNK_SIZE`
- Handle wraparound: if a chunk spans the end of the buffer, split into two `SDManager::write()` calls
- On `stopRecording()`, flush all remaining data before closing the file

### WAV File Lifecycle
1. `SDManager::createNewFile(fileName)` — open file, write 44-byte placeholder header (`dataLength = 0`)
2. `SDManager::write(buf, size)` — append raw PCM; accumulate `_dataLength`
3. `SDManager::closeFile()` — `seek(0)`, rewrite header with correct `_dataLength`, then `close()`

### NTP + Filename Format
- `configTime(gmtOffset, dstOffset, NTP_SERVER)` to set system clock
- `getLocalTime(&timeinfo)` to retrieve; `strftime(buf, size, "%Y%m%d_%H%M%S", &timeinfo)` for Android-style timestamp
- Fallback: `"BOOT_" + String(millis())` if no NTP sync

### LLM Transcription
- OpenAI-compatible Whisper endpoint: `POST /v1/audio/transcriptions`
- Format: `multipart/form-data` with `file` (WAV binary, streamed from SD) and `model` fields
- Use `WiFiClientSecure` for HTTPS; set `client.setInsecure()` for dev/self-hosted, or load a CA cert for production
- Parse JSON response: `{ "text": "..." }` — use `ArduinoJson`
- Save transcript as Markdown at the same base path as the WAV (`.md` extension)

### NVS / Preferences
- Four namespaces: `"wifi"`, `"llm"`, `"upload"`, `"device"` (max 15 chars each)
- Seeded at boot from `/config.json` on the SD card if present; file is renamed to `config.bak` after loading
- Key names are defined as `#define NVS_KEY_*` in `Config.h`
- Always provide compile-time defaults in `Config.h` for every NVS key

### Module Coordination (main sketch owns the flow)

`transcriber` and `uploader` calls are guarded by the `auto_transcribe` and `auto_upload` NVS flags respectively; read them once in `setup()` and branch accordingly.

```
loop():
  recorder.loop()          ← captures audio, manages ring buffer flush
  if recorder.isNewRecordingReady():
    if auto_transcribe:
      transcriber.transcribe(recorder.getLastFileName())   ← WAV → .md
    if auto_upload:
      uploader.uploadFile(recorder.getLastFileName())
      uploader.uploadFile(recorder.getLastMarkdownName())
    recorder.clearNewRecordingFlag()
```

---

## What NOT To Change

- `marci/` — Stage 1 is complete and frozen. Do not modify its source files.
- Do not create shared code between stages. Copy-and-improve is the intended pattern.
- Do not commit real credentials, API keys, or passwords anywhere in the repository.
- Do not change the per-stage folder structure or main sketch naming convention.
