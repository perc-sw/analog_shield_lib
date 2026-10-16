/**
 * @file AnalogShield.h
 * @brief Library for the U of T Physics Analog Shield (Introduction to Electronics).
 *
 * Hardware reference: https://perc-sw.github.io/analog_shield/
 *
 * Pins use Arduino names (A0, 2, ...), never MCU GPIO numbers, so the library
 * works on any Uno-style board (Phyduino Pico, Uno R3/R4).
 */
#pragma once

#include <Arduino.h>

/**
 * @brief Shield signals by Arduino pin, from the Rev-C schematic.
 */
namespace AnalogShieldPins {

/// @name Analog header (J4)
/// A2 is not connected.
/// @{
constexpr uint8_t TEMP_SIG_MCU = A0;  ///< Conditioned temperature signal, JP2 in "MCU ADC" position
constexpr uint8_t SENSOR_IN = A1;     ///< Raw temperature sensor (JP1: on-board U1 or external J1)
constexpr uint8_t TEMP_SIG_EXT = A3;  ///< Conditioned temperature signal, JP2 in "external ADC" position
constexpr uint8_t IR_VOUT = A4;       ///< Sharp GP2Y0A21YK0F IR distance sensor (J2)
constexpr uint8_t POT = A5;           ///< Potentiometer RV1 wiper (0 V to VDD)
/// @}

/// @name Digital header (J7)
/// @{
constexpr uint8_t ENC_SW = 2;  ///< Rotary encoder push switch, active low (pulled up to VDD)
constexpr uint8_t ENC_A = 3;   ///< Rotary encoder channel A, active low (pulled up to VDD)
constexpr uint8_t ENC_B = 4;   ///< Rotary encoder channel B, active low (pulled up to VDD)
constexpr uint8_t LED2 = 5;    ///< D2 red LED, active low (anode to VDD)
constexpr uint8_t LED1 = 6;    ///< D1 green LED, active low (anode to VDD)
/// @}

}  // namespace AnalogShieldPins

/**
 * @brief Temperature signal to read with AnalogShield::TempRaw() or AnalogShield::TempMillivolts().
 */
enum class TempSource : uint8_t {
  SENSOR,  ///< Raw sensor output on SENSOR_IN (A1)
  MCU,     ///< Conditioned and attenuated for the MCU ADC on TEMP_SIG_MCU (A0), JP2 in "MCU ADC"
  EXT,     ///< Conditioned for an external ADC on TEMP_SIG_EXT (A3), JP2 in "external ADC"
};

/**
 * @brief Encoder, switch, temperature signals and LEDs on the Analog Shield.
 *
 * Create one AnalogShield and call begin() in setup(). The encoder and its
 * switch are tracked with pin interrupts from then on.
 */
class AnalogShield {
 public:
  /**
   * @brief Called on every encoder step.
   * @param count New encoder count, as returned by EncoderCount().
   * @param step +1 when A leads B, -1 when B leads A.
   * @warning Runs inside the interrupt: keep it short, no Serial or delay().
   */
  using EncoderCallback = void (*)(int32_t count, int8_t step);

  /**
   * @brief Called when the encoder switch is pressed or released.
   * @param pressed true on press, false on release.
   * @warning Runs inside the interrupt: keep it short, no Serial or delay().
   */
  using SwitchCallback = void (*)(bool pressed);

  /**
   * @brief Sets pin modes, turns both LEDs off and starts tracking the encoder and its switch.
   *
   * Resets the encoder count to 0. Callbacks already set are kept.
   */
  void begin();

  /**
   * @brief Stops tracking the encoder and its switch.
   */
  void end();

  /**
   * @brief Encoder steps since begin().
   * @return Count, 4 per quadrature cycle, positive when A leads B.
   */
  [[nodiscard]] int32_t EncoderCount() const noexcept;

  /**
   * @brief Sets the function called on every encoder step.
   * @param callback Function to call, or nullptr to remove it.
   */
  void SetEncoderCallback(EncoderCallback callback) noexcept;

  /**
   * @brief Reads the encoder switch.
   * @return true while the switch is held down.
   */
  [[nodiscard]] bool SwitchPressed() const noexcept;

  /**
   * @brief Sets the function called when the encoder switch is pressed or released.
   * @param callback Function to call, or nullptr to remove it.
   */
  void SetSwitchCallback(SwitchCallback callback) noexcept;

  /**
   * @brief Reads a temperature signal with the ADC.
   * @param source Signal to read.
   * @return ADC reading, 0 to 1023 (default 10-bit resolution).
   */
  [[nodiscard]] int TempRaw(TempSource source) const;

  /**
   * @brief Reads the voltage of a temperature signal.
   * @param source Signal to read.
   * @return Voltage in mV, assuming a 3.3 V ADC reference (JP3 at 3.3 V).
   */
  [[nodiscard]] int32_t TempMillivolts(TempSource source) const;

  /**
   * @brief Turns an LED on or off.
   * @param index AnalogShieldPins::LED1 or AnalogShieldPins::LED2; other pins are ignored.
   * @param on true to turn the LED on, false to turn it off.
   */
  void LedON(uint8_t index, bool on = true) noexcept;

  /**
   * @brief Toggles an LED.
   * @param index AnalogShieldPins::LED1 or AnalogShieldPins::LED2; other pins are ignored.
   */
  void LedToggle(uint8_t index) noexcept;

 private:
  /// @brief ISR for a change on ENC_A.
  static void OnEncoderA();
  /// @brief ISR for a change on ENC_B.
  static void OnEncoderB();
  /// @brief Adds @p step to the count and calls the encoder callback.
  static void EncoderStep(int8_t step);
  /// @brief ISR for a change on ENC_SW.
  static void OnSwitch();

  /// The ISRs are plain functions, so they reach the shield through this.
  static AnalogShield* instance;

  volatile int32_t count = 0;                         ///< Encoder steps since begin()
  volatile EncoderCallback callback = nullptr;        ///< Set by SetEncoderCallback()
  volatile SwitchCallback switch_callback = nullptr;  ///< Set by SetSwitchCallback()
  volatile bool switch_pressed = false;               ///< Last state reported to switch_callback
};
