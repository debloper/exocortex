#ifndef SDManager_h
#define SDManager_h

#include <Arduino.h>
#include "FS.h"
#include "SD.h"

class SDManager {
public:
    SDManager();
    bool begin();
    void startNewRun();
    void createNewFile();
    void write(const uint8_t* buffer, size_t size);
    void closeFile();
    void finishRecording();
    String getLastFileName() { return currentFileName; }
    int getRunCount() { return currentRun; }

private:
    void writeWavHeader();
    String getNextFilename();
    File currentFile;
    String currentFileName;
    uint32_t dataLength;
    int currentRun;
    int runCount;
};

#endif
