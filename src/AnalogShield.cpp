/**
 * @file AnalogShield.cpp
 * @brief AnalogShield implementation: pin set-up, encoder and switch ISRs, analog reads.
 */
#include "AnalogShield.h"

namespace {

// Default analogRead() range, with the ADC referenced to VDD (JP3 at 3.3 V)
constexpr int32_t ADC_MAX = 1023;
constexpr int32_t ADC_REF_MV = 3300;

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

  // LEDs are active low
  pinMode(AnalogShieldPins::LED1, OUTPUT);
  pinMode(AnalogShieldPins::LED2, OUTPUT);

  digitalWrite(AnalogShieldPins::LED1, LOW);
  digitalWrite(AnalogShieldPins::LED2, LOW);

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

int AnalogShield::TempRaw(const TempSource source) const { return analogRead(TempPin(source)); }

int32_t AnalogShield::TempMillivolts(const TempSource source) const {
  return static_cast<int32_t>(TempRaw(source)) * ADC_REF_MV / ADC_MAX;
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
