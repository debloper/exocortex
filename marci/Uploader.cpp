#include "Uploader.h"
#include <WiFi.h>
#include <SD.h>
#include "Config.h"

Uploader::Uploader() {}

bool Uploader::ensureConnected() {
#if AUTO_UPLOAD_ENABLED
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    Serial.println("Attempting to connect to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < (unsigned long)WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("Failed to connect to WiFi.");
        return false;
    }
#else
    Serial.println("Auto upload disabled in Config.h; skipping WiFi connect.");
    return false;
#endif
}

void Uploader::uploadFile(String fullpath) {
    if (!ensureConnected()) {
        return;
    }

    WiFiClient client;
    if (!client.connect(SERVER_HOST, SERVER_PORT)) {
        Serial.println("Failed to connect to server.");
        return;
    }

    String filepath = fullpath.substring(0, fullpath.lastIndexOf('/'));
    String filename = fullpath.substring(fullpath.lastIndexOf('/') + 1);

    File file = SD.open(fullpath.c_str(), FILE_READ);
    if (!file) {
        Serial.println("Failed to open file: " + fullpath);
        client.stop();
        return;
    }

    size_t fileSize = file.size();

    client.println("PUT " + String(UPLOAD_PATH) + filepath.substring(1) + "/" + filename + " HTTP/1.1");
    client.println("Host: " + String(SERVER_HOST) + ":" + String(SERVER_PORT));
    client.println("Content-Type: application/octet-stream");
    client.println("Content-Length: " + String(fileSize));
    client.println("Connection: close");
    client.println();

    uint8_t buffer[512];
    while (file.available()) {
        size_t len = file.read(buffer, sizeof(buffer));
        client.write(buffer, len);
    }

    file.close();

    // Read response
    String response = "";
    while (client.available()) {
        response += client.readString();
    }
    client.stop();

    if (response.indexOf("200 OK") > -1 || response.indexOf("HTTP/1.1 200") > -1) {
        Serial.println("File uploaded successfully: " + fullpath);
    } else {
        Serial.println("Failed to upload file: " + fullpath);
        Serial.println("Response: " + response);
    }
}

void Uploader::syncDirectory(String dirPath) {
    if (!ensureConnected()) {
        return;
    }

    File dir = SD.open(dirPath.c_str());
    if (!dir || !dir.isDirectory()) {
        Serial.println("Failed to open directory: " + dirPath);
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filePath = dirPath + "/" + file.name();
            Serial.println("Syncing file: " + filePath);
            uploadFile(filePath);
        }
        file = dir.openNextFile();
    }
    dir.close();
    Serial.println("Directory sync complete: " + dirPath);
}
