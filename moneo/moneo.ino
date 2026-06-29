// ============================================================
//  MONEO
//  moneo.ino — Main Sketch
//
//  Board:  Seeed Studio XIAO ESP32S3 Sense
//  PSRAM:  Tools → PSRAM → OPI PSRAM  ← MUST ENABLE
//
//  Flow:
//    Touch D1 → start recording (LED ON)
//    Speak for as long as needed (10s segments flushed to SD)
//    Touch D1 → stop recording (LED OFF)
//    Device connects to WiFi (WiFiMulti picks the strongest known network)
//    WAV file is saved on the SD card
//
//  NOTE: AI note-generation (AIClient) is not part of this build yet. Those
//  lines are commented out and marked TODO; they will be wired in once
//  AIClient is added to the repo.
// ============================================================

#include "Config.h"
#include "Recorder.h"
#include "WiFiManager.h"
// #include "AIClient.h"   // TODO: enable when AIClient is added

Recorder    recorder;
WiFiManager wifiMgr;
// AIClient aiClient;       // TODO: enable when AIClient is added

bool _wasRecording = false;

void IRAM_ATTR onTouch() {
    recorder.requestToggle();
}

void setup() {
    Serial.begin(115200);
    unsigned long serialTimeout = millis() + SERIAL_WAIT_MS;
    while (!Serial && millis() < serialTimeout) { delay(10); }

    Serial.println("╔══════════════════════════════════╗");
    Serial.println("║         MONEO — Starting         ║");
    Serial.println("╚══════════════════════════════════╝");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // TODO: detect AI provider once AIClient is integrated
    // if (!aiClient.begin()) {
    //     Serial.println("[FATAL] AI client init failed. Check API key in Config.h");
    //     _errorBlink();
    // }

    // Init recorder
    if (!recorder.begin()) {
        Serial.println("[FATAL] Recorder init failed.");
        _errorBlink();
    }

    touchAttachInterrupt(TOUCH_PIN, onTouch, TOUCH_THRESHOLD);
    // recorder.begin() already logged the "ready / touch to start" message.
}

void loop() {
    recorder.loop();

    // Detect transition: was recording → stopped
    bool nowRecording = recorder.isRecording();
    if (_wasRecording && !nowRecording) {
        if (recorder.hasError()) {
            // Recording failed mid-way (SD write error). The partial file was
            // finalized; signal the failure instead of treating it as success.
            Serial.println("[Moneo] Recording FAILED (SD write error). File may be incomplete.");
            _errorBlink();   // halts here, blinking the LED
        }
        // Recording just stopped — process it
        String wavPath = recorder.lastRecordingPath();
        if (wavPath.length() > 0) {
            _processRecording(wavPath);
        }
    }
    _wasRecording = nowRecording;

    delay(10);
}

void _processRecording(const String& wavPath) {
    Serial.println("[Moneo] Recording complete. Connecting to WiFi...");

    // Blink LED while connecting
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);

    if (!wifiMgr.connect()) {
        Serial.println("[Moneo] WiFi failed.");
        Serial.println("[Moneo] WAV file saved locally: " + wavPath);
        // Graceful failure — the WAV is safe on the SD card
        return;
    }

    Serial.println("[Moneo] WiFi connected.");
    Serial.printf("[Moneo] File saved: %s\n", wavPath.c_str());

    // TODO: AIClient integration — send the WAV to the AI and save notes.
    // Enabled once AIClient is added to the repo.
    //
    // String notes = aiClient.generateNotes(wavPath);
    // if (notes.isEmpty()) {
    //     Serial.println("[Moneo] AI returned no notes.");
    //     return;
    // }
    // String notesPath = wavPath;
    // notesPath.replace(".wav", ".md");          // e.g. rec_….wav → rec_….md
    // File f = SD.open(notesPath.c_str(), FILE_WRITE);
    // if (f) {
    //     f.print(notes);
    //     f.close();
    //     Serial.println("[Moneo] ✓ Notes saved: " + notesPath);
    // } else {
    //     Serial.println("[Moneo] Failed to save notes to SD.");
    // }

    Serial.println("[Moneo] Done! Touch pin to record again.");
}

void _errorBlink() {
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH); delay(200);
        digitalWrite(LED_BUILTIN, LOW);  delay(200);
    }
}
