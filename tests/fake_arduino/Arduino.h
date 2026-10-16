/**
 * @file Arduino.h
 * @brief Fake Arduino API for PC-side tests: just enough for src/ to compile.
 *
 * Pin numbering follows the Uno and Phyduino Pico (digital 0-13, A0-A5 = 14-19).
 * Calls are recorded in fake::pins; tests set fake::pins[p].input_level and
 * fake::pins[p].analog_value to drive inputs. Add functions here as the library
 * starts to use them.
 */
#pragma once

#include <cstdint>

using pin_size_t = uint8_t;

constexpr uint8_t LOW  = 0;
constexpr uint8_t HIGH = 1;

constexpr uint8_t INPUT        = 0;
constexpr uint8_t OUTPUT       = 1;
constexpr uint8_t INPUT_PULLUP = 2;

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

namespace fake {

constexpr uint8_t UNSET = 0xFF;

struct Pin {
  uint8_t mode = UNSET;           // last pinMode(), UNSET if never called
  uint8_t out_level = LOW;        // last digitalWrite()
  uint8_t level_at_output = UNSET;// out_level when pinMode(OUTPUT) was called
  uint8_t input_level = LOW;      // returned by digitalRead() unless OUTPUT
  int analog_value = 0;           // returned by analogRead()
};

extern Pin pins[NUM_PINS];

// Back to power-on state; call at the start of every test
void reset();

}  // namespace fake
