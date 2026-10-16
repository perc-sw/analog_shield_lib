#include "Arduino.h"

namespace fake {

Pin pins[NUM_PINS];

void reset() {
  for (Pin& p : pins) p = Pin{};
}

void setInput(pin_size_t pin, uint8_t level) {
  Pin& p = pins[pin];
  const uint8_t old = p.input_level;
  p.input_level = level ? HIGH : LOW;
  if (p.isr == nullptr || p.input_level == old) return;
  const bool rising = p.input_level == HIGH;
  if (p.isr_mode == CHANGE || (p.isr_mode == RISING && rising) || (p.isr_mode == FALLING && !rising)) p.isr();
}

}  // namespace fake

void pinMode(pin_size_t pin, uint8_t mode) {
  fake::Pin& p = fake::pins[pin];
  p.mode = mode;
  if (mode == OUTPUT) p.level_at_output = p.out_level;
}

void digitalWrite(pin_size_t pin, uint8_t value) { fake::pins[pin].out_level = value ? HIGH : LOW; }

int digitalRead(pin_size_t pin) {
  const fake::Pin& p = fake::pins[pin];
  return p.mode == OUTPUT ? p.out_level : p.input_level;
}

int analogRead(pin_size_t pin) { return fake::pins[pin].analog_value; }

int digitalPinToInterrupt(pin_size_t pin) { return pin; }

void attachInterrupt(int interrupt, void (*isr)(), uint8_t mode) {
  fake::pins[interrupt].isr = isr;
  fake::pins[interrupt].isr_mode = mode;
}

void detachInterrupt(int interrupt) {
  fake::pins[interrupt].isr = nullptr;
  fake::pins[interrupt].isr_mode = fake::UNSET;
}
