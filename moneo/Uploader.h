#ifndef Uploader_h
#define Uploader_h

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include "Config.h"

#define MAX_QUEUE 20

struct UploadJob {
    int    chunkIndex;
    String filePath;
    int    retryCount;
};

class Uploader {
public:
    Uploader();
    bool begin();
    void enqueue(int chunkIndex, const String& filePath);
    void loop();
    bool ensureWiFi();
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool hasItems();
    bool downloadNote(int chunkIndex, const String& savePath);

private:
    bool _uploadFile(const UploadJob& job);
    UploadJob     _queue[MAX_QUEUE];
    int           _queueHead;
    int           _queueTail;
    unsigned long _lastReconnectAttempt;
};

#endif