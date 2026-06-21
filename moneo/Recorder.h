#ifndef Recorder_h
#define Recorder_h

#include <Arduino.h>
#include <ESP_I2S.h>
#include <SD.h>
#include <FS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Config.h"

// ============================================================
// Recorder — Records audio into a SINGLE WAV file.
//
// Flow:
//   Touch start → create dated WAV file
//   Every 10s: flush PSRAM buffer segment to WAV file
//   Touch stop → finalize WAV header → send to AI
//
// The WAV file grows continuously — one file per session.
// No chunk files. No gateway. File sent directly to AI when done.
// ============================================================

class Recorder {
public:
    Recorder();
    bool begin();
    void loop();
    void requestToggle();

    bool isRecording() const { return _recording; }
    String lastRecordingPath() const { return _wavPath; }

private:
    void _startRecording();
    void _stopRecording();
    void _writeWavHeader(File& f, uint32_t dataLen);
    String _generateFilename();

    static void _captureTaskEntry(void* arg);
    static void _writerTaskEntry(void* arg);
    void _captureTask();
    void _writerTask();

    I2SClass     _i2s;

    // PSRAM capture buffer: filled by the capture task, drained by the
    // writer task every segment. Access is guarded by _bufMutex.
    uint8_t*     _psramBuf;
    size_t       _psramWritten;

    File         _wavFile;
    String       _wavPath;
    uint32_t     _dataLength;

    volatile bool _recording;
    volatile bool _toggleRequested;
    volatile bool _stopWriter;
    unsigned long _lastToggleTime;
    unsigned long _segmentStartMs;

    TaskHandle_t _captureTask_h;
    TaskHandle_t _writerTask_h;

    SemaphoreHandle_t _bufMutex;
};

#endif
