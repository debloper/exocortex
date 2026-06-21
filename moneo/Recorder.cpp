#include "Recorder.h"
#include <time.h>

Recorder::Recorder()
    : _psramBuf(nullptr), _psramWritten(0),
      _dataLength(0), _recording(false),
      _toggleRequested(false), _stopWriter(false),
      _lastToggleTime(0), _segmentStartMs(0),
      _captureTask_h(nullptr), _writerTask_h(nullptr),
      _bufMutex(nullptr)
{}

bool Recorder::begin() {
    if (!SD.begin(SD_CARD_PIN)) {
        DLOG("[Recorder] SD card mount failed!");
        return false;
    }
    DLOG("[Recorder] SD card mounted.");

    _psramBuf = (uint8_t*)ps_malloc(PSRAM_BUFFER_SIZE);
    if (!_psramBuf) {
        DLOG("[Recorder] PSRAM allocation failed!");
        return false;
    }
    DLOG("[Recorder] PSRAM buffer allocated.");

    _bufMutex = xSemaphoreCreateMutex();
    if (!_bufMutex) {
        DLOG("[Recorder] Mutex creation failed!");
        return false;
    }

    _i2s.setPinsPdmRx(I2S_BCLK_PIN, I2S_LRCLK_PIN);
    if (!_i2s.begin(I2S_MODE_PDM_RX, SAMPLE_RATE,
                    I2S_DATA_BIT_WIDTH_8BIT, I2S_SLOT_MODE_MONO)) {
        DLOG("[Recorder] I2S init failed!");
        return false;
    }
    DLOG("[Recorder] I2S initialized (16kHz, 8-bit).");

    DLOG("[Recorder] Ready. Touch pin to start.");
    return true;
}

void Recorder::loop() {
    if (!_toggleRequested) return;
    _toggleRequested = false;

    unsigned long now = millis();
    if (now - _lastToggleTime < DEBOUNCE_DELAY) return;
    _lastToggleTime = now;

    if (!_recording) {
        _startRecording();
    } else {
        _stopRecording();
    }
}

void Recorder::requestToggle() {
    _toggleRequested = true;
}

// ── Generate datetime filename ─────────────────────────────
String Recorder::_generateFilename() {
    // Try to get real time from NTP if available
    // Fall back to millis-based name if no time sync
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 1000)) {
        char buf[32];
        strftime(buf, sizeof(buf), "/rec_%Y%m%d_%H%M%S.wav", &timeinfo);
        return String(buf);
    }

    // Fallback: use millis
    unsigned long ms = millis();
    unsigned long secs = ms / 1000;
    char buf[32];
    snprintf(buf, sizeof(buf), "/rec_%05lu.wav", secs);
    return String(buf);
}

// ── WAV header ─────────────────────────────────────────────
void Recorder::_writeWavHeader(File& f, uint32_t dataLen) {
    uint32_t sampleRate  = SAMPLE_RATE;
    uint16_t bitsPerSamp = BITS_PER_SAMPLE;
    uint16_t numChan     = NUM_CHANNELS;
    uint32_t byteRate    = sampleRate * numChan * bitsPerSamp / 8;
    uint16_t blockAlign  = numChan * bitsPerSamp / 8;
    uint32_t chunkSize   = dataLen + 36;
    uint32_t subChunk1   = 16;
    uint16_t audioFormat = 1;

    f.seek(0);
    f.write((const uint8_t*)"RIFF", 4);
    f.write((uint8_t*)&chunkSize,   4);
    f.write((const uint8_t*)"WAVE", 4);
    f.write((const uint8_t*)"fmt ", 4);
    f.write((uint8_t*)&subChunk1,   4);
    f.write((uint8_t*)&audioFormat, 2);
    f.write((uint8_t*)&numChan,     2);
    f.write((uint8_t*)&sampleRate,  4);
    f.write((uint8_t*)&byteRate,    4);
    f.write((uint8_t*)&blockAlign,  2);
    f.write((uint8_t*)&bitsPerSamp, 2);
    f.write((const uint8_t*)"data", 4);
    f.write((uint8_t*)&dataLen,     4);
}

// ── Start ──────────────────────────────────────────────────
void Recorder::_startRecording() {
    DLOG("[Recorder] Starting...");
    _recording     = true;
    _stopWriter    = false;
    _dataLength    = 0;
    _psramWritten  = 0;

    // Try NTP time sync
    configTime(19800, 0, "pool.ntp.org");  // UTC+5:30 for IST
    delay(500);

    _wavPath = _generateFilename();
    DLOGF("[Recorder] File: %s\n", _wavPath.c_str());

    _wavFile = SD.open(_wavPath.c_str(), FILE_WRITE);
    if (!_wavFile) {
        DLOG("[Recorder] Cannot create WAV file!");
        _recording = false;
        return;
    }

    // Write placeholder header (updated on stop)
    _writeWavHeader(_wavFile, 0);

    _segmentStartMs = millis();
    digitalWrite(LED_BUILTIN, HIGH);

    xTaskCreatePinnedToCore(_captureTaskEntry, "Capture", 4096,
                            this, 5, &_captureTask_h, 1);
    xTaskCreatePinnedToCore(_writerTaskEntry,  "Writer",  8192,
                            this, 3, &_writerTask_h,  1);

    DLOG("[Recorder] Recording started.");
}

// ── Stop ───────────────────────────────────────────────────
void Recorder::_stopRecording() {
    DLOG("[Recorder] Stopping...");
    _recording  = false;
    _stopWriter = true;

    digitalWrite(LED_BUILTIN, LOW);

    // Give tasks time to finish naturally
    vTaskDelay(pdMS_TO_TICKS(3000));

    _captureTask_h = nullptr;
    _writerTask_h  = nullptr;

    // Flush remaining PSRAM buffer to SD
    if (_psramWritten > 0 && _wavFile) {
        _wavFile.write(_psramBuf, _psramWritten);
        _dataLength  += _psramWritten;
        _psramWritten = 0;
        DLOGF("[Recorder] Final flush: %u bytes\n", _dataLength);
    }

    // Finalize WAV header
    if (_wavFile) {
        _wavFile.seek(0);
        _writeWavHeader(_wavFile, _dataLength);
        _wavFile.close();
        DLOGF("[Recorder] WAV saved: %s (%u bytes)\n",
              _wavPath.c_str(), _dataLength);
    }

    DLOG("[Recorder] Stopped.");
}

// ── Capture task ───────────────────────────────────────────
void Recorder::_captureTaskEntry(void* arg) {
    ((Recorder*)arg)->_captureTask();
}

void Recorder::_captureTask() {
    uint8_t buf[I2S_BUFFER_SIZE];
    DLOG("[Capture] Task started.");

    while (_recording) {
        int avail = _i2s.available();
        if (avail > 0) {
            int n = _i2s.readBytes((char*)buf, min(avail, I2S_BUFFER_SIZE));
            if (n > 0) {
                xSemaphoreTake(_bufMutex, portMAX_DELAY);
                size_t space = PSRAM_BUFFER_SIZE - _psramWritten;
                size_t toWrite = min((size_t)n, space);
                if (toWrite > 0) {
                    memcpy(_psramBuf + _psramWritten, buf, toWrite);
                    _psramWritten += toWrite;
                }
                xSemaphoreGive(_bufMutex);
            }
        }
        taskYIELD();
    }

    DLOG("[Capture] Task finished.");
    vTaskDelete(nullptr);
}

// ── Writer task ────────────────────────────────────────────
void Recorder::_writerTaskEntry(void* arg) {
    ((Recorder*)arg)->_writerTask();
}

void Recorder::_writerTask() {
    DLOG("[Writer] Task started.");

    while (!_stopWriter) {
        // Every SEGMENT_DURATION_MS, flush PSRAM buffer to SD
        if (millis() - _segmentStartMs >= SEGMENT_DURATION_MS) {
            xSemaphoreTake(_bufMutex, portMAX_DELAY);

            if (_psramWritten > 0 && _wavFile) {
                size_t written = _psramWritten;
                _wavFile.write(_psramBuf, written);
                _dataLength  += written;
                _psramWritten = 0;
                DLOGF("[Writer] Flushed %u bytes to WAV (total: %u)\n",
                      written, _dataLength);
            }

            xSemaphoreGive(_bufMutex);
            _segmentStartMs = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    DLOG("[Writer] Task finished.");
    vTaskDelete(nullptr);
}
