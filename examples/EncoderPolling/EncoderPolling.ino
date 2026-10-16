/*
 * EncoderPolling
 * Reads the rotary encoder and its push switch from loop(). Prints the count
 * when the knob turns, and toggles the green LED (LED1) on each press.
 *
 * The library still counts encoder steps in the background; loop() just
 * checks EncoderCount() and SwitchPressed() whenever it runs.
 */
#include <AnalogShield.h>

using namespace AnalogShieldPins;

AnalogShield shield;

int32_t last_count = 0;
bool last_pressed = false;

void setup() {
  Serial.begin(115200);
  shield.begin();
  shield.LedON(LED1, false);
  shield.LedON(LED2, false);
}

void loop() {
  // Encoder: print only when the count has changed
  const int32_t count = shield.EncoderCount();

  if (count != last_count) {
    Serial.print("Count: ");
    Serial.println(count);
    last_count = count;
  }

  // Switch: act on the press, not while it is held
  const bool pressed = shield.SwitchPressed();

  if (pressed && !last_pressed) {
    Serial.print("Pressed at count ");
    Serial.println(count);
    shield.LedToggle(LED1);
  }

  last_pressed = pressed;
}
