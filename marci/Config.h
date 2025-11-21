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

#endif
