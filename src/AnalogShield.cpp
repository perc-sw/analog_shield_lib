/**
 * @file AnalogShield.cpp
 * @brief AnalogShield implementation: pin set-up, encoder and switch ISRs, analog reads.
 */
#include "AnalogShield.h"

#include <math.h>

namespace {

// ADC referenced to VDD (JP3 at 3.3 V). SetAdcResolution() range: the
// RP2040/RP2350 ADC is 12-bit, and every supported core accepts 8 to 12.
constexpr float ADC_REF_MV = 3300.0F;
constexpr uint8_t ADC_MIN_BITS = 8;
constexpr uint8_t ADC_MAX_BITS = 12;

// Signal path, from the Rev-C schematic. TEMP_SIG_EXT = 4.3 * sensor - 638 mV;
// TEMP_SIG_MCU = TEMP_SIG_EXT through the R15 (6.98K) / 10K divider.
constexpr float COND_GAIN = 4.3F;
constexpr float COND_OFFSET_MV = 638.0F;
constexpr float MCU_ATTN = 6.98F / (6.98F + 10.0F);

// LMT84 parabolic fit (datasheet Equation 1):
// V = 870.6 mV - 5.506 mV/°C * (T - 30) - 0.00176 mV/°C² * (T - 30)²
constexpr float LMT84_V30_MV = 870.6F;
constexpr float LMT84_SLOPE = 5.506F;
constexpr float LMT84_CURVE = 0.00176F;

uint8_t TempPin(const TempSource source) {
  switch (source) {
    case TempSource::MCU:
      return AnalogShieldPins::TEMP_SIG_MCU;
    case TempSource::EXT:
      return AnalogShieldPins::TEMP_SIG_EXT;
    case TempSource::SENSOR:
    default:
      return AnalogShieldPins::SENSOR_IN;
  }
}

}  // namespace

AnalogShield* AnalogShield::instance = nullptr;

void AnalogShield::begin() {
  // Encoder lines have external pull-ups and RC debounce on the shield
  pinMode(AnalogShieldPins::ENC_SW, INPUT);
  pinMode(AnalogShieldPins::ENC_A, INPUT);
  pinMode(AnalogShieldPins::ENC_B, INPUT);

  // LEDs are active low. Set the pin mode first: on the RP2040/RP2350,
  // pinMode() resets the output level, so a write before it would be lost.
  pinMode(AnalogShieldPins::LED1, OUTPUT);
  pinMode(AnalogShieldPins::LED2, OUTPUT);

  digitalWrite(AnalogShieldPins::LED1, HIGH);
  digitalWrite(AnalogShieldPins::LED2, HIGH);

  // Quadrature decoding on every edge of A and B
  count = 0;
  instance = this;

  attachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_A), OnEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_B), OnEncoderB, CHANGE);

  // Press and release of the encoder switch
  switch_pressed = SwitchPressed();
  attachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_SW), OnSwitch, CHANGE);
}

void AnalogShield::end() {
  detachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_A));
  detachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_B));
  detachInterrupt(digitalPinToInterrupt(AnalogShieldPins::ENC_SW));

  if (instance == this) {
    instance = nullptr;
  }
}

// Aligned 32-bit reads and writes are atomic on the RP2040/RP2350
int32_t AnalogShield::EncoderCount() const noexcept { return count; }

int32_t AnalogShield::EncoderDetents() const noexcept { return EncoderCount() / COUNTS_PER_DETENT; }

// Interrupts off so an encoder step cannot land between the ISR's read and write of count
void AnalogShield::SetEncoderCount(const int32_t value) noexcept {
  noInterrupts();
  count = value;
  interrupts();
}

void AnalogShield::SetEncoderCallback(const EncoderCallback cb) noexcept {
  callback = cb;
}

void AnalogShield::EncoderStep(const int8_t step) {
  const int32_t value = instance->count + step;
  instance->count = value;

  const EncoderCallback callback = instance->callback;

  if (callback != nullptr) {
    callback(value, step);
  }
}

// After an A edge, A != B means A moved first (A leads B)
void AnalogShield::OnEncoderA() {
  if (instance == nullptr) {
    return;
  }

  auto a = digitalRead(AnalogShieldPins::ENC_A);
  auto b = digitalRead(AnalogShieldPins::ENC_B);

  EncoderStep(a != b ? 1 : -1);
}

// After a B edge, A == B means A moved first (A leads B)
void AnalogShield::OnEncoderB() {
  if (instance == nullptr) {
    return;
  }

  auto a = digitalRead(AnalogShieldPins::ENC_A);
  auto b = digitalRead(AnalogShieldPins::ENC_B);

  EncoderStep(a == b ? 1 : -1);
}

// Switch is active low
bool AnalogShield::SwitchPressed() const noexcept { return digitalRead(AnalogShieldPins::ENC_SW) == LOW; }

void AnalogShield::SetSwitchCallback(const SwitchCallback cb) noexcept {
  switch_callback = cb;
}

void AnalogShield::OnSwitch() {
  if (instance == nullptr) {
    return;
  }

  // Only report real changes, in case an edge is seen twice
  const bool pressed = instance->SwitchPressed();

  if (pressed == instance->switch_pressed) {
    return;
  }

  instance->switch_pressed = pressed;

  const SwitchCallback callback = instance->switch_callback;

  if (callback != nullptr) {
    callback(pressed);
  }
}

bool AnalogShield::SetAdcResolution(const uint8_t bits) {
  if (bits < ADC_MIN_BITS || bits > ADC_MAX_BITS) {
    return false;
  }

#if defined(ARDUINO_ARCH_AVR)
  // The Uno R3 core has no analogReadResolution(): 10 bits only
  return bits == adc_bits;
#else
  analogReadResolution(bits);
  adc_bits = bits;
  return true;
#endif
}

uint8_t AnalogShield::AdcResolution() const noexcept { return adc_bits; }

float AnalogShield::ReadAverage(const uint8_t pin, const uint8_t samples) {
  const uint8_t n = samples == 0 ? 1 : samples;
  uint32_t sum = 0;

  for (uint8_t i = 0; i < n; ++i) {
    sum += static_cast<uint32_t>(analogRead(pin));
  }

  return static_cast<float>(sum) / n;
}

float AnalogShield::CountsToMillivolts(const float counts) const noexcept {
  const auto full_scale = static_cast<float>((1UL << adc_bits) - 1);
  return counts * ADC_REF_MV / full_scale;
}

// Readings are never negative, so + 0.5 rounds to nearest
int AnalogShield::TempRaw(const TempSource source, const uint8_t samples) const {
  return static_cast<int>(ReadAverage(TempPin(source), samples) + 0.5F);
}

int32_t AnalogShield::TempMillivolts(const TempSource source, const uint8_t samples) const {
  return static_cast<int32_t>(CountsToMillivolts(ReadAverage(TempPin(source), samples)) + 0.5F);
}

float AnalogShield::MillivoltsToCelsius(float mv, const TempSource source) noexcept {
  // Back to the sensor output voltage
  switch (source) {
    case TempSource::MCU:
      mv = (mv / MCU_ATTN + COND_OFFSET_MV) / COND_GAIN;
      break;
    case TempSource::EXT:
      mv = (mv + COND_OFFSET_MV) / COND_GAIN;
      break;
    case TempSource::SENSOR:
    default:
      break;
  }

  // Equation 1 solved for T (datasheet Equation 2)
  const float root = sqrtf(LMT84_SLOPE * LMT84_SLOPE + 4.0F * LMT84_CURVE * (LMT84_V30_MV - mv));
  return (LMT84_SLOPE - root) / (2.0F * -LMT84_CURVE) + 30.0F;
}

// Unrounded mV rather than TempMillivolts(), which rounds to whole mV
float AnalogShield::TempCelsius(const TempSource source, const uint8_t samples) const {
  return MillivoltsToCelsius(CountsToMillivolts(ReadAverage(TempPin(source), samples)), source);
}

float AnalogShield::TempFahrenheit(const TempSource source, const uint8_t samples) const {
  return TempCelsius(source, samples) * 9.0F / 5.0F + 32.0F;
}

float AnalogShield::TempKelvin(const TempSource source, const uint8_t samples) const {
  return TempCelsius(source, samples) + 273.15F;
}

int AnalogShield::PotRaw(const uint8_t samples) const {
  return static_cast<int>(ReadAverage(AnalogShieldPins::POT, samples) + 0.5F);
}

int32_t AnalogShield::PotMillivolts(const uint8_t samples) const {
  return static_cast<int32_t>(CountsToMillivolts(ReadAverage(AnalogShieldPins::POT, samples)) + 0.5F);
}

// The wiper spans the full ADC range, so this does not depend on the reference
float AnalogShield::PotPercent(const uint8_t samples) const {
  const auto full_scale = static_cast<float>((1UL << adc_bits) - 1);
  return ReadAverage(AnalogShieldPins::POT, samples) * 100.0F / full_scale;
}

void AnalogShield::LedON(const uint8_t index, const bool on) noexcept{
  if (index == AnalogShieldPins::LED1 || index == AnalogShieldPins::LED2) {
    digitalWrite(index, on ? (LOW) : (HIGH));
  }
}

void AnalogShield::LedToggle(const uint8_t index) noexcept{
  if (index == AnalogShieldPins::LED1 || index == AnalogShieldPins::LED2) {
    auto value = digitalRead(index);
    digitalWrite(index, value == LOW ? HIGH : LOW);
  }
}
