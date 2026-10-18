/*
 * LedFade
 * Fades the green LED (D1) in and out with PWM while the red LED (D2) does
 * the opposite, so one brightens as the other dims. One full cycle, in and
 * out, takes about 2 s.
 *
 * Jumpers: JP3 at 3.3 V.
 * Output: the LEDs only; nothing is printed.
 *
 * Both LEDs are active low: the anode goes to VDD and the pin sinks the
 * current, so a lower analogWrite() value gives a brighter LED. setBrightness()
 * writes 255 - brightness, so 0 is off and 255 is full brightness.
 */
#include <AnalogShield.h>

using namespace AnalogShieldPins;

static AnalogShield Shield;

constexpr uint32_t STEP_MS = 4;  // time per brightness step; 2 × 256 steps ≈ 2 s

// Sets an active-low LED from 0 (off) to 255 (full brightness)
void setBrightness(uint8_t pin, uint8_t brightness) {
  analogWrite(pin, 255 - brightness);
}

void setup() {
  Shield.begin();
}

void loop() {
  // D1 fades in, D2 fades out
  for (int brightness = 0; brightness <= 255; ++brightness) {
    setBrightness(LED1, brightness);
    setBrightness(LED2, 255 - brightness);
    delay(STEP_MS);
  }

  // D1 fades out, D2 fades in
  for (int brightness = 255; brightness >= 0; --brightness) {
    setBrightness(LED1, brightness);
    setBrightness(LED2, 255 - brightness);
    delay(STEP_MS);
  }
}
