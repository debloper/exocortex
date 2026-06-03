#ifndef SDWriter_h
#define SDWriter_h

#include <Arduino.h>
#include <SD.h>
#include "Config.h"

// ============================================================
// SDWriter — Writes audio data to WAV files on SD card.
// Handles WAV header creation and finalization.
// One SDWriter instance per chunk file.
// ============================================================

class SDWriter {
public:
    SDWriter();

    bool openFile(const String& path);
    void write(const uint8_t* data, size_t len);
    bool closeFile();         // finalize header and close
    bool isOpen() const { return _file; }
    uint32_t bytesWritten() const { return _dataLength; }

private:
    void _writeWavHeader();
    void _finalizeWavHeader();

    File     _file;
    String   _path;
    uint32_t _dataLength;
};

#endif
