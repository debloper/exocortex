#ifndef Config_h
#define Config_h

// ============================================================
// MONEO V2 — Config.h
// ============================================================

// ─── PIN CONFIGURATION ───────────────────────────────────────
const int TOUCH_PIN       = 1;
const int SD_CARD_PIN     = 21;
const int I2S_BCLK_PIN    = 42;
const int I2S_LRCLK_PIN   = 41;

// ─── TOUCH SENSOR ────────────────────────────────────────────
const int          TOUCH_THRESHOLD  = 50000;
const unsigned long DEBOUNCE_DELAY  = 5000;

// ─── STARTUP ─────────────────────────────────────────────────
const unsigned long SERIAL_WAIT_MS = 100;   // max wait for USB serial at boot

// ─── AUDIO SETTINGS ──────────────────────────────────────────
const int SAMPLE_RATE     = 16000;
const int BITS_PER_SAMPLE = 8;        // 8-bit audio
const int NUM_CHANNELS    = 1;
const int I2S_BUFFER_SIZE = 512;

// ─── RECORDING SETTINGS ──────────────────────────────────────
// Each segment is buffered in PSRAM then appended to the single WAV file
const unsigned long SEGMENT_DURATION_MS = 10000;  // 10 seconds per segment
const size_t PSRAM_BUFFER_SIZE = 160000;           // 10s * 16000Hz * 1 byte = 160KB

// ─── WIFI — add as many networks as needed ───────────────────
// Format: { "SSID", "PASSWORD" }
// Device tries each in order, connects to first available
#define WIFI_NETWORK_COUNT 2
extern const char* WIFI_NETWORKS[WIFI_NETWORK_COUNT][2];
const unsigned long WIFI_CONNECT_TIMEOUT_MS  = 10000;
const unsigned long WIFI_RECONNECT_INTERVAL  = 5000;

// ─── AI API — auto-detected from key prefix ──────────────────
// Paste your own API key here. Provider is detected automatically:
//   sk-...        → OpenAI  (gpt-4o-audio-preview)
//   AIza...       → Gemini  (gemini-2.5-flash-lite)
//   gsk_...       → Groq
#define AI_API_KEY   "YOUR_API_KEY_HERE"

// ─── FEATURE TOGGLES ─────────────────────────────────────────
#define DEBUG_LOGS_ENABLED    1

// ─── DEBUG HELPER ────────────────────────────────────────────
#if DEBUG_LOGS_ENABLED
  #define DLOG(x)     Serial.println(x)
  #define DLOGF(...)  Serial.printf(__VA_ARGS__)
#else
  #define DLOG(x)
  #define DLOGF(...)
#endif

#endif
