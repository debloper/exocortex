#include "SDManager.h"
#include "Config.h"

SDManager::SDManager() : dataLength(0), runCount(0), currentRun(-1) {}

bool SDManager::begin() {
    if (!SD.begin(SD_CARD_PIN)) {
        Serial.println("Can't mount SD Card!");
        return false;
    }
    
    // Determine the run count by checking existing directories
    while (SD.exists("/" + String(runCount))) {
        runCount++;
    }
    
    Serial.println("SD card mounted.");
    return true;
}

void SDManager::startNewRun() {
    currentRun = runCount;
    SD.mkdir("/" + String(currentRun));
    runCount++;
}

String SDManager::getNextFilename() {
    return "/" + String(currentRun) + "/" + String(millis()) + ".wav";
}

void SDManager::createNewFile() {
    currentFileName = getNextFilename();
    currentFile = SD.open(currentFileName.c_str(), FILE_WRITE);
    if (!currentFile) {
        Serial.println("Can't open file to write!");
        return;
    }
    dataLength = 0;
    writeWavHeader(); // Write placeholder header
}

void SDManager::write(const uint8_t* buffer, size_t size) {
    if (currentFile) {
        currentFile.write(buffer, size);
        dataLength += size;
    }
}

void SDManager::closeFile() {
    if (currentFile) {
        finishRecording();
        currentFile.close();
        Serial.print("Saved: ");
        Serial.println(currentFileName);
    }
}

void SDManager::finishRecording() {
    if (currentFile) {
        currentFile.seek(0);
        writeWavHeader(); // Update header with correct data length
    }
}

void SDManager::writeWavHeader() {
    uint32_t sampleRate = SAMPLE_RATE;
    uint16_t bitsPerSample = BITS_PER_SAMPLE;
    uint16_t numChannels = NUM_CHANNELS;
    uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    uint16_t blockAlign = numChannels * bitsPerSample / 8;
    uint32_t chunkSize = dataLength + 36;

    currentFile.seek(0);
    currentFile.write((const uint8_t*)"RIFF", 4);
    currentFile.write((uint8_t *)&chunkSize, 4);
    currentFile.write((const uint8_t*)"WAVE", 4);
    currentFile.write((const uint8_t*)"fmt ", 4);
    uint32_t subChunk1Size = 16;
    currentFile.write((uint8_t *)&subChunk1Size, 4);
    uint16_t audioFormat = 1;
    currentFile.write((uint8_t *)&audioFormat, 2);
    currentFile.write((uint8_t *)&numChannels, 2);
    currentFile.write((uint8_t *)&sampleRate, 4);
    currentFile.write((uint8_t *)&byteRate, 4);
    currentFile.write((uint8_t *)&blockAlign, 2);
    currentFile.write((uint8_t *)&bitsPerSample, 2);
    currentFile.write((const uint8_t*)"data", 4);
    currentFile.write((uint8_t *)&dataLength, 4);
}
