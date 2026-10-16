#include "Arduino.h"

namespace fake {

Pin pins[NUM_PINS];

void reset() {
  for (Pin& p : pins) p = Pin{};
}

}  // namespace fake

void pinMode(pin_size_t pin, uint8_t mode) {
  fake::Pin& p = fake::pins[pin];
  p.mode = mode;
  if (mode == OUTPUT) p.level_at_output = p.out_level;
}

void digitalWrite(pin_size_t pin, uint8_t value) {
  fake::pins[pin].out_level = value ? HIGH : LOW;
}

int digitalRead(pin_size_t pin) {
  const fake::Pin& p = fake::pins[pin];
  return p.mode == OUTPUT ? p.out_level : p.input_level;
}

int analogRead(pin_size_t pin) {
  return fake::pins[pin].analog_value;
}
