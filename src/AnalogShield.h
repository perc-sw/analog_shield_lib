/*
 * AnalogShield.h
 * Library for the U of T Physics Analog Shield (Introduction to Electronics).
 * Hardware reference: https://github.com/perc-sw/analog_shield
 *
 * Pins use Arduino names (A0, 2, ...), never MCU GPIO numbers, so the library
 * works on any Uno-style board (Phyduino Pico, Uno R3/R4).
 */
#pragma once

#include <Arduino.h>

namespace AnalogShieldPins {

// Analog header (J4)
constexpr uint8_t TEMP_SIG_MCU = A0;  // Conditioned temperature signal, JP2 in "MCU ADC" position
constexpr uint8_t SENSOR_IN    = A1;  // Raw temperature sensor (JP1: on-board U1 or external J1)
// A2 not connected
constexpr uint8_t TEMP_SIG_EXT = A3;  // Conditioned temperature signal, JP2 in "external ADC" position
constexpr uint8_t IR_VOUT      = A4;  // Sharp GP2Y0A21YK0F IR distance sensor (J2)
constexpr uint8_t POT          = A5;  // Potentiometer RV1 wiper (0 V to VDD)

// Digital header (J7)
constexpr uint8_t ENC_SW = 2;  // Rotary encoder push switch, active low (pulled up to VDD)
constexpr uint8_t ENC_A  = 3;  // Rotary encoder channel A, active low (pulled up to VDD)
constexpr uint8_t ENC_B  = 4;  // Rotary encoder channel B, active low (pulled up to VDD)
constexpr uint8_t LED2   = 5;  // D2 red LED, active low (anode to VDD)
constexpr uint8_t LED1   = 6;  // D1 green LED, active low (anode to VDD)

}  // namespace AnalogShieldPins

class AnalogShield {
public:
  // Sets pin modes and turns both LEDs off.
  void begin();
};
