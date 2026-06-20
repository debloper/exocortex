#ifndef WiFiManager_h
#define WiFiManager_h

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "Config.h"

// ============================================================
// WiFiManager — Connects to the best available known network.
//
// Networks are provisioned in Config.h (the config layer).
// Selection is delegated to WiFiMulti: it scans and connects to
// the strongest-signal known network on its own. There is no
// hand-rolled scan/match/priority logic, and no "last connected"
// network to remember — WiFiMulti handles that internally.
// ============================================================

class WiFiManager {
public:
    WiFiManager();
    bool begin();              // register provisioned networks with WiFiMulti
    bool connect();            // bring up the best available known network
    bool ensureConnected();    // reconnect if the link dropped
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    String currentSSID() const { return WiFi.SSID(); }
    String localIP() const { return WiFi.localIP().toString(); }

private:
    WiFiMulti     _wifiMulti;
    bool          _begun;
    unsigned long _lastReconnectAttempt;
};

#endif
