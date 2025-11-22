#ifndef Uploader_h
#define Uploader_h

#include <Arduino.h>

class Uploader {
public:
    Uploader();
    void uploadFile(String fullpath);
    void syncDirectory(String dirPath);
    bool ensureConnected();
};

#endif