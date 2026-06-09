#include "WiFiManager.h"

// Add your networks here — the device scans and connects to the first available.
// Do NOT commit real credentials; replace these placeholders with your own locally.
const char* WIFI_NETWORKS[WIFI_NETWORK_COUNT][2] = {
    { "YOUR_WIFI_SSID",     "YOUR_WIFI_PASSWORD"   },
    { "SECOND_WIFI_SSID",   "SECOND_WIFI_PASSWORD" },   // e.g. laptop hotspot
};

WiFiManager::WiFiManager() : _lastReconnectAttempt(0) {}

bool WiFiManager::connect() {
    // Try last known network first (from NVS)
    String lastSSID, lastPass;
    _loadLastNetwork(lastSSID, lastPass);

    if (lastSSID.length() > 0) {
        DLOGF("[WiFi] Trying last known: %s\n", lastSSID.c_str());
        if (_tryNetwork(lastSSID.c_str(), lastPass.c_str())) return true;
    }

    // Scan and try all configured networks
    DLOG("[WiFi] Scanning for known networks...");
    int found = WiFi.scanNetworks();

    // Iterate over our known networks (priority order), not the scan list.
    // Cost scales with the few provisioned networks, not the many nearby ones.
    for (int i = 0; i < WIFI_NETWORK_COUNT; i++) {
        bool available = false;
        for (int j = 0; j < found; j++) {
            if (WiFi.SSID(j) == String(WIFI_NETWORKS[i][0])) {
                available = true;
                break;  // this known network is in range — stop scanning for it
            }
        }
        if (!available) continue;

        DLOGF("[WiFi] Found: %s\n", WIFI_NETWORKS[i][0]);
        if (_tryNetwork(WIFI_NETWORKS[i][0], WIFI_NETWORKS[i][1])) {
            _saveLastNetwork(WIFI_NETWORKS[i][0], WIFI_NETWORKS[i][1]);
            return true;
        }
    }

    DLOG("[WiFi] No known network found.");
    return false;
}

bool WiFiManager::ensureConnected() {
    if (isConnected()) return true;
    unsigned long now = millis();
    if (now - _lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) return false;
    _lastReconnectAttempt = now;
    return connect();
}

bool WiFiManager::_tryNetwork(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(300);
    }
    if (WiFi.status() == WL_CONNECTED) {
        DLOGF("[WiFi] Connected to %s — IP: %s\n", ssid,
              WiFi.localIP().toString().c_str());
        return true;
    }
    WiFi.disconnect();
    return false;
}

void WiFiManager::_saveLastNetwork(const char* ssid, const char* password) {
    // Runtime state managed by the firmware (last connected network, future:
    // last file synced, etc.) — kept separate from the provisioned network list.
    _prefs.begin("moneo_runtime", false);
    _prefs.putString("ssid", ssid);
    _prefs.putString("pass", password);
    _prefs.end();
}

void WiFiManager::_loadLastNetwork(String& ssid, String& password) {
    _prefs.begin("moneo_runtime", true);
    ssid     = _prefs.getString("ssid", "");
    password = _prefs.getString("pass", "");
    _prefs.end();
}
