#ifndef Recorder_h
#define Recorder_h

#include <Arduino.h>
#include "ESP_I2S.h"
#include "SDManager.h"
#include "Config.h"

class Recorder {
public:
    Recorder();
    bool begin();
    void loop();
    void toggleRecording();

private:
    void startRecording();
    void stopRecording();
    
    I2SClass I2S;
    SDManager sdManager;
    bool isRecording;
    volatile bool toggleRequested;
    unsigned long lastToggleTime;
    unsigned long recordingStartTime;
};

#endif
