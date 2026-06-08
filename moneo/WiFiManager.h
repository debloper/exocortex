#ifndef WiFiManager_h
#define WiFiManager_h

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "Config.h"

// ============================================================
// WiFiManager — Auto-connects to known networks.
// Uses NVS (Preferences) to remember last connected network.
// Tries last known first, then scans all configured networks.
// ============================================================

class WiFiManager {
public:
    WiFiManager();
    bool connect();
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    bool ensureConnected();
    String currentSSID() const { return WiFi.SSID(); }
    String localIP() const { return WiFi.localIP().toString(); }

private:
    bool _tryNetwork(const char* ssid, const char* password);
    void _saveLastNetwork(const char* ssid, const char* password);
    void _loadLastNetwork(String& ssid, String& password);

    Preferences _prefs;
    unsigned long _lastReconnectAttempt;
};

#endif
