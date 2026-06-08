#ifndef Config_h
#define Config_h

// ============================================================
// MONEO — Exocortex Iteration II
// Config.h — Central Configuration Layer
// Edit this file to customize your deployment.
// ============================================================

// ─── PIN CONFIGURATION ───────────────────────────────────────
const int TOUCH_PIN       = 1;
const int SD_CARD_PIN     = 21;
const int I2S_BCLK_PIN    = 42;   // PDM CLK (BCLK repurposed)
const int I2S_LRCLK_PIN   = 41;   // PDM DATA

// ─── TOUCH SENSOR ────────────────────────────────────────────
const int          TOUCH_THRESHOLD  = 50000;
const unsigned long DEBOUNCE_DELAY  = 5000;   // ms

// ─── AUDIO SETTINGS ──────────────────────────────────────────
const int SAMPLE_RATE    = 16000;
const int BITS_PER_SAMPLE = 16;
const int NUM_CHANNELS   = 1;
const int I2S_BUFFER_SIZE = 512;

// ─── CHUNK / RECORDING SETTINGS ──────────────────────────────
// Each chunk is one independently-uploadable WAV file.
// Recommended: 10000 ms (10 seconds)
const unsigned long CHUNK_DURATION_MS = 10000;

// PSRAM ring buffer size (must fit in PSRAM — 512 KB safe default)
const size_t RING_BUFFER_SIZE = 512 * 1024;

// ─── WIFI SETTINGS ───────────────────────────────────────────
// NOTE: Put your own network here. Do NOT commit real credentials.
#define WIFI_SSID                "YOUR_WIFI_SSID"
#define WIFI_PASSWORD            "YOUR_WIFI_PASSWORD"
#define WIFI_CONNECT_TIMEOUT_MS  10000
#define WIFI_RECONNECT_INTERVAL  5000   // ms between reconnect attempts

// ─── LAPTOP GATEWAY SETTINGS ─────────────────────────────────
#define GATEWAY_HOST   "192.168.137.1"   // IP of your laptop hotspot gateway
#define GATEWAY_PORT   5000
#define UPLOAD_ENDPOINT "/upload"
#define NOTES_ENDPOINT  "/notes"

// ─── UPLOAD SETTINGS ─────────────────────────────────────────
#define MAX_UPLOAD_RETRIES  5
#define UPLOAD_RETRY_DELAY  3000    // ms between retries

// ─── FEATURE TOGGLES ─────────────────────────────────────────
#define AUTO_UPLOAD_ENABLED   1   // 1 = upload chunks to gateway
#define NOTE_SYNC_ENABLED     1   // 1 = download notes from gateway
#define TRANSCRIPT_ENABLED    1   // handled server-side; toggle here for future
#define DEBUG_LOGS_ENABLED    1   // 1 = verbose serial output

// ─── DEBUG HELPER ────────────────────────────────────────────
#if DEBUG_LOGS_ENABLED
  #define DLOG(x)   Serial.println(x)
  #define DLOGF(...)  Serial.printf(__VA_ARGS__)
#else
  #define DLOG(x)
  #define DLOGF(...)
#endif

#endif // Config_h
