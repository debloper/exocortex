// ============================================================
//  MONEO — Exocortex Iteration II
//  moneo.ino — Main Arduino Sketch
//
//  Board:  Seeed Studio XIAO ESP32S3 Sense
//  PSRAM:  Tools → PSRAM → OPI PSRAM  ← MUST ENABLE THIS
//
//  Workflow:
//    Touch pin → start recording (LED ON)
//    Touch pin → stop recording  (LED OFF)
//    Chunks auto-upload to laptop gateway
//    Gateway generates transcripts + notes via AI
//    Notes auto-sync back to SD card
// ============================================================

#include "Config.h"
#include "Recorder.h"

Recorder recorder;

// ISR — called by capacitive touch interrupt.
// Minimal: just sets a flag, recorder handles it in loop().
void IRAM_ATTR onTouch() {
    recorder.requestToggle();
}

void setup() {
    Serial.begin(115200);

    // Brief wait for serial monitor (non-blocking)
    unsigned long t = millis();
    while (!Serial && millis() - t < 500) delay(10);

    Serial.println("╔══════════════════════════════╗");
    Serial.println("║  MONEO — Exocortex Iter. II  ║");
    Serial.println("╚══════════════════════════════╝");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Attach touch interrupt
    touchAttachInterrupt(TOUCH_PIN, onTouch, TOUCH_THRESHOLD);

    // Initialize recorder (I2S + SD + session + uploader)
    if (!recorder.begin()) {
        Serial.println("[FATAL] Initialization failed. Halting.");
        // Rapid blink = error
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH); delay(100);
            digitalWrite(LED_BUILTIN, LOW);  delay(100);
        }
    }

    Serial.println("[Moneo] Ready. Touch pin " + String(TOUCH_PIN) + " to start.");
}

void loop() {
    // Handles touch debounce and session start/stop
    recorder.loop();
    delay(10);
}
