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
 * @brief Encoder, switch, temperature signals, potentiometer and LEDs on the Analog Shield.
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

  /// @name Encoder resolution (PEC11L-4220F-S0015: 15 pulses, 30 detents per turn)
  /// @{
  static constexpr int32_t COUNTS_PER_DETENT = 2;  ///< EncoderCount() steps per click
  static constexpr int32_t DETENTS_PER_REV = 30;   ///< Clicks per full turn
  /// @}

  /**
   * @brief Encoder steps since begin() or the last SetEncoderCount().
   * @return Count, 4 per quadrature cycle, positive when A leads B.
   */
  [[nodiscard]] int32_t EncoderCount() const noexcept;

  /**
   * @brief Encoder clicks (detents) since begin() or the last SetEncoderCount().
   * @return EncoderCount() / COUNTS_PER_DETENT, rounded toward zero, so the
   *         value changes once per click.
   */
  [[nodiscard]] int32_t EncoderDetents() const noexcept;

  /**
   * @brief Sets the encoder count, for example to zero it at a home position.
   * @param count New count, in EncoderCount() steps. Leave out to reset to 0.
   */
  void SetEncoderCount(int32_t count = 0) noexcept;

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
   * @brief Sets the ADC resolution used by analogRead() and by every reading below.
   *
   * Use this rather than calling analogReadResolution() directly, so the
   * millivolt and temperature conversions use the same full scale.
   *
   * @param bits 8 to 12. The RP2040/RP2350 ADC is 12-bit; the Arduino default is 10.
   * @return false, and no change, if @p bits is out of range or the board
   *         cannot change resolution (Uno R3 stays at 10 bits).
   */
  bool SetAdcResolution(uint8_t bits);

  /**
   * @brief ADC resolution set by SetAdcResolution().
   * @return Bits, 10 until SetAdcResolution() is called.
   */
  [[nodiscard]] uint8_t AdcResolution() const noexcept;

  /// @name Readings
  /// Every reading takes an optional @p samples: the number of ADC reads to
  /// average (1 to 255, 0 counts as 1). More samples give a steadier value.
  /// @{

  /**
   * @brief Reads a temperature signal with the ADC.
   * @param source Signal to read.
   * @param samples ADC reads to average.
   * @return ADC reading, 0 to 2^AdcResolution() - 1 (1023 at the default 10 bits).
   */
  [[nodiscard]] int TempRaw(TempSource source, uint8_t samples = 1) const;

  /**
   * @brief Reads the voltage of a temperature signal.
   * @param source Signal to read.
   * @param samples ADC reads to average.
   * @return Voltage in mV, assuming a 3.3 V ADC reference (JP3 at 3.3 V).
   */
  [[nodiscard]] int32_t TempMillivolts(TempSource source, uint8_t samples = 1) const;

  /**
   * @brief Reads a temperature signal and converts it to degrees Celsius.
   *
   * Undoes the shield's signal path for @p source (MCU attenuator, then the
   * conditioning stage Vout = 4.3 Vin - 0.638 V), then applies the LMT84
   * parabolic transfer function (datasheet Equation 2). Accurate over the
   * sensor's -50 to 150 °C range; outside it the result is extrapolated.
   *
   * @param source Signal to read.
   * @param samples ADC reads to average.
   * @return Temperature in °C, assuming a 3.3 V ADC reference (JP3 at 3.3 V)
   *         and the on-board LMT84 (JP1 at U1).
   * @note TempSource::EXT swings up to 4.96 V for an external ADC. Read on the
   *       3.3 V MCU ADC it saturates below about 21 °C.
   */
  [[nodiscard]] float TempCelsius(TempSource source, uint8_t samples = 1) const;

  /**
   * @brief Reads a temperature signal and converts it to degrees Fahrenheit.
   * @param source Signal to read.
   * @param samples ADC reads to average.
   * @return Temperature in °F; same assumptions and limits as TempCelsius().
   */
  [[nodiscard]] float TempFahrenheit(TempSource source, uint8_t samples = 1) const;

  /**
   * @brief Reads a temperature signal and converts it to kelvins.
   * @param source Signal to read.
   * @param samples ADC reads to average.
   * @return Temperature in K; same assumptions and limits as TempCelsius().
   */
  [[nodiscard]] float TempKelvin(TempSource source, uint8_t samples = 1) const;

  /**
   * @brief Reads the potentiometer RV1 with the ADC.
   * @param samples ADC reads to average.
   * @return ADC reading, 0 to 2^AdcResolution() - 1.
   */
  [[nodiscard]] int PotRaw(uint8_t samples = 1) const;

  /**
   * @brief Reads the potentiometer wiper voltage.
   * @param samples ADC reads to average.
   * @return Voltage in mV, assuming a 3.3 V ADC reference (JP3 at 3.3 V).
   */
  [[nodiscard]] int32_t PotMillivolts(uint8_t samples = 1) const;

  /**
   * @brief Reads the potentiometer position.
   * @param samples ADC reads to average.
   * @return 0.0 at one end of travel to 100.0 at the other (0 V to VDD).
   */
  [[nodiscard]] float PotPercent(uint8_t samples = 1) const;

  /// @}

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
  /// @brief Average of @p samples analogRead() calls on @p pin, in ADC counts.
  [[nodiscard]] static float ReadAverage(uint8_t pin, uint8_t samples);
  /// @brief Converts an ADC reading in counts to mV at the current resolution.
  [[nodiscard]] float CountsToMillivolts(float counts) const noexcept;
  /// @brief Converts the voltage of a temperature signal from @p source to °C (see TempCelsius()).
  [[nodiscard]] static float MillivoltsToCelsius(float mv, TempSource source) noexcept;

  /// The ISRs are plain functions, so they reach the shield through this.
  static AnalogShield* instance;

  uint8_t adc_bits = 10;  ///< Set by SetAdcResolution()

  volatile int32_t count = 0;                         ///< Encoder steps since begin() or SetEncoderCount()
  volatile EncoderCallback callback = nullptr;        ///< Set by SetEncoderCallback()
  volatile SwitchCallback switch_callback = nullptr;  ///< Set by SetSwitchCallback()
  volatile bool switch_pressed = false;               ///< Last state reported to switch_callback
};
