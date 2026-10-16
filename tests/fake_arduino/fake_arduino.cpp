#include "Arduino.h"

namespace fake {

Pin pins[NUM_PINS];
int analog_resolution = 10;
int analog_reads = 0;
bool interrupts_on = true;

void reset() {
  for (Pin& p : pins) p = Pin{};
  analog_resolution = 10;
  analog_reads = 0;
  interrupts_on = true;
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

void pinMode(pin_size_t pin, uint8_t mode) { fake::pins[pin].mode = mode; }

void digitalWrite(pin_size_t pin, uint8_t value) {
  fake::Pin& p = fake::pins[pin];
  p.out_level = value ? HIGH : LOW;
  p.mode_at_write = p.mode;
}

int digitalRead(pin_size_t pin) {
  const fake::Pin& p = fake::pins[pin];
  return p.mode == OUTPUT ? p.out_level : p.input_level;
}

int analogRead(pin_size_t pin) {
  fake::Pin& p = fake::pins[pin];
  ++fake::analog_reads;
  if (p.analog_sequence.empty()) return p.analog_value;
  const int value = p.analog_sequence[p.analog_next];
  p.analog_next = (p.analog_next + 1) % p.analog_sequence.size();
  return value;
}

void analogReadResolution(int bits) { fake::analog_resolution = bits; }

void noInterrupts() { fake::interrupts_on = false; }
void interrupts() { fake::interrupts_on = true; }

int digitalPinToInterrupt(pin_size_t pin) { return pin; }

void attachInterrupt(int interrupt, void (*isr)(), uint8_t mode) {
  fake::pins[interrupt].isr = isr;
  fake::pins[interrupt].isr_mode = mode;
}

void detachInterrupt(int interrupt) {
  fake::pins[interrupt].isr = nullptr;
  fake::pins[interrupt].isr_mode = fake::UNSET;
}
