#ifndef Recorder_h
#define Recorder_h

#include <Arduino.h>
#include <ESP_I2S.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Config.h"
#include "RingBuffer.h"
#include "SDWriter.h"
#include "SessionManager.h"
#include "Uploader.h"

class Recorder {
public:
    Recorder();
    bool begin();
    void loop();
    void requestToggle();

private:
    void _startSession();
    void _stopSession();
    static void _audioCaptureTaskEntry(void* arg);
    static void _chunkWriterTaskEntry(void* arg);
    static void _uploadTaskEntry(void* arg);
    void _audioCaptureTask();
    void _chunkWriterTask();
    void _uploadTask();

    I2SClass       _i2s;
    RingBuffer     _ringBuf;
    SDWriter       _writer;
    SessionManager _session;
    Uploader       _uploader;

    volatile bool  _recording;
    volatile bool  _toggleRequested;
    unsigned long  _lastToggleTime;
    unsigned long  _chunkStartMs;
    int            _currentChunkIndex;
    TaskHandle_t   _captureTask;
    TaskHandle_t   _writerTask;
    TaskHandle_t   _uploadTaskHandle;
    volatile bool  _stopWriting;
    volatile bool  _stopUploading;
};

#endif