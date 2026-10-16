/*
 * EncoderInterrupt
 * Uses callbacks for the rotary encoder and its push switch. The callbacks
 * run inside the interrupt, so they only record what happened; loop() does
 * the printing. Prints the count when the knob turns, toggles the green LED
 * (LED1) on each press and lights the red LED (LED2) while the switch is held.
 */
#include <AnalogShield.h>

using namespace AnalogShieldPins;

static AnalogShield shield;

// Shared with the callbacks, so volatile
volatile bool turned = false;
volatile bool press_seen = false;

// Runs on every encoder step: keep it short, no Serial or delay()
void onEncoder(int32_t count, int8_t step) {
  (void)count;
  (void)step;
  turned = true;
}

// Runs on every press and release: keep it short, no Serial or delay()
void onSwitch(bool pressed) {
  shield.LedON(LED2, pressed);

  if (pressed) {
    press_seen = true;
  }
}

void setup() {
  Serial.begin(115200);
  shield.begin();
  shield.LedON(LED1, false);
  shield.LedON(LED2, false);

  shield.SetEncoderCallback(onEncoder);
  shield.SetSwitchCallback(onSwitch);
}

void loop() {
  if (turned) {
    turned = false;
    Serial.print("Count: ");
    Serial.println(shield.EncoderCount());
  }

  if (press_seen) {
    press_seen = false;
    Serial.print("Pressed at count ");
    Serial.println(shield.EncoderCount());
    shield.LedToggle(LED1);
  }
}
