#ifndef Config_h
#define Config_h

// Pin Configurations
const int TOUCH_PIN = 1;
const int SD_CARD_PIN = 21;
const int I2S_BCLK_PIN = 42;
const int I2S_LRCLK_PIN = 41;

// Touch Sensor
const int TOUCH_THRESHOLD = 65536;
const unsigned long DEBOUNCE_DELAY = 1000;

// Audio Settings
const int SAMPLE_RATE = 16000;
const int BITS_PER_SAMPLE = 16;
const int NUM_CHANNELS = 1;

// Recording Settings
const unsigned long RECORDING_DURATION_MS = 60000;
const int I2S_BUFFER_SIZE = 512;

// Auto-upload toggle
#define AUTO_UPLOAD_ENABLED 1

// WiFi Settings
#define WIFI_SSID "wifi_ssid"
#define WIFI_PASSWORD "wifi_password"
#define WIFI_CONNECT_TIMEOUT_MS 10000

// Server Settings
#define SERVER_HOST "192.168.1.1"
#define SERVER_PORT 3923
#define UPLOAD_PATH "/"

#endif
