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
//   Touch start → create dated WAV file with a placeholder header, close it
//   Every 10s:   open (append) → write the buffered audio → close
//   Touch stop:  append the remainder, then reopen once to write the real
//                length into the header → send to AI
//
// One file per session, no chunk files. The file is kept closed between
// segments, so a power loss can corrupt at most the latest 10s, not the
// whole recording.
// ============================================================

class Recorder {
public:
    Recorder();
    bool begin();
    void loop();
    void IRAM_ATTR requestToggle();

    bool isRecording() const { return _recording; }
    bool hasError() const { return _writeError; }
    String lastRecordingPath() const { return _wavPath; }

private:
    void _startRecording();
    void _stopRecording();
    void _writeWavHeader(File& f, uint32_t dataLen);
    void _finalizeHeader();
    bool _flushSegment();
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

    String       _wavPath;
    uint32_t     _dataLength;

    volatile bool _recording;
    volatile bool _toggleRequested;
    volatile bool _stopWriter;
    volatile bool _writeError;
    unsigned long _lastToggleTime;
    unsigned long _segmentStartMs;

    TaskHandle_t _captureTask_h;
    TaskHandle_t _writerTask_h;

    SemaphoreHandle_t _bufMutex;
};

#endif
