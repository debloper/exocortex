#include "Uploader.h"
#include <WiFiClient.h>
Uploader::Uploader() : _lastReconnectAttempt(0), _queueHead(0), _queueTail(0) {}

bool Uploader::begin() { return true; }

void Uploader::enqueue(int chunkIndex, const String& filePath) {
#if AUTO_UPLOAD_ENABLED
    if ((_queueTail + 1) % MAX_QUEUE == _queueHead) {
        DLOG("[Uploader] Queue full, dropping chunk.");
        return;
    }
    _queue[_queueTail].chunkIndex = chunkIndex;
    _queue[_queueTail].filePath   = filePath;
    _queue[_queueTail].retryCount = 0;
    _queueTail = (_queueTail + 1) % MAX_QUEUE;
    DLOGF("[Uploader] Enqueued chunk %d\n", chunkIndex);
#endif
}

bool Uploader::hasItems() { return _queueHead != _queueTail; }

void Uploader::loop() {
#if AUTO_UPLOAD_ENABLED
    if (!hasItems()) return;
    if (!ensureWiFi()) return;

    UploadJob job = _queue[_queueHead];
_queueHead = (_queueHead + 1) % MAX_QUEUE;

delay(500);  // let WiFi stack breathe
bool ok = _uploadFile(job);
    if (!ok && job.retryCount < MAX_UPLOAD_RETRIES) {
        job.retryCount++;
        _queue[_queueTail] = job;
        _queueTail = (_queueTail + 1) % MAX_QUEUE;
        DLOGF("[Uploader] Retry %d for chunk %d\n", job.retryCount, job.chunkIndex);
    }
#endif
}

bool Uploader::ensureWiFi() {
    if (WiFi.status() == WL_CONNECTED) return true;
    unsigned long now = millis();
    if (now - _lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) return false;
    _lastReconnectAttempt = now;
    DLOG("[Uploader] Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < (unsigned long)WIFI_CONNECT_TIMEOUT_MS) {
        delay(300);
    }
    if (WiFi.status() == WL_CONNECTED) {
        DLOGF("[Uploader] WiFi connected. IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    DLOG("[Uploader] WiFi failed.");
    return false;
}

bool Uploader::_uploadFile(const UploadJob& job) {
    File f = SD.open(job.filePath.c_str(), FILE_READ);
    if (!f) { DLOGF("[Uploader] Cannot open: %s\n", job.filePath.c_str()); return false; }
    size_t fileSize = f.size();
    DLOGF("[Uploader] Uploading chunk %d (%u bytes)...\n", job.chunkIndex, fileSize);

    WiFiClient client;
    client.setTimeout(10000);
int retries = 3;
bool connected = false;
while (retries-- > 0) {
    if (client.connect(GATEWAY_HOST, GATEWAY_PORT)) {
        connected = true;
        break;
    }
    delay(1000);
}
if (!connected) {
    DLOG("[Uploader] Cannot connect to gateway.");
    f.close();
    return false;
}

    // Send HTTP headers manually — no malloc needed
    client.println("POST /upload HTTP/1.1");
    client.println("Host: " + String(GATEWAY_HOST) + ":" + String(GATEWAY_PORT));
    client.println("Content-Type: audio/wav");
    client.println("X-Chunk: " + String(job.chunkIndex));
    client.println("X-Session: " + job.filePath);
    client.println("Content-Length: " + String(fileSize));
    client.println("Connection: close");
    client.println();

    // Stream file in small chunks — no large malloc
    uint8_t buf[512];
    size_t sent = 0;
    while (f.available()) {
        size_t n = f.read(buf, sizeof(buf));
        client.write(buf, n);
        sent += n;
    }
    f.close();

    // Read response
    unsigned long timeout = millis();
    while (client.available() == 0 && millis() - timeout < 10000) delay(100);

    String response = "";
    while (client.available()) response += (char)client.read();
    client.stop();

    if (response.indexOf("200") > -1 || response.indexOf("201") > -1) {
        DLOGF("[Uploader] Chunk %d uploaded OK\n", job.chunkIndex);
        return true;
    }
    DLOGF("[Uploader] Upload failed. Response: %s\n", response.c_str());
    return false;
}

bool Uploader::downloadNote(int chunkIndex, const String& savePath) {
    if (!ensureWiFi()) return false;
    HTTPClient http;
    String url = String("http://") + GATEWAY_HOST + ":" + GATEWAY_PORT +
                 NOTES_ENDPOINT + "/" + String(chunkIndex);
    http.begin(url);
    int code = http.GET();
    if (code != 200) { http.end(); return false; }
    String body = http.getString();
    http.end();
    if (body.isEmpty()) return false;
    File f = SD.open(savePath.c_str(), FILE_WRITE);
    if (!f) return false;
    f.print(body);
    f.close();
    DLOGF("[Uploader] Note saved: %s\n", savePath.c_str());
    return true;
}