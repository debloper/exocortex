#include "Recorder.h"

Recorder::Recorder()
    : _ringBuf(RING_BUFFER_SIZE),
      _recording(false),
      _toggleRequested(false),
      _lastToggleTime(0),
      _chunkStartMs(0),
      _currentChunkIndex(0),
      _captureTask(nullptr),
      _writerTask(nullptr),
      _uploadTaskHandle(nullptr),
      _stopWriting(false),
      _stopUploading(false)
{}

bool Recorder::begin() {
    DLOG("[Recorder] Initializing...");
    if (!_ringBuf.begin()) { DLOG("[Recorder] Ring buffer init failed!"); return false; }
    if (!SD.begin(SD_CARD_PIN)) { DLOG("[Recorder] SD card mount failed!"); return false; }
    DLOG("[Recorder] SD card mounted.");
    if (!_session.begin()) { DLOG("[Recorder] Session manager init failed!"); return false; }
    if (!_uploader.begin()) { DLOG("[Recorder] Uploader init failed!"); return false; }
    _i2s.setPinsPdmRx(I2S_BCLK_PIN, I2S_LRCLK_PIN);
    if (!_i2s.begin(I2S_MODE_PDM_RX, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        DLOG("[Recorder] I2S init failed!"); return false;
    }
    DLOG("[Recorder] I2S initialized.");
    DLOG("[Recovery] Scanning for incomplete sessions...");
    DLOG("[Recorder] Ready. Touch pin to start recording.");
    return true;
}

void Recorder::loop() {
    if (!_toggleRequested) return;
    unsigned long now = millis();
    if (now - _lastToggleTime < DEBOUNCE_DELAY) { _toggleRequested = false; return; }
    _lastToggleTime = now;
    _toggleRequested = false;
    if (!_recording) { _startSession(); } else { _stopSession(); }
}

void Recorder::requestToggle() { _toggleRequested = true; }

void Recorder::_startSession() {
    DLOG("[Recorder] Starting session...");
    _recording    = true;
    _stopWriting  = false;
    _stopUploading= false;
    _ringBuf.reset();
    _session.startNewSession();
    _currentChunkIndex = 0;
    digitalWrite(LED_BUILTIN, HIGH);

    xTaskCreatePinnedToCore(_audioCaptureTaskEntry, "AudioCapture", 4096, this, 5, &_captureTask, 1);
    xTaskCreatePinnedToCore(_chunkWriterTaskEntry,  "ChunkWriter",  8192, this, 3, &_writerTask,  1);
    xTaskCreatePinnedToCore(_uploadTaskEntry, "UploadTask", 8192, this, 1, &_uploadTaskHandle, 0);

    DLOG("[Recorder] All tasks started.");
}

void Recorder::_stopSession() {
    DLOG("[Recorder] Stopping session...");
    _recording   = false;
    _stopWriting = true;
    digitalWrite(LED_BUILTIN, LOW);

    // Wait for writer to finish
    vTaskDelay(pdMS_TO_TICKS(3000));

    _captureTask = nullptr;
    _writerTask  = nullptr;

    DLOG("[Recorder] Recording stopped. Upload continuing...");
    // Upload task keeps running until queue is empty
}

void Recorder::_audioCaptureTaskEntry(void* arg) { ((Recorder*)arg)->_audioCaptureTask(); }
void Recorder::_chunkWriterTaskEntry(void* arg)  { ((Recorder*)arg)->_chunkWriterTask(); }
void Recorder::_uploadTaskEntry(void* arg)       { ((Recorder*)arg)->_uploadTask(); }

void Recorder::_audioCaptureTask() {
    uint8_t buf[I2S_BUFFER_SIZE];
    DLOG("[AudioCapture] Task started.");
    while (_recording) {
        int avail = _i2s.available();
        if (avail > 0) {
            int n = _i2s.readBytes((char*)buf, min(avail, I2S_BUFFER_SIZE));
            if (n > 0) _ringBuf.write(buf, n);
        }
        taskYIELD();
    }
    DLOG("[AudioCapture] Task finished.");
    vTaskDelete(nullptr);
}

void Recorder::_chunkWriterTask() {
    DLOG("[ChunkWriter] Task started.");
    uint8_t buf[512];
    String path = _session.nextChunkPath();
    _currentChunkIndex = _session.getCurrentChunkIndex();
    _writer.openFile(path);
    _chunkStartMs = millis();

    while (!_stopWriting || !_ringBuf.isEmpty()) {
        if (_recording && (millis() - _chunkStartMs >= CHUNK_DURATION_MS)) {
            _writer.closeFile();
            DLOGF("[ChunkWriter] Chunk %d done.\n", _currentChunkIndex);
            _uploader.enqueue(_currentChunkIndex, path);
            path = _session.nextChunkPath();
            _currentChunkIndex = _session.getCurrentChunkIndex();
            _writer.openFile(path);
            _chunkStartMs = millis();
        }
        size_t n = _ringBuf.read(buf, sizeof(buf));
        if (n > 0) { _writer.write(buf, n); }
        else { vTaskDelay(pdMS_TO_TICKS(2)); }
    }

    if (_writer.isOpen()) {
        _writer.closeFile();
        DLOGF("[ChunkWriter] Final chunk %d saved.\n", _currentChunkIndex);
        _uploader.enqueue(_currentChunkIndex, path);
    }
    DLOG("[ChunkWriter] Task finished.");
    vTaskDelete(nullptr);
}

void Recorder::_uploadTask() {
    DLOG("[UploadTask] Task started.");

    // Wait for WiFi then drain upload queue
    while (true) {
        _uploader.loop();

        // Exit only when recording stopped AND queue empty
        if (!_recording && !_uploader.hasItems()) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    DLOG("[UploadTask] All chunks uploaded. Task finished.");
    _uploadTaskHandle = nullptr;
    vTaskDelete(nullptr);
}