#include "WiFiManager.h"

// Provisioned networks live in Config.h (the config layer) — see WIFI_NETWORKS.
// Add your networks there; do NOT commit real credentials.
// WiFiMulti owns selection: it scans and connects to the strongest known AP,
// so order here does not imply priority.
const char* WIFI_NETWORKS[WIFI_NETWORK_COUNT][2] = {
    { "YOUR_WIFI_SSID",     "YOUR_WIFI_PASSWORD"   },
    { "SECOND_WIFI_SSID",   "SECOND_WIFI_PASSWORD" },   // e.g. laptop hotspot
};

WiFiManager::WiFiManager() : _begun(false), _lastReconnectAttempt(0) {}

bool WiFiManager::begin() {
    if (_begun) return true;

    // Hand every provisioned network to WiFiMulti once.
    for (int i = 0; i < WIFI_NETWORK_COUNT; i++) {
        _wifiMulti.addAP(WIFI_NETWORKS[i][0], WIFI_NETWORKS[i][1]);
    }
    _begun = true;
    DLOGF("[WiFi] %d network(s) provisioned.\n", WIFI_NETWORK_COUNT);
    return true;
}

bool WiFiManager::connect() {
    if (!_begun) begin();

    DLOG("[WiFi] Connecting (WiFiMulti picks the strongest known network)...");
    // run() scans and connects to the best-RSSI known AP; blocks up to timeout.
    if (_wifiMulti.run(WIFI_CONNECT_TIMEOUT_MS) == WL_CONNECTED) {
        DLOGF("[WiFi] Connected to %s — IP: %s\n",
              WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
        return true;
    }

    DLOG("[WiFi] No known network available.");
    return false;
}

bool WiFiManager::ensureConnected() {
    if (isConnected()) return true;
    unsigned long now = millis();
    if (now - _lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) return false;
    _lastReconnectAttempt = now;
    return connect();
}
