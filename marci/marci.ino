#include "Config.h"
#include "Recorder.h"

Recorder recorder;

// This is the interrupt service routine (ISR) for the touch pin.
// It calls the recorder's method to signal a toggle request.
void IRAM_ATTR toggle() {
  recorder.toggleRecording();
}

void setup() {
  Serial.begin(115200);
  // Wait for a maximum of 500ms for the serial connection.
  // This is a compromise to allow for debugging without a long delay on startup.
  unsigned long epoch = millis();
  while (!Serial && millis() - epoch < 500) { delay(10); }

  pinMode(LED_BUILTIN, OUTPUT);
  // Attach the interrupt to the touch pin.
  touchAttachInterrupt(TOUCH_PIN, toggle, TOUCH_THRESHOLD);

  // Initialize the recorder. This also initializes the I2S bus and SD card.
  if (!recorder.begin()) {
    Serial.println("Initialization failed!");
    // Blink LED to indicate error
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
  }
  Serial.println("Initialization complete!");
}

void loop() {
  // The main loop just needs to call the recorder's loop function,
  // which handles all the recording logic.
  recorder.loop();
}
