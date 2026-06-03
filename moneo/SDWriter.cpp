#include "SDWriter.h"

SDWriter::SDWriter() : _dataLength(0) {}

bool SDWriter::openFile(const String& path) {
    _path = path;
    _dataLength = 0;
    _file = SD.open(path.c_str(), FILE_WRITE);
    if (!_file) {
        DLOGF("[SDWriter] Failed to open: %s\n", path.c_str());
        return false;
    }
    _writeWavHeader();   // placeholder, updated on close
    DLOGF("[SDWriter] Opened: %s\n", path.c_str());
    return true;
}

void SDWriter::write(const uint8_t* data, size_t len) {
    if (!_file) return;
    _file.write(data, len);
    _dataLength += len;
}

bool SDWriter::closeFile() {
    if (!_file) return false;
    _finalizeWavHeader();
    _file.close();
    DLOGF("[SDWriter] Closed: %s (%u bytes)\n", _path.c_str(), _dataLength);
    return true;
}

void SDWriter::_writeWavHeader() {
    uint32_t sampleRate   = SAMPLE_RATE;
    uint16_t bitsPerSample= BITS_PER_SAMPLE;
    uint16_t numChannels  = NUM_CHANNELS;
    uint32_t byteRate     = sampleRate * numChannels * bitsPerSample / 8;
    uint16_t blockAlign   = numChannels * bitsPerSample / 8;
    uint32_t dataLen      = _dataLength;
    uint32_t chunkSize    = dataLen + 36;
    uint32_t subChunk1    = 16;
    uint16_t audioFormat  = 1;

    _file.seek(0);
    _file.write((const uint8_t*)"RIFF",       4);
    _file.write((uint8_t*)&chunkSize,          4);
    _file.write((const uint8_t*)"WAVE",       4);
    _file.write((const uint8_t*)"fmt ",       4);
    _file.write((uint8_t*)&subChunk1,         4);
    _file.write((uint8_t*)&audioFormat,       2);
    _file.write((uint8_t*)&numChannels,       2);
    _file.write((uint8_t*)&sampleRate,        4);
    _file.write((uint8_t*)&byteRate,          4);
    _file.write((uint8_t*)&blockAlign,        2);
    _file.write((uint8_t*)&bitsPerSample,     2);
    _file.write((const uint8_t*)"data",       4);
    _file.write((uint8_t*)&dataLen,           4);
}

void SDWriter::_finalizeWavHeader() {
    // Rewind and rewrite the header with correct data length
    _file.seek(0);
    _writeWavHeader();
}
