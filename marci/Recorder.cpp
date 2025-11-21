#include "Recorder.h"

Recorder::Recorder() : isRecording(false), toggleRequested(false), lastToggleTime(0), recordingStartTime(0) {}

bool Recorder::begin() {
    Serial.println("Initializing I2S bus...");
    I2S.setPinsPdmRx(I2S_BCLK_PIN, I2S_LRCLK_PIN);
    if (!I2S.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        Serial.println("Can't initialize I2S bus!");
        return false;
    }
    Serial.println("I2S bus initialized.");

    return sdManager.begin();
}

void Recorder::loop() {
    if (toggleRequested) {
        unsigned long currentTime = millis();
        if (currentTime - lastToggleTime >= DEBOUNCE_DELAY) {
            lastToggleTime = currentTime;
            if (!isRecording) {
                startRecording();
            } else {
                stopRecording();
            }
        }
        toggleRequested = false;
    }

    if (isRecording) {
        if (millis() - recordingStartTime >= RECORDING_DURATION_MS) {
            sdManager.closeFile();
            sdManager.createNewFile();
            recordingStartTime = millis();
        }

        int bytesAvailable = I2S.available();
        if (bytesAvailable > 0) {
            uint8_t buffer[I2S_BUFFER_SIZE];
            int bytesToRead = (bytesAvailable < I2S_BUFFER_SIZE) ? bytesAvailable : I2S_BUFFER_SIZE;
            int bytesRead = I2S.readBytes((char*)buffer, bytesToRead);
            if (bytesRead > 0) {
                sdManager.write(buffer, bytesRead);
            }
        }
    }
}

void Recorder::toggleRecording() {
    toggleRequested = true;
}

void Recorder::startRecording() {
    isRecording = true;
    digitalWrite(LED_BUILTIN, isRecording);
    sdManager.createNewFile();
    recordingStartTime = millis();
    Serial.println("Recording started.");
}

void Recorder::stopRecording() {
    isRecording = false;
    digitalWrite(LED_BUILTIN, isRecording);
    sdManager.closeFile();
    Serial.println("Recording stopped.");
}
