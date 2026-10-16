/**
 * @file Arduino.h
 * @brief Fake Arduino API for PC-side tests: just enough for src/ to compile.
 *
 * Pin numbering follows the Uno and Phyduino Pico (digital 0-13, A0-A5 = 14-19).
 * Calls are recorded in fake::pins; tests set fake::pins[p].input_level and
 * fake::pins[p].analog_value to drive inputs, or fake::setInput() to also fire
 * the pin's interrupt. Add functions here as the library
 * starts to use them.
 */
#pragma once

#include <cstdint>

using pin_size_t = uint8_t;

constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;

constexpr uint8_t INPUT = 0;
constexpr uint8_t OUTPUT = 1;
constexpr uint8_t INPUT_PULLUP = 2;

constexpr uint8_t CHANGE = 1;
constexpr uint8_t FALLING = 2;
constexpr uint8_t RISING = 3;

constexpr pin_size_t A0 = 14;
constexpr pin_size_t A1 = 15;
constexpr pin_size_t A2 = 16;
constexpr pin_size_t A3 = 17;
constexpr pin_size_t A4 = 18;
constexpr pin_size_t A5 = 19;

constexpr int NUM_PINS = 20;

void pinMode(pin_size_t pin, uint8_t mode);
void digitalWrite(pin_size_t pin, uint8_t value);
int digitalRead(pin_size_t pin);
int analogRead(pin_size_t pin);

// Interrupt numbers equal pin numbers, as on the RP2040/RP2350
int digitalPinToInterrupt(pin_size_t pin);
void attachInterrupt(int interrupt, void (*isr)(), uint8_t mode);
void detachInterrupt(int interrupt);

namespace fake {

constexpr uint8_t UNSET = 0xFF;

struct Pin {
  uint8_t mode = UNSET;             // last pinMode(), UNSET if never called
  uint8_t out_level = LOW;          // last digitalWrite()
  uint8_t mode_at_write = UNSET;    // mode when digitalWrite() was last called
  uint8_t input_level = LOW;        // returned by digitalRead() unless OUTPUT
  int analog_value = 0;             // returned by analogRead()
  void (*isr)() = nullptr;          // last attachInterrupt(), nullptr once detached
  uint8_t isr_mode = UNSET;         // mode passed to attachInterrupt()
};

extern Pin pins[NUM_PINS];

// Back to power-on state; call at the start of every test
void reset();

// Sets a pin's input level and runs its ISR if the edge matches its mode
void setInput(pin_size_t pin, uint8_t level);

}  // namespace fake
